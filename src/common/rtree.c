/****************************************************************************
* RTree.C
*
* MODULE:       R-Tree library
*
* AUTHOR(S):    Antonin Guttman - original code
*               Daniel Green (green@superliminal.com) - major clean-up
*                               and implementation of bounding spheres
*
* PURPOSE:      Multi Dimensional Index
*
* COPYRIGHT:    (C) 2001 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*               License (>=v2). Read the file COPYING that comes with GRASS
*               for details.
*
* LAST MODIFY:  ZhangLiang (cheungmine@gmail.com) - 2021-08
*****************************************************************************/
#include "rtree.h"

/**
 * Precomputed volumes of the unit spheres for the first few dimensions
 */
static const double rtree_sphere_volumes[] = {
    0.000000,  /* dimension   0 */
    2.000000,  /* dimension   1 */
    3.141593,  /* dimension   2 */
    4.188790,  /* dimension   3 */
    4.934802,  /* dimension   4 */
    5.263789,  /* dimension   5 */
    5.167713,  /* dimension   6 */
    4.724766,  /* dimension   7 */
    4.058712,  /* dimension   8 */
    3.298509,  /* dimension   9 */
    2.550164,  /* dimension  10 */
    1.884104,  /* dimension  11 */
    1.335263,  /* dimension  12 */
    0.910629,  /* dimension  13 */
    0.599265,  /* dimension  14 */
    0.381443,  /* dimension  15 */
    0.235331,  /* dimension  16 */
    0.140981,  /* dimension  17 */
    0.082146,  /* dimension  18 */
    0.046622,  /* dimension  19 */
    0.025807,  /* dimension  20 */
};

#define RTREE_SPHERE_VOL  (rtree_sphere_volumes[RTREE_DIMS])

#define	RTREE_PARTITIONS	 1

#define RTREE_INVALIDMBR(x)  ((x)->bound[0] > (x)->bound[RTREE_DIMS])

#define RTREE_MIN2(a, b)  ((a) < (b) ? (a) : (b))

#define RTREE_MAX2(a, b)  ((a) > (b) ? (a) : (b))

/* max branching factor of a node */
#define RTREE_MAXCARD    ((int)((RTREE_PAGESZ-(2*sizeof(int))) / sizeof(rtree_branch_t)))

#define RTREE_NODECARD   (RTREE_MAXCARD)
#define RTREE_LEAFCARD   (RTREE_MAXCARD)

/**
 * balance criteria for node splitting
 * NOTE: can be changed if needed.
 */
#define RTREE_MINNODEFILL  (RTREE_NODECARD / 2)
#define RTREE_MINLEAFFILL  (RTREE_LEAFCARD / 2)

#define RTREE_MAXKIDS(n)   ((n)->level > 0 ? RTREE_NODECARD : RTREE_LEAFCARD)
#define RTREE_MINFILL(n)   ((n)->level > 0 ? RTREE_MINNODEFILL : RTREE_MINLEAFFILL)


/* variables for finding a partition */
typedef struct
{
    int			partition[RTREE_MAXCARD + 1];
    int			total;
    int			minfill;
    int			taken[RTREE_MAXCARD+1];
    int			count[2];
    rtree_mbr_t	cover[2];
    RTREE_REAL	area[2];
} rtree_partition_t;


typedef struct _rtree_node_t
{
    int	count;
    int	level;  /* 0 is leaf, others positive */
    rtree_branch_t branch[RTREE_MAXCARD];
} rtree_node_t;


typedef struct _rtree_nodelist_t
{
     struct _rtree_nodelist_t *next;
     rtree_node_t *node;
} rtree_nodelist_t;


typedef struct _rtree_root_t
{
    rtree_node_t*	rootNode;
    rtree_branch_t  branchBuf[RTREE_MAXCARD + 1];
    int				branchNum;
    rtree_mbr_t		coverSplit;
    RTREE_REAL		coverSplitArea;
    rtree_partition_t	partitions[RTREE_PARTITIONS];

    /**
     * If passed to a tree search, this callback function will be called
     * with the ID of each data mbr that overlaps the search mbr
     * plus whatever user specific pointer was passed to the search.
     * It can terminate the search early by returning 0 in which case
     * the search will return the number of hits found up to that point.
     */
    int (*searchCallback)(void* dataid, void* userarg);

    rtree_mbr_t * mbrlimit;
    char mbr[0];
} rtree_root_t;


/**
 * Load branch buffer with branches from full node plus the extra branch.
 */
