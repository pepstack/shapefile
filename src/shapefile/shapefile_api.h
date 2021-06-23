/******************************************************************************
 * shapefile.h
 *
 * v 1.4 2005/03/24 14:26:44
 *
 * Last updated by cheungmine on 2021-06-10.
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
#ifndef _SHAPEFILE_H_INCLUDED
#define _SHAPEFILE_H_INCLUDED

/**
 * Compilation on Linux:
 * $ gcc -O2 -shared -fPIC dbfopen.c shpopen.c shptree.c -o libshape.so
 */
#ifdef __cplusplus
extern "C" {
#endif

#if defined(SHAPEFILE_DLL)
/* win32 dynamic dll */
# ifdef SHAPEFILE_EXPORTS
#   define SHAPEFILE_API __declspec(dllexport)
# else
#   define SHAPEFILE_API __declspec(dllimport)
# endif
#else
/* static lib or linux so */
# define SHAPEFILE_API  extern
#endif


#include "shapefile_def.h"

#define SHAPEFILE_RECORDS_MAX   256000000

#ifndef SHAPEFILE_ASSERT
        #define SHAPEFILE_ASSERT(expr)
#endif


#ifndef TRUE
        #define TRUE 1
        #define FALSE 0
#endif

#ifndef EXIT_OUTMEMORY
        #define EXIT_OUTMEMORY (-4)
#endif

/* -------------------------------------------------------------------- */
/*      Should the DBFReadStringAttribute() strip leading and           */
/*      trailing white space?                                           */
/* -------------------------------------------------------------------- */
#define TRIM_DBF_WHITESPACE

/* -------------------------------------------------------------------- */
/*      Should we write measure values to the Multipatch object?        */
/*      Reportedly ArcView crashes if we do write it, so for now it     */
/*      is disabled.                                                    */
/* -------------------------------------------------------------------- */
#define DISABLE_MULTIPATCH_MEASURE

/************************************************************************/
/*                             SHP Support.                             */
/************************************************************************/
typedef struct _SHPInfo*  SHPHandle;

#define SHAPE_TYPE_MASK     7    /* 111 = Type mask */

#define SHAPE_TYPE_NIL      0    /* Null  type */
#define SHAPE_TYPE_POINT    1    /* Point type */
#define SHAPE_TYPE_LINE     2    /* Line type */
#define SHAPE_TYPE_POLYGON  4    /* Polygon type */

/* -------------------------------------------------------------------- */
/*      Shape types (nSHPType)                                          */
/* -------------------------------------------------------------------- */
#define SHPT_NULL           0

/* 2D Shape Types (pre ArcView 3.x): */
#define SHPT_POINT          1	/* Points */
#define SHPT_ARC            3	/* Arcs (Polylines, possible in parts) */
#define SHPT_POLYGON        5	/* Polygons (possible in parts) */
#define SHPT_MULTIPOINT     8	/* MultiPoint (related points) */

/* 3D Shape Types (may include "measure" values for vertices): */
#define SHPT_POINTZ        11
#define SHPT_ARCZ          13
#define SHPT_POLYGONZ      15
#define SHPT_MULTIPOINTZ   18

/* 2D + Measure Types: */
#define SHPT_POINTM        21
#define SHPT_ARCM          23
#define SHPT_POLYGONM      25
#define SHPT_MULTIPOINTM   28

/* Complex (TIN-like) with Z, and Measure: */
#define SHPT_MULTIPATCH    31


/* -------------------------------------------------------------------- */
/*      Part types - everything but SHPT_MULTIPATCH just uses           */
/*      SHPP_RING.                                                      */
/* -------------------------------------------------------------------- */
#define SHPP_TRISTRIP       0
#define SHPP_TRIFAN         1
#define SHPP_OUTERRING      2
#define SHPP_INNERRING      3
#define SHPP_FIRSTRING      4
#define SHPP_RING           5

/* -------------------------------------------------------------------- */
/*              SHPVertexType, SHPEnvelope,  SHPBounds,                 */
/*                       SHPObjectExtent                                */
/* -------------------------------------------------------------------- */
typedef struct _SHPPointType
{
    double  x;
    double  y;
} SHPVertexType, SHPPointType;

typedef struct _SHPEnvelope
{
    double      XMin;
    double      YMin;
    double      XMax;
    double      YMax;
} SHPEnvelope;

typedef struct _SHPBounds
{
    union {
        struct {
            double   XMin;
            double   YMin;
            double   XMax;
            double   YMax;
        };
        SHPEnvelope  _Env;
    };

    double      ZMin;
    double      ZMax;
    double      MMin;    
    double      MMax;
} SHPBounds;

typedef struct _SHPObjectExtent
{
    union {
        struct {
            double      dfXMin;
            double      dfYMin;
            double      dfZMin;
            double      dfMMin;
        };
        double minBounds[4];
    };

    union {
        struct {
            double      dfXMax;
            double      dfYMax;
            double      dfZMax;
            double      dfMMax;
        };
        double maxBounds[4];
    };
} SHPObjectExtent;

/* -------------------------------------------------------------------- */
/*      SHPObject - represents on shape (without attributes) read       */
/*      from the .shp file.                                             */
/* -------------------------------------------------------------------- */
typedef struct _SHPObject
{
    int         nSHPType;      /* Shape Type (SHPT_* - see list above) */
    int         nShapeId;      /* Shape Number (-1 is unknown/unassigned) ID */

    int         nParts;        /* of Parts (0 implies single part with no info) */

    int         *panPartStart; /* Start Vertex of part */
    int         *panPartType;  /* Part Type (SHPP_RING if not SHPT_MULTIPATCH) */
  
    int         nVertices;     /* Vertex list */

    double      *padfX;
    double      *padfY;
    double      *padfZ;        /* (all zero if not provided) */
    double      *padfM;        /* (all zero if not provided) */

    union {
        struct {
            double      dfXMin;    /* Bounds in X, Y, Z and M dimensions */
            double      dfYMin;
            double      dfZMin;
            double      dfMMin;

            double      dfXMax;
            double      dfYMax;
            double      dfZMax;
            double      dfMMax;
        };
        SHPObjectExtent  extent;
    };
} SHPObject, *SHPObjectHandle;

/* -------------------------------------------------------------------- */
/*      SHPObjectEx - represents on shape (without attributes) read     */
/*      from the .shp file.                                             */
/* -------------------------------------------------------------------- */
typedef struct _SHPObjectEx
{
    int         nSHPType;
    int         nShapeId;       /* -1 is unknown/unassigned. 0-based */

    int         nPartsSize;     /* total size of Parts Buffer */
    int         nParts;
    int         *panPartStart;
    int         *panPartType;

    int         nPointsSize;    /* total size of pPoints Buffer */
    int         nVertices;      /* actual points count */

    SHPPointType  *pPoints;   

    double      *padfZ;
    double      *padfM;

    union{
        struct{
            double      dfXMin;
            double      dfYMin;
            double      dfXMax;
            double      dfYMax;

            double      dfZMin;
            double      dfZMax;
            double      dfMMin;
            double      dfMMax;
        };
        SHPBounds       _Bounds;
    };
} SHPObjectEx, *SHPObjectExHandle;

/* -------------------------------------------------------------------- */
/*      SHP API Prototypes                                              */
/* -------------------------------------------------------------------- */
SHAPEFILE_API SHPHandle SHPOpen (const char *pszShapeFile, const char *pszAccess);

SHAPEFILE_API SHPHandle SHPCreate (const char *pszShapeFile, int nShapeType);

SHAPEFILE_API void SHPGetInfo (SHPHandle hSHP, int *pnEntities, int *pnShapeType, double *padfMinBound, double *padfMaxBound);

SHAPEFILE_API int SHPGetType (SHPHandle hSHP, int *bHasZ, int *bHasM);

SHAPEFILE_API SHPObject* SHPReadObject (SHPHandle hSHP, int iShape);

SHAPEFILE_API int SHPReadObjectEx (SHPHandle psSHP, int iShape, SHPObjectEx *psShape);

SHAPEFILE_API int SHPReadObjectBounds (SHPHandle hSHP, int iShape, SHPBounds *Bounds);

SHAPEFILE_API int SHPWriteObject (SHPHandle hSHP, int iShape, SHPObject *psObject);

SHAPEFILE_API void SHPDestroyObject (SHPObject * psObject);

SHAPEFILE_API SHPObjectEx* SHPCreateObjectEx (SHPObjectEx ** ppsObject);

SHAPEFILE_API void SHPDestroyObjectEx (SHPObjectEx *psObject);

SHAPEFILE_API void SHPComputeExtents (SHPObject *psObject);

SHAPEFILE_API SHPObject* SHPCreateObject (int nSHPType, int nShapeId,
    int nParts, const int *panPartStart, const int *panPartType,
    int nVertices, const double *padfX, const double *padfY, const double *padfZ, const double *padfM);

SHAPEFILE_API SHPObject* SHPCreateObject2 (int nSHPType, int nShapeId,
    int nParts, const int *panPartStart, const int *panPartType,
    int nVertices, const SHPPointType *padXY, const double *padfZ, const double *padfM);

SHAPEFILE_API SHPObject* SHPCreateSimpleObject (int nSHPType,
    int nVertices, const double *padfX, const double *padfY, const double *padfZ);

SHAPEFILE_API int SHPRewindObject (SHPObject *psObject);

SHAPEFILE_API int SHPRewindObjectEx (SHPObjectEx *psObjectEx);

SHAPEFILE_API void SHPClose (SHPHandle hSHP);

SHAPEFILE_API void SHPWriteHeader (SHPHandle hSHP);

SHAPEFILE_API const char* SHPTypeName (int nSHPType);

SHAPEFILE_API const char* SHPPartTypeName (int nPartType);

SHAPEFILE_API double SHPLengthOfXYs (double *padfX, double *padfY, int start, int end);

SHAPEFILE_API double SHPLengthOfPoints (SHPPointType *pPoints, int start, int end);

SHAPEFILE_API double SHPAreaOfXYs (double *padfX, double *padfY, int start, int end, int *CCW);

SHAPEFILE_API double SHPAreaOfPoints (SHPPointType *pPoints, int start, int end, int *CCW);

SHAPEFILE_API double SHPObjectExGetLength (const SHPObjectEx *psObject);

/**
 * SHPObjectExGetArea
 *   get area of polygon object
 * Returns:
 *   > 0: CCW, Counter Clock Wise, RHR = Right Hand Rule
 *   < 0: CW, Clock Wise, LHR = Left Hand Rule
 */
SHAPEFILE_API double SHPObjectExGetArea (const SHPObjectEx *psObject);

SHAPEFILE_API double SHPObjectGetLength (const SHPObject *psObject);

/**
 * SHPObjectGetArea
 *   get area of polygon object
 * Returns:
 *   > 0: CCW, Counter Clock Wise, RHR = Right Hand Rule
 *   < 0: CW, Clock Wise, LHR = Left Hand Rule
 */
SHAPEFILE_API double SHPObjectGetArea (const SHPObject *psObject);

SHAPEFILE_API void SHPObjectReversePoints (SHPObject *psObject);

SHAPEFILE_API void SHPObjectExReversePoints (SHPObjectEx *psObject);

/**
 * SHPObjectValidatePolygon
*   make points of polygon as given isCCW flag
 * Parameters:
 *   isCCW - 1 for CCW; 0 for CW
 * Returns:
 *   = 1: done
 *   = 0: no operation
 *   = -1: error type
 */
SHAPEFILE_API int SHPObjectValidatePolygon (SHPObject *psObject, int isCCW);

/**
 * SHPObjectExValidatePolygon
 *   make points of polygon as given isCCW flag
 * Parameters:
 *   isCCW - 1 for CCW; 0 for CW
 * Returns:
 *   = 1: make points is done
 *   = 0: no operation
 *   = -1: error type
 */
SHAPEFILE_API int SHPObjectExValidatePolygon (SHPObjectEx *psObject, int isCCW);

SHAPEFILE_API int SHPObject2WKB (const SHPObject *psObject, void *wkbBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM);

SHAPEFILE_API int SHPObject2WKT (const SHPObject *psObject, char *wktBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM,
    int nDecimalsXY, int nDecimalsZ, int nDecimalsM);

SHAPEFILE_API int SHPObjectEx2WKB (const SHPObjectEx *psObject, void *wkbBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM);

SHAPEFILE_API int SHPObjectEx2WKT (const SHPObjectEx *psObject, char *wktBuffer,
    double offsetX, double offsetY, double offsetZ, double offsetM,
    int nDecimalsXY, int nDecimalsZ, int nDecimalsM);

/* -------------------------------------------------------------------- */
/*      Shape quadtree indexing API.                                    */
/*      !! Deprecated !!                                                */
/* -------------------------------------------------------------------- */
/* this can be two or four for binary or quad tree */
#define MAX_SUBNODE     4

typedef struct shape_tree_node * SHPTreeNodeHandle;
typedef struct shape_tree_root * SHPTreeHandle;

SHAPEFILE_API SHPTreeHandle SHPCreateTree (SHPHandle hSHP, int nDimension, int nMaxDepth, double *padfBoundsMin, double *padfBoundsMax);

SHAPEFILE_API void  SHPDestroyTree (SHPTreeHandle hTree);

SHAPEFILE_API int SHPTreeAddShapeId (SHPTreeHandle hTree, SHPObject *psObject);

SHAPEFILE_API void  SHPTreeTrimExtraNodes (SHPTreeHandle hTree);

SHAPEFILE_API int*  SHPTreeFindLikelyShapes (SHPTreeHandle hTree, double *padfBoundsMin, double *padfBoundsMax, int *pnShapeCount);

SHAPEFILE_API int SHPCheckBoundsOverlap (double *padfBox1Min, double *padfBox1Max, double *padfBox2Min, double *padfBox2Max, int nDimension);


/*************************************************************************
 *                             DBF API
 ************************************************************************/
typedef struct _DBFInfo *DBFHandle;

typedef enum {
    FTString   = 0,
    FTInteger  = 1,
    FTDouble   = 2,
    FTLogical  = 3,
    FTInvalid  = 4,
    FTDateS   = 5    /*Format: YYYY-MM-DD */
} DBFFieldType;


static char *DBFFieldTypeName[] = {
    "String",
    "Integer",
    "Double",
    "Logical",
    "Invalid",
    "DateS"
};


#define MAX_DBF_FIELD_NAME_LEN  11

typedef struct {
    int       iField;
    DBFFieldType eType;
    int       nWidth;
    int       nDecimals;
    int       isNotNull;        /* 0 - can be null; 1 - must be NOT null */
    char      szFieldName[MAX_DBF_FIELD_NAME_LEN+1];    /* max len is 11 */

    union {
        int     intVal;
        double  dblVal;
        char    logVal[8];  /* logical value */
        char    dtVal[20];  /* datetime value: YYYY-MM-DD HH:mm:SS\0 */
    };
    char   *pszVal;       /* nWidth+1 */
} DBFFieldInfo;

#define XBASE_FLDHDR_SZ       32

SHAPEFILE_API DBFHandle DBFOpen (const char * pszDBFFile, const char * pszAccess);

SHAPEFILE_API DBFHandle DBFCreate (const char * pszDBFFile);

SHAPEFILE_API int DBFGetFieldCount (DBFHandle psDBF);

SHAPEFILE_API int DBFGetRecordCount (DBFHandle psDBF);

SHAPEFILE_API int DBFAddField (DBFHandle hDBF, const char *pszFieldName, DBFFieldType eType, int nWidth, int nDecimals);

SHAPEFILE_API DBFFieldType DBFGetFieldInfo (DBFHandle psDBF, int iField, char *pszFieldName, int *pnWidth, int *pnDecimals);

SHAPEFILE_API void DBFGetFieldInfo2 (DBFHandle psDBF, int iField, DBFFieldInfo *pFieldInfo);

SHAPEFILE_API int DBFGetFieldIndex (DBFHandle psDBF, const char *pszFieldName);

SHAPEFILE_API int DBFReadIntegerAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API double DBFReadDoubleAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API const char* DBFReadStringAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API int DBFReadCopyStringAttribute (DBFHandle hDBF, int iShape, int iField, char *buffer);

SHAPEFILE_API const char* DBFReadLogicalAttribute (DBFHandle hDBF, int iShape, int iField);

SHAPEFILE_API int DBFIsAttributeNULL (DBFHandle hDBF, int iShape, int iField );

SHAPEFILE_API int DBFWriteIntegerAttribute (DBFHandle hDBF, int iShape, int iField, int nFieldValue);

SHAPEFILE_API int DBFWriteDoubleAttribute (DBFHandle hDBF, int iShape, int iField, double dFieldValue);

SHAPEFILE_API int DBFWriteStringAttribute (DBFHandle hDBF, int iShape, int iField, const char * pszFieldValue);

SHAPEFILE_API int DBFWriteLogicalAttribute (DBFHandle hDBF, int iShape, int iField, const char lFieldValue);
         
SHAPEFILE_API int DBFWriteAttributeDirectly (DBFHandle psDBF, int hEntity, int iField, void * pValue);
                                                             
SHAPEFILE_API const char* DBFReadTuple (DBFHandle psDBF, int hEntity);

SHAPEFILE_API int DBFWriteTuple (DBFHandle psDBF, int hEntity, void *pRawTuple);

SHAPEFILE_API DBFHandle DBFCloneEmpty (DBFHandle psDBF, const char * pszFilename);
 
SHAPEFILE_API void  DBFClose (DBFHandle hDBF);

SHAPEFILE_API void  DBFUpdateHeader (DBFHandle hDBF);

SHAPEFILE_API char DBFGetNativeFieldType (DBFHandle hDBF, int iField);

#ifdef __cplusplus
}
#endif

#endif /* _SHAPEFILE_H_INCLUDED */
