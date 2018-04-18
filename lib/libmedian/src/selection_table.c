/*INDENT OFF*/

/* Description: C source code for quickselect tables */
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
unsigned char selection_breakpoint[SELECTION_BREAKPOINTS][SELECTION_TYPES] = {
/* DISTRIBUTED OTHERS nmemb DISTRIBUTED OTHERS nmemb DISTRIBUTED OTHERS nmemb */
    {   1U,   2U },/*  4*/    {   1U,   2U },/*  5*/    {   1U,   3U },/*  6*/
    {   2U,   5U },/*  7*/    {   2U,   5U },/*  8*/    {   3U,   5U },/*  9*/
    {   3U,   7U },/* 10*/    {   4U,   8U },/* 11*/    {   4U,   9U },/* 12*/
    {   5U,  10U },/* 13*/    {   5U,  10U },/* 14*/    {   5U,  11U },/* 15*/
    {   6U,  12U },/* 16*/    {   6U,  13U },/* 17*/    {   7U,  14U },/* 18*/
    {   7U,  15U },/* 19*/    {   8U,  15U },/* 20*/    {   8U,  16U },/* 21*/
    {   8U,  17U },/* 22*/    {   9U,  18U },/* 23*/    {   9U,  19U },/* 24*/
    {   5U,  17U },/* 25*/    {   7U,  17U },/* 26*/    {   7U,  19U },/* 27*/
    {   6U,  19U },/* 28*/    {   8U,  21U },/* 29*/    {   8U,  21U },/* 30*/
    {   8U,  23U },/* 31*/    {   9U,  23U },/* 32*/    {   9U,  24U },/* 33*/
    {   9U,  25U },/* 34*/    {  10U,  26U },/* 35*/    {  10U,  26U },/* 36*/
    {  10U,  27U },/* 37*/    {  11U,  28U },/* 38*/    {  11U,  29U },/* 39*/
    {  11U,  30U },/* 40*/    {  11U,  31U },/* 41*/    {  12U,  31U },/* 42*/
    {  12U,  32U },/* 43*/    {  12U,  33U },/* 44*/    {  12U,  34U },/* 45*/
    {  13U,  35U },/* 46*/    {  13U,  36U },/* 47*/    {  11U,  36U },/* 48*/
    {  14U,  37U },/* 49*/    {  14U,  38U },/* 50*/    {  14U,  39U },/* 51*/
    {  12U,  40U },/* 52*/    {  15U,  41U },/* 53*/    {  15U,  41U },/* 54*/
    {  15U,  42U },/* 55*/    {  13U,  43U },/* 56*/    {  13U,  44U },/* 57*/
    {  16U,  45U },/* 58*/    {  17U,  46U },/* 59*/    {  14U,  46U },/* 60*/
    {  14U,  48U },/* 61*/    {  17U,  48U },/* 62*/    {  18U,  49U },/* 63*/
    {  15U,  50U },/* 64*/    {  15U,  51U },/* 65*/    {  15U,  52U },/* 66*/
    {  19U,  53U },/* 67*/    {  16U,  53U },/* 68*/    {  16U,  54U },/* 69*/
    {  16U,  55U },/* 70*/    {  20U,  56U },/* 71*/    {  17U,  57U },/* 72*/
    {  17U,  58U },/* 73*/    {  17U,  59U },/* 74*/    {  21U,  60U },/* 75*/
    {  18U,  61U },/* 76*/    {  18U,  61U },/* 77*/    {  18U,  62U },/* 78*/
    {  22U,  63U },/* 79*/    {  19U,  64U },/* 80*/    {  19U,  65U },/* 81*/
    {  19U,  66U },/* 82*/    {  24U,  67U },/* 83*/    {  20U,  68U },/* 84*/
    {  20U,  68U },/* 85*/    {  20U,  69U },/* 86*/    {  25U,  70U },/* 87*/
    {  21U,  71U },/* 88*/    {  21U,  72U },/* 89*/    {  21U,  73U },/* 90*/
    {  21U,  73U },/* 91*/    {  22U,  74U },/* 92*/    {  22U,  75U },/* 93*/
    {  22U,  76U },/* 94*/    {  22U,  77U },/* 95*/    {  23U,  78U },/* 96*/
    {  23U,  79U },/* 97*/    {  23U,  79U },/* 98*/    {  23U,  80U },/* 99*/
    {  23U,  81U },/*100*/    {  23U,  82U },/*101*/    {  24U,  83U },/*102*/
    {  24U,  84U },/*103*/    {  24U,  85U },/*104*/    {  24U,  85U },/*105*/
    {  25U,  86U },/*106*/    {  25U,  87U },/*107*/    {  25U,  88U },/*108*/
    {  26U,  89U },/*109*/    {  26U,  90U },/*110*/    {  26U,  91U },/*111*/
    {  26U,  92U },/*112*/    {  26U,  92U },/*113*/    {  27U,  93U },/*114*/
    {  22U,  90U },/*115*/    {  22U,  91U },/*116*/    {  22U,  92U },/*117*/
    {  26U,  93U },/*118*/    {  26U,  94U },/*119*/    {  23U,  95U },/*120*/
    {  23U,  96U },/*121*/    {  23U,  97U },/*122*/    {  27U,  98U },/*123*/
    {  27U,  99U },/*124*/    {  24U, 100U },/*125*/    {  24U, 100U },/*126*/
    {  28U, 101U },/*127*/    {  28U, 102U },/*128*/    {  29U, 103U },/*129*/
    {  25U, 104U },/*130*/    {  25U, 105U },/*131*/    {  29U, 106U },/*132*/
    {  30U, 107U },/*133*/    {  30U, 108U },/*134*/    {  26U, 109U },/*135*/
    {  30U, 110U },/*136*/    {  31U, 110U },/*137*/    {  31U, 111U },/*138*/
    {  31U, 112U },/*139*/    {  27U, 113U },/*140*/    {  32U, 114U },/*141*/
    {  32U, 115U },/*142*/    {  32U, 116U },/*143*/    {  32U, 117U },/*144*/
    {  28U, 118U },/*145*/    {  33U, 118U },/*146*/    {  33U, 120U },/*147*/
    {  33U, 120U },/*148*/    {  33U, 121U },/*149*/    {  29U, 122U },/*150*/
    {  34U, 123U },/*151*/    {  34U, 124U },/*152*/    {  35U, 125U },/*153*/
    {  35U, 126U },/*154*/    {  35U, 126U },/*155*/    {  35U, 128U },/*156*/
    {  36U, 129U },/*157*/    {  36U, 129U },/*158*/    {  36U, 130U },/*159*/
    {  36U, 131U },/*160*/    {  36U, 132U },/*161*/    {  37U, 133U },/*162*/
    {  37U, 134U },/*163*/    {  37U, 135U },/*164*/    {  37U, 136U },/*165*/
    {  38U, 136U },/*166*/    {  38U, 137U },/*167*/    {  38U, 138U },/*168*/
    {  38U, 139U },/*169*/    {  39U, 140U },/*170*/    {  39U, 141U },/*171*/
    {  39U, 142U },/*172*/    {  39U, 143U },/*173*/    {  39U, 143U },/*174*/
    {  40U, 144U },/*175*/    {  40U, 145U },/*176*/    {  40U, 146U },/*177*/
    {  40U, 147U },/*178*/    {  41U, 148U },/*179*/    {  41U, 149U },/*180*/
    {  41U, 150U },/*181*/    {  41U, 150U },/*182*/    {  41U, 151U },/*183*/
    {  42U, 152U },/*184*/    {  42U, 153U },/*185*/    {  42U, 154U },/*186*/
    {  42U, 155U },/*187*/    {  43U, 155U },/*188*/    {  43U, 157U },/*189*/
    {  37U, 157U },/*190*/    {  43U, 158U },/*191*/    {  44U, 159U },/*192*/
    {  44U, 160U },/*193*/    {  44U, 161U },/*194*/    {  38U, 162U },/*195*/
    {  45U, 163U },/*196*/    {  45U, 163U },/*197*/    {  45U, 164U },/*198*/
    {  45U, 165U },/*199*/    {  39U, 166U },/*200*/    {  46U, 167U },/*201*/
    {  39U, 168U },/*202*/    {  46U, 169U },/*203*/    {  46U, 169U },/*204*/
    {  40U, 170U },/*205*/    {  40U, 171U },/*206*/    {  47U, 172U },/*207*/
    {  47U, 173U },/*208*/    {  47U, 174U },/*209*/    {  41U, 175U },/*210*/
    {  41U, 175U },/*211*/    {  48U, 177U },/*212*/    {  48U, 177U },/*213*/
    {  49U, 178U },/*214*/    {  42U, 179U },/*215*/    {  42U, 180U },/*216*/
    {  42U, 181U },/*217*/    {  50U, 182U },/*218*/    {  50U, 182U },/*219*/
    {  43U, 183U },/*220*/    {  43U, 184U },/*221*/    {  43U, 185U },/*222*/
    {  43U, 186U },/*223*/    {  51U, 187U },/*224*/    {  44U, 188U },/*225*/
    {  44U, 188U },/*226*/    {  44U, 189U },/*227*/    {  44U, 190U },/*228*/
    {  44U, 191U },/*229*/    {  45U, 192U },/*230*/    {  45U, 193U },/*231*/
    {  45U, 194U },/*232*/    {  45U, 195U },/*233*/    {  45U, 195U },/*234*/
    {  46U, 196U },/*235*/    {  46U, 197U },/*236*/    {  46U, 198U },/*237*/
    {  46U, 199U },/*238*/    {  46U, 200U },/*239*/    {  47U, 201U },/*240*/
    {  47U, 201U },/*241*/    {  47U, 203U },/*242*/    {  47U, 203U },/*243*/
    {  47U, 205U },/*244*/    {  48U, 205U },/*245*/    {  48U, 206U },/*246*/
    {  48U, 207U },/*247*/    {  48U, 208U },/*248*/    {  48U, 209U },/*249*/
    {  48U, 210U },/*250*/    {  49U, 210U },/*251*/    {  49U, 211U },/*252*/
    {  49U, 212U },/*253*/    {  49U, 213U },/*254*/    {  50U, 214U },/*255*/
    {  50U, 215U },/*256*/    {  50U, 216U },/*257*/    {  50U, 217U },/*258*/
    {  50U, 218U },/*259*/    {  50U, 218U },/*260*/    {  51U, 219U },/*261*/
    {  51U, 220U },/*262*/    {  51U, 221U },/*263*/    {  51U, 222U },/*264*/
    {  51U, 223U },/*265*/    {  52U, 224U },/*266*/    {  52U, 225U },/*267*/
    {  52U, 226U },/*268*/    {  52U, 227U },/*269*/    {  53U, 227U },/*270*/
    {  53U, 228U },/*271*/    {  53U, 229U },/*272*/    {  53U, 230U },/*273*/
    {  53U, 231U },/*274*/    {  54U, 232U },/*275*/    {  54U, 233U },/*276*/
    {  54U, 234U },/*277*/    {  54U, 235U },/*278*/    {  54U, 235U },/*279*/
    {  54U, 236U },/*280*/    {  54U, 237U },/*281*/    {  55U, 238U },/*282*/
    {  55U, 239U },/*283*/    {  55U, 240U },/*284*/    {  56U, 241U },/*285*/
    {  56U, 242U },/*286*/    {  56U, 243U },/*287*/    {  56U, 244U },/*288*/
    {  56U, 245U },/*289*/    {  56U, 245U },/*290*/    {  57U, 246U },/*291*/
    {  57U, 247U },/*292*/    {  57U, 248U },/*293*/    {  57U, 249U },/*294*/
    {  57U, 250U },/*295*/    {  58U, 251U },/*296*/    {  58U, 252U },/*297*/
    {  58U, 253U },/*298*/    {  58U, 254U },/*299*/    {  58U, 254U },/*300*/
    {  58U, 255U },/*301*/
};