static void _RTreeGetBranches(rtree_root root, rtree_node_t *node, rtree_branch_t *br)
{
    int i;

    RTREE_ASSERT(node && br);

    /* load the branch buffer */
    for (i=0; i<RTREE_MAXKIDS(node); i++) {
        RTREE_ASSERT(node->branch[i].child); /* n should have every entry full */
        root->branchBuf[i] = node->branch[i];
    }
    root->branchBuf[RTREE_MAXKIDS(node)] = *br;
    root->branchNum = RTREE_MAXKIDS(node) + 1;

    /* calculate mbr containing all in the set */
    root->coverSplit = root->branchBuf[0].mbr;
    for (i=1; i<RTREE_MAXKIDS(node)+1; i++) {
        root->coverSplit = rtree_mbr_union(&root->coverSplit, &root->branchBuf[i].mbr);
    }

    root->coverSplitArea = rtree_mbr_spherical_volume(&root->coverSplit);
    rtree_init_node(node);
}


/**
 * Put a branch in one of the groups.
 */
static void _RTreeClassify(rtree_root root, int i, int group, rtree_partition_t *p)
{
    RTREE_ASSERT(p);
    RTREE_ASSERT(!p->taken[i]);

    p->partition[i] = group;
    p->taken[i] = RTREE_TRUE;

    if (p->count[group] == 0) {
        p->cover[group] = root->branchBuf[i].mbr;
    } else {
        p->cover[group] = rtree_mbr_union(&root->branchBuf[i].mbr, &p->cover[group]);
    }
    p->area[group] = rtree_mbr_spherical_volume(&p->cover[group]);
    p->count[group]++;
}


/**
 * Pick two rects from set to be the first elements of the two groups.
 * Pick the two that waste the most area if covered by a single rectangle.
 */
static void _RTreePickSeeds(rtree_root root, rtree_partition_t *p)
{
    int i, j, seed0=0, seed1=0;
    RTREE_REAL worst, waste, area[RTREE_MAXCARD+1];

    for (i=0; i<p->total; i++) {
        area[i] = rtree_mbr_spherical_volume(&root->branchBuf[i].mbr);
    }
    worst = -root->coverSplitArea - 1;

    for (i=0; i<p->total-1; i++) {
        for (j=i+1; j<p->total; j++) {
            rtree_mbr_t one_rect;
            one_rect = rtree_mbr_union(&root->branchBuf[i].mbr, &root->branchBuf[j].mbr);
            waste = rtree_mbr_spherical_volume(&one_rect) - area[i] - area[j];
            if (waste > worst) {
                worst = waste;
                seed0 = i;
                seed1 = j;
            }
        }
    }
    _RTreeClassify(root, seed0, 0, p);
    _RTreeClassify(root, seed1, 1, p);
}


/**
 * Copy branches from the buffer into two nodes according to the partition.
 */
static void _RTreeLoadNodes(rtree_root root, rtree_node_t *n, rtree_node_t *q, rtree_partition_t *p)
{
    int i;
    RTREE_ASSERT(n && q && p);

    for (i=0; i<p->total; i++) {
        RTREE_ASSERT(p->partition[i] == 0 || p->partition[i] == 1);
        if (p->partition[i] == 0) {
            rtree_add_branch(root, &root->branchBuf[i], n, NULL);
        } else if (p->partition[i] == 1) {
            rtree_add_branch(root, &root->branchBuf[i], q, NULL);
        }
    }
}


/**
 * Initialize a rtree_partition_t structure.
 */
static void _RTreeInitPart( rtree_partition_t *p, int maxrects, int minfill)
{
    int i;
    RTREE_ASSERT(p);

    p->count[0] = p->count[1] = 0;
    p->cover[0] = p->cover[1] = rtree_mbr_null();
    p->area[0] = p->area[1] = (RTREE_REAL)0;
    p->total = maxrects;
    p->minfill = minfill;

    for (i=0; i<maxrects; i++) {
        p->taken[i] = RTREE_FALSE;
        p->partition[i] = -1;
    }
}


/**
 * Print out data for a partition from rtree_partition_t struct.
 */
