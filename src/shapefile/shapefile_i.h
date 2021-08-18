/******************************************************************************
 * shapefile_i.h
 *
 * v 1.6 2001/05/23 13:36:52 warmerda
 *
 * Project:  Shapelib
 * Purpose:  Primary include file for Shapelib.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ** Last modified: cheungmine
 *
 * Copyright (c) 1999, Frank Warmerdam
 *
 * This software is available under the following "MIT Style" license,
 * or at the option of the licensee under the LGPL (see LICENSE.LGPL).  This
 * option is discussed in more detail in shapelib.html.
 *
 * --
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/
#ifndef _SHAPEFILE_I_H_
#define _SHAPEFILE_I_H_

#ifdef _MSC_VER
# pragma warning (disable : 4996)
#endif

#ifdef    __cplusplus
extern "C" {
#endif

static const char LIBNAME[] = "shapefile";
static const char LIBVERSION[] = "0.0.1";


#include <common/mscrtdbg.h>
#include <common/cstrbuf.h>
#include <common/memapi.h>
#include <common/emerglog.h>

#define RTREE_DIMS  2
#include <common/rtree.h>

#include <assert.h>
#include <limits.h>
#include <math.h>

#include "shapefile_api.h"
#include "shp2wkb.h"
#include "shp2wkt.h"


#ifdef _MSC_VER
 /* disable annoying warning: 4996 */
 # pragma warning (disable : 4996)
#endif

#define ByteCopy(a, b, c)  memcpy((b), (a), (c))

#ifndef MAX_V2
 #define MAX_V2(a, b)  ((a)>(b) ? (a) : (b))
#endif

#ifndef MIN_V2
 #define MIN_V2(a, b)  ((a)<(b) ? (a) : (b))
#endif

#ifndef SGNOF
 #define SGNOF(a)  ((a) > 0 ? 1 : ((a) < 0 ? (-1) : 0))
#endif

#define  MEM_BLKSIZE  128


typedef struct  _SHPInfo
{
    FILE        *fpSHP;
    FILE        *fpSHX;

    /* r-tree for RectXY */
    rtree_root  hEnvTree;

    int         nShapeType; /* SHPT_* */
    int         nFileSize;  /* SHP file */
    int         nRecords;
    int         nMaxRecords;
    int         *panRecOffset;
    int         *panRecSize;

    double      adBoundsMin[4];
    double      adBoundsMax[4];

    int         bUpdated;
    unsigned char *pabyRec;
    int         nBufSize;
} SHPInfo;


typedef struct _DBFInfo
{
    FILE        *fp;

    int         nRecords;

    int         nRecordLength;
    int         nHeaderLength;
    int         nFields;
    int         *panFieldOffset;
    int         *panFieldSize;
    int         *panFieldDecimals;
    char        *pachFieldType;

    char        *pszHeader;

    int         nCurrentRecord;
    int         bCurrentRecordModified;
    char        *pszCurrentRecord;

    int         bNoHeader;
    int         bUpdated;

    double      dDoubleField;
    char        szStringField[257];    /* max is 256 chars */
    char        *pReturnTuple;
    int         nTupleLen;
} DBFInfo;


typedef struct _SHPRectTree
{
    rtree_root tree;
} SHPRectTree_t;


/**
 * Swap double
 */
STATIC_INLINE void SwapDouble(double *a, double *b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

/**
 * Swap SHPPointType
 */
STATIC_INLINE void SwapPointType(SHPPointType *p, SHPPointType *q)
{
    double t = p->x;
    p->x = q->x;
    q->x = t;

    t = p->y;
    p->y = q->y;
    q->y = t;
}


/**
 * A realloc cover function that will access a 0 pointer as a valid input
 */
STATIC_INLINE void* SfRealloc (void * pMem, int nNewSize)
{
    void *p;
    if (pMem == 0) {
        p = malloc(nNewSize);
        if (!p) {
            exit(EXIT_OUTMEMORY);
        }
        return p;
    } else {
        p = realloc (pMem, nNewSize);
        if (!p) {
            exit(EXIT_OUTMEMORY);
        }
        return p;
    }
}


STATIC_INLINE int SHPTypeHasParts(int nSHPType)
{
    return (nSHPType == SHPT_POLYGON ||
        nSHPType == SHPT_ARC ||
        nSHPType == SHPT_POLYGONZ ||
        nSHPType == SHPT_POLYGONM ||
        nSHPType == SHPT_ARCZ ||
        nSHPType == SHPT_ARCM ||
        nSHPType == SHPT_MULTIPATCH);
}


STATIC_INLINE void StringToUpper (char *str, int len)
{
    char *p = str;
    while (len-- > 0 && *p) {
       *p = toupper(*p);
        p++;
    }
}

#ifdef    __cplusplus
}
#endif

#endif /* _SHAPEFILE_I_H_ */