/*INDENT OFF*/

/* Description: C source code for generating grap files */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    %M% copyright 2016-2017 %Q%.   \ make_grap.c $
* This software is provided 'as-is', without any express or implied warranty.
* In no event will the authors be held liable for any damages arising from the
* use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it freely,
* subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not claim
*    that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
****************************** (end of license) ******************************/
/* $Id: ~|^` @(#)   This is %M% version %I% dated %O%. \ $ */
/* You may send bug reports to %Y% with subject "make_grap" */
/*****************************************************************************/
/* maintenance note: master file %P% */

/********************** Long description and rationale: ***********************
* read input from file or stdin, write grap to stdout
******************************************************************************/

/* configuration options */
#define ASSERT_CODE                 0
#define DEBUG_CODE                  1
#define SILENCE_WHINEY_COMPILERS    0

/* nothing (much) to configure below this line */

#if ! DEBUG_CODE
# if ! ASSERT_CODE
#  define NDEBUG 1
# endif
#endif

/* Minimum _XOPEN_SOURCE version for C99 (else compilers on illumos have a tantrum) */
#if defined(__STDC__) && ( __STDC_VERSION__ >= 199901L)
# define MIN_XOPEN_SOURCE_VERSION 600
#else
# define MIN_XOPEN_SOURCE_VERSION 500
#endif

/* feature test macros defined before any header files are included */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#if defined(_XOPEN_SOURCE) && ( _XOPEN_SOURCE < MIN_XOPEN_SOURCE_VERSION )
# undef _XOPEN_SOURCE
# define _XOPEN_SOURCE MIN_XOPEN_SOURCE_VERSION
#endif
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif

