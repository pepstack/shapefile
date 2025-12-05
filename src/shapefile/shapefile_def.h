/**
 * @file shapefile_def.h
 * @brief shapefile public definitions.
 *
 * @author cheungmine@qq.com
 * @copyright © 2024-2025 anomura.cc, All Rights Reserved.
 * @version 0.0.2
 * @date 2025-12-05 02:23:33
 * @note
 * @since 2024-10-10 20:24:42
 */

/******************************************************************************
 * v 1.4 2005/03/24 14:26:44
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
#ifndef SHAPEFILE_DEF_H_INCLUDED
#define SHAPEFILE_DEF_H_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#if defined (_SVR4) || defined (SVR4) || defined (__OpenBSD__) || \
    defined (_sgi) || defined (__sun) || defined (sun) || \
    defined (__digital__) || defined (__HP_cc)
    # include <inttypes.h>
#elif defined (_MSC_VER) && _MSC_VER < 1600
  # ifndef TYPEDEF_HAS_STDINT
  #   define TYPEDEF_HAS_STDINT
    /* VS 2010 (_MSC_VER 1600) has stdint.h */
    typedef __int8 int8_t;
    typedef unsigned __int8 uint8_t;
    typedef __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
  # endif
#elif defined (_AIX)
    # include <sys/inttypes.h>
#else
    # include <inttypes.h>
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

typedef int SHAPEFILE_BOOL;

#define SHAPEFILE_TRUE    1
#define SHAPEFILE_FALSE   0


typedef int SHAPEFILE_RESULT;

#define SHAPEFILE_SUCCESS     0
#define SHAPEFILE_ERROR     (-1)

/*
 * WKB_ByteOrder: 1 byte
 */
#define WKB_BYTEORDER_NDR          ((unsigned char) 0x01)  /* little endian */
#define WKB_BYTEORDER_XDR          ((unsigned char) 0x00)  /* big endian */

/*
 * WKB_Type: 4 bytes
 */
#define WKB_Geometry                  0
#define WKB_GeometryZ              1000
#define WKB_GeometryM              2000
#define WKB_GeometryZM             3000
#define WKB_Geometry2D             WKB_Geometry

#define WKB_Point                     1
#define WKB_PointZ                 1001
#define WKB_PointM                 2001
#define WKB_PointZM                3001
#define WKB_Point2D                WKB_Point

#define WKB_LineString                2
#define WKB_LineStringZ            1002
#define WKB_LineStringM            2002
#define WKB_LineStringZM           3002
#define WKB_LineString2D           WKB_LineString

#define WKB_Polygon                   3
#define WKB_PolygonZ               1003
#define WKB_PolygonM               2003
#define WKB_PolygonZM              3003
#define WKB_Polygon2D              WKB_Polygon

#define WKB_MultiPoint                4
#define WKB_MultiPointZ            1004
#define WKB_MultiPointM            2004
#define WKB_MultiPointZM           3004
#define WKB_MultiPoint2D           WKB_MultiPoint

#define WKB_MultiLineString           5
#define WKB_MultiLineStringZ       1005
#define WKB_MultiLineStringM       2005
#define WKB_MultiLineStringZM      3005
#define WKB_MultiLineString2D      WKB_MultiLineString

#define WKB_MultiPolygon              6
#define WKB_MultiPolygonZ          1006
#define WKB_MultiPolygonM          2006
#define WKB_MultiPolygonZM         3006
#define WKB_MultiPolygon2D         WKB_MultiPolygon

#define WKB_GeometryCollection     0007
#define WKB_GeometryCollectionZ    1007
#define WKB_GeometryCollectionM    2007
#define WKB_GeometryCollectionZM   3007
#define WKB_GeometryCollection2D   WKB_GeometryCollection

#define WKB_CircularString            8
#define WKB_CircularStringZ        1008
#define WKB_CircularStringM        2008
#define WKB_CircularStringZM       3008
#define WKB_CircularString2D       WKB_CircularString

#define WKB_CompoundCurve             9
#define WKB_CompoundCurveZ         1009
#define WKB_CompoundCurveM         2009
#define WKB_CompoundCurveZM        3009
#define WKB_CompoundCurve2D        WKB_CompoundCurve

#define WKB_CurvePolygon             10
#define WKB_CurvePolygonZ          1010
#define WKB_CurvePolygonM          2010
#define WKB_CurvePolygonZM         3010
#define WKB_CurvePolygon2D         WKB_CurvePolygon

#define WKB_MultiCurve               11
#define WKB_MultiCurveZ            1011
#define WKB_MultiCurveM            2011
#define WKB_MultiCurveZM           3011
#define WKB_MultiCurve2D           WKB_MultiCurve

#define WKB_MultiSurface             12
#define WKB_MultiSurfaceZ          1012
#define WKB_MultiSurfaceM          2012
#define WKB_MultiSurfaceZM         3012
#define WKB_MultiSurface2D         WKB_MultiSurface

#define WKB_Curve                    13
#define WKB_CurveZ                 1013
#define WKB_CurveM                 2013
#define WKB_CurveZM                3013
#define WKB_Curve2D                WKB_Curve