static void _RTreePrintPart(rtree_root root, rtree_partition_t *p)
{
    int i;
    RTREE_ASSERT(p);

    fprintf(stdout, "\npartition:\n");
    for (i=0; i<p->total; i++) {
        fprintf(stdout, "%3d\t", i);
    }
    fprintf(stdout, "\n");
    for (i=0; i<p->total; i++) {
        if (p->taken[i]) {
            fprintf(stdout, "  t\t");
        } else {
            fprintf(stdout, "\t");
        }
    }
    fprintf(stdout, "\n");
    for (i=0; i<p->total; i++) {
        fprintf(stdout, "%3d\t", p->partition[i]);
    }
    fprintf(stdout, "\n");

    fprintf(stdout, "count[0] = %d  area = %f\n", p->count[0], p->area[0]);
    fprintf(stdout, "count[1] = %d  area = %f\n", p->count[1], p->area[1]);
    if (p->area[0] + p->area[1] > 0) {
        fprintf(stdout, "total area = %f  effectiveness = %3.2f\n",
            p->area[0] + p->area[1], (float)root->coverSplitArea / (p->area[0] + p->area[1]));
    }
    fprintf(stdout, "cover[0]:\n");
    rtree_mbr_print(&p->cover[0], 0);

    fprintf(stdout, "cover[1]:\n");
    rtree_mbr_print(&p->cover[1], 0);
}


/**
 * Method #0 for choosing a partition:
 * As the seeds for the two groups, pick the two rects that would waste the
 * most area if covered by a single rectangle, i.e. evidently the worst pair
 * to have in the same group.
 * Of the remaining, one at a time is chosen to be put in one of the two groups.
 * The one chosen is the one with the greatest difference in area expansion
 * depending on which group - the mbr most strongly attracted to one group
 * and repelled from the other.
 * If one group gets too full (more would force other group to violate min
 * fill requirement) then other group gets the rest.
 * These last are the ones that can go in either group most easily.
 */
static void _RTreeMethodZero(rtree_root root, rtree_partition_t *p, int minfill )
{
    int i;
    RTREE_REAL biggestDiff;
    int group, chosen=0, betterGroup=0;
    RTREE_ASSERT(p);

    _RTreeInitPart(p, root->branchNum, minfill);
    _RTreePickSeeds(root, p);

    while (p->count[0] + p->count[1] < p->total &&
        p->count[0] < p->total - p->minfill &&
        p->count[1] < p->total - p->minfill) {
        biggestDiff = (RTREE_REAL)-1.;
        for (i=0; i<p->total; i++) {
            if (!p->taken[i]) {
                rtree_mbr_t *r, rect_0, rect_1;
                RTREE_REAL growth0, growth1, diff;

                r = &root->branchBuf[i].mbr;
                rect_0 = rtree_mbr_union(r, &p->cover[0]);
                rect_1 = rtree_mbr_union(r, &p->cover[1]);
                growth0 = rtree_mbr_spherical_volume(&rect_0) - p->area[0];
                growth1 = rtree_mbr_spherical_volume(&rect_1) - p->area[1];
                diff = growth1 - growth0;

                if (diff >= 0) {
                    group = 0;
                } else {
                    group = 1;
                    diff = -diff;
                }

                if (diff > biggestDiff) {
                    biggestDiff = diff;
                    chosen = i;
                    betterGroup = group;
                } else if (diff==biggestDiff && p->count[group]<p->count[betterGroup]) {
                    chosen = i;
                    betterGroup = group;
                }
            }
        }
        _RTreeClassify(root, chosen, betterGroup, p);
    }

    /* if one group too full, put remaining rects in the other */
    if (p->count[0] + p->count[1] < p->total) {
        if (p->count[0] >= p->total - p->minfill) {
            group = 1;
        } else {
            group = 0;
        }
        for (i=0; i<p->total; i++) {
            if (!p->taken[i])
                _RTreeClassify(root, i, group, p);
        }
    }

    RTREE_ASSERT(p->count[0] + p->count[1] == p->total);
    RTREE_ASSERT(p->count[0] >= p->minfill && p->count[1] >= p->minfill);
}


/**
 * Initialize one branch cell in a node.
 */
static void _RTreeInitBranch( rtree_branch_t *br )
{
    rtree_mbr_init(&(br->mbr));
    br->child = NULL;
}


static void _RTreePrintBranch( rtree_branch_t *br, int depth )
{
    rtree_mbr_print(&(br->mbr), depth);
    rtree_print_node(br->child, depth);
}


/**
 * Inserts a new data rectangle into the index structure.
 * Recursively descends tree, propagates splits back up.
 * Returns 0 if node was not split.  Old node updated.
 * If node was split, returns 1 and sets the pointer pointed to by
 * new_node to point to the new node.  Old node updated to become one of two.
 * The level argument specifies the number of steps up from the leaf
 * level to insert; e.g. a data rectangle goes in at level = 0.
 */
