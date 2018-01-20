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
#include "snn.h"                /* sng snl snround snul */
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
#include <math.h>               /* exp2 floor log2 */
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
#define OPTSTRING "C:c:dh:H:l:L:M:mn:p:T:V:w:x:Xy:Yz:"
#define USAGE_STRING     "[-C color] [-c color] [-d] [-h ht] [-H val [color]] [-l legend-text] [-L lim] [-M axis maximum] [-m] [-n [x][y]] [-p offset] [-T \"title\"] [-V val [color]] [-w wid] [-x \"x-label\"] [-X] [-y \"y-label\"] [-Y] [-z [x][y]]\n\
-C color\tset color for plotting symbol\n\
-c color\tset color for plot line\n\
-h ht\tspecify graph height\n\
-H val [color]\tdraw a dashed horizontal line at val with optional color\n\
-l legend-text\tlegend for data (may be repeated for multiplots)\n\
-L lim\tset threshold for data reduction\n\
-M[x|y]n\t clip data points exceeding value n on x or y axis\n\
-m\tmark maximum data point\n\
-n[x][y]\tomit x- or y-axis ticks\n\
-p offset\tset page offset\n\
-T \"title\"\tset graph title\n\
-V val [color]\tdraw a dashed vertical line at val with optional color\n\
-w wid\tspecify graph width\n\
-x \"xlabel\"\tlabel x-axis\n\
-X\t use log scaling for the x-axis\n\
-y \"y-label\"\tlabel y-axis\n\
-Y\t use log scaling for the y-axis\n\
-z[x][y]\tforcezero value on axis\n\
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
    const char *line_color;
    const char *symbol_color;
};

