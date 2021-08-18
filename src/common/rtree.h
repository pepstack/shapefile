/****************************************************************************
* RTree.H
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
* LAST MODIFY:
*    2007-11:   Initially created by ZhangLiang (cheungmine@gmail.com)
*    2008-5:    Multiple threads safe supports
*    2021-8:    Update codes and renames
*****************************************************************************/
#ifndef  RTREE_H_API_INCLUDED
#define  RTREE_H_API_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <assert.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTREE_ASSERT
  #define  RTREE_ASSERT(x)  assert(x)
#endif

#define RTREE_PTR_TO_INT(pv)      ((int) (uintptr_t) (void*) (pv))
#define RTREE_INT_TO_PTR(iv)      ((void*) (uintptr_t) (int) (iv))

#define RTREE_PTR_TO_INT64(pv)    ((int64_t) (uint64_t) (void*) (pv))
#define RTREE_INT64_TO_PTR(iv)    ((void*) (uint64_t) (int64_t) (iv))


/* RTREE_PAGESZ is normally the natural page size of the machine */
#ifndef RTREE_PAGESZ
  #define  RTREE_PAGESZ  4096
#endif

/* number of dimensions: 2, 3, 4, 5, 6 ... 20 */
#ifndef RTREE_DIMS
  #define  RTREE_DIMS    2
#endif

#define  RTREE_SIDES   ((RTREE_DIMS)*2)

#ifndef RTREE_REAL
  #define RTREE_REAL   double
#endif

/* do not change the following */
#define  RTREE_TRUE	   1
#define  RTREE_FALSE   0

#if RTREE_DIMS > 20
  #error "not enough precomputed sphere volumes"
#endif

/**
 * public type definitions
 */
typedef struct _rtree_node_t*  rtree_node;
typedef struct _rtree_root_t*  rtree_root;


typedef struct
{
    /* [Min1, Min2, ..., MinN, Max1, Max2, ..., MaxN] */
    RTREE_REAL bound[RTREE_SIDES];
} rtree_mbr_t;


typedef struct
{
    rtree_mbr_t	mbr;

    /* mbr id */
    struct _rtree_node_t *child;
} rtree_branch_t;


/**
 * Initialize a rectangle to have all 0 coordinates.
 */
void rtree_mbr_init(rtree_mbr_t *mbr);


/**
 * Return a mbr whose first low side is higher than its opposite side -
 *   interpreted as an undefined mbr.
 */
rtree_mbr_t rtree_mbr_null(void);


/**
 * Print out the data for a rectangle.
 */
void rtree_mbr_print(rtree_mbr_t *mbr, int depth);


/**
 * Calculate the 2-dimensional area of a rectangle
 */
RTREE_REAL rtree_mbr_area(rtree_mbr_t *mbr);


/**
 * Calculate the n-dimensional volume of a rectangle
 */
RTREE_REAL rtree_mbr_volume(rtree_mbr_t *mbr);


/**
 * Calculate the n-dimensional volume of the bounding sphere of a rectangle
 * The exact volume of the bounding sphere for the given rtree_mbr_t.
 */
RTREE_REAL rtree_mbr_spherical_volume(rtree_mbr_t *mbr);


/**
 * Calculate the n-dimensional surface area of a rectangle
 */
RTREE_REAL rtree_mbr_surface_area(rtree_mbr_t *mbr);


/**
 * Combine two rectangles, make one that includes both.
 */
rtree_mbr_t rtree_mbr_union(rtree_mbr_t *rc1, rtree_mbr_t *rc2);


/**
 * Decide whether two rectangles overlap.
 */
int rtree_mbr_overlap(const rtree_mbr_t *rc1, const rtree_mbr_t *rc2);


/**
 * Decide whether rectangle r is contained in rectangle s.
 */
int rtree_mbr_contained(const rtree_mbr_t *r, const rtree_mbr_t *s);


/**
 * Split a node.
 * Divides the nodes branches and the extra one between two nodes.
 * Old node is one of the new ones, and one really new one is created.
 * Tries more than one method for choosing a partition, uses best result.
 */
void rtree_split_node(rtree_root root, rtree_node node, rtree_branch_t *br, rtree_node *new_node);


/**
 * Initialize a rtree_node_t structure.
 */
void rtree_init_node( rtree_node node );


/**
 * Make a new node and initialize to have all branch cells empty.
 */
rtree_node rtree_new_node(void);


void rtree_free_node( rtree_node node );


/**
 * Print out the data in a node.
 */
void rtree_print_node(rtree_node node, int depth);


/**
 * Find the smallest rectangle that includes all rectangles in branches of a node.
 */
rtree_mbr_t rtree_node_cover(rtree_node node);


/**
 * Pick a branch.  Pick the one that will need the smallest increase
 * in area to accomodate the new rectangle.  This will result in the
 * least total area for the covering rectangles in the current node.
 * In case of a tie, pick the one which was smaller before, to get
 * the best resolution when searching.
 */
int rtree_pick_branch(rtree_mbr_t *mbr, rtree_node node);


/**
 * Add a branch to a node.  Split the node if necessary.
 * Returns 0 if node not split.  Old node updated.
 * Returns 1 if node split, sets *new_node to address of new node.
 * Old node updated, becomes one of two.
 */
int rtree_add_branch(rtree_root root, rtree_branch_t *br, rtree_node node, rtree_node *new_node);


/**
 * Disconnect a dependent node.
 */
void rtree_cut_branch(rtree_node node, int i);


/**
 * Destroy (free) node recursively.
 */
void rtree_delete_node(rtree_node node);


/**
 * Create a new rtree index, empty. Consists of a single node.
 */
rtree_root rtree_create(const rtree_mbr_t *limit, int (*RTreeSearchCallback)(void*, void*));


/**
 * Destroy a rtree root must be a root of rtree. Free all memory.
 */
void rtree_destroy(rtree_root root);


/**
 * Get saved max bound rect
 */
const rtree_mbr_t * rtree_get_mbr_limit(rtree_root root);


/**
 * Search in an index tree for all data rectangles that overlap the argument rectangle.
 * Return the number of qualifying data rects.
 */
int rtree_search_mbr(rtree_root root, const rtree_mbr_t *mbr, void* userarg, int (*searchCallback)(void*, void*));


/**
 * Insert a data rectangle into an index structure.
 * RTreeInsertRect provides for splitting the root;
 * returns 1 if root was split, 0 if it was not.
 * The level argument specifies the number of steps up from the leaf
 * level to insert; e.g. a data rectangle goes in at level = 0.
 * _RTreeInsertRect does the recursion.
 */
int rtree_insert_mbr(rtree_root root, rtree_mbr_t *data_mbr, void* data_id, int level);


/**
 * Delete a data rectangle from an index structure.
 * Pass in a pointer to a rtree_mbr_t, the tid of the record, ptr to ptr to root node.
 * Returns 1 if record not found, 0 if success.
 * RTreeDeleteRect provides for eliminating the root.
 */
int rtree_delete_mbr(rtree_root root, rtree_mbr_t *data_mbr, void* data_id);

#ifdef __cplusplus
}
#endif

#endif /* RTREE_H_API_INCLUDED */
