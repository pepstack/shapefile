/**
 * wkbtype.h
 *   WKB Type definitions
 */
#ifndef _WKBTYPE_H_INCLUDED
#define _WKBTYPE_H_INCLUDED

#pragma pack(1)

#include <common/unitypes.h>
#include <common/bo.h>


/*
 * WKB_ByteOrder: 1 byte
 */
#define WKB_BYTEORDER_NDR          ((ub1) 0x01)  /* little endian */
#define WKB_BYTEORDER_XDR          ((ub1) 0x00)  /* big endian */

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
    ub1       byteOrder;
    ub4       wkbType; 
} WKBGeometryHeader;

typedef struct WKBPoint
{
    ub1         byteOrder;
    ub4         wkbType;
    struct PointXY point;
} WKBPoint;

typedef struct WKBLineString
{
    ub1         byteOrder;
    ub4         wkbType;
    ub4         numPoints;
    struct PointXY points[1];
} WKBLineString;

typedef struct WKBPolygon
{
    ub1         byteOrder;
    ub4         wkbType;
    ub4         numRings;
    LinearRing  rings[1];
} WKBPolygon;

typedef struct WKBMultiPoint
{
    ub1         byteOrder;
    ub4         wkbType;
    ub4         numPoints;
    WKBPoint    points[1];
} WKBMultiPoint;

typedef struct WKBMultiLineString
{
    ub1         byteOrder;
    ub4         wkbType;
    ub4         numLineStrings;
    WKBLineString  lineStrings[1];
} WKBMultiLineString;
 
typedef struct WKBMultiPolygon
{
    ub1      byteOrder;
    ub4      wkbType;
    ub4      numPolygons;
    WKBPolygon  polygons[1];
} WKBMultiPolygon;

#pragma pack()

#endif /* _WKBTYPE_H_INCLUDED */
