/******************************************************************************
 * shp2wkt.h
 *
 * v 1.0 2013/04/28
 *
 * Project:  Shapelib
 * Purpose:  Shapelib extending functions
 * Author:   cheungmine@gmail.com
 * 2013/04/28
 *
 * Copyright (c) 2013, cheungmine
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

/*
 * Well-known text (WKT) is a text markup language for representing vector
 * geometry objects on a map, spatial reference systems of spatial objects
 * and transformations between spatial reference systems. A binary equivalent,
 * known as well-known binary (WKB) is used to transfer and store the same
 * information on databases, such as PostGIS, Microsoft SQL Server and DB2.
 * The formats were originally defined by the Open Geospatial Consortium (OGC)
 * and described in their Simple Feature Access and Coordinate Transformation
 * Service specifications. The current standard definition is in the ISO/IEC
 * 13249-3:2011 standard, "Information technology -- Database languages --
 * SQL multimedia and application packages -- Part 3: Spatial" (SQL/MM).
 *
 * reference:
 *   http://en.wikipedia.org/wiki/Well-known_text
 *
 * POINT(6 10)
 * LINESTRING(3 4,10 50,20 25)
 * POLYGON((1 1,5 1,5 5,1 5,1 1),(2 2,2 3,3 3,3 2,2 2))
 * MULTIPOINT(3.5 5.6, 4.8 10.5)
 * MULTILINESTRING((3 4,10 50,20 25),(-5 -8,-10 -8,-15 -4))
 * MULTIPOLYGON(((1 1,5 1,5 5,1 5,1 1),(2 2,2 3,3 3,3 2,2 2)),((6 3,9 2,9 4,6 3)))
 * GEOMETRYCOLLECTION(POINT(4 6),LINESTRING(4 6,7 10))
 * POINT ZM (1 1 5 60)
 * POINT M (1 1 80)
 * POINT EMPTY
 * MULTIPOLYGON EMPTY
 */

#ifndef _SHP2WKT_H_INCLUDED
#define _SHP2WKT_H_INCLUDED

#ifdef _MSC_VER
 /* disable annoying warning: 4996 */
 #pragma warning (disable : 4996)
#endif

#include "shapefile_i.h"


static int Point2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;

    if (pbBuf) {
        cb = sprintf(pbBuf, "POINT (%.*lf %.*lf)",
            dig, *pObj->padfX + offX,
            dig, *pObj->padfY + offY);
    } else {
        static char tmpbuf[128];
        cb = sprintf(tmpbuf, "POINT (%.*lf %.*lf)",
            dig, *pObj->padfX + offX,
            dig, *pObj->padfY + offY);
    }

    return cb;
}


static int Arc2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, "MULTILINESTRING (");
        } else {
            cb += sprintf(pbBuf, "LINESTRING ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, ")");
        }
    } else {
        static char tmpbuf[128];

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, "MULTILINESTRING (");
        } else {
            cb += sprintf(tmpbuf, "LINESTRING ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, ")");
        }
    }

    return cb;
}


static int Polygon2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "POLYGON (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "POLYGON (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        cb += sprintf(tmpbuf, ")");
    }

    return cb;
}


static int MultiPoint2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;
    int i = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "MULTIPOINT (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf),",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY);
            } else {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf)",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY);
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "MULTIPOINT (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf),",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY);
            } else {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf)",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY);
            }
        }

        cb += sprintf(tmpbuf + cb, ")");
    }

    return cb;
}


static int PointZ2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;

    if (pbBuf) {
        cb = sprintf(pbBuf, "POINT Z (%.*lf %.*lf %.*lf)",
            dig, *pObj->padfX + offX,
            dig, *pObj->padfY + offY,
            digZ, *pObj->padfZ + offZ);
    } else {
        static char tmpbuf[128];
        cb = sprintf(tmpbuf, "POINT Z (%.*lf %.*lf %.*lf)",
            dig, *pObj->padfX + offX,
            dig, *pObj->padfY + offY,
            digZ, *pObj->padfZ + offZ);
    }

    return cb;
}


static int ArcZ2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, "MULTILINESTRING Z (");
        } else {
            cb += sprintf(pbBuf, "LINESTRING Z ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, ")");
        }
    } else {
        static char tmpbuf[128];

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, "MULTILINESTRING Z (");
        } else {
            cb += sprintf(tmpbuf, "LINESTRING Z ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, ")");
        }
    }

    return cb;
}


