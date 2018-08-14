/*INDENT OFF*/
/* XXX obsolete source */
#error "obsolete source "

/r Description: C source code for quickselect tables */
/******************************************************************************
* This software is covered by the zlib/libpng license.
* The zlib/libpng license is a recognized open source license by
* the Open Source Initiative: http://opensource.org/licenses/Zlib
* The zlib/libpng license is a recognized "free" software license by
* the Free Software Foundation: https://directory.fsf.org/wiki/License:Zlib
*******************************************************************************
******************* Copyright notice (part of the license) ********************
* $Id: ~|^` @(#)    selection_table.c copyright 2017-2018 Bruce Lilly.   \ selection_table.c $
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
/* $Id: ~|^` @(#)   This is selection_table.c version 1.3 dated 2018-04-18T00:42:19Z. \ $ */
/* You may send bug reports to bruce.lilly@gmail.com with subject "quickselect" */
/*****************************************************************************/
/* maintenance note: master file /data/projects/automation/940/lib/libmedian/src/s.selection_table.c */

/********************** Long description and rationale: ***********************
 For a large number of array elements and relatively few desired order statistic
 ranks, selection is more efficient than a full sort.  However, if the number of
 desired order statistics ranks becomes large relative to the sub-array size, a
 full sort (which avoids the overhead of examining ranks) may be faster.  The
 table defined in this file is used to decide whether to select or sort for
 small subarray sizes.
******************************************************************************/

/* Nothing to configure below this line. */
/* XXX obsolete source */

/* ID_STRING_PREFIX file name and COPYRIGHT_DATE are constant, other components
   are version control fields.
   ID_STRING_PREFIX is suitable for the what(1) and ident(1) utilities.
   MODULE_DATE uses modern SCCS extensions.
*/
#undef ID_STRING_PREFIX
#undef SOURCE_MODULE
#undef MODULE_VERSION
#undef MODULE_DATE
#undef COPYRIGHT_HOLDER
#undef COPYRIGHT_DATE
#define ID_STRING_PREFIX "$Id: selection_table.c ~|^` @(#)"
#define SOURCE_MODULE "selection_table.c"
#define MODULE_VERSION "1.3"
#define MODULE_DATE "2018-04-18T00:42:19Z"
#define COPYRIGHT_HOLDER "Bruce Lilly"
#define COPYRIGHT_DATE "2017-2018"

/* local header files needed */
#include "tables.h"             /* OTHERS DISTRIBUTED SELECTION_BREAKPOINTS */

