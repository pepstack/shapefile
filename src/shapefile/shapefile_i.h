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
static const char LIBVERSION[] = "1.2.1";

#include <common/unitypes.h>
#include <common/bo.h>

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

// copy memory from const a to b
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

typedef struct _SHPInfoRTree
{
    RTREE_ROOT   rtRoot;
} SHPInfoRTree;


typedef struct  _SHPInfo
{
    FILE        *fpSHP;
    FILE        *fpSHX;

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

    /* RTree */
    SHPInfoRTree MBRTree;
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
    RTREE_ROOT tree;
} SHPRectTree_t;

void * SfRealloc (void * pMem, int nNewSize);

#ifdef    __cplusplus
}
#endif

#endif /* _SHAPEFILE_I_H_ */
