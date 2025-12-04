#include "shp2wkb.h"

int Point2WKB(const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_Point2D);
        ValBufCopy(pv, cb, v4);

        v8 = BO_float64_htobe(*pObj->padfX + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(*pObj->padfY + offY);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointSize(1);
    }

    return cb;
}


int Arc2WKB(const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_LineString2D);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_float64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringSize(pObj->nVertices);
    }

    return cb;
}


int Polygon2WKB(const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_Polygon2D);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_htobe32(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_htobe32(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_float64_htobe(pObj->padfX[p] + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->padfY[p] + offY);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        cb = WKBHeaderSize;

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


int MultiPoint2WKB(const SHPObject *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_MultiPoint2D);
        ValBufCopy(pv, cb, v4);

        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            WriteWkbBoXdr(pv, cb);

            v4 = BO_htobe32(WKB_Point2D);
            ValBufCopy(pv, cb, v4);

            v8 = BO_float64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointSize(pObj->nVertices);
    }

    return cb;
}


int PointZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_PointZ);
        ValBufCopy(pv, cb, v4);

        v8 = BO_float64_htobe(*pObj->padfX + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(*pObj->padfY + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(*pObj->padfZ + offZ);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointZSize(1);
    }

    return cb;
}


int ArcZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_LineStringZ);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_float64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringZSize(pObj->nVertices);
    }

    return cb;
}


int PolygonZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_PolygonZ);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_htobe32(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_htobe32(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_float64_htobe(pObj->padfX[p] + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->padfY[p] + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->padfZ[p] + offZ);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


int MultiPointZ2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_MultiPointZ);
        ValBufCopy(pv, cb, v4);

        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            WriteWkbBoXdr(pv, cb);

            v4 = BO_htobe32(WKB_PointZ);
            ValBufCopy(pv, cb, v4);

            v8 = BO_float64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointZSize(pObj->nVertices);
    }

    return cb;
}


int PointM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_PointM);
        ValBufCopy(pv, cb, v4);

        v8 = BO_float64_htobe(*pObj->padfX + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(*pObj->padfY + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(*pObj->padfM + offM);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointMSize(1);
    }

    return cb;
}


int ArcM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_LineStringM);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_float64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringMSize(pObj->nVertices);
    }

    return cb;
}


int PolygonM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_PolygonM);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_htobe32(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_htobe32(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_float64_htobe(pObj->padfX[p] + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->padfY[p] + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->padfM[p] + offM);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


int MultiPointM2WKB(const SHPObject *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_MultiPointM);
        ValBufCopy(pv, cb, v4);

        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            WriteWkbBoXdr(pv, cb);

            v4 = BO_htobe32(WKB_PointM);
            ValBufCopy(pv, cb, v4);

            v8 = BO_float64_htobe(pObj->padfX[iPoint] + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfY[iPoint] + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointMSize(pObj->nVertices);
    }

    return cb;
}


int exPoint2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_Point2D);
        ValBufCopy(pv, cb, v4);

        v8 = BO_float64_htobe(pObj->pPoints[0].x + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(pObj->pPoints[0].y + offY);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointSize(1);
    }

    return cb;
}


int exArc2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_LineString2D);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_float64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringSize(pObj->nVertices);
    }

    return cb;
}


int exPolygon2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_Polygon2D);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_htobe32(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_htobe32(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_float64_htobe(pObj->pPoints[p].x + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->pPoints[p].y + offY);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


int exMultiPoint2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_MultiPoint2D);
        ValBufCopy(pv, cb, v4);

        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            WriteWkbBoXdr(pv, cb);

            v4 = BO_htobe32(WKB_Point2D);
            ValBufCopy(pv, cb, v4);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointSize(pObj->nVertices);
    }

    return cb;
}


int exPointZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_PointZ);
        ValBufCopy(pv, cb, v4);

        v8 = BO_float64_htobe(pObj->pPoints[0].x + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(pObj->pPoints[0].y + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(*pObj->padfZ + offZ);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointZSize(1);
    }

    return cb;
}


int exArcZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_LineStringZ);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_float64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringZSize(pObj->nVertices);
    }

    return cb;
}


int exPolygonZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_PolygonZ);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_htobe32(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_htobe32(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_float64_htobe(pObj->pPoints[p].x + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->pPoints[p].y + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->padfZ[p] + offZ);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


int exMultiPointZ2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offZ)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_MultiPointZ);
        ValBufCopy(pv, cb, v4);

        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            WriteWkbBoXdr(pv, cb);

            v4 = BO_htobe32(WKB_PointZ);
            ValBufCopy(pv, cb, v4);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfZ[iPoint] + offZ);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointZSize(pObj->nVertices);
    }

    return cb;
}


int exPointM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_PointM);
        ValBufCopy(pv, cb, v4);

        v8 = BO_float64_htobe(pObj->pPoints[0].x + offX);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(pObj->pPoints[0].y + offY);
        ValBufCopy(pv, cb, v8);

        v8 = BO_float64_htobe(*pObj->padfM + offM);
        ValBufCopy(pv, cb, v8);
    } else {
        cb = WKBPointMSize(1);
    }

    return cb;
}


int exArcM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_LineStringM);
        ValBufCopy(pv, cb, v4);

        /* num of points */
        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            v8 = BO_float64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBLineStringMSize(pObj->nVertices);
    }

    return cb;
}


int exPolygonM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double   v8;
    int cb = 0;

    if (pv) {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        v4 = BO_htobe32(WKB_PolygonM);
        ValBufCopy(pv, cb, v4);

        /* num of rings */
        v4 = BO_htobe32(pObj->nParts);
        ValBufCopy(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int p = pObj->panPartStart[iPart];
            int p1 = pObj->panPartStart[iPart+1];

            /* num of points */
            v4 = BO_htobe32(p1 - p);
            ValBufCopy(pv, cb, v4);

            for (; p < p1; p++) {
                v8 = BO_float64_htobe(pObj->pPoints[p].x + offX);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->pPoints[p].y + offY);
                ValBufCopy(pv, cb, v8);

                v8 = BO_float64_htobe(pObj->padfM[p] + offM);
                ValBufCopy(pv, cb, v8);
            }
        }
    } else {
        int iPart = 0;

        /* byte order flag */
        WriteWkbBoXdr(pv, cb);

        /* wkb type */
        ValBufOffset(pv, cb, v4);

        /* num of rings */
        ValBufOffset(pv, cb, v4);

        for (; iPart < pObj->nParts; iPart++) {
            int np = pObj->panPartStart[iPart+1] - pObj->panPartStart[iPart];

            /* num of points */
            ValBufOffset(pv, cb, v4);

            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
            ValBufOffsetN(pv, cb, v8, np);
        }
    }

    return cb;
}


int exMultiPointM2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY, double offM)
{
    ub4 v4;
    double v8;
    int cb = 0;

    if (pv) {
        int iPoint = 0;
        WriteWkbBoXdr(pv, cb);

        v4 = BO_htobe32(WKB_MultiPointM);
        ValBufCopy(pv, cb, v4);

        v4 = BO_htobe32(pObj->nVertices);
        ValBufCopy(pv, cb, v4);

        for (; iPoint < pObj->nVertices; iPoint++) {
            WriteWkbBoXdr(pv, cb);

            v4 = BO_htobe32(WKB_PointM);
            ValBufCopy(pv, cb, v4);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].x + offX);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->pPoints[iPoint].y + offY);
            ValBufCopy(pv, cb, v8);

            v8 = BO_float64_htobe(pObj->padfM[iPoint] + offM);
            ValBufCopy(pv, cb, v8);
        }
    } else {
        cb = WKBHeaderSize + sizeof(v4) + WKBPointMSize(pObj->nVertices);
    }

    return cb;
}


int MultiPatch2WKB(const SHPObject *pObj, void *pv, double offX, double offY)
{
    /* TODO */
    return 0;
}


int exMultiPatch2WKB(const SHPObjectEx *pObj, void *pv, double offX, double offY)
{
    /* TODO */
    return 0;
}
