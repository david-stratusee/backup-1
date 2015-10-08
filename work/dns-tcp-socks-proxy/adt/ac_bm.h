/***************************************************
 * Copyright (C), 1996-2013, Venus Info Tech Inc.
 * Program name:
 * @file ac_bm.h
 * @version v 1.0 
 * @date 1/29/2013 13:42
 * @description
 *****************************************/
#ifndef _AC_BM_H
#define _AC_BM_H
/**
 * @file ac_bm.h
 * @defgroup ac_bm acbm tree operation
 * @{
 */

enum CASE_SENSI_en {
    ACBM_NO_CASE_SENSI = 0,
    ACBM_CASE_SENSI = 1
};    /* -- end of enum CASE_SENSI -- */

typedef struct _acbm_pattern_tree_node {
    uint16_t index;               /**< index in the data pool */

    int16_t label;
    int16_t depth;

    int16_t GSshift;                        /**<  good suffix offset bits */
    int16_t sensitive;                        /**<  case sensitive */
    
    unsigned char ch;                    /**< node character */
    unsigned char one_child;            /**< the first child */

    int16_t nchild;                            /**< the number of children */
    int16_t parent_index;
    int16_t childs_index[256];
} acbm_pattern_tree_node;

#define ACBM_PATTERN_LEN 128
typedef struct _acbm_pattern_data {
    unsigned char *data;    //[ACBM_PATTERN_LEN];                /**< pattern data */
    int16_t len;                            /**< the len of pattern */
    int16_t sensitive;                        /**< case sensitive */
} acbm_pattern_data;

#define MAX_ACBM_TREE_NODE_NUM 65535
typedef struct _acbm_pattern_tree {
    int16_t root_index;                    /**< tree root */
    int16_t max_depth;
    int16_t min_pattern_size;
    int16_t sensitive;                     /**< case sensitive */
    int16_t BCshift[256];                  /**< bad character offset bits */
    uint16_t alloc_node_num;                /**< alloc node number in node_pool */
    uint16_t max_alloc_node_num;

    char init_ok;
    char pre_alloc;

    /* free and unuse after initial */
    int16_t pattern_count;                 /**< pattern number */
    acbm_pattern_data *pattern_list;   /**< pattern list */

    acbm_pattern_tree_node node_pool[0]; /**< node memory pool */
} acbm_pattern_tree;
#define GET_ACBM_TREE_NODE(ptree, index)     ((ptree)->node_pool + (index))
#define ACBM_TREE_INDEX_VALID(index)         (index >= 0)

#define GET_INITIAL_ACBM_TREE_SIZE()         ((int)sizeof(acbm_pattern_tree) + MAX_ACBM_TREE_NODE_NUM * sizeof(acbm_pattern_tree_node))
#define GET_ACBM_TREE_SIZE(ptree)            ((int)sizeof(acbm_pattern_tree) + (ptree)->alloc_node_num * sizeof(acbm_pattern_tree_node))

#define ACBM_INIT_OK(ptree)                  ((ptree)->init_ok)

#if 0
typedef struct _acbm_matched_info {
    int pattern_i;            /**< the ist pattern*/
    uint64_t offset;
} acbm_matched_info_t;
#endif

/*********acbm_equ***********************/
/**
 * acbm equal tree init
 * @param patterns pointer
 * @param npattern pattern number
 */
acbm_pattern_tree *acbm_equ_init(acbm_pattern_data *patterns, int npattern, acbm_pattern_tree *ptree_prealloc);
/**
 * acbm equal match
 * @param ptree tree handle
 * @param text match text
 * @param text_len length of text
 * @param matched_indexs saved match result
 * @param nmax_index max result number
 * @param pos result of matched offset
*/
int acbm_equ(acbm_pattern_tree *ptree, unsigned char *text, int text_len, \
        int matched_indexs[], int nmax_index, int pos[]);
/**
 * free and delete acbm equal tree
 * @param ptree tree handle
 */
void acbm_equ_clean(acbm_pattern_tree *ptree);


/*********acbm_tailequ***********************/
/**
 * acbm tail equal tree init
 * @param patterns pointer
 * @param npattern pattern number
 */
acbm_pattern_tree *acbm_tailequ_init(acbm_pattern_data *patterns, int npattern, acbm_pattern_tree *ptree_prealloc);
/**
 * acbm tail equal match
 * @param ptree tree handle
 * @param text match text
 * @param text_len length of text
 * @param matched_indexs saved match result
 * @param nmax_index max result number
 * @param pos result of matched offset
*/
int acbm_tailequ(acbm_pattern_tree *ptree, unsigned char *text, int text_len, \
        int matched_indexs[], int nmax_index, int pos[]);
/**
 * free and delete acbm tail equal tree
 * @param ptree tree handle
 */
void acbm_tailequ_clean(acbm_pattern_tree *ptree);


/*********acbm_search***********************/
/**
 * acbm tree init
 * @param patterns pointer
 * @param npattern pattern number
 */
acbm_pattern_tree *acbm_search_init(acbm_pattern_data *patterns, int npattern, acbm_pattern_tree *ptree_prealloc);
/**
 * acbm match
 * @param ptree tree handle
 * @param text match text
 * @param text_len length of text
 * @param matched_indexs saved match result
 * @param nmax_index max result number
 * @param pos result of matched offset
*/
int acbm_search(acbm_pattern_tree *ptree, unsigned char *text, int text_len, \
        int matched_indexs[], int nmax_index);
/**
 * acbm match
 * @param ptree tree handle
 * @param text match text
 * @param text_len length of text
 * @param matched_indexs saved match result
 * @param nmax_index max result number
 * @param pos result of matched offset
*/
int acbm_search_pos(acbm_pattern_tree *ptree, unsigned char *text, int text_len, \
        int matched_indexs[], int nmax_index, int pos[]);

/**
 * free and delete acbm tree
 * @param ptree tree handle
 */
void acbm_search_clean(acbm_pattern_tree *ptree);

#endif