#define WKB_Surface                  14
#define WKB_SurfaceZ               1014
#define WKB_SurfaceM               2014
#define WKB_SurfaceZM              3014
#define WKB_Surface2D              WKB_Surface

#define WKB_PolyhedralSurface        15
#define WKB_PolyhedralSurfaceZ     1015
#define WKB_PolyhedralSurfaceM     2015
#define WKB_PolyhedralSurfaceZM    3015
#define WKB_PolyhedralSurface2D    WKB_PolyhedralSurface

#define WKB_TIN                      16
#define WKB_TINZ                   1016
#define WKB_TINM                   2016
#define WKB_TINZM                  3016
#define WKB_TIN2D                  WKB_TIN

#define WKB_Triangle                 17
#define WKB_TriangleZ              1017
#define WKB_TriangleM              2017
#define WKB_TriangleZM             3017
#define WKB_Triangle2D             WKB_Triangle

#pragma pack(1)

enum WKBByteOrder
{
    wkbXDR = 0x00,    /* Big Endian */
    wkbNDR = 0x01     /* Little Endian */
};

#define WKBDefaultByteOrder wkbXDR

enum WKBGeometryType
{
    wkbNull                 = 0,
    wkbPoint                = WKB_Point,
    wkbLineString           = WKB_LineString,
    wkbPolygon              = WKB_Polygon,
    wkbMultiPoint           = WKB_MultiPoint,
    wkbMultiLineString      = WKB_MultiLineString,
    wkbMultiPolygon         = WKB_MultiPolygon,
    wkbGeometryCollection   = WKB_GeometryCollection
};

struct PointXY
{
    double x;
    double y;
};


typedef struct LinearRing
{
    int     numPoints;
    struct PointXY points[1];
} LinearRing;

typedef struct WKBGeometryHeader
{
    unsigned char  byteOrder;
    uint32_t       wkbType;
} WKBGeometryHeader;

typedef struct WKBPoint
{
    unsigned char    byteOrder;
    uint32_t         wkbType;
    struct PointXY point;
} WKBPoint;

typedef struct WKBLineString
{
    unsigned char    byteOrder;
    uint32_t         wkbType;
    uint32_t         numPoints;
    struct PointXY points[1];
} WKBLineString;

typedef struct WKBPolygon
{
    unsigned char    byteOrder;
    uint32_t         wkbType;
    uint32_t         numRings;
    LinearRing  rings[1];
} WKBPolygon;

typedef struct WKBMultiPoint
{
    unsigned char    byteOrder;
    uint32_t         wkbType;
    uint32_t         numPoints;
    WKBPoint    points[1];
} WKBMultiPoint;

typedef struct WKBMultiLineString
{
    unsigned char    byteOrder;
    uint32_t         wkbType;
    uint32_t         numLineStrings;
    WKBLineString  lineStrings[1];
} WKBMultiLineString;

typedef struct WKBMultiPolygon
{
    unsigned char byteOrder;
    uint32_t      wkbType;
    uint32_t      numPolygons;
    WKBPolygon  polygons[1];
} WKBMultiPolygon;

#pragma pack()

typedef struct _DBFInfo *DBFHandle;
typedef struct _SHPRectTree * SHPRectTree;


typedef enum {
    FTString   = 0,
    FTInteger  = 1,
    FTDouble   = 2,
    FTLogical  = 3,
    FTInvalid  = 4,
    FTDateS   = 5    /*Format: YYYY-MM-DD */
} DBFFieldType;

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



/* -------------------------------------------------------------------- */
/*      Shape quadtree indexing API.                                    */
/*      !! Deprecated !!                                                */
/* -------------------------------------------------------------------- */
/* this can be two or four for binary or quad tree */
#define MAX_SUBNODE     4

typedef struct shape_tree_node * SHPTreeNodeHandle;
typedef struct shape_tree_root * SHPTreeHandle;

typedef struct _SHPInfoRTree   * SHPMBRTree;


#define SHAPEFILE_RECORDS_MAX   256000000

#ifndef SHAPEFILE_ASSERT
    #define SHAPEFILE_ASSERT(expr)
#endif


#ifndef SHAPEFILE_TRUE
    #define SHAPEFILE_TRUE 1
    #define SHAPEFILE_FALSE 0
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
#define SHPT_POINT          1   /* Points */
#define SHPT_ARC            3   /* Arcs (Polylines, possible in parts) */
#define SHPT_POLYGON        5   /* Polygons (possible in parts) */
#define SHPT_MULTIPOINT     8   /* MultiPoint (related points) */

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
    int32_t      nSHPType;      /* Shape Type (SHPT_* - see list above) */
    int32_t      nShapeId;      /* Shape Number (-1 is unknown/unassigned) ID */

    int32_t      nParts;        /* of Parts (0 implies single part with no info) */
    int32_t     *panPartStart;  /* Start Vertex of part */
    int32_t     *panPartType;   /* Part Type (SHPP_RING if not SHPT_MULTIPATCH) */

    int32_t      nVertices;     /* Vertex list */
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

#if defined(__cplusplus)
}
#endif

#endif /* SHAPEFILE_DEF_H_INCLUDED */