int main(int argc, char *argv[]) /* XPG (see exec()) */
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
    const char *xlabel=NULL, *ylabel=NULL, *path, *pcc, *title=NULL;
    const char *commentstart = ".\\\"";
    const char *commentend = "\n";
    const char *grapcommentstart = "#";
    const char *grapcommentend = "\n";
    char *endptr=NULL;
    struct symbol_struct symbols[] = {
        { "bullet", "\\(bu",                NULL, NULL, NULL } ,
        { "times",  "\\(mu",                NULL, NULL, NULL } ,
        { "plus",   "\\(pl",                NULL, NULL, NULL } ,
        { "delta",  "\\(*D",                NULL, NULL, NULL } ,
        { "square", "\\(sq",                NULL, NULL, NULL } ,
        { "dot",    "\\v~-0.2m~.\\v~0.2m~", NULL, NULL, NULL } , /* almost invisible */
        { "box",    "\\f(ZD\\N~110~\\fP",   NULL, NULL, NULL } , /* can obscure others */
        { "star",   "\\(**",                NULL, NULL, NULL } , /* can be confused for overlayed x and + */
        { "ring",   "\\(ci",                NULL, NULL, NULL } ,
        { "htick",  "\\(em",                NULL, NULL, NULL } , /* poor for nearly horizontal plots */
        { "vtick",  "|",                    NULL, NULL, NULL } , /* poor for nearly vertical plots */
    };
    size_t nmarks = sizeof(symbols)/sizeof(symbols[0]);
    size_t dim, line_number = 0UL, symbol_number = 0UL, legend_number = 0UL, len, maxdim, q, wordlen, xlines;
    size_t xc, label_num, start_label, line_len, max_len;
    int bufsz, c, i, ingrap=0, optind;
    int maxlen = 1024;
    int sockfd = -1;
    int htp, maxxticks, maxyticks, ps=12, vs=14, widp, xm, xp, xw, ym, yp, yw;
    unsigned char flags[256];
    unsigned char **pp=NULL;
    unsigned int seqid = 1U;
    unsigned int tzknown = 1U;
    unsigned int errs=0U;
    volatile unsigned long count, l, r, ul;
    pid_t pid;
    void (*f)(int, void *, const char *, ...) = logger;
    void *log_arg;
    struct logger_struct ls;
    FILE *fp;
    double d, dx,dy, hline, lim = 0.70, lx,ly, ox=-1.0e300, oy=-1.0e300, vline;
    double maxx, maxlx, maxly, maxy, maxyx=0.0, minx, minlx, minly, miny;
    double xlim=1.0e300, ylim=1.0e300;
    double *x=NULL, *plx=NULL, **py=NULL, **ply=NULL;
    double minytick, maxytick, ytickintvl, minxtick, maxxtick, tick, xtickintvl;
    double cw, deltax, deltay, xres, yres;
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
                default :
                    flags[c] = 1U;
                break;
                case 'C' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    symbols[symbol_number%nmarks].symbol_color = pcc;
                    symbol_number++;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'c' :
                    flags[c] = 1U;
                    if ('\0' == *(++pcc))
                        pcc = argv[++optind];
                    symbols[line_number%nmarks].line_color = pcc;
                    line_number++;
                    for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                    pcc--;
                break;
                case 'd' :
                    flags[c] = 1U;
                    debug++;
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
                    hline = strtod(pcc, &endptr);
                    pcc=endptr;
                    if ('\0' == *(pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* color */
                    }
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
                case 'M' : 
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind];
                    }
                    for (; '\0' != *pcc; pcc++)
                        switch (*pcc) {
                            case 'x' : /*FALLTHROUGH*/
                            case 'X' :
                                flags[c] |= 2U;
                                i='x';
                            break;
                            case 'y' : /*FALLTHROUGH*/
                            case 'Y' :
                                flags[c] |= 4U;
                                i='y';
                            break;
                            case ' ': /*FALLTHROUGH*/ case '\t': /* whitespace */
                            break;
                            default : /* a number */
                                d = strtod(pcc, &endptr);
                                if (i=='x') xlim=d;
                                else if (i=='y') ylim=d;
                                pcc=endptr;
                                pcc--;
                            break;
                        }
                    pcc--;
                break;
                case 'n' : 
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind];
                    }
                    for (; '\0' != *pcc; pcc++)
                        switch (*pcc) {
                            case 'x' : /*FALLTHROUGH*/
                            case 'X' :
                                flags[c] |= 2U;
                            break;
                            case 'y' : /*FALLTHROUGH*/
                            case 'Y' :
                                flags[c] |= 4U;
                            break;
                        }
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
                    vline = strtod(pcc, &endptr);
                    pcc=endptr;
                    if ('\0' == *(pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind]; /* color */
                    }
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
                case 'z' : 
                    flags[c] = 1U;
                    if ('\0' == *(++pcc)) {
                        if ('-' == argv[optind+1][0]) {
                            /* next arg */
                            pcc--;
                            continue;
                        }
                        pcc = argv[++optind];
                    }
                    for (; '\0' != *pcc; pcc++)
                        switch (*pcc) {
                            case 'x' : /*FALLTHROUGH*/
                            case 'X' :
                                flags[c] |= 2U;
                            break;
                            case 'y' : /*FALLTHROUGH*/
                            case 'Y' :
                                flags[c] |= 4U;
                            break;
                        }
                    pcc--;
                break;
                case '?' :
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
    (void)fprintf(stdout, "%s%s %s (%s %s %s) COMPILER_USED=\"%s\"%s",
        ingrap?grapcommentend:commentend,ingrap?grapcommentstart:commentstart,
        HOST_FQDN, OS, OSVERSION, DISTRIBUTION, COMPILER_USED,
        ingrap?grapcommentend:commentend);
#if SILENCE_WHINEY_COMPILERS
    (void)fprintf(stdout, "%s SILENCE_WHINEY_COMPILERS = %d %s",
        ingrap?grapcommentstart:commentstart,SILENCE_WHINEY_COMPILERS,
        ingrap?grapcommentend:commentend);