static int _RTreeInsertMbr(rtree_root root, rtree_mbr_t *mbr, void* tid,  rtree_node_t *node, rtree_node_t **new_node, int level)
{
    int i;
    rtree_branch_t b;
    rtree_node_t *n2;

    RTREE_ASSERT(mbr && node && new_node);
    RTREE_ASSERT(level >= 0 && level <= node->level);

    /* Still above level for insertion, go down tree recursively */
    if (node->level > level) {
        i = rtree_pick_branch(mbr, node);
        if (!_RTreeInsertMbr(root, mbr, tid, node->branch[i].child, &n2, level)) {
            /* child was not split */
            node->branch[i].mbr = rtree_mbr_union(mbr, &(node->branch[i].mbr));
            return 0;
        }

        /* child was split */
        node->branch[i].mbr = rtree_node_cover(node->branch[i].child);
        b.child = n2;
        b.mbr = rtree_node_cover(n2);

        return rtree_add_branch(root, &b, node, new_node);
    } else if (node->level == level) {
        /* Have reached level for insertion. Add mbr, split if necessary */
        b.mbr = *mbr;
        b.child = ( rtree_node_t *) tid;

        /* child field of leaves contains tid of data record */
        return rtree_add_branch(root, &b, node, new_node);
    }

    /* Not supposed to happen */
    RTREE_ASSERT (RTREE_FALSE);
    return 0;
}


/**
 * Allocate space for a node in the list used in DeletRect to
 * store Nodes that are too empty.
 */
static rtree_nodelist_t * _RTreeNewListNode(void)
{
    return (rtree_nodelist_t *) malloc(sizeof(rtree_nodelist_t));
}


static void _RTreeFreeListNode(rtree_nodelist_t *p)
{
    free(p);
}


/**
 * Add a node to the reinsertion list.  All its branches will later
 * be reinserted into the index structure.
 */
static void _RTreeReInsert(rtree_node_t *node, rtree_nodelist_t **nlpp)
{
    rtree_nodelist_t *ln = _RTreeNewListNode();
    ln->node = node;
    ln->next = *nlpp;
    *nlpp = ln;
}


/**
 * Delete a rectangle from non-root part of an index structure.
 * Called by RTreeDeleteRect.  Descends tree recursively,
 * merges branches on the way back up.
 * Returns 1 if record not found, 0 if success.
 */
static int _RTreeDeleteMbr(rtree_mbr_t *mbr, void* tid, rtree_node_t *node, rtree_nodelist_t **nlpp)
{
    int i;

    RTREE_ASSERT(mbr && node && nlpp);
    RTREE_ASSERT(node->level >= 0);

    if (node->level > 0) {
        /* not a leaf node */
        for (i = 0; i < RTREE_NODECARD; i++) {
            if (node->branch[i].child && rtree_mbr_overlap( mbr, &(node->branch[i].mbr ))) {
                if (!_RTreeDeleteMbr( mbr, tid, node->branch[i].child, nlpp)) {
                    if (node->branch[i].child->count >= RTREE_MINNODEFILL) {
                        node->branch[i].mbr = rtree_node_cover(	node->branch[i].child );
                    } else {
                        /* not enough entries in child, eliminate child node */
                        _RTreeReInsert(node->branch[i].child, nlpp);
                        rtree_cut_branch(node, i);
                    }
                    return 0;
                }
            }
        }
        return 1;
    }

    /* a leaf node */
    for (i = 0; i < RTREE_LEAFCARD; i++) {
        if ( node->branch[i].child && node->branch[i].child == (rtree_node_t *) tid ) {
            rtree_cut_branch( node, i );
            return 0;
        }
    }

    return 1;
}


static void _RTreeTabIn(int depth)
{
    int i;
    for(i=0; i<depth; i++) {
        putchar('\t');
    }
}


/**
 * Search in an index tree or subtree for all data rectangles that overlap the argument rectangle.
 * Return the number of qualifying data rects.
 */
static int _RTreeSearchMbr(rtree_node_t *node, const rtree_mbr_t *mbr, int (*searchCallback)(void*, void*), void* cbParam)
{
    /* Fix not yet tested. */
    int hitCount = 0;
    int i;

    RTREE_ASSERT(node && mbr);
    RTREE_ASSERT(node->level >= 0);

    if (node->level > 0) {
        /* this is an internal node in the tree */
        for (i=0; i<RTREE_NODECARD; i++) {
            if (node->branch[i].child && rtree_mbr_overlap(mbr, &node->branch[i].mbr))
                hitCount += _RTreeSearchMbr(node->branch[i].child, mbr, searchCallback, cbParam);
        }
    } else {
        /* this is a leaf node */
        for (i=0; i<RTREE_LEAFCARD; i++) {
            if (node->branch[i].child && rtree_mbr_overlap(mbr, &node->branch[i].mbr)) {
                hitCount++;

                /* call the user-provided callback and return if callback wants to terminate search early */
                if (searchCallback && ! searchCallback((void*)node->branch[i].child, cbParam)) {
                    return hitCount;
                }
            }
        }
    }
    return hitCount;
}