unsigned char stable_selection_breakpoint[SELECTION_BREAKPOINTS][SELECTION_TYPES] = {
    {   1U,   2U },/*  4*/    {   1U,   2U },/*  5*/    {   1U,   2U },/*  6*/
    {   1U,   2U },/*  7*/    {   1U,   3U },/*  8*/    {   1U,   4U },/*  9*/
    {   1U,   4U },/* 10*/    {   2U,   5U },/* 11*/    {   2U,   6U },/* 12*/
    {   2U,   7U },/* 13*/    {   3U,   8U },/* 14*/    {   3U,   8U },/* 15*/
    {   3U,   9U },/* 16*/    {   3U,  10U },/* 17*/    {   4U,  11U },/* 18*/
    {   4U,  11U },/* 19*/    {   4U,  12U },/* 20*/    {   4U,  13U },/* 21*/
    {   5U,  14U },/* 22*/    {   6U,  15U },/* 23*/    {   5U,  15U },/* 24*/
    {   4U,  16U },/* 25*/    {   4U,  17U },/* 26*/    {   4U,  18U },/* 27*/
    {   5U,  19U },/* 28*/    {   5U,  19U },/* 29*/    {   5U,  20U },/* 30*/
    {   5U,  20U },/* 31*/    {   5U,  21U },/* 32*/    {   5U,  21U },/* 33*/
    {   5U,  22U },/* 34*/    {   6U,  23U },/* 35*/    {   5U,  23U },/* 36*/
    {   6U,  24U },/* 37*/    {   6U,  25U },/* 38*/    {   6U,  25U },/* 39*/
    {   6U,  26U },/* 40*/    {   7U,  27U },/* 41*/    {   6U,  28U },/* 42*/
    {   7U,  29U },/* 43*/    {   7U,  29U },/* 44*/    {   7U,  30U },/* 45*/
    {   8U,  31U },/* 46*/    {   8U,  32U },/* 47*/    {   7U,  32U },/* 48*/
    {   7U,  33U },/* 49*/    {   8U,  34U },/* 50*/    {   8U,  35U },/* 51*/
    {   9U,  36U },/* 52*/    {   9U,  36U },/* 53*/    {   8U,  37U },/* 54*/
    {   8U,  38U },/* 55*/    {   9U,  39U },/* 56*/    {   9U,  39U },/* 57*/
    {  10U,  40U },/* 58*/    {  10U,  41U },/* 59*/    {   9U,  41U },/* 60*/
    {   9U,  42U },/* 61*/    {   9U,  42U },/* 62*/    {   9U,  43U },/* 63*/
    {   9U,  44U },/* 64*/    {   9U,  44U },/* 65*/    {  10U,  45U },/* 66*/
    {  10U,  46U },/* 67*/    {  10U,  47U },/* 68*/    {  10U,  47U },/* 69*/
    {  10U,  48U },/* 70*/    {  10U,  48U },/* 71*/    {  10U,  49U },/* 72*/
    {  10U,  50U },/* 73*/    {  10U,  50U },/* 74*/    {  11U,  51U },/* 75*/
    {  11U,  52U },/* 76*/    {  10U,  53U },/* 77*/    {  10U,  53U },/* 78*/
    {  10U,  54U },/* 79*/    {  11U,  55U },/* 80*/    {  11U,  55U },/* 81*/
    {  12U,  56U },/* 82*/    {  12U,  57U },/* 83*/    {  11U,  58U },/* 84*/
    {  11U,  59U },/* 85*/    {  12U,  60U },/* 86*/    {  13U,  60U },/* 87*/
    {  13U,  61U },/* 88*/    {  13U,  62U },/* 89*/    {  13U,  63U },/* 90*/
    {  12U,  64U },/* 91*/    {  12U,  65U },/* 92*/    {  13U,  65U },/* 93*/
    {  14U,  66U },/* 94*/    {  14U,  67U },/* 95*/    {  14U,  68U },/* 96*/
    {  14U,  69U },/* 97*/    {  13U,  69U },/* 98*/    {  14U,  70U },/* 99*/
    {  15U,  71U },/*100*/    {  15U,  72U },/*101*/    {  15U,  73U },/*102*/
    {  15U,  73U },/*103*/    {  15U,  74U },/*104*/    {  14U,  75U },/*105*/
    {  14U,  76U },/*106*/    {  16U,  77U },/*107*/    {  16U,  77U },/*108*/
    {  16U,  78U },/*109*/    {  16U,  79U },/*110*/    {  16U,  80U },/*111*/
    {  15U,  81U },/*112*/    {  15U,  81U },/*113*/    {  15U,  82U },/*114*/
    {  13U,  83U },/*115*/    {  13U,  84U },/*116*/    {  13U,  84U },/*117*/
    {  13U,  85U },/*118*/    {  15U,  86U },/*119*/    {  14U,  87U },/*120*/
    {  14U,  87U },/*121*/    {  14U,  88U },/*122*/    {  14U,  89U },/*123*/
    {  14U,  89U },/*124*/    {  14U,  90U },/*125*/    {  14U,  91U },/*126*/
    {  14U,  91U },/*127*/    {  14U,  92U },/*128*/    {  14U,  93U },/*129*/
    {  15U,  93U },/*130*/    {  15U,  94U },/*131*/    {  15U,  95U },/*132*/
    {  15U,  96U },/*133*/    {  15U,  97U },/*134*/    {  15U,  98U },/*135*/
    {  15U,  98U },/*136*/    {  15U,  99U },/*137*/    {  16U, 100U },/*138*/
    {  16U, 101U },/*139*/    {  16U, 102U },/*140*/    {  16U, 102U },/*141*/
    {  16U, 102U },/*142*/    {  16U, 103U },/*143*/    {  15U, 103U },/*144*/
    {  15U, 104U },/*145*/    {  16U, 105U },/*146*/    {  16U, 106U },/*147*/
    {  16U, 107U },/*148*/    {  17U, 107U },/*149*/    {  17U, 108U },/*150*/
    {  17U, 109U },/*151*/    {  17U, 109U },/*152*/    {  16U, 110U },/*153*/
    {  16U, 111U },/*154*/    {  16U, 112U },/*155*/    {  16U, 112U },/*156*/
    {  18U, 113U },/*157*/    {  18U, 114U },/*158*/    {  18U, 115U },/*159*/
    {  18U, 115U },/*160*/    {  18U, 116U },/*161*/    {  17U, 117U },/*162*/
    {  17U, 118U },/*163*/    {  17U, 119U },/*164*/    {  19U, 119U },/*165*/
    {  19U, 120U },/*166*/    {  19U, 121U },/*167*/    {  19U, 122U },/*168*/
    {  19U, 123U },/*169*/    {  19U, 124U },/*170*/    {  18U, 125U },/*171*/
    {  19U, 126U },/*172*/    {  20U, 126U },/*173*/    {  20U, 127U },/*174*/
    {  20U, 128U },/*175*/    {  20U, 129U },/*176*/    {  20U, 130U },/*177*/
    {  20U, 130U },/*178*/    {  20U, 131U },/*179*/    {  20U, 132U },/*180*/
    {  21U, 133U },/*181*/    {  21U, 134U },/*182*/    {  21U, 134U },/*183*/
    {  21U, 135U },/*184*/    {  21U, 136U },/*185*/    {  21U, 137U },/*186*/
    {  21U, 138U },/*187*/    {  21U, 138U },/*188*/    {  22U, 139U },/*189*/
    {  22U, 140U },/*190*/    {  22U, 140U },/*191*/    {  22U, 141U },/*192*/
    {  22U, 142U },/*193*/    {  22U, 143U },/*194*/    {  22U, 144U },/*195*/
    {  22U, 145U },/*196*/    {  22U, 145U },/*197*/    {  21U, 146U },/*198*/
    {  23U, 147U },/*199*/    {  23U, 147U },/*200*/    {  23U, 149U },/*201*/
    {  23U, 149U },/*202*/    {  23U, 150U },/*203*/    {  23U, 151U },/*204*/
    {  23U, 152U },/*205*/    {  23U, 153U },/*206*/    {  22U, 153U },/*207*/
    {  22U, 153U },/*208*/    {  24U, 155U },/*209*/    {  24U, 155U },/*210*/
    {  24U, 157U },/*211*/    {  24U, 157U },/*212*/    {  24U, 158U },/*213*/
    {  24U, 159U },/*214*/    {  25U, 160U },/*215*/    {  23U, 159U },/*216*/
    {  23U, 161U },/*217*/    {  23U, 161U },/*218*/    {  23U, 163U },/*219*/
    {  25U, 163U },/*220*/    {  25U, 164U },/*221*/    {  25U, 165U },/*222*/
    {  25U, 166U },/*223*/    {  25U, 165U },/*224*/    {  24U, 167U },/*225*/
    {  24U, 167U },/*226*/    {  24U, 169U },/*227*/    {  24U, 169U },/*228*/
    {  24U, 170U },/*229*/    {  24U, 171U },/*230*/    {  26U, 172U },/*231*/
    {  26U, 171U },/*232*/    {  26U, 173U },/*233*/    {  25U, 173U },/*234*/
    {  25U, 175U },/*235*/    {  25U, 175U },/*236*/    {  25U, 176U },/*237*/
    {  25U, 177U },/*238*/    {  25U, 178U },/*239*/    {  25U, 177U },/*240*/
    {  25U, 179U },/*241*/    {  27U, 179U },/*242*/    {  26U, 181U },/*243*/
    {  26U, 181U },/*244*/    {  26U, 182U },/*245*/    {  26U, 183U },/*246*/
    {  26U, 183U },/*247*/    {  26U, 183U },/*248*/    {  26U, 185U },/*249*/
    {  26U, 185U },/*250*/    {  26U, 186U },/*251*/    {  26U, 187U },/*252*/
    {  26U, 187U },/*253*/    {  26U, 188U },/*254*/    {  26U, 188U },/*255*/
    {  26U, 189U },/*256*/    {  27U, 190U },/*257*/    {  27U, 191U },/*258*/
    {  27U, 192U },/*259*/    {  27U, 193U },/*260*/    {  27U, 193U },/*261*/
    {  27U, 194U },/*262*/    {  27U, 195U },/*263*/    {  27U, 195U },/*264*/
    {  27U, 196U },/*265*/    {  28U, 197U },/*266*/    {  28U, 198U },/*267*/
    {  28U, 199U },/*268*/    {  28U, 200U },/*269*/    {  28U, 200U },/*270*/
    {  28U, 201U },/*271*/    {  28U, 201U },/*272*/    {  28U, 203U },/*273*/
    {  28U, 203U },/*274*/    {  28U, 204U },/*275*/    {  29U, 205U },/*276*/
    {  29U, 206U },/*277*/    {  29U, 207U },/*278*/    {  29U, 207U },/*279*/
    {  29U, 207U },/*280*/    {  29U, 208U },/*281*/    {  29U, 208U },/*282*/
    {  27U, 209U },/*283*/    {  27U, 210U },/*284*/    {  27U, 210U },/*285*/
    {  29U, 211U },/*286*/    {  29U, 211U },/*287*/    {  29U, 212U },/*288*/
    {  29U, 213U },/*289*/    {  28U, 214U },/*290*/    {  28U, 215U },/*291*/
    {  28U, 215U },/*292*/    {  28U, 216U },/*293*/    {  28U, 217U },/*294*/
    {  30U, 218U },/*295*/    {  30U, 218U },/*296*/    {  30U, 219U },/*297*/
    {  30U, 220U },/*298*/    {  30U, 221U },/*299*/    {  29U, 221U },/*300*/
    {  29U, 223U },/*301*/
#if 0
                              {  29U, 223U },/*302*/    {  29U, 224U },/*303*/
    {  29U, 224U },/*304*/    {  29U, 225U },/*305*/    {  29U, 226U },/*306*/
    {  31U, 227U },/*307*/    {  31U, 228U },/*308*/    {  31U, 228U },/*309*/
    {  30U, 229U },/*310*/    {  30U, 230U },/*311*/    {  30U, 231U },/*312*/
    {  30U, 231U },/*313*/    {  30U, 232U },/*314*/    {  30U, 233U },/*315*/
    {  30U, 234U },/*316*/    {  30U, 234U },/*317*/    {  30U, 235U },/*318*/
    {  32U, 236U },/*319*/    {  31U, 236U },/*320*/    {  31U, 237U },/*321*/
    {  31U, 238U },/*322*/    {  31U, 239U },/*323*/    {  31U, 240U },/*324*/
    {  31U, 241U },/*325*/    {  31U, 242U },/*326*/    {  31U, 243U },/*327*/
    {  33U, 243U },/*328*/    {  34U, 245U },/*329*/    {  32U, 245U },/*330*/
    {  32U, 246U },/*331*/    {  32U, 247U },/*332*/    {  32U, 248U },/*333*/
    {  32U, 249U },/*334*/    {  32U, 250U },/*335*/    {  34U, 249U },/*336*/
    {  35U, 251U },/*337*/    {  35U, 251U },/*338*/    {  35U, 253U },/*339*/
    {  33U, 253U },/*340*/    {  33U, 254U },/*341*/    {  33U, 255U },/*342*/
#endif
};