#endif
    (void)fprintf(stdout, "%s %s %s", ingrap?grapcommentstart:commentstart,
        build_options+4,ingrap?grapcommentend:commentend);

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
// if (debug) (void)fprintf(stderr, "x=%G, buf %s\n",dx, buf);
        if ((dx!=0.0) && (dx<minx)) minx=dx;
        if ((dx>maxx)&&(dx<=xlim)) maxx=dx;
        if (0U!=flags['X']) {
            if (dx==0.0) {
                if (minx<maxx) dx=minx; /* fudge data to prevent log error */
                else dx=1.0e-6;
            }
            lx = log2(dx);
            if (lx<minlx) minlx=lx;
            if ((lx>maxlx)&&(dx<=xlim)) maxlx=lx;
        }
        dy = strtod(endptr, &endptr);
// if (debug) (void)fprintf(stderr, "y=%G, buf %s\n",dy, buf);
        if ((dy!=0.0) && (dy<miny)) miny=dy;
        if ((dy>maxy)&&(dy<=ylim)) maxy=dy, maxyx=dx;
        if (0U!=flags['Y']) {
            if (dy==0.0) {
                if (miny<maxy) dy=miny; /* fudge data to prevent log error */
                else dy=1.0e-6;
            }
            ly = log2(dy);
            if (ly<minly) minly=ly;
            if ((ly>maxly)&&(dy<=ylim)) maxly=ly;
        }
        if (dx==ox) { /* another dimension, same count */
            dim++;
        } else { /* next count */
            if (0UL<count) {
                d=dx-ox;
                if (0.0>d) d=0.0-d;
                if ((d<deltax)&&(0.0<d)) deltax=d;
                d=dy-oy;
                if (0.0>d) d=0.0-d;
                if ((d<deltay)&&(0.0<d)) deltay=d;
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
        if (widp < 433) {
            if (widp < 361) {
                if (widp < 289) {
                    printf(".ps -3\n");
                    printf(".vs -3\n");
                    ps -= 3;
                    vs -= 3;
                } else {
                    printf(".ps -2\n");
                    printf(".vs -2\n");
                    ps -= 2;
                    vs -= 2;
                }
            } else {
                printf(".ps -1\n");
                printf(".vs -1\n");
                ps -= 1;
                vs -= 1;
            }
        }
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
        ym=snmagnitude(maxy,NULL,NULL);
        if (0>ym) ym++;
        yp=formatprecision(stderr,miny,maxy,deltay);
        if (yp-ym>8) yp=8-ym;
        yw=formatwidth(stderr, miny,maxy,yp);
        maxyticks=htp*4*4/5/5/vs; /* assumes ticks 80% of graph ht and 80% maximum text density */
(void)fprintf(stderr,"%s %s line %d: widp=%d, miny=%.15G, maxy=%.15G, deltay=%.15G, yp=%d, yw=%d, maxyticks=%d %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,widp,miny,maxy,deltay,yp,yw,maxyticks,ingrap?grapcommentend:commentend);
        if (0U!=flags['Y']) {
            flags['z'] &= (~4U); /* can't have zero on log axis */
            ytickintvl=intvl(stderr, minly, maxly, 1.0, minly, maxly, maxyticks, 0U);
            minytick=mintick(stderr, minly, maxly, 1.0, ytickintvl, minly, maxly);
            maxytick=maxtick(stderr, minly, maxly, 1.0, ytickintvl, minly, maxly);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: minly=%.15G, maxly=%.15G, ytickintvl=%.15G, minytick=%.15G, maxytick=%.15G%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,minly,maxly,ytickintvl,minytick,maxytick,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
            ytickintvl=snround(exp2(ytickintvl),NULL,NULL);
            minytick=exp2(minytick);
            maxytick=exp2(maxytick);
            while (minytick*ytickintvl<miny) minytick*=ytickintvl;
            while (maxytick>maxy*ytickintvl) maxytick/=ytickintvl;
            yp=formatprecision(stderr,minytick,maxytick,ytickintvl);
            yw=formatwidth(stderr, minytick,maxytick,yp);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: ytickintvl=%.15G, minytick=%.15G, maxytick=%.15G, yw=%d, yp=%d %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,ytickintvl,minytick,maxytick,yw,yp,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
        } else {
            for (yres=1.0,i=0; i<yp; i++) yres/=10.0;
            if (0U!=flags['z']) {
                if (4U==(4U&flags['z'])) {
                    if (miny>0.0) miny=0.0;
                    if (maxy<0.0) maxy=0.0;
                }
            }
            ytickintvl=intvl(stderr, miny, maxy, yres, miny, maxy, maxyticks, 0U);
            minytick=mintick(stderr, miny, maxy, yres, ytickintvl, miny, maxy);
            maxytick=maxtick(stderr, miny, maxy, yres, ytickintvl, miny, maxy);
            yp=formatprecision(stderr,minytick,maxytick,ytickintvl);
            i=formatprecision(stderr,ytickintvl,ytickintvl,ytickintvl);
            if (yp>i) yp=i;
            yw=formatwidth(stderr, minytick,maxytick,yp);
(void)fprintf(stderr,"%s %s line %d: minytick=%.15G, maxytick=%.15G, ym=%d, yp=%d, yw=%d, ytickintvl=%.15G%s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,minytick,maxytick,ym,yp,yw,ytickintvl,ingrap?grapcommentend:commentend);
        }
        xm=snmagnitude(maxx,NULL,NULL);
        if (0>xm) xm++;
        xp=formatprecision(stderr,minx,maxx,deltax);
        if (xp-xm>8) xp=8-xm;
        xw=formatwidth(stderr,minx,maxx,xp);
        maxxticks=widp*3*4/5/(xw+xp+1-(0U!=flags['X']?1:0))/ps; /* assumes digit width 4 pt @ ps=12, ticks 80% of graph width */
(void)fprintf(stderr,"%s %s line %d: widp=%d, maxx=%G, xm=%d, xp=%d, xw=%d, maxxticks=%d %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,widp,maxx,xm,xp,xw,maxxticks,ingrap?grapcommentend:commentend);
        if (0U!=flags['X']) {
            flags['z'] &= (~2U); /* can't have zero on log axis */
            d=(maxlx-minlx)/(maxxticks-1);
            if (d>=1.5)
                d=snmultiple(d,2.0,NULL,NULL);
            else
                d=1.0;
            xtickintvl=intvl(stderr, minlx, maxlx, d, minlx, maxlx, maxxticks, 0U);
            xtickintvl=snround(exp2(xtickintvl),NULL,NULL);
            minxtick=minx;
            maxxtick=maxx;
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: minlx=%G, maxlx=%G, xtickintvl=%G, minxtick=%G, maxxtick=%G %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,minlx,maxlx,xtickintvl,minxtick,maxxtick,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
            xp=formatprecision(stderr,minxtick,maxxtick,xtickintvl);
            xw=formatwidth(stderr, minxtick,maxxtick,xp);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: xtickintvl=%G, minxtick=%G, maxxtick=%G, xw=%d, xp=%d %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,xtickintvl,minxtick,maxxtick,xw,xp,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
        } else {
            for (xres=1.0,i=0; i<xp; i++) xres/=10.0;
            if (0U!=flags['z']) {
                if (2U==(2U&flags['z'])) {
                    if (minx>0.0) minx=0.0;
                    if (maxx<0.0) maxx=0.0;
                }
            }
            xtickintvl=intvl(stderr, minx, maxx, xres, minx, maxx, maxxticks, 0U);
            minxtick=mintick(stderr, minx, maxx, xres, xtickintvl, minx, maxx);
            maxxtick=maxtick(stderr, minx, maxx, xres, xtickintvl, minx, maxx);
            xp=formatprecision(stderr,minxtick,maxxtick,xtickintvl);
            xw=formatwidth(stderr, minxtick,maxxtick,xp);
        }
        if (NULL!=title) {
            printf("label top %s\n", title);
        }
        /* tick numbers look better if width is left unspecified */
        if (0U==(flags['n']&4U)) {
            if (0U!=flags['Y']) {
                for (tick=minytick; tick<maxytick*ytickintvl*9.0/11.0; tick*=ytickintvl) {
                    c=snf(buf,sizeof(buf),NULL,NULL,tick,'0',1,yp,logger,log_arg);
                    i=sng(buf2,sizeof(buf2),NULL,NULL,tick,(yp+ym),3,logger,log_arg);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: tick=%G, snf \"%s\" (%d), sng \"%s\" (%d) %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,tick,buf,c,buf2,i,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
                    (void)printf("tick left out at %.9G \"%s\"\n",tick,c<=i?buf:buf2);
                }
            } else {
                for (tick=minytick; tick<maxytick+ytickintvl; tick+=ytickintvl) {
                    c=snf(buf,sizeof(buf),NULL,NULL,tick,'0',1,yp,logger,log_arg);
                    i=sng(buf2,sizeof(buf2),NULL,NULL,tick,(yp+ym),3,logger,log_arg);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: tick=%G, snf \"%s\" (%d), sng \"%s\" (%d) %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,tick,buf,c,buf2,i,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
                    (void)printf("tick left out at %.9G \"%s\"\n",tick,c<=i?buf:buf2);
                }
            }
        } else printf("ticks left off\n");
        if (NULL!=ylabel)
            printf("label left %s\n",ylabel);
        if (0U==(flags['n']&2U)) {
            if (0U!=flags['X']) {
                for (tick=minxtick; tick<maxxtick*xtickintvl*9.0/11.0; tick*=xtickintvl) {
                    c=snf(buf,sizeof(buf),NULL,NULL,tick,'0',1,xp,logger,log_arg);
                    i=sng(buf2,sizeof(buf2),NULL,NULL,tick,(xp+xm),3,logger,log_arg);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: tick=%G, snf \"%s\" (%d), sng \"%s\" (%d) %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,tick,buf,c,buf2,i,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
                    (void)printf("tick bot out at %.9G \"%s\"\n",tick,c<=i?buf:buf2);
                }
            } else {
                for (tick=minxtick; tick<maxxtick+xtickintvl; tick+=xtickintvl) {
                    c=snf(buf,sizeof(buf),NULL,NULL,tick,'0',1,xp,logger,log_arg);
                    i=sng(buf2,sizeof(buf2),NULL,NULL,tick,(xp+xm),3,logger,log_arg);
#if DEBUG_CODE
(void)fprintf(stderr,"%s %s line %d: tick=%G, snf \"%s\" (%d), sng \"%s\" (%d) %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,tick,buf,c,buf2,i,ingrap?grapcommentend:commentend);
#endif /* DEBUG_CODE */
                    (void)printf("tick bot out at %.9G \"%s\"\n",tick,c<=i?buf:buf2);
                }
            }
        } else printf("ticks bot off\n");
        /* legend */
/* Faber's grap through version 1.45 does not handle multiple labels per axis */
#define MULTILABEL 0
        buf[0]='\0';
        xlines=0UL;
        dy=(double)vs/72.0; /* text point size vertical space for text labels */
        if (0U==(flags['n']&4U)) {
            i=11;
            if (0U!=flags['z']) {
                if (4U==(4U&flags['z'])) {
                   yw=1;
                }
            }
        } else {
            yw=i=0;
            if (0U!=flags['z']) {
                if (4U==(4U&flags['z'])) {
                   yw=1,i=11;
                }
            }
        }
        c=3; /* character width is presumed to be ps/c points */
#if MULTILABEL
        if (0UL<legend_number) {
            q = nmarks;
            if (dim<q) q=dim;
            if (0UL<q) {
                len=strlcat(buf,"Legend:",sizeof(buf));
                xlines++;
            }
            for (ul=0UL; ul<q; ul++) {
                endptr=symbols[ul].legend_text;
                if (NULL==endptr) wordlen=1;
                else wordlen=strlen(endptr);
#if 1
if (0!=ingrap)
(void)printf("%s %s line %d: ps=%d, c=%d, i=%d, yw=%d, widp=%d, wordlen=%lu, len=%lu, ul=%lu, xlines=%lu%s",grapcommentstart,__func__,__LINE__,ps,c,i,yw,widp,wordlen,len,ul,xlines,grapcommentend);
else
(void)printf("%s %s line %d: ps=%d, c=%d, i=%d, yw=%d, widp=%d, wordlen=%lu, len=%lu, ul=%lu, xlines=%lu%s",commentstart,__func__,__LINE__,ps,c,i,yw,widp,wordlen,len,ul,xlines,commentend);
#endif
                if (widp<=ps*(len+wordlen+(0UL<ul?6UL:4UL))/c) {
                    strlcat(buf,"\" \"",sizeof(buf));
                    len=0UL;
                    xlines++;
                } else {
                    snprintf(buf2,sizeof(buf2),"\\0%s",0UL<ul?"\\0\\0":"");
                    len+=(0UL<ul?4UL:2UL);
                    strlcat(buf,buf2,sizeof(buf));
                }
                snprintf(buf2,sizeof(buf2),"%s\\0%s", symbols[ul].mchar,
                    NULL!=endptr?endptr:"?"
                );
                len+=wordlen+2UL;
                strlcat(buf,buf2,sizeof(buf));
            }
        }
#endif
        /* X-axis label and/or legend */
        if (NULL!=xlabel) {
            xlines++;
            if ('\0'!=buf[0]) {
                printf("label bot %s \"%s\" down %.6f\n",xlabel,buf,
                    (double)(xlines-1UL)*dy);
            } else {
                printf("label bot %s\n",xlabel);
            }
        } else if ('\0'!=buf[0]) {
            xlines++;
            printf("label bot %s down %.6f\n",buf,(double)(xlines-1UL)*dy);
        }
#if ! MULTILABEL
        /* workaround for lack of multi-label support in Faber grap */
        if (0UL<legend_number) {
            printf(
                "%s DWB grap parameterizes the frame with framewid and frameht; Faber grap does not%s",
                grapcommentstart,grapcommentend);
            printf("pic frameht2=Frame.n.y-Frame.s.y\n");
            printf("pic framewid2=Frame.e.x-Frame.w.x\n");
            printf(
                "%s An invisible box is used for the text, with height 1/6 inch at default text size (i.e. ps) (like DWB grap)%s",
                grapcommentstart,grapcommentend);
            printf(
                "%s The box is widened to the left to make use of the space below the y-axis ticks and label%s",
                grapcommentstart,grapcommentend);
            printf(
                "%s The widening is not essential, as text associated with a line can be wider than the length of the line%s",
                grapcommentstart,grapcommentend);
            printf(
                "%s However, widening the box avoids having to apply a manual x offset for centering the text%s",
                grapcommentstart,grapcommentend);
            printf(
                "%s With multiple label support, individual labels can be positioned (up, down, left, right) and justified (ljust, rjust)%s",
                grapcommentstart,grapcommentend);
            printf(
                "%s An invisible line is used for alignment of the text (like Faber grap)%s",
                grapcommentstart,grapcommentend);
            buf[0]='\0';
            q = nmarks;
            if (dim<q) q=dim;
            if (0UL<q) {
                len=strlcat(buf,"Legend:",sizeof(buf));
                xlines++;
            }
            d=(double)(yw*ps/c+i)/72.0; /* space for y-axis ticks and tick numbers, in inches */
            max_len = yw+(widp+i)*c/ps;
            cw = (double)ps / (double)c / 72.0; /* character width, inches */
# if 1
if (0!=ingrap)
(void)printf("%s %s line %d: ps=%d, c=%d, i=%d, yw=%d, widp=%d, max_len=%lu%s",grapcommentstart,__func__,__LINE__,ps,c,i,yw,widp,max_len,grapcommentend);
else
(void)printf("%s %s line %d: ps=%d, c=%d, i=%d, yw=%d, widp=%d, max_len=%lu%s",commentstart,__func__,__LINE__,ps,c,i,yw,widp,max_len,commentend);
# endif
            line_len = 8UL; /* "Legend:\0" */
            for (start_label=ul=0UL; ul<=q; ul++) {
                if (ul<q) endptr=symbols[ul].legend_text;
                else endptr="";
                if (NULL!=endptr) {
                    if (ul<q) wordlen=strlen(endptr);
                    else wordlen = max_len;
                    if (ul>start_label) xc=5UL; else xc=2UL;
                    if (line_len + wordlen + xc <= max_len)
                        line_len += wordlen + xc;
                    else {
                        /* overall box */
                        printf(
                            "pic Label: box invis wid framewid2+%.6f ht %.6f with .ne at Frame.se -0,%.6f\n",
                            d,dy,dy+(double)xlines*dy);
                        /* box for necessary length line_len */
                        printf(
                            "pic Textbox: box invis wid %lu*%.6f ht %.6f with .c at Label.c\n",
                            line_len,cw,dy);
                        for (label_num=start_label; label_num<ul; label_num++) {
                            if (0UL==label_num) {
                                /* box for "Legend:\0" */
                                printf(
                                    "pic Legendtextbox: box invis wid %lu*%.6f ht %.6f with .w at Textbox.w\n",
                                    8UL,cw,dy);
                                /* Line with "Legend:\0" text */
                                printf(
                                    "pic line invis from Legendtextbox.w to Legendtextbox.e \"%s\" aligned\n",
                                    "Legend:\0"); 
                                /* box for symbol (after "Legend:\0" box) */
                                printf(
                                    "pic Symbolbox%lu: box invis wid %lu*%.6f ht %.6f with .w at Legendtextbox.e + %.6f,0\n",
                                    label_num,2UL,cw,dy,cw);
                            } else {
                                if (label_num > start_label) {
                                    /* box for symbol (offset from last label's box) */
                                    printf(
                                        "pic Symbolbox%lu: box invis wid %lu*%.6f ht %.6f with .w at Subtextbox%lu.e + 3.0 * %.6f,0\n",
                                        label_num,2UL,cw,dy,label_num-1UL,cw);
                                } else {
                                    /* box for symbol (at start of line) */
                                    printf(
                                        "pic Symbolbox%lu: box invis wid %lu*%.6f ht %.6f with .w at Textbox.w\n",
                                        label_num,2UL,cw,dy);
                                }
                            }
                            endptr=symbols[label_num].legend_text;
                            if (NULL!=endptr) {
                                wordlen=strlen(endptr);
                                /* line with symbol (possibly colored) */
                                printf(
                                    "pic line invis from last box.w to last box.e \"%s\"",
                                    symbols[label_num%nmarks].mchar); 
                                if ((NULL!=symbols[label_num].symbol_color)
                                &&('\0'!=symbols[label_num].symbol_color[0])
                                ) {
                                    printf(" color \"%s\"", symbols[label_num].symbol_color);
                                }
                                printf(" aligned\n");
                                /* box for legend text */
                                printf(
                                    "pic Subtextbox%lu: box invis wid %lu*%.6f ht %.6f with .w at last box.e + %.6f,0\n",
                                    label_num,wordlen,cw,dy,cw);
                                /* line with legend text (possibly colored) */
                                printf(
                                    "pic line invis from last box.w to last box.e \"%s\"",
                                    symbols[label_num].legend_text); 
                                if ((NULL!=symbols[label_num].line_color)
                                &&('\0'!=symbols[label_num].line_color[0])
                                ) {
                                    printf(" color \"%s\"", symbols[label_num].line_color);
                                }
                                printf(" aligned\n");
                            }
                        }
                        xlines++;
                        if (ul==q) break;
                        start_label = ul--;
                        line_len = 0UL;
                    }
                }
            }
        }
#endif

        for (ul=0UL; ul<dim; ul++) {
            printf("draw data%lu solid ", ul);
            if ((NULL!=symbols[ul%nmarks].line_color)
            &&('\0'!=symbols[ul%nmarks].line_color[0]))
                printf("color \"%s\" ", symbols[ul%nmarks].line_color);
            printf("%s", symbols[ul%nmarks].marker);
            if ((NULL!=symbols[ul%nmarks].symbol_color)
            &&('\0'!=symbols[ul%nmarks].symbol_color[0]))
                printf(" color \"%s\"", symbols[ul%nmarks].symbol_color);
            printf("\n");
            l=0UL,r=count-1UL;
#if 1
            for (q=0UL; q<count; q++) {
                if ((x[q]>xlim)||(py[ul][q]>ylim)) {
                    pp[ul][q]=1U;
                    if (q==l) l++;
                    else if (q==r) {
                        while (0U<pp[ul][q]) {
                            r--;
                            if (l==q) break;
                            q--;
                        }
                    }
                }
            }
#endif
            if (0U!=flags['X']) {
                if (0U!=flags['Y'])
                    q=trim(plx, ply[ul], pp[ul], l, r, lim);
                else
                    q=trim(plx, py[ul], pp[ul], l, r, lim);
            } else if (0U!=flags['Y'])
                q=trim(x, ply[ul], pp[ul], l, r, lim);
            else
                q=trim(x, py[ul], pp[ul], l, r, lim);
            (void)fprintf(stderr,"%s %s line %d: trim reports %lu remaining data points (trimmed from %lu) %s",ingrap?grapcommentstart:commentstart,__func__,__LINE__,q,count,ingrap?grapcommentend:commentend);
            for (q=0UL; q<count; q++) {
                if (0U<pp[ul][q]) continue;
                printf("next data%lu at %G, %G\n", ul, x[q], py[ul][q]);
            }
        }

        if (1U<flags['z']) {
            printf("draw zero invis\n");
            printf("next zero at %G, %G\n", minx, miny);
            printf("next zero at %G, %G\n", maxx, miny);
            if (2U==(flags['z']&flags['n']&2U)) {
                printf("tick bot out at 0.0 \"0\"\n");
            }
            if (4U==(flags['z']&flags['n']&4U)) {
                printf("tick left out at 0.0 \"0\"\n");
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
                            hline = strtod(pcc, &endptr);
                            pcc=endptr;
                            if ('\0' == *(pcc)) {
                                if ('-' != argv[optind+1][0]) {
                                    endptr=argv[++optind]; /* color */
                                }
                            }
                            for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                            pcc--;
                            if ('\0'!=*endptr)
                                printf("line dashed color \"%s\" from %G,%G to %G,%G\n", endptr, minxtick, hline, maxxtick, hline);
                            else
                                printf("line dashed from %G,%G to %G,%G\n", minxtick, hline, maxxtick, hline);
                        break;
                        case 'V' :
                            if ('\0' == *(++pcc))
                                pcc = argv[++optind];
                            vline = strtod(pcc, &endptr);
                            pcc=endptr;
                            if ('\0' == *(pcc)) {
                                if ('-' != argv[optind+1][0]) {
                                    endptr=argv[++optind]; /* color */
                                }
                            }
                            for (; '\0' != *pcc; pcc++) ;   /* pass over arg to satisfy loop conditions */
                            pcc--;
                            if ('\0'!=*endptr)
                                printf("line dashed color \"%s\" from %G,%G to %G,%G\n", endptr, vline, minytick, vline, maxytick);
                            else
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
        if (widp < 433) {
            printf(".ps\n");
            printf(".vs\n");
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