/**********************************************************************
 *								Public functions:                     *
 **********************************************************************/
/**
 * Initialize a rectangle to have all 0 coordinates.
 */
void rtree_mbr_init( rtree_mbr_t *mbr)
{
    int i;
    for (i=0; i<RTREE_SIDES; i++) {
        mbr->bound[i] = (RTREE_REAL) 0;
    }
}


/**
 * Return a mbr whose first low side is higher than its opposite side -
 * interpreted as an undefined mbr.
 */
rtree_mbr_t rtree_mbr_null(void)
{
    rtree_mbr_t mbr;
    int i;

    mbr.bound[0] = (RTREE_REAL) 1;
    mbr.bound[RTREE_DIMS] = (RTREE_REAL) - 1;
    for (i = 1; i < RTREE_DIMS; i++) {
        mbr.bound[i] = mbr.bound[i+RTREE_DIMS] = (RTREE_REAL) 0;
    }

    return mbr;
}


/**
 * Print out the data for a rectangle.
 */
 void rtree_mbr_print( rtree_mbr_t *mbr, int depth)
{
    int i;

    _RTreeTabIn(depth);
    fprintf(stdout, "mbr:\n");
    for (i = 0; i < RTREE_DIMS; i++) {
        _RTreeTabIn(depth+1);
        fprintf(stdout, "%f\t%f\n", mbr->bound[i], mbr->bound[i + RTREE_DIMS]);
    }
}


/**
 * Calculate the 2-dimensional area of a rectangle
 */
 RTREE_REAL rtree_mbr_area( rtree_mbr_t *mbr )
{
    if (RTREE_INVALIDMBR(mbr)) {
        return (RTREE_REAL) 0;
    }
    return (mbr->bound[RTREE_DIMS] - mbr->bound[0]) * (mbr->bound[RTREE_DIMS+1] - mbr->bound[1]);
}


/**
 * Calculate the n-dimensional volume of a rectangle
 */
 RTREE_REAL rtree_mbr_volume( rtree_mbr_t *mbr )
{
    int i;
    RTREE_REAL vol = (RTREE_REAL) 1;

    if (RTREE_INVALIDMBR(mbr)) {
        return (RTREE_REAL) 0;
    }
    for(i=0; i<RTREE_DIMS; i++) {
        vol *= (mbr->bound[i+RTREE_DIMS] - mbr->bound[i]);
    }
    RTREE_ASSERT(vol >= 0.0);
    return vol;
}


/**
 * Calculate the n-dimensional volume of the bounding sphere of a rectangle.
 * The exact volume of the bounding sphere for the given rtree_mbr_t.
 */
RTREE_REAL rtree_mbr_spherical_volume(rtree_mbr_t *mbr)
{
    int i;
    double sumsqr = 0, halfext, radius;
    if (RTREE_INVALIDMBR(mbr)) {
        return (RTREE_REAL) 0;
    }
    for (i=0; i<RTREE_DIMS; i++) {
        halfext = (mbr->bound[i+RTREE_DIMS] - mbr->bound[i]) / 2;
        sumsqr += halfext * halfext;
    }
    radius = sqrt(sumsqr);
    return (RTREE_REAL)(pow(radius, RTREE_DIMS) * RTREE_SPHERE_VOL);
}


/**
 * Calculate the n-dimensional surface area of a rectangle
 */
 RTREE_REAL rtree_mbr_surface_area( rtree_mbr_t *mbr )
{
    int i, j;
    RTREE_REAL sum = (RTREE_REAL) 0;

    if (RTREE_INVALIDMBR(mbr)) {
        return (RTREE_REAL) 0;
    }
    for (i=0; i<RTREE_DIMS; i++) {
        RTREE_REAL face_area = (RTREE_REAL)1;

        for (j=0; j<RTREE_DIMS; j++) {
            /* exclude i extent from product in this dimension */
            if (i != j) {
                RTREE_REAL j_extent =	mbr->bound[j+RTREE_DIMS] - mbr->bound[j];
                face_area *= j_extent;
            }
        }

        sum += face_area;
    }
    return 2 * sum;
}