static int PolygonZ2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "POLYGON Z (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "POLYGON Z (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        cb += sprintf(tmpbuf, ")");
    }

    return cb;
}


static int MultiPointZ2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;
    int i = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "MULTIPOINT Z (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digZ, pObj->padfZ[i] + offZ);
            } else {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digZ, pObj->padfZ[i] + offZ);
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "MULTIPOINT Z (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digZ, pObj->padfZ[i] + offZ);
            } else {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digZ, pObj->padfZ[i] + offZ);
            }
        }

        cb += sprintf(tmpbuf + cb, ")");
    }

    return cb;
}


static int PointM2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;

    if (pbBuf) {
        cb = sprintf(pbBuf, "POINT M (%.*lf %.*lf %.*lf)",
            dig, *pObj->padfX + offX,
            dig, *pObj->padfY + offY,
            digM, *pObj->padfM + offM);
    } else {
        static char tmpbuf[128];
        cb = sprintf(tmpbuf, "POINT M (%.*lf %.*lf %.*lf)",
            dig, *pObj->padfX + offX,
            dig, *pObj->padfY + offY,
            digM, *pObj->padfM + offM);
    }

    return cb;
}


static int ArcM2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, "MULTILINESTRING M (");
        } else {
            cb += sprintf(pbBuf, "LINESTRING M ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, ")");
        }
    } else {
        static char tmpbuf[128];

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, "MULTILINESTRING M (");
        } else {
            cb += sprintf(tmpbuf, "LINESTRING M ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, ")");
        }
    }

    return cb;
}


static int PolygonM2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "POLYGON M (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "POLYGON M (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->padfX[at] + offX,
                        dig, pObj->padfY[at] + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        cb += sprintf(tmpbuf, ")");
    }

    return cb;
}


static int MultiPointM2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;
    int i = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "MULTIPOINT M (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digM, pObj->padfM[i] + offM);
            } else {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digM, pObj->padfM[i] + offM);
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "MULTIPOINT M (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digM, pObj->padfM[i] + offM);
            } else {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->padfX[i] + offX,
                    dig, pObj->padfY[i] + offY,
                    digM, pObj->padfM[i] + offM);
            }
        }

        cb += sprintf(tmpbuf + cb, ")");
    }

    return cb;
}


static int exPoint2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;

    if (pbBuf) {
        cb = sprintf(pbBuf, "POINT (%.*lf %.*lf)",
            dig, pObj->pPoints->x + offX,
            dig, pObj->pPoints->y + offY);
    } else {
        static char tmpbuf[128];
        cb = sprintf(tmpbuf, "POINT (%.*lf %.*lf)",
            dig, pObj->pPoints->x + offX,
            dig, pObj->pPoints->y + offY);
    }

    return cb;
}


static int exPointZ2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;

    if (pbBuf) {
        cb = sprintf(pbBuf, "POINT Z (%.*lf %.*lf %.*lf)",
            dig, pObj->pPoints->x + offX,
            dig, pObj->pPoints->y + offY,
            digZ, *pObj->padfZ + offZ);
    } else {
        static char tmpbuf[128];
        cb = sprintf(tmpbuf, "POINT Z (%.*lf %.*lf %.*lf)",
            dig, pObj->pPoints->x + offX,
            dig, pObj->pPoints->y + offY,
            digZ, *pObj->padfZ + offZ);
    }

    return cb;
}


static int exPointM2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;

    if (pbBuf) {
        cb = sprintf(pbBuf, "POINT M (%.*lf %.*lf %.*lf)",
            dig, pObj->pPoints->x + offX,
            dig, pObj->pPoints->y + offY,
            digM, *pObj->padfM + offM);
    } else {
        static char tmpbuf[128];
        cb = sprintf(tmpbuf, "POINT M (%.*lf %.*lf %.*lf)",
            dig, pObj->pPoints->x + offX,
            dig, pObj->pPoints->y + offY,
            digM, *pObj->padfM + offM);
    }

    return cb;
}


static int exMultiPoint2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;
    int i = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "MULTIPOINT (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf),",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY);
            } else {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf)",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY);
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "MULTIPOINT (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf),",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY);
            } else {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf)",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY);
            }
        }

        cb += sprintf(tmpbuf + cb, ")");
    }

    return cb;
}


