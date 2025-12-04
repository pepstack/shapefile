#include "shp2wkt.h"

int Point2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
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


int Arc2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
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


int Polygon2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
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


int MultiPoint2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
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


int PointZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int ArcZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int PolygonZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int MultiPointZ2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int PointM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int ArcM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int PolygonM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int MultiPointM2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int exPoint2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
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


int exPointZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int exPointM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int exMultiPoint2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
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


int exMultiPointZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int exMultiPointM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int exArc2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
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


int exArcZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int exArcM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int exPolygon2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
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


int exPolygonZ2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offZ, int dig, int digZ)
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


int exPolygonM2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, double offM, int dig, int digM)
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


int MultiPatch2WKT(const SHPObject *pObj, char *pbBuf, double offX, double offY, int dig)
{
    /* TODO */
    return 0;
}


int exMultiPatch2WKT(const SHPObjectEx *pObj, char *pbBuf, double offX, double offY, int dig)
{
    /* TODO */
    return 0;
}