/* data structures */
/* If many order statistics are desired, sorting may be more efficient than
   explicit selection.  The breakpoint where that tradeoff occurs depends on the
   size of the sub-array, the number of desired order statistics, and the
   distribution of the desired order statistic ranks.  For large sub-arrays,
   sorting is more efficient for widely distributed order statistics if their
   quantity is greater than about 10% of the number of sub-array elements.  If
   the order statistics are grouped together or split between the ends of the
   sub-array, selection is more efficient for large sub-arrays with as many as
   90% of the possible ranks as desired order statistics.  Small sub-arrays
   deviate from these rules of thumb, so a small table is used for breakpoints.
   A table of sets of entries with unsigned values less than 256 is used in
   order to keep the table size small.
*/
/* lookup table; raw distribution to selection_distribution */
/* indices 0U through SELECTION_TYPES-1 */
/* order is SEPARATED=0U, ENDS=1U, MIDDLE=2U, DISTRIBUTED=3U */
/* The number of array elements is determined by the number of possible raw
   distribution vales, i.e. 8.
*/
unsigned char selection_distribution_remap[8] = {
    OTHERS,      /* 000 */
    OTHERS,      /* 001 */
    OTHERS,      /* 010 */
    OTHERS,      /* 011 */
    OTHERS,      /* 100 */
    OTHERS,      /* 101 */
    OTHERS,      /* 110 */
    DISTRIBUTED, /* 111 */
};
/* SELECTION_BREAKPOINT_OFFSET is based on avoidance of recomparisons while
       partitioning.
       Selection for median-of-medians is for a single order statistic (the
       median of the medians) and MINMAX is only usable if that desired rank is
       one of the first two or last two elements.  If the first or last, N-1
       comparisons are required (2 at size 3), and if the second or second-last,
       2N-2 comparisons are required (4 at size 3).  At size 3, sorting is
       always at least as efficient as selection.  At size 4, selection is only
       efficient for the first or last element.
*/
/* Selection breakpoints for nmemb SELECTION_BREAKPOINT_OFFSET through
   SELECTION_BREAKPOINT_OFFSET+SELECTION_BREAKPOINTS-1:
   First index is nmemb-BREAKPOINT_OFFSET
   Second index is order statistic distribution remapped via
      selection_distribution_remap[distribution]
      where distribution itself has been remapped from raw distribution via
      sampling_distribution_remap[distribution]
   Value is maximum number of desired order statistics with a certain
     distribution which can be selected via multiple selection; for more
     order statistics, sort the sub-array.
   Two tables; one for non-stable selection, the other for stable selection.
*/
/* XXX obsolete source */
unsigned char selection_breakpoint[SELECTION_BREAKPOINTS][SELECTION_TYPES] = {
/* DISTRIBUTED OTHERS nmemb DISTRIBUTED OTHERS nmemb DISTRIBUTED OTHERS nmemb */
    {   1U,   2U },/*  4*/    {   1U,   3U },/*  5*/    {   2U,   4U },/*  6*/
    {   2U,   5U },/*  7*/    {   3U,   6U },/*  8*/    {   2U,   4U },/*  9*/
    {   2U,   6U },/* 10*/    {   3U,   7U },/* 11*/    {   5U,  10U },/* 12*/
    {   5U,  11U },/* 13*/    {   6U,  12U },/* 14*/    {   6U,  13U },/* 15*/
    {   7U,  14U },/* 16*/    {   7U,  14U },/* 17*/    {   5U,  12U },/* 18*/
    {   5U,  13U },/* 19*/    {   5U,  14U },/* 20*/    {   6U,  15U },/* 21*/
    {   6U,  16U },/* 22*/    {   6U,  17U },/* 23*/    {   7U,  18U },/* 24*/
    {   7U,  19U },/* 25*/    {  11U,  22U },/* 26*/    {  11U,  22U },/* 27*/
    {  11U,  23U },/* 28*/    {  12U,  24U },/* 29*/    {  12U,  25U },/* 30*/
    {  13U,  26U },/* 31*/    {  13U,  27U },/* 32*/    {  13U,  27U },/* 33*/
    {  14U,  28U },/* 34*/    {  14U,  29U },/* 35*/    {  15U,  30U },/* 36*/
    {  15U,  31U },/* 37*/    {  16U,  32U },/* 38*/    {  16U,  33U },/* 39*/
    {  17U,  34U },/* 40*/    {  17U,  35U },/* 41*/    {  18U,  36U },/* 42*/
    {  18U,  36U },/* 43*/    {  18U,  37U },/* 44*/    {  19U,  38U },/* 45*/
    {  19U,  39U },/* 46*/    {  20U,  40U },/* 47*/    {  20U,  41U },/* 48*/
    {  21U,  42U },/* 49*/    {  21U,  43U },/* 50*/    {  22U,  44U },/* 51*/
    {  22U,  45U },/* 52*/    {  23U,  46U },/* 53*/    {  23U,  46U },/* 54*/
    {  23U,  48U },/* 55*/    {  24U,  48U },/* 56*/    {  24U,  49U },/* 57*/
    {  29U,  54U },/* 58*/    {  25U,  51U },/* 59*/    {  18U,  50U },/* 60*/
    {  19U,  51U },/* 61*/    {  19U,  52U },/* 62*/    {  19U,  53U },/* 63*/
    {  20U,  54U },/* 64*/    {  20U,  55U },/* 65*/    {  20U,  56U },/* 66*/
    {  21U,  57U },/* 67*/    {  21U,  58U },/* 68*/    {  21U,  59U },/* 69*/
    {  22U,  60U },/* 70*/    {  22U,  61U },/* 71*/    {  23U,  62U },/* 72*/
    {  23U,  63U },/* 73*/    {  23U,  64U },/* 74*/    {  23U,  65U },/* 75*/
    {  24U,  66U },/* 76*/    {  24U,  67U },/* 77*/    {  25U,  68U },/* 78*/
    {  25U,  69U },/* 79*/    {  25U,  69U },/* 80*/    {  25U,  70U },/* 81*/
    {  36U,  73U },/* 82*/    {  36U,  74U },/* 83*/    {  37U,  74U },/* 84*/
    {  37U,  75U },/* 85*/    {  38U,  76U },/* 86*/    {  38U,  77U },/* 87*/
    {  39U,  78U },/* 88*/    {  39U,  79U },/* 89*/    {  40U,  80U },/* 90*/
    {  40U,  81U },/* 91*/    {  41U,  82U },/* 92*/    {  41U,  83U },/* 93*/
    {  41U,  84U },/* 94*/    {  42U,  85U },/* 95*/    {  42U,  86U },/* 96*/
    {  43U,  87U },/* 97*/    {  43U,  88U },/* 98*/    {  32U,  88U },/* 99*/
    {  44U,  89U },/*100*/    {  45U,  90U },/*101*/    {  33U,  91U },/*102*/
    {  46U,  92U },/*103*/    {  46U,  93U },/*104*/    {  34U,  94U },/*105*/
    {  47U,  95U },/*106*/    {  47U,  96U },/*107*/    {  35U,  97U },/*108*/
    {  48U,  98U },/*109*/    {  49U,  99U },/*110*/    {  36U,  99U },/*111*/
    {  50U, 100U },/*112*/    {  50U, 101U },/*113*/    {  37U, 102U },/*114*/
    {  37U, 103U },/*115*/    {  52U, 104U },/*116*/    {  38U, 105U },/*117*/
    {  53U, 106U },/*118*/    {  53U, 107U },/*119*/    {  39U, 108U },/*120*/
    {  54U, 109U },/*121*/    {  54U, 110U },/*122*/    {  40U, 111U },/*123*/
    {  40U, 112U },/*124*/    {  56U, 113U },/*125*/    {  41U, 114U },/*126*/
    {  57U, 115U },/*127*/    {  57U, 116U },/*128*/    {  58U, 117U },/*129*/
    {  58U, 118U },/*130*/    {  58U, 118U },/*131*/    {  43U, 119U },/*132*/
    {  43U, 120U },/*133*/    {  43U, 121U },/*134*/    {  44U, 122U },/*135*/
    {  44U, 123U },/*136*/    {  61U, 124U },/*137*/    {  45U, 125U },/*138*/
    {  45U, 126U },/*139*/    {  63U, 127U },/*140*/    {  46U, 128U },/*141*/
    {  64U, 129U },/*142*/    {  64U, 130U },/*143*/    {  47U, 131U },/*144*/
    {  47U, 132U },/*145*/    {  66U, 133U },/*146*/    {  48U, 134U },/*147*/
    {  67U, 135U },/*148*/    {  67U, 135U },/*149*/    {  49U, 136U },/*150*/
    {  49U, 137U },/*151*/    {  68U, 138U },/*152*/    {  50U, 139U },/*153*/
    {  69U, 140U },/*154*/    {  70U, 141U },/*155*/    {  51U, 142U },/*156*/
    {  51U, 143U },/*157*/    {  71U, 144U },/*158*/    {  73U, 147U },/*159*/
    {  73U, 148U },/*160*/    {  73U, 148U },/*161*/    {  74U, 150U },/*162*/
    {  74U, 150U },/*163*/    {  75U, 151U },/*164*/    {  75U, 152U },/*165*/
    {  76U, 153U },/*166*/    {  76U, 154U },/*167*/    {  78U, 156U },/*168*/
    {  77U, 156U },/*169*/    {  78U, 157U },/*170*/    {  78U, 158U },/*171*/
    {  78U, 158U },/*172*/    {  79U, 160U },/*173*/    {  80U, 161U },/*174*/
    {  80U, 162U },/*175*/    {  81U, 163U },/*176*/    {  81U, 164U },/*177*/
    {  82U, 165U },/*178*/    {  82U, 166U },/*179*/    {  83U, 167U },/*180*/
    {  83U, 167U },/*181*/    {  83U, 168U },/*182*/    {  84U, 169U },/*183*/
    {  84U, 170U },/*184*/    {  85U, 171U },/*185*/    {  85U, 172U },/*186*/
    {  86U, 173U },/*187*/    {  86U, 174U },/*188*/    {  87U, 175U },/*189*/
    {  87U, 176U },/*190*/    {  88U, 177U },/*191*/    {  88U, 178U },/*192*/
    {  89U, 179U },/*193*/    {  89U, 180U },/*194*/    {  90U, 181U },/*195*/
    {  90U, 182U },/*196*/    {  90U, 181U },/*197*/    {  65U, 182U },/*198*/
    {  65U, 183U },/*199*/    {  91U, 184U },/*200*/    {  66U, 185U },/*201*/
    {  92U, 186U },/*202*/    {  93U, 187U },/*203*/    {  67U, 188U },/*204*/
    {  94U, 189U },/*205*/    {  94U, 190U },/*206*/    {  68U, 191U },/*207*/
    {  95U, 192U },/*208*/    {  96U, 193U },/*209*/    {  69U, 194U },/*210*/
    {  97U, 195U },/*211*/    {  97U, 196U },/*212*/    {  98U, 197U },/*213*/
    {  98U, 198U },/*214*/    {  99U, 199U },/*215*/    {  99U, 200U },/*216*/
    {  99U, 201U },/*217*/    { 100U, 202U },/*218*/    { 100U, 202U },/*219*/
    { 101U, 204U },/*220*/    { 101U, 204U },/*221*/    { 102U, 206U },/*222*/
    { 102U, 206U },/*223*/    { 103U, 207U },/*224*/    { 103U, 208U },/*225*/
    { 104U, 209U },/*226*/    { 104U, 210U },/*227*/    { 105U, 211U },/*228*/
    { 105U, 212U },/*229*/    { 106U, 213U },/*230*/    { 106U, 214U },/*231*/
    { 107U, 215U },/*232*/    { 107U, 216U },/*233*/    { 108U, 217U },/*234*/
    { 108U, 218U },/*235*/    { 109U, 220U },/*236*/    { 110U, 221U },/*237*/
    { 110U, 222U },/*238*/    { 111U, 223U },/*239*/    { 111U, 224U },/*240*/
    { 112U, 225U },/*241*/    { 112U, 226U },/*242*/    { 112U, 227U },/*243*/
    { 113U, 228U },/*244*/    { 114U, 229U },/*245*/    { 114U, 229U },/*246*/
    { 114U, 231U },/*247*/    { 115U, 231U },/*248*/    { 115U, 233U },/*249*/
    { 116U, 234U },/*250*/    { 116U, 234U },/*251*/    { 117U, 235U },/*252*/
    { 117U, 236U },/*253*/    { 118U, 237U },/*254*/    { 118U, 238U },/*255*/
    { 119U, 239U },/*256*/    { 119U, 239U },/*257*/    { 119U, 240U },/*258*/
#if 0
    { 120U, 241U },/*259*/    { 120U, 242U },/*260*/    { 121U, 243U },/*261*/
    { 121U, 244U },/*262*/    { 122U, 245U },/*263*/    { 122U, 246U },/*264*/
    { 123U, 247U },/*265*/    { 123U, 248U },/*266*/    { 123U, 249U },/*267*/
    { 124U, 250U },/*268*/    { 125U, 251U },/*269*/    { 125U, 252U },/*270*/
    { 126U, 253U },/*271*/    { 126U, 254U },/*272*/    { 126U, 255U },/*273*/
#endif
};