/**
 * Combine two rectangles, make one that includes both.
 */
 rtree_mbr_t rtree_mbr_union( rtree_mbr_t *rc1, rtree_mbr_t *rc2 )
{
    int i, j;
    rtree_mbr_t new_rect;

    RTREE_ASSERT(rc1 && rc2);

    if (RTREE_INVALIDMBR(rc1)) {
        return *rc2;
    }
    if (RTREE_INVALIDMBR(rc2)) {
        return *rc1;
    }
    for (i = 0; i < RTREE_DIMS; i++) {
        new_rect.bound[i] = RTREE_MIN2(rc1->bound[i], rc2->bound[i]);
        j = i + RTREE_DIMS;
        new_rect.bound[j] = RTREE_MAX2(rc1->bound[j], rc2->bound[j]);
    }
    return new_rect;
}


/**
 * Decide whether two rectangles overlap.
 */
int rtree_mbr_overlap(const rtree_mbr_t *rc1, const rtree_mbr_t *rc2)
{
    if (rc1 == rc2) {
        return RTREE_TRUE;
    } else {
        int i, j;

        for (i=0; i<RTREE_DIMS; i++) {
            j = i + RTREE_DIMS;  /* index for high sides */
            if (rc1->bound[i] > rc2->bound[j] || rc2->bound[i] > rc1->bound[j]) {
                return RTREE_FALSE;
            }
        }
        return RTREE_TRUE;
    }
}


/**
 * Decide whether rectangle r is contained in rectangle s.
 */
int rtree_mbr_contained(const rtree_mbr_t *r, const rtree_mbr_t *s)
{
    int i, j, result;
    RTREE_ASSERT(r && s);

    /* undefined mbr is contained in any other */
    if (RTREE_INVALIDMBR(r)) {
        return RTREE_TRUE;
    }
    /* no mbr (except an undefined one) is contained in an undef mbr */
    if (RTREE_INVALIDMBR(s)) {
        return RTREE_FALSE;
    }
    result = RTREE_TRUE;
    for (i = 0; i < RTREE_DIMS; i++) {
        j = i + RTREE_DIMS;  /* index for high sides */
        result = result	&& r->bound[i] >= s->bound[i] && r->bound[j] <= s->bound[j];
    }
    return result;
}


/**
 * Split a node.
 * Divides the nodes branches and the extra one between two nodes.
 * Old node is one of the new ones, and one really new one is created.
 * Tries more than one method for choosing a partition, uses best result.
 */
void rtree_split_node(rtree_root root, rtree_node node, rtree_branch_t *br, rtree_node *new_node)
{
    rtree_partition_t *p;
    int level;

    RTREE_ASSERT(node && br);

    /* load all the branches into a buffer, initialize old node */
    level = node->level;
    _RTreeGetBranches(root, node, br);

    /* find partition */
    p = &root->partitions[0];

    /* Note: can't use RTREE_MINFILL(n) below since node was cleared by GetBranches() */
    _RTreeMethodZero(root, p, (level>0 ? RTREE_MINNODEFILL : RTREE_MINLEAFFILL));

    /* put branches from buffer into 2 nodes according to chosen partition	*/
    *new_node = rtree_new_node();
    (*new_node)->level = node->level = level;
    _RTreeLoadNodes(root, node, *new_node, p);

    RTREE_ASSERT(node->count+(*new_node)->count == p->total);
}


/**
 * Initialize a rtree_node_t structure.
 */
void rtree_init_node(rtree_node node)
{
    int i;
    node->count = 0;
    node->level = -1;
    for (i = 0; i < RTREE_MAXCARD; i++) {
        _RTreeInitBranch(&(node->branch[i]));
    }
}


/**
 * Make a new node and initialize to have all branch cells empty.
 */
rtree_node_t *rtree_new_node(void)
{
    rtree_node_t *node = (rtree_node_t*) malloc(sizeof(rtree_node_t));
    RTREE_ASSERT(node);
    rtree_init_node(node);
    return node;
}


void rtree_free_node(rtree_node node)
{
    RTREE_ASSERT(node);
    free(node);
}


/**
 * Print out the data in a node.
 */
 void rtree_print_node(rtree_node node, int depth)
{
    int i;
    RTREE_ASSERT(node);

    _RTreeTabIn(depth);
    fprintf(stdout, "node");

    if (node->level == 0) {
        fprintf(stdout, " LEAF");
    } else if (node->level > 0) {
        fprintf(stdout, " NONLEAF");
    } else {
        fprintf(stdout, " TYPE=?");
    }

    fprintf(stdout, "  level=%d  count=%d  address=%p\n", node->level, node->count, node);

    for (i=0; i<node->count; i++) {
        if(node->level == 0) {
            /* _RTreeTabIn(depth); */
            fprintf(stdout, "\t%d: data = %p\n", i, node->branch[i].child);
        } else {
            _RTreeTabIn(depth);
            fprintf(stdout, "branch %d\n", i);
            _RTreePrintBranch(&node->branch[i], depth+1);
        }
    }
}