/* for hstrerror in netdb.h (included by get_ips.h) */
/* prevent errors from non-standard types in NetBSD stdlib.h:
/usr/include/stdlib.h:268:28: error: unknown type name 'u_char'; did you mean 'char'?
void     arc4random_addrandom(u_char *, int);
*/
/*
#ifndef _BSD_SOURCE
# define _BSD_SOURCE 1
#endif
#ifndef _SVID_SOURCE
# define _SVID_SOURCE 1
#endif
*/
#ifndef _NETBSD_SOURCE
# define _NETBSD_SOURCE 1
#endif

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components are version control fields */
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: make_grap.c ~|^` @(#)"
#define SOURCE_MODULE "%M%"
#define MODULE_VERSION "%I%"
#define MODULE_DATE "%O%"
#define COPYRIGHT_HOLDER "%Q%"
#define COPYRIGHT_DATE "2016-2017"

/* local header files needed */
#include "get_host_name.h"      /* get_host_name */
#include "get_ips.h"            /* get_ips */
#include "graphing.h"           /* formatprecision formatwidth mintick maxtick prec rem intvl */
#include "logger.h"             /* logger */
#include "paths_decl.h"         /* path_basename */
#include "snn.h"                /* sng snl snul */
#include "trim.h"               /* trim */
#include "zz_build_str.h"       /* build_id build_strings_registered copyright_id register_build_strings */

#include <sys/param.h>          /* MAXHOSTNAMELEN */
#include <sys/resource.h>       /* RUSAGE_* struct rusage */
#include <sys/time.h>           /* struct timeval */
#include <sys/times.h>          /* times */
#include <sys/types.h>          /* *_t */
#include <assert.h>             /* assert */
#include <ctype.h>              /* isdigit */
#include <errno.h>
#include <limits.h>             /* *_MAX */
#include <math.h>               /* ceil exp2 floor log2 round */
#include <stdio.h>
#include <stdlib.h>             /* strtoul */
#include <string.h>             /* strncpy strrchr */
#include <time.h>
#include <unistd.h>             /* getpid */

#undef optind

#undef xbuildstr
#define xbuildstr(s) buildstr(s)
#undef buildstr
#define buildstr(s) #s

/* XXX cleanup */
#define OPTSTRING "dh:H:l:L:mp:T:V:w:x:Xy:Y"
#define USAGE_STRING     "[-d] [-h ht] [-H val] [-l legend-text] [-L lim] [-m] [-p offset] [-T \"title\"] [-V val] [-w wid] [-x \"x-label\"] [-X] [-y \"y-label\"] [-Y]\n\
-h ht\tspecify graph height\n\
-H val\tdraw a dashed horizontal line at val\n\
-l legend-text\tlegend for data (may be repeated for multiplots)\n\
-L lim\tset threshold for data reduction\n\
-m\tmark maximum data point\n\
-p offset\tset page offset\n\
-T \"title\"\tset graph title\n\
-V val\tdraw a dashed vertical line at val\n\
-w wid\tspecify graph width\n\
-x \"xlabel\"\tlabel x-axis\n\
-X\t use log scaling for the x-axis\n\
-y \"y-label\"\tlabel y-axis\n\
-Y\t use log scaling for the y-axis\n\
"

/* space-saving */
#undef V
#define V void
#undef A
#define A(me) assert(me)

volatile int debug = 0;

static char make_grap_initialized = (char)0;
static const char *filenamebuf = __FILE__ ;
static const char *source_file = NULL;
static const char *build_options = "@(#)make_grap.c: " "built with configuration options: "
                  "ASSERT_CODE=" xbuildstr(ASSERT_CODE)
                  ", DEBUG_CODE=" xbuildstr(DEBUG_CODE)
                  ;

/* initialize at run-time */
static void initialize_make_grap(void)
{
    const char *s;

    s = strrchr(filenamebuf, '/');
    if (NULL == s)
        s = filenamebuf;
    else
        s++;
    make_grap_initialized = register_build_strings(build_options, &source_file, s);
}

struct symbol_struct {
    const char *marker;
    const char *mchar;
    const char *legend_text;
};

int main(int argc, const char * const *argv)
{
    char buf[4096], buf2[4096];     /* RATS: ignore (big enough) */
    char prog[PATH_MAX];
    char host[MAXHOSTNAMELEN];  /* RATS: ignore (big enough) */
    char iplist[1024];          /* RATS: ignore (size is checked) */
    char logbuf[65536];         /* RATS: ignore (it's big enough) */
    char loghost[MAXHOSTNAMELEN] = "localhost";  /* RATS: ignore (big enough) */
    char procid[32];            /* RATS: ignore (used with sizeof()) */
    char graph_height[16], graph_width[16];
    const char *page_offset="0.4i";
    const char *dq, *xlabel=NULL, *ylabel=NULL, *path, *pcc, *sp, *title=NULL;
    const char *commentstart = ".\\\"";
    const char *commentend = "\n";
    const char *grapcommentstart = "#";
    const char *grapcommentend = "\n";
    char *endptr=NULL;
    struct symbol_struct symbols[] = {
        { "bullet", "\\(bu",                 NULL } ,
        { "times",  "\\(mu",                 NULL } ,
        { "plus",   "\\(pl",                 NULL } ,
        { "delta",  "\\(*D",                 NULL } ,
        { "square", "\\(sq",                 NULL } ,
        { "box",    "\\f(ZD\\N~110~\fP",     NULL } , /* can obscure others */
        { "star",   "\\(**",                 NULL } , /* can be confused for overlayed x and + */
        { "htick",  "\\(em",                 NULL } , /* poor for nearly horizontal plots */
        { "vtick",  "|",                     NULL } , /* poor for nearly vertical plots */
        { "dot",    "\\(v~-0.2m~.\\v~0.2m~", NULL } , /* almost invisible */
    };
    size_t nmarks = sizeof(symbols)/sizeof(symbols[0]);
    size_t dim, legend_number = 0UL, len, maxdim, q, wordlen, xlines;
    int bufsz, c, i, ingrap=0, optind;
    int maxlen = 1024;
    int sockfd = -1;
    int htp, maxxticks, maxyticks, widp, xp, xw, yp, yw;
    unsigned char flags[256];
    unsigned char **pp=NULL;
    unsigned int seqid = 1U;
    unsigned int tzknown = 1U;
    unsigned int errs=0U;
    volatile unsigned long count, ul;
    pid_t pid;
    void (*f)(int, void *, const char *, ...) = logger;
    void *log_arg;
    struct logger_struct ls;
    FILE *fp;
    double d, dx,dy, hline, lim = 0.70, lx,ly, ox=-1.0e300, oy=-1.0e300, vline;
    double maxx, maxlx, maxly, maxy, maxyx=0.0, minx, minlx, minly, miny;
    double *x=NULL, *plx=NULL, **py=NULL, **ply=NULL;
    double minytick, maxytick, ytickintvl, minxtick, maxxtick, xtickintvl;
    double deltax, deltay, xres, yres;
#if ! defined(PP__FUNCTION__)
    static const char __func__[] = "main";
#endif

    /* I/O initialization */
    (void) setvbuf(stdout, NULL, (int)_IONBF, BUFSIZ);
    (void) setvbuf(stderr, NULL, (int)_IONBF, BUFSIZ);
    initialize_make_grap();
    /* variable initialization */
    host[0] = '\0';
    iplist[0] = '\0';
    procid[0] = '\0';
    (void)path_basename(argv[0], prog, sizeof(prog));
    pid = getpid();
    /* initial logging configuration */
    memset(&ls, 0, sizeof(struct logger_struct));
    ls.options = LOG_PID /* | LOG_PERROR | LOG_CONS */ ;
    ls.logfac = LOG_DAEMON ;
    ls.logmask = LOG_UPTO(LOG_INFO) ;
    ls.location = LOGGER_LOCATION_STREAM | LOGGER_LOCATION_SYSLOG_HOST ;
#if 0
    ls.format = LOGGER_FORMAT_RFC5424_UTC ;
#else
    ls.format = LOGGER_FORMAT_PLAIN ;
#endif
    ls.ext_msg_start = LOGGER_MESSAGE_ALL ;
    ls.psysUpTime = NULL;
    ls.psyncAccuracy = NULL;
    ls.ptzKnown = &tzknown;
    ls.pisSynced = NULL;
    ls.stream = stderr;
    ls.sysloghost = loghost;
    ls.fqdn = host;             /* filled in below (uses logger for error reporting) */
    ls.ident = prog;
    ls.msgid = "make_grap";
    ls.procid = procid;         /* filled in below (uses logger for error reporting) */
    ls.enterpriseId = NULL;
    ls.software = prog;
    ls.swVersion = "%I%";       /* maintained by version control system */
    ls.language = NULL;
    ls.pip = iplist;            /* filled in below (uses logger for error reporting) */
    ls.func = NULL;
    ls.pmax_msg_len = &maxlen;
    bufsz = sizeof(logbuf);
    ls.pbufsz = &bufsz;
    ls.buf = logbuf;
    ls.psockfd = &sockfd;
    ls.psequenceId = &seqid;
    log_arg = &ls;
    (void)get_host_name(host, sizeof(host), logger, log_arg);
    /* host address(es) */
    (void)get_ips(iplist, sizeof(iplist), host, AF_UNSPEC, AI_ADDRCONFIG, logger, log_arg);
    (void)snul(procid, sizeof(procid), NULL, NULL, (unsigned long)pid, 10, '0', 1, logger, log_arg); /* logging of snn OK from logger caller after initialization */
    /* process command-line arguments */
    /* initialize flags */
    for (q=0UL; q<sizeof(flags); q++) flags[q] = (unsigned char)0U;
    /* defaults */
    strncpy(graph_height,"7.0866",sizeof(graph_height));
    htp=510;
    strncpy(graph_width,"7.0866",sizeof(graph_width));
    widp=510;
    /* emulate getopt (too many implementation differences to rely on it) */
    for (optind = 1; (optind < argc) && (argv[optind][0] == '-'); optind++) {
        if (0 == strcmp(argv[optind], "--")) {
            optind++;
            break;
        }
        for (pcc = argv[optind] + 1; (int)'\0' != (c = (int)(*pcc)); pcc++) {
            if ((':' == (char)c) || (NULL == strchr(OPTSTRING, (char)c)))
                c = (int)'?';
            switch (c) {
                /* this program has no options (that's what a compiler diagnostic for the loop is about) */
#if SILENCE_WHINEY_COMPILERS
                case ':': /* clang! STFU!! */ /* cannot happen here, but clang is unable to figure that out */
                break;
#endif
                case 'd' : /*FALLTHROUGH*/
                case 'D' : /*FALLTHROUGH*/
                case 'm' : /*FALLTHROUGH*/
                case 'X' : /*FALLTHROUGH*/
                case 'Y' : /*FALLTHROUGH*/
                    flags[c] = 1U;
                break;
                case 'h' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    d = strtod(pcc, &endptr);
                    /* magic undocumented unit conversion */
                    switch (*endptr) {
                        case 'c' :
                            d /= 2.54;
                        break;
                        case 'p' :
                            d /= 72.0;
                        break;
                        case 'P' :
                            d /= 6.0;
                        break;
                    }
                    (void)snf(graph_height,sizeof(graph_height),NULL,NULL,d,' ',1,-6,logger,log_arg);
                    htp=(int)floor(d*72.0);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'H' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    hline = strtod(pcc, NULL);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'l' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    symbols[legend_number%nmarks].legend_text = pcc;
                    legend_number++;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'L' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    lim = strtod(pcc, NULL);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'p' : 
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    page_offset = pcc;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
#if 0
                case 'r' :
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    reduction = strtoul(pcc, NULL, 10);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
#endif
                case 'T' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    title = pcc;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'V' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    vline = strtod(pcc, NULL);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'w' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    d = strtod(pcc, &endptr);
                    /* magic undocumented unit conversion */
                    switch (*endptr) {
                        case 'c' :
                            d /= 2.54;
                        break;
                        case 'p' :
                            d /= 72.0;
                        break;
                        case 'P' :
                            d /= 6.0;
                        break;
                    }
                    (void)snf(graph_width,sizeof(graph_width),NULL,NULL,d,' ',1,-6,logger,log_arg);
                    widp=(int)floor(d*72.0);
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'x' : 
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    xlabel = pcc;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'y' : 
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    ylabel = pcc;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                default:
                    logger(LOG_ERR, log_arg,
                        "%s: %s: %s line %d: unrecognized option %s",
                        prog, __func__, __FILE__, __LINE__, argv[optind]);
                    logger(LOG_ERR, log_arg,
                        "%s: usage: %s %s",
                        prog, prog, USAGE_STRING);
                return 1;
            }
        }
    }
    (void)fprintf(stdout, "%s %s", ingrap?grapcommentstart:commentstart,prog);
    for (i = 1; i < argc; i++) { (void)fprintf(stdout, " %s", argv[i]); }
    (void)fprintf(stdout, ":%s%s %s (%s %s %s) COMPILER_USED=\"%s\"%s",
        ingrap?grapcommentend:commentend,ingrap?grapcommentstart:commentstart,
        HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED,
        ingrap?grapcommentend:commentend);
#if SILENCE_WHINEY_COMPILERS
    (void)fprintf(stdout, "%s SILENCE_WHINEY_COMPILERS = %d%s",
        ingrap?grapcommentstart:commentstart,SILENCE_WHINEY_COMPILERS,
        ingrap?grapcommentend:commentend);
#endif
    (void)fprintf(stdout, "%s %s%s", ingrap?grapcommentstart:commentstart,
        build_options+4,ingrap?grapcommentend:commentend);

    if (0U<flags['d']) debug=1;

    errs = 0U;

    if (optind >= argc) {
        fp = stdin;
    } else {
        if (0==strcmp(argv[optind],"-")) fp = stdin;
        else {
            path = argv[optind];
            fp = fopen(path,"r");
        }
    }

    maxx=maxlx=maxy=-1.0e300, minx=minlx=miny=0.0-maxx;
    deltax=deltay=1.0e300;
    /* read input */
    for (maxdim=dim=count=0UL; NULL!=fgets(buf,sizeof(buf),fp); ox=dx,oy=dy) {
        dx = strtod(buf, &endptr);
        if ((dx!=0.0) && (dx<minx)) minx=dx;
        if (dx>maxx) maxx=dx;
        if (0U!=flags['X']) {
            if (dx==0.0) {
                if (minx<maxx) dx=minx; /* fudge data to prevent log error */
                else dx=1.0e-6;
            }
            lx = log2(dx);
            if (lx<minlx) minlx=lx;
            if (lx>maxlx) maxlx=lx;
        }
        dy = strtod(endptr, &endptr);
        if ((dy!=0.0) && (dy<miny)) miny=dy;
        if (dy>maxy) maxy=dy, maxyx=dx;
        if (0U!=flags['Y']) {
            if (dy==0.0) {
                if (miny<maxy) dy=miny; /* fudge data to prevent log error */
                else dy=1.0e-6;
            }
            ly = log2(dy);
            if (ly<minly) minly=ly;
            if (ly>maxly) maxly=ly;
        }
        if (dx==ox) { /* another dimension, same count */
            dim++;
        } else { /* next count */
            if (0UL<count) {
                d=dx-ox;
                if (0.0>d) d=0.0-d;
                if (d<deltax) deltax=d;
                d=dy-oy;
                if (0.0>d) d=0.0-d;
                if (d<deltay) deltay=d;
            }
            count++;
            if (NULL!=x) { x=realloc(x,sizeof(double)*count);
            } else { x=malloc(sizeof(double)*count); }
            x[count-1UL]=dx;
            if (0U!=flags['X']) {
                if (NULL!=plx) { plx=realloc(plx,sizeof(double)*count);
                } else { plx=malloc(sizeof(double)*count); }
                plx[count-1UL]=lx;
            }
            dim=1UL;
        }
        if (dim>maxdim) {
            if (NULL!=py) { py=realloc(py,sizeof(double *)*dim); py[dim-1UL]=NULL;
            } else { py=calloc(sizeof(double *),dim); }
            if (0U!=flags['Y']) {
                if (NULL!=ply) { ply=realloc(ply,sizeof(double *)*dim); ply[dim-1UL]=NULL;
                } else { ply=calloc(sizeof(double *),dim); }
            }
            if (NULL!=pp) { pp=realloc(pp,sizeof(unsigned char *)*dim); pp[dim-1UL]=NULL;
            } else { pp=calloc(sizeof(unsigned char *),dim); }
            maxdim=dim;
        }
        if (NULL!=py[dim-1UL]) { py[dim-1UL]=realloc(py[dim-1UL],sizeof(double)*count); }
        else { py[dim-1UL]=malloc(sizeof(double)*count); }
        py[dim-1UL][count-1UL]=dy;
        if (0U!=flags['Y']) {
            if (NULL!=ply[dim-1UL]) { ply[dim-1UL]=realloc(ply[dim-1UL],sizeof(double)*count);
            } else { ply[dim-1UL]=malloc(sizeof(double)*count); }
            ply[dim-1UL][count-1UL]=ly;
        }
        if (NULL!=pp[dim-1UL]) { pp[dim-1UL]=realloc(pp[dim-1UL],sizeof(unsigned char)*count); }
        else { pp[dim-1UL]=malloc(sizeof(unsigned char)*count); }
        pp[dim-1UL][count-1UL]=0U;
    }

#if DEBUG_CODE > 1
    for (ul=0UL; ul<dim; ul++) {
        for (q=0UL; q<count; q++) {
            if (0U<pp[ul][q]) continue;
                (void)fprintf(stderr,"%G\t%G\n",x[q],py[ul][q]);
        }
    }
#endif /* DEBUG_CODE */

    if (0UL<count) {
        /* output troff and grap lines */
        if (0U!=flags['p'])
            printf(".po %s\n", page_offset);
        printf(".G1\n");
        ingrap=1;
        if (0U!=flags['X']) {
            if (0U!=flags['Y'])
                printf("coord log x log y\n");
            else
                printf("coord log x\n");
        } else if (0U!=flags['Y'])
            printf("coord log y\n");
/* XXX maybe make frame types settable via option args */
        printf("frame ht %s wid %s top invis right invis\n",graph_height, graph_width);
        yp=formatprecision(stderr,miny,maxy,deltay);
        yw=formatwidth(stderr, miny,maxy,yp);
        maxyticks=htp*3/4/12; /* assumes 12 pt spacing, ticks 75% of graph ht */
(void)fprintf(stderr,"%s %s line %d: widp=%d, miny=%.9G, maxy=%.9G, deltay=%.9G, yp=%d, yw=%d, maxyticks=%d%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,widp,miny,maxy,deltay,yp,yw,maxyticks,ingrap?grapcommentend:commentend);
        if (0U!=flags['Y']) {
            ytickintvl=intvl(stderr, minly, maxly, 1.0, minly, maxly, maxyticks, 0U);
            minytick=mintick(stderr, minly, maxly, 1.0, ytickintvl, minly, maxly);
            maxytick=maxtick(stderr, minly, maxly, 1.0, ytickintvl, minly, maxly);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: minly=%.6G, maxly=%.6G, ytickintvl=%.6G, minytick=%.6G, maxytick=%.6G%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,minly,maxly,ytickintvl,minytick,maxytick,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
            ytickintvl=round(exp2(ytickintvl));
            minytick=exp2(minytick);
            maxytick=exp2(maxytick);
            while (minytick*ytickintvl<miny) minytick*=ytickintvl;
            while (maxytick>maxy*ytickintvl) maxytick/=ytickintvl;
            yp=formatprecision(stderr,minytick,maxytick,ytickintvl);
            yw=formatwidth(stderr, minytick,maxytick,yp);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: ytickintvl=%.6G, minytick=%.6G, maxytick=%.6G, yw=%d, yp=%d%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,ytickintvl,minytick,maxytick,yw,yp,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
        } else {
            for (yres=1.0,i=0; i<yp; i++) yres/=10.0;
            ytickintvl=intvl(stderr, miny, maxy, yres, miny, maxy, maxyticks, 0U);
            minytick=mintick(stderr, miny, maxy, yres, ytickintvl, miny, maxy);
            maxytick=maxtick(stderr, miny, maxy, yres, ytickintvl, miny, maxy);
            yp=formatprecision(stderr,minytick,maxytick,ytickintvl);
            yw=formatwidth(stderr, minytick,maxytick,yp);
(void)fprintf(stderr,"%s %s line %d: minytick=%.6G, maxytick=%.6G, yp=%d, yw=%d, ytickintvl=%.6G%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,minytick,maxytick,yp,yw,ytickintvl,ingrap?grapcommentend:commentend);
        }
        xp=formatprecision(stderr,minx,maxx,deltax);
        xw=formatwidth(stderr,minx,maxx,xp);
        maxxticks=widp*4/5/(xw+xp+1-(0U!=flags['X']?2:0))/4; /* assumes digit width 4 pt, ticks 80% of graph width */
(void)fprintf(stderr,"%s %s line %d: widp=%d, maxx=%G, xp=%d, xw=%d, maxxticks=%d%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,widp,maxx,xp,xw,maxxticks,ingrap?grapcommentend:commentend);
        if (0U!=flags['X']) {
            xtickintvl=intvl(stderr, minlx, maxlx, 1.0, minlx, maxlx, maxxticks, 0U);
            minxtick=mintick(stderr, minlx, maxlx, 1.0, xtickintvl, minlx, maxlx);
            maxxtick=maxtick(stderr, minlx, maxlx, 1.0, xtickintvl, minlx, maxlx);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: minlx=%G, maxlx=%G, xtickintvl=%G, minxtick=%G, maxxtick=%G%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,minlx,maxlx,xtickintvl,minxtick,maxxtick,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
            xtickintvl=round(exp2(xtickintvl));
            minxtick=exp2(minxtick);
            maxxtick=exp2(maxxtick);
            while (minxtick*xtickintvl<minx) minxtick*=xtickintvl;
            while (maxxtick>maxx*xtickintvl) maxxtick/=xtickintvl;
            xp=formatprecision(stderr,minxtick,maxxtick,xtickintvl);
            xw=formatwidth(stderr, minxtick,maxxtick,xp);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: xtickintvl=%G, minxtick=%G, maxxtick=%G, xw=%d, xp=%d%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,xtickintvl,minxtick,maxxtick,xw,xp,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
        } else {
            for (xres=1.0,i=0; i<xp; i++) xres/=10.0;
            xtickintvl=intvl(stderr, minx, maxx, xres, minx, maxx, maxxticks, 0U);
            minxtick=mintick(stderr, minx, maxx, xres, xtickintvl, minx, maxx);
            maxxtick=maxtick(stderr, minx, maxx, xres, xtickintvl, minx, maxx);
            xp=formatprecision(stderr,minxtick,maxxtick,xtickintvl);
            xw=formatwidth(stderr, minxtick,maxxtick,xp);
        }
        if (NULL!=title) {
#if 0
            printf("label top \"");
            len=0UL;
            dq=strchr(title,'"');
            if (NULL==dq) pcc=title; else pcc=dq+1;
            do {
                sp=strchr(pcc,' ');
                dq=strchr(pcc,'"');
                if ((NULL==sp)&&(NULL!=dq)) sp=dq;
                else if ((NULL!=sp)&&(NULL!=dq)&&(dq<sp)) sp=dq;
                if (NULL==sp) wordlen=strlen(pcc); else wordlen=(sp-pcc);
                if (widp*3UL/5UL<=len+wordlen) { printf("\" \""); len=0UL; }
                printf("%*.*s", (int)wordlen, (int)wordlen, pcc);
                len+=wordlen;
                if (NULL!=sp) { printf(" "); len++; pcc=sp+1; }
                else pcc=sp;
            } while (NULL!=pcc);
            printf("\"\n");
#else
            printf("label top %s\n", title);
#endif
        }
        (void)sng(buf,sizeof(buf),NULL,NULL,minytick,0-(yw+yp),3,logger,log_arg);
        (void)sng(buf2,sizeof(buf2),NULL,NULL,maxytick,0-(yw+yp),3,logger,log_arg);
        /* tick numbers look better if width is left unspecified */
        if (0U!=flags['Y'])
            printf("ticks left out from %s to %s by *%.6f \"%%.%df\"\n", buf, buf2, ytickintvl, yp);
        else
            printf("ticks left out from %s to %s by %.6f \"%%.%df\"\n", buf, buf2, ytickintvl, yp);
        if (NULL!=ylabel)
            printf("label left %s\n",ylabel);
        (void)sng(buf,sizeof(buf),NULL,NULL,minxtick,0-(xw+xp),3,logger,log_arg);
        (void)sng(buf2,sizeof(buf2),NULL,NULL,maxxtick,0-(xw+xp),3,logger,log_arg);
        if (0U!=flags['X'])
            printf("ticks bot out from %s to %s by *%.6f \"%%.%df\"\n", buf, buf2, xtickintvl, xp);
        else
            printf("ticks bot out from %s to %s by %.6f \"%%.%df\"\n", buf, buf2, xtickintvl, xp);
        /* legend */
        buf[0]='\0';
        xlines=0UL;
        if (0UL<legend_number) {
            q = nmarks;
            if (dim<q) q=dim;
            if (0UL<q) {
                len=strlcat(buf,"Legend:",sizeof(buf));
                xlines++;
            }
            for (ul=0UL; ul<q; ul++) {
                wordlen=strlen(symbols[ul].legend_text);
                if (widp*1UL/3UL<=len+wordlen+(0UL<ul?6UL:4UL)) {
                    strlcat(buf,"\" \"",sizeof(buf));
                    len=0UL;
                    xlines++;
                } else {
                    snprintf(buf2,sizeof(buf2),"\\0%s",0UL<ul?"\\0\\0":"");
                    len+=(0UL<ul?4UL:2UL);
                    strlcat(buf,buf2,sizeof(buf));
                }
                snprintf(buf2,sizeof(buf2),"%s\\0%s", symbols[ul].mchar,
                    NULL!=symbols[ul].legend_text?symbols[ul].legend_text:"?"
                );
                len+=wordlen+2UL;
                strlcat(buf,buf2,sizeof(buf));
            }
        }
        /* X-axis label and/or legend */
        if (NULL!=xlabel) {
            xlines++;
            if ('\0'!=buf[0]) {
                printf("label bot %s \"%s\" down %.6f\n",xlabel,buf,
                    (double)(xlines-1UL)*0.1);
            } else {
                printf("label bot %s\n",xlabel);
            }
        } else if ('\0'!=buf[0]) {
            printf("label bot %s down %.6f\n",buf,(double)(xlines-1UL)*0.1);
        }

        for (ul=0UL; ul<dim; ul++) {
            printf("draw data%lu solid %s\n", ul, symbols[ul%nmarks].marker);
            if (0U!=flags['X']) {
                if (0U!=flags['Y'])
                    q=trim(plx, ply[ul], pp[ul], 0UL, count-1UL, lim);
                else
                    q=trim(plx, py[ul], pp[ul], 0UL, count-1UL, lim);
            } else if (0U!=flags['Y'])
                q=trim(x, ply[ul], pp[ul], 0UL, count-1UL, lim);
            else
                q=trim(x, py[ul], pp[ul], 0UL, count-1UL, lim);
            (void)fprintf(stderr,"%s %s line %d: trim reports %lu trimmed data points (from %lu) %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,q,count,ingrap?grapcommentend:commentend);
            for (q=0UL; q<count; q++) {
                if (0U<pp[ul][q]) continue;
                printf("next data%lu at %G, %G\n", ul, x[q], py[ul][q]);
            }
        }

        if (0U!=flags['m']) {
            if (maxyx < maxx/2.0)
                printf("\"\\0(%.0f,%.3f)\" ljust above at %G,%G\n", maxyx, maxy, maxyx, maxy);
            else
                printf("\"(%.0f,%.3f)\\0\" rjust above at %G,%G\n", maxyx, maxy, maxyx, maxy);
        }
        /* support multiple horizontal, vertical lines by re-reading argv */
        if ((0U!=flags['H'])||(0U!=flags['V'])) {
            for (optind = 1; (optind < argc) && (argv[optind][0] == '-'); optind++) {
                if (0 == strcmp(argv[optind], "--")) {
                    optind++;
                    break;
                }
                for (pcc = argv[optind] + 1; (int)'\0' != (c = (int)(*pcc)); pcc++) {
                    if ((':' == (char)c)
                    || (NULL == (endptr=strchr(OPTSTRING, (char)c))))
                        c = (int)'?';
                    switch (c) {
                        /* this program has no options (that's what a compiler diagnostic for the loop is about) */
#if SILENCE_WHINEY_COMPILERS
                        case ':': /* clang! STFU!! */ /* cannot happen here, but clang is unable to figure that out */
                        break;
#endif
                        case 'H' :
                            if ('\0' == *(++pcc))
                                pcc = argv[++optind];
                            hline = strtod(pcc, NULL);
                            for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                            pcc--;
                            printf("line dashed from %G,%G to %G,%G\n", minxtick, hline, maxxtick, hline);
                        break;
                        case 'V' :
                            if ('\0' == *(++pcc))
                                pcc = argv[++optind];
                            vline = strtod(pcc, NULL);
                            for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                            pcc--;
                            printf("line dashed from %G,%G to %G,%G\n", vline, minytick, vline, maxytick);
                        break;
                        default :
                            if (':'==endptr[1]) {
                                /* skip over option args */
                                if ('\0' == *(++pcc))
                                    pcc = argv[++optind];
                                for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                                pcc--;
                            }
                        break;
                    }
                }
            }
        }
        printf(".G2\n");
        ingrap=0;
    }

    if (NULL!=x) { free(x); x=NULL; }
    if (NULL!=py) {
        for (ul=0UL; ul<dim; ul++) {
            free(py[ul]); py[ul]=NULL;
        }
        free(py); py=NULL;
    }
    if (NULL!=ply) {
        for (ul=0UL; ul<dim; ul++) {
            free(ply[ul]); ply[ul]=NULL;
        }
        free(ply); ply=NULL;
    }
    if (NULL!=pp) {
        for (ul=0UL; ul<dim; ul++) {
            free(pp[ul]); pp[ul]=NULL;
        }
        free(pp); pp=NULL;
    }

    if (stdin!=fp) fclose(fp);

    return (0U==errs?0:1);

#undef buildstr
#undef xbuildstr
}