static int exMultiPointZ2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;
    int i = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "MULTIPOINT Z (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digZ, pObj->padfZ[i] + offZ);
            } else {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digZ, pObj->padfZ[i] + offZ);
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "MULTIPOINT Z (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digZ, pObj->padfZ[i] + offZ);
            } else {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digZ, pObj->padfZ[i] + offZ);
            }
        }

        cb += sprintf(tmpbuf + cb, ")");
    }

    return cb;
}


static int exMultiPointM2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;
    int i = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "MULTIPOINT M (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digM, pObj->padfM[i] + offM);
            } else {
                cb += sprintf(pbBuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digM, pObj->padfM[i] + offM);
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "MULTIPOINT M (");

        for (; i < pObj->nVertices; i++) {
            if (i < pObj->nVertices -1) {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf),",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digM, pObj->padfM[i] + offM);
            } else {
                cb += sprintf(tmpbuf + cb, "(%.*lf %.*lf %.*lf)",
                    dig, pObj->pPoints[i].x + offX,
                    dig, pObj->pPoints[i].y + offY,
                    digM, pObj->padfM[i] + offM);
            }
        }

        cb += sprintf(tmpbuf + cb, ")");
    }

    return cb;
}


static int exArc2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, "MULTILINESTRING (");
        } else {
            cb += sprintf(pbBuf, "LINESTRING ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, ")");
        }
    } else {
        static char tmpbuf[128];

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, "MULTILINESTRING (");
        } else {
            cb += sprintf(tmpbuf, "LINESTRING ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, ")");
        }
    }

    return cb;
}


static int exArcZ2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, "MULTILINESTRING Z (");
        } else {
            cb += sprintf(pbBuf, "LINESTRING Z ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, ")");
        }
    } else {
        static char tmpbuf[128];

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, "MULTILINESTRING Z (");
        } else {
            cb += sprintf(tmpbuf, "LINESTRING Z ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, ")");
        }
    }

    return cb;
}


static int exArcM2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, "MULTILINESTRING M (");
        } else {
            cb += sprintf(pbBuf, "LINESTRING M ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(pbBuf, ")");
        }
    } else {
        static char tmpbuf[128];

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, "MULTILINESTRING M (");
        } else {
            cb += sprintf(tmpbuf, "LINESTRING M ");
        }

        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        if (pObj->nParts > 1) {
            cb += sprintf(tmpbuf, ")");
        }
    }

    return cb;
}


static int exPolygon2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "POLYGON (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "POLYGON (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        cb += sprintf(tmpbuf, ")");
    }

    return cb;
}


static int exPolygonZ2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "POLYGON Z (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "POLYGON Z (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digZ, pObj->padfZ[at] + offZ);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        cb += sprintf(tmpbuf, ")");
    }

    return cb;
}


static int exPolygonM2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
{
    int cb = 0;
    int start, end, at;
    int iPart = 0;

    if (pbBuf) {
        cb += sprintf(pbBuf, "POLYGON M (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(pbBuf + cb, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(pbBuf + cb, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(pbBuf + cb, "),");
            } else {
                cb += sprintf(pbBuf + cb, ")");
            }
        }

        cb += sprintf(pbBuf + cb, ")");
    } else {
        static char tmpbuf[128];

        cb += sprintf(tmpbuf, "POLYGON M (");
        for (; iPart < pObj->nParts; iPart++) {
            start = pObj->panPartStart[iPart];
            end = pObj->panPartStart[iPart+1];

            cb += sprintf(tmpbuf, "(");

            for (at = start; at < end; at++) {
                if (at < end -1) {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf,",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                } else {
                    cb += sprintf(tmpbuf, "%.*lf %.*lf %.*lf",
                        dig, pObj->pPoints[at].x + offX,
                        dig, pObj->pPoints[at].y + offY,
                        digM, pObj->padfM[at] + offM);
                }
            }

            if (iPart < pObj->nParts -1) {
                cb += sprintf(tmpbuf, "),");
            } else {
                cb += sprintf(tmpbuf, ")");
            }
        }

        cb += sprintf(tmpbuf, ")");
    }

    return cb;
}


static int MultiPatch2WKT (const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
{
    /* TODO */
    return 0;
}


static int exMultiPatch2WKT (const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
{
    /* TODO */
    return 0;
}

#endif /* _SHP2WKT_H_INCLUDED */