/**
 * Find the smallest rectangle that includes all rectangles in branches of a node.
 */
rtree_mbr_t rtree_node_cover(rtree_node node)
{
    int i, first_time=1;
    rtree_mbr_t mbr;
    RTREE_ASSERT(node);

    rtree_mbr_init(&mbr);

    for (i = 0; i < RTREE_MAXKIDS(node); i++) {
        if (node->branch[i].child) {
            if (first_time) {
                mbr = node->branch[i].mbr;
                first_time = 0;
            } else {
                mbr = rtree_mbr_union(&mbr, &(node->branch[i].mbr));
            }
        }
    }
    return mbr;
}


/**
 * Pick a branch.  Pick the one that will need the smallest increase
 * in area to accomodate the new rectangle.  This will result in the
 * least total area for the covering rectangles in the current node.
 * In case of a tie, pick the one which was smaller before, to get
 * the best resolution when searching.
 */
int rtree_pick_branch(rtree_mbr_t *mbr, rtree_node node)
{
    rtree_mbr_t *r;
    int i, first_time = 1;
    RTREE_REAL increase, bestIncr=(RTREE_REAL)-1, area, bestArea=0;
    int best=0;
    rtree_mbr_t tmp_rect;
    RTREE_ASSERT(mbr && node);

    for (i=0; i<RTREE_MAXKIDS(node); i++) {
        if (node->branch[i].child) {
            r = &node->branch[i].mbr;
            area = rtree_mbr_spherical_volume(r);
            tmp_rect = rtree_mbr_union(mbr, r);
            increase = rtree_mbr_spherical_volume(&tmp_rect) - area;
            if (increase < bestIncr || first_time) {
                best = i;
                bestArea = area;
                bestIncr = increase;
                first_time = 0;
            } else if (increase == bestIncr && area < bestArea) {
                best = i;
                bestArea = area;
                bestIncr = increase;
            }
        }
    }
    return best;
}


/**
 * Add a branch to a node.  Split the node if necessary.
 * Returns 0 if node not split.  Old node updated.
 * Returns 1 if node split, sets *new_node to address of new node.
 * Old node updated, becomes one of two.
 */
int rtree_add_branch(rtree_root root, rtree_branch_t *br, rtree_node node, rtree_node *new_node)
{
    int i;
    RTREE_ASSERT(br && node);

    if (node->count < RTREE_MAXKIDS(node)) {
        /* split won't be necessary */
        for (i = 0; i < RTREE_MAXKIDS(node); i++) {
            /* find empty branch */
            if (node->branch[i].child == NULL) {
                node->branch[i] = *br;
                node->count++;
                break;
            }
        }
        return 0;
    }

    RTREE_ASSERT(new_node);
    rtree_split_node(root, node, br, new_node);
    return 1;
}


/**
 * Disconnect a dependent node.
 */
void rtree_cut_branch(rtree_node node, int i)
{
    RTREE_ASSERT(node && i>=0 && i<RTREE_MAXKIDS(node));
    RTREE_ASSERT(node->branch[i].child);
    _RTreeInitBranch(&(node->branch[i]));
    node->count--;
}


/**
 * Destroy (free) node recursively.
 */
 void rtree_delete_node(rtree_node node)
{
    int i;
    if (node->level > 0) {
        /* it is not leaf -> destroy childs */
        for (i = 0; i < RTREE_NODECARD; i++) {
            if (node->branch[i].child) {
                rtree_delete_node(node->branch[i].child);
            }
        }
    }
    /* Free this node */
    rtree_free_node( node );
}


/**
 * Create a new rtree index, empty. Consists of a single node.
 */
rtree_root rtree_create(const rtree_mbr_t *limit, int (*RTreeSearchCallback)(void*, void*))
{
    rtree_root_t * root;

    if (limit) {
        root = (rtree_root_t*) malloc(sizeof(rtree_root_t) + sizeof(rtree_mbr_t));
        root->mbrlimit = (rtree_mbr_t *) root->mbr;
        memcpy(root->mbrlimit, limit, sizeof(rtree_mbr_t));
    } else {
        root = (rtree_root_t*) malloc(sizeof(rtree_root_t));
        root->mbrlimit = NULL;
    }

    RTREE_ASSERT(root);
    root->rootNode = rtree_new_node();
    RTREE_ASSERT(root->rootNode);
    root->rootNode->level = 0;		/* leaf */
    root->searchCallback = RTreeSearchCallback;
    return root;
}