/* XXX obsolete source */
unsigned char stable_selection_breakpoint[SELECTION_BREAKPOINTS][SELECTION_TYPES] = {
/* DISTRIBUTED OTHERS nmemb DISTRIBUTED OTHERS nmemb DISTRIBUTED OTHERS nmemb */
    {   1U,   2U },/*  4*/    {   1U,   2U },/*  5*/    {   1U,   2U },/*  6*/
    {   1U,   2U },/*  7*/    {   1U,   3U },/*  8*/    {   1U,   3U },/*  9*/
    {   1U,   3U },/* 10*/    {   1U,   3U },/* 11*/    {   1U,   3U },/* 12*/
    {   2U,   5U },/* 13*/    {   2U,   5U },/* 14*/    {   2U,   7U },/* 15*/
    {   2U,   5U },/* 16*/    {   3U,   7U },/* 17*/    {   3U,   7U },/* 18*/
    {   3U,   9U },/* 19*/    {   3U,   7U },/* 20*/    {   4U,   9U },/* 21*/
    {   4U,   9U },/* 22*/    {   4U,  11U },/* 23*/    {   4U,   9U },/* 24*/
    {   4U,  11U },/* 25*/    {   4U,  11U },/* 26*/    {   4U,  13U },/* 27*/
    {   4U,  11U },/* 28*/    {   3U,  13U },/* 29*/    {   3U,  13U },/* 30*/
    {   4U,  15U },/* 31*/    {   4U,  13U },/* 32*/    {   4U,  15U },/* 33*/
    {   4U,  15U },/* 34*/    {   4U,  17U },/* 35*/    {   5U,  15U },/* 36*/
    {   5U,  29U },/* 37*/    {   5U,  20U },/* 38*/    {   5U,  19U },/* 39*/
    {   5U,  19U },/* 40*/    {   6U,  19U },/* 41*/    {   5U,  19U },/* 42*/
    {   6U,  21U },/* 43*/    {   6U,  19U },/* 44*/    {   6U,  21U },/* 45*/
    {   6U,  21U },/* 46*/    {   6U,  23U },/* 47*/    {   6U,  21U },/* 48*/
    {   6U,  23U },/* 49*/    {   6U,  23U },/* 50*/    {   6U,  25U },/* 51*/
    {   6U,  23U },/* 52*/    {   7U,  25U },/* 53*/    {   7U,  25U },/* 54*/
    {   7U,  27U },/* 55*/    {   7U,  25U },/* 56*/    {   7U,  27U },/* 57*/
    {   7U,  27U },/* 58*/    {   7U,  29U },/* 59*/    {   7U,  27U },/* 60*/
    {   7U,  29U },/* 61*/    {   7U,  29U },/* 62*/    {   8U,  31U },/* 63*/
    {   7U,  29U },/* 64*/    {   8U,  31U },/* 65*/    {   8U,  31U },/* 66*/
    {   8U,  33U },/* 67*/    {   8U,  31U },/* 68*/    {   9U,  33U },/* 69*/
    {   9U,  34U },/* 70*/    {   9U,  42U },/* 71*/    {   8U,  33U },/* 72*/
    {   8U,  35U },/* 73*/    {   9U,  35U },/* 74*/    {   9U,  37U },/* 75*/
    {   9U,  35U },/* 76*/    {   9U,  37U },/* 77*/    {  10U,  37U },/* 78*/
    {  10U,  39U },/* 79*/    {   9U,  37U },/* 80*/    {   9U,  39U },/* 81*/
    {  10U,  39U },/* 82*/    {  10U,  41U },/* 83*/    {  10U,  39U },/* 84*/
    {  10U,  41U },/* 85*/    {  11U,  41U },/* 86*/    {  11U,  43U },/* 87*/
    {  10U,  41U },/* 88*/    {  10U,  43U },/* 89*/    {  10U,  43U },/* 90*/
    {  10U,  45U },/* 91*/    {  10U,  43U },/* 92*/    {  11U,  45U },/* 93*/
    {  11U,  45U },/* 94*/    {  12U,  47U },/* 95*/    {  11U,  94U },/* 96*/
    {  11U,  47U },/* 97*/    {  11U,  47U },/* 98*/    {  11U,  49U },/* 99*/
    {  11U,  47U },/*100*/    {  11U,  49U },/*101*/    {  11U,  49U },/*102*/
    {  12U,  51U },/*103*/    {  12U,  49U },/*104*/    {  12U,  51U },/*105*/
    {  12U,  51U },/*106*/    {  12U,  53U },/*107*/    {  12U,  51U },/*108*/
    {  12U, 107U },/*109*/    {  12U,  53U },/*110*/    {  12U,  55U },/*111*/
    {  12U,  53U },/*112*/    {  13U,  55U },/*113*/    {  13U,  55U },/*114*/
    {  58U, 113U },/*115*/    {  13U,  55U },/*116*/    {  12U,  57U },/*117*/
    {  13U,  57U },/*118*/    {  13U,  59U },/*119*/    {  13U,  57U },/*120*/
    {  13U, 119U },/*121*/    {  14U,  59U },/*122*/    {  14U,  61U },/*123*/
    {  14U,  59U },/*124*/    {  11U,  61U },/*125*/    {  11U,  61U },/*126*/
    {  11U,  63U },/*127*/    {  12U,  61U },/*128*/    {  12U,  63U },/*129*/
    {  12U,  63U },/*130*/    {  12U,  65U },/*131*/    {  12U,  63U },/*132*/
    {  12U, 131U },/*133*/    {  12U,  65U },/*134*/    {  13U,  67U },/*135*/
    {  14U, 134U },/*136*/    {  13U,  67U },/*137*/    {  14U,  67U },/*138*/
    {  14U,  69U },/*139*/    {  13U,  67U },/*140*/    {  13U,  69U },/*141*/
    {  13U,  69U },/*142*/    {  13U,  71U },/*143*/    {  13U,  69U },/*144*/
    {  13U,  71U },/*145*/    {  14U,  71U },/*146*/    {  14U,  73U },/*147*/
    {  15U,  71U },/*148*/    {  15U,  73U },/*149*/    {  14U,  73U },/*150*/
    {  14U,  75U },/*151*/    {  14U, 149U },/*152*/    {  14U,  75U },/*153*/
    {  15U,  75U },/*154*/    {  15U,  77U },/*155*/    {  15U,  75U },/*156*/
    {  16U,  77U },/*157*/    {  16U, 156U },/*158*/    {  16U,  79U },/*159*/
    {  15U,  77U },/*160*/    {  15U,  79U },/*161*/    {  15U,  79U },/*162*/
    {  15U,  81U },/*163*/    {  16U,  79U },/*164*/    {  16U,  81U },/*165*/
    {  16U,  81U },/*166*/    {  17U,  83U },/*167*/    {  17U,  81U },/*168*/
    {  17U,  83U },/*169*/    {  16U,  83U },/*170*/    {  16U,  85U },/*171*/
    {  16U,  83U },/*172*/    {  16U,  85U },/*173*/    {  16U,  85U },/*174*/
    {  16U,  87U },/*175*/    {  17U,  85U },/*176*/    {  18U,  87U },/*177*/
    {  18U,  87U },/*178*/    {  18U,  89U },/*179*/    {  17U,  87U },/*180*/
    {  17U,  89U },/*181*/    {  17U,  89U },/*182*/    {  17U,  91U },/*183*/
    {  17U,  89U },/*184*/    {  17U,  91U },/*185*/    {  17U,  91U },/*186*/
    {  19U,  93U },/*187*/    {  19U,  91U },/*188*/    {  19U,  93U },/*189*/
    {  18U,  93U },/*190*/    {  18U,  95U },/*191*/    {  18U, 190U },/*192*/
    {  18U,  95U },/*193*/    {  18U,  95U },/*194*/    {  18U,  97U },/*195*/
    {  18U,  95U },/*196*/    {  18U,  97U },/*197*/    {  18U,  97U },/*198*/
    {  20U,  99U },/*199*/    {  19U,  97U },/*200*/    {  19U,  99U },/*201*/
    {  19U,  99U },/*202*/    {  19U, 101U },/*203*/    {  19U,  99U },/*204*/
    {  19U, 101U },/*205*/    {  19U, 101U },/*206*/    {  19U, 103U },/*207*/
    {  19U, 101U },/*208*/    {  19U, 103U },/*209*/    {  19U, 103U },/*210*/
    {  19U, 105U },/*211*/    {  19U, 103U },/*212*/    {  20U, 105U },/*213*/
    {  20U, 159U },/*214*/    {  20U, 132U },/*215*/    {  20U, 105U },/*216*/
    {  20U, 107U },/*217*/    {  20U, 107U },/*218*/    {  20U, 109U },/*219*/
    {  19U, 218U },/*220*/    {  19U, 219U },/*221*/    {  20U, 109U },/*222*/
    {  20U, 111U },/*223*/    {  20U, 109U },/*224*/    {  20U, 111U },/*225*/
    {  21U, 111U },/*226*/    {  21U, 113U },/*227*/    {  21U, 226U },/*228*/
    {  21U, 113U },/*229*/    {  21U, 113U },/*230*/    {  20U, 115U },/*231*/
    {  20U, 113U },/*232*/    {  20U, 115U },/*233*/    {  20U, 175U },/*234*/
    {  20U, 117U },/*235*/    {  20U, 115U },/*236*/    {  20U, 117U },/*237*/
    {  22U, 117U },/*238*/    {  22U, 119U },/*239*/    {  22U, 117U },/*240*/
    {  22U, 119U },/*241*/    {  21U, 119U },/*242*/    {  21U, 121U },/*243*/
    {  21U, 119U },/*244*/    {  21U, 121U },/*245*/    {  21U, 121U },/*246*/
    {  21U, 123U },/*247*/    {  21U, 121U },/*248*/    {  21U, 123U },/*249*/
    {  21U, 123U },/*250*/    {  21U, 125U },/*251*/    {  23U, 123U },/*252*/
    {  22U, 125U },/*253*/    {  22U, 125U },/*254*/    {  22U, 127U },/*255*/
    {  22U, 125U },/*256*/    {  22U, 127U },/*257*/    {  22U, 127U },/*258*/
#if 0

    {  22U, 257U },/*259*/
#endif
/* XXX obsolete source */
};