/**
 * Destroy a rtree root must be a root of rtree. Free all memory.
 */
void rtree_destroy(rtree_root root)
{
    rtree_delete_node (root->rootNode);
    root->rootNode = 0;
    free(root);
}


const rtree_mbr_t * rtree_get_mbr_limit(rtree_root root)
{
    return root->mbrlimit;
}


/**
 * Search in an index tree for all data rectangles that overlap the argument rectangle.
 * Return the number of qualifying data rects.
 */
int rtree_search_mbr(rtree_root root, const rtree_mbr_t *mbr, void* userarg, int (*searchCallback)(void*, void*))
{
    return  _RTreeSearchMbr(root->rootNode, mbr, (searchCallback? searchCallback : root->searchCallback), userarg);
}


/**
 * Insert a data rectangle into an index structure.
 * RTreeInsertRect provides for splitting the root;
 * returns 1 if root was split, 0 if it was not.
 * The level argument specifies the number of steps up from the leaf
 * level to insert; e.g. a data rectangle goes in at level = 0.
 * _RTreeInsertMbr does the recursion.
 */
int rtree_insert_mbr(rtree_root root, rtree_mbr_t *mbr, void* tid, int level)
{
#ifdef _DEBUG
    int i;
#endif

    rtree_node_t	*newroot;
    rtree_node_t	*newnode;
    rtree_branch_t b;

    RTREE_ASSERT(mbr && root);
    RTREE_ASSERT(level >= 0 && level <= root->rootNode->level);

#ifdef _DEBUG
    for (i=0; i<RTREE_DIMS; i++) {
        RTREE_ASSERT(mbr->bound[i] <= mbr->bound[RTREE_DIMS+i]);
    }
#endif

    /* root split */
    if (_RTreeInsertMbr(root, mbr, tid, root->rootNode, &newnode, level)) {
        newroot = rtree_new_node();  /* grow a new root, & tree taller */
        newroot->level = root->rootNode->level + 1;
        b.mbr = rtree_node_cover(root->rootNode);
        b.child = root->rootNode;
        rtree_add_branch(root, &b, newroot, NULL);
        b.mbr = rtree_node_cover(newnode);
        b.child = newnode;
        rtree_add_branch(root, &b, newroot, NULL);
        root->rootNode = newroot;
        return 1;
    }

    return 0;
}


/**
 * Delete a data rectangle from an index structure.
 * Pass in a pointer to a rtree_mbr_t, the tid of the record, ptr to ptr to root node.
 * Returns 1 if record not found, 0 if success.
 * RTreeDeleteRect provides for eliminating the root.
 */
int rtree_delete_mbr(rtree_root root, rtree_mbr_t *mbr, void* tid)
{
    int		i;
    rtree_node_t		*tmp_nptr = NULL;
    rtree_nodelist_t	*reInsertList = NULL;
    rtree_nodelist_t	*e;

    RTREE_ASSERT(mbr && root && root->rootNode);

    if (!_RTreeDeleteMbr(mbr, tid, root->rootNode, &reInsertList)) {
        /* found and deleted a data item */

        /* reinsert any branches from eliminated nodes */
        while (reInsertList) {
            tmp_nptr = reInsertList->node;

            for (i = 0; i < RTREE_MAXKIDS(tmp_nptr); i++) {
                if (tmp_nptr->branch[i].child) {
                    rtree_insert_mbr(root, &(tmp_nptr->branch[i].mbr), (void*)tmp_nptr->branch[i].child, tmp_nptr->level);
                }
            }

            e = reInsertList;
            reInsertList = reInsertList->next;
            rtree_free_node(e->node);
            _RTreeFreeListNode(e);
        }

        /* check for redundant root (not leaf, 1 child) and eliminate */
        if (root->rootNode->count == 1 && root->rootNode->level > 0) {
            for (i = 0; i < RTREE_NODECARD; i++) {
                tmp_nptr = root->rootNode->branch[i].child;
                if (tmp_nptr) {
                    break;
                }
            }
            RTREE_ASSERT(tmp_nptr);
            rtree_free_node(root->rootNode);
            root->rootNode = tmp_nptr;
        }
        return 0;
    }
    return 1;
}
/*------------------ end of rtree.c ------------------*/
