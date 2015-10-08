
#include "common/string_s.h"
#include "common/umemory.h"
#include "common/debug.h"
#include "adt/ac_bm.h"

#define acbm_smalloc    umalloc_empty
#define acbm_sfree      ufree_setnull

#define ACBM_DEBUG(fmt, args...)
#define ACBM_MSG(fmt, args...)     printf("[%s:%u]"fmt, __func__, __LINE__, ##args)
#define ACBM_ERROR(fmt, args...)   fprintf(stderr, "[E-%s:%u]"fmt, __func__, __LINE__, ##args)

#if 0
/*
 * _print_tree
 */
static void _print_tree(acbm_pattern_tree *ptree, int root_index)
{
    int i;

    if (root_index < 0) {
        return;
    }

    acbm_pattern_tree_node *root = GET_ACBM_TREE_NODE(ptree, root_index);
    acbm_pattern_tree_node *child_node = NULL;
    // print this node
    printf("ch:%c shift:%d label:%d depth:%d childs:\n", root->ch, root->GSshift, root->label, root->depth);

    for (i = 0; i < 256; i++) {
        if (root->sensitive == ACBM_NO_CASE_SENSI) {
            if (i >= 'A' && i <= 'Z') {
                continue;
            }
        }

        if (root->childs_index[i] >= 0) {
            child_node = GET_ACBM_TREE_NODE(ptree, root->childs_index[i]);
            printf("%c ", child_node->ch);
        }
    }

    printf("\n");

    // print child tree recursively
    for (i = 0; i < 256; i++) {
        if (root->childs_index[i] >= 0) {
            _print_tree(ptree, root->childs_index[i]);
        }
    }

    return;
}
#endif

#if 0
static inline int _acbm_enlarge_node_pool(acbm_pattern_tree *ptree, int new_node_num)
{
    /* enlarge node pool */
    acbm_pattern_tree_node *node_pool = acbm_scalloc(new_node_num, sizeof(acbm_pattern_tree_node));
    if (PTR_NULL(node_pool)) {
        ACBM_ERROR("error when alloc (%u:%u)\n", new_node_num, (unsigned int)sizeof(acbm_pattern_tree_node));
        return -1;
    }

    memcpy(node_pool, ptree->node_pool, ptree->alloc_node_num * sizeof(acbm_pattern_tree_node));
    acbm_sfree(ptree->node_pool);
    ptree->node_pool = node_pool;

    ptree->max_alloc_node_num = new_node_num;
    return 0;
}
#endif

static acbm_pattern_tree_node *_acbm_alloc_node(acbm_pattern_tree *ptree)
{
    if (unlikely(ptree->alloc_node_num >= ptree->max_alloc_node_num)) {
        return NULL;
    }

    acbm_pattern_tree_node *ptr = GET_ACBM_TREE_NODE(ptree, ptree->alloc_node_num);
    ptree->alloc_node_num += 1;
    return ptr;
}

static inline acbm_pattern_tree *_acbm_recondition_node_pool(acbm_pattern_tree *ptree)
{
    if (ptree->alloc_node_num > 3 * (ptree->max_alloc_node_num - ptree->alloc_node_num)) {
        return ptree;
    }

    acbm_pattern_tree *tmp = acbm_smalloc(GET_ACBM_TREE_SIZE(ptree));
    if (PTR_NULL(tmp)) {
        return ptree;
    }

    memcpy(tmp, ptree, GET_ACBM_TREE_SIZE(ptree));
    tmp->max_alloc_node_num = ptree->alloc_node_num;

    acbm_sfree(ptree);

    return tmp;
}

static void _clean_tree(acbm_pattern_tree *ptree, int root_index)
{
    int i;
    acbm_pattern_tree_node *root = NULL;

    if (unlikely(root_index < 0)) {
        return;
    }

    root = GET_ACBM_TREE_NODE(ptree, root_index);

    for (i = 0; i < 256; i++) {
        if (root && root->sensitive == ACBM_NO_CASE_SENSI) {
            if (i >= 'A' && i <= 'Z') {
                continue;
            }
        }

        if (root && ACBM_TREE_INDEX_VALID(root->childs_index[i])) {
            _clean_tree(ptree, root->childs_index[i]);
            root->childs_index[i] = -1;
        }
    }

    return;
}


/*
 * ACtree_compute_BCshifts
 */
static int ACtree_compute_BCshifts(acbm_pattern_tree *ptree)
{
    int i, j = 0;

    for (i = 0; i < 256; i++) {
        ptree->BCshift[i] = ptree->min_pattern_size;
    }

    for (i = ptree->min_pattern_size - 1; i > 0; i--) {
        for (j = 0; j < ptree->pattern_count; j++) {
            unsigned char ch = 0;
            ch = (ptree->pattern_list + j)->data[i];

            if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                ch = tolower(ch);
            }

            ptree->BCshift[ch] = i;

            if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                if (ch >= 'a' && ch <= 'z') {
                    ptree->BCshift[ch - 32] = i;
                }
            }
        }
    }

    return 0;
}

/*
 * set_GSshift
 */
static int set_GSshift(acbm_pattern_tree *ptree, unsigned char *pat, int depth, int shift)
{
    int i;
    int node_index = 0;
    acbm_pattern_tree_node *node = NULL;

    if (unlikely(ptree == NULL || ptree->root_index == -1)) {
        goto err;
    }

    node_index = ptree->root_index;

    for (i = 0; i < depth; i++) {
        node = GET_ACBM_TREE_NODE(ptree, node_index);
        node_index = node->childs_index[pat[i]];

        if (!ACBM_TREE_INDEX_VALID(node_index)) {
            goto err;
        }
    }

    node = GET_ACBM_TREE_NODE(ptree, node_index);
    node->GSshift = MIN(node->GSshift, shift);
    return 0;
err:
    return -1;
}

/*
 * compute_GSshift
 */
static int compute_GSshift(acbm_pattern_tree *ptree, unsigned char *pat1, \
                           int pat1_len, unsigned char *pat2, int pat2_len)
{
    unsigned char first_char = 0;
    int i;
    int pat1_index, pat2_index, offset;

    if (pat1 == NULL || pat2 == NULL || pat1_len < 0 || pat2_len < 0) {
        goto err;
    }

    if (pat1_len == 0 || pat2_len == 0) {
        return 0;
    }

    first_char = pat1[0];

    if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
        first_char = tolower(first_char);
    }

    /* handle char 0 separately */
    for (i = 1; i < pat2_len; i++) {
        if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
            if (tolower(pat2[i]) != first_char) {
                break;
            }
        } else {
            if (pat2[i] != first_char) {
                break;
            }
        }
    }

    set_GSshift(ptree, pat1, 1, i);
    i = 1;

    while (1) {
        // search first char in pat2
        if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
            while (i < pat2_len && tolower(pat2[i]) != first_char) {
                i++;
            }
        } else {
            while (i < pat2_len && pat2[i] != first_char) {
                i++;
            }
        }

        if (i == pat2_len) { // not found the first char
            break;
        }

        pat2_index = i;
        pat1_index = 0;
        offset = i;

        /* optimization: >min_pattern_size, not check*/
        if (offset > ptree->min_pattern_size) {
            break;
        }

        while (pat2_index < pat2_len && pat1_index < pat1_len) {
            if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                if (tolower(pat1[pat1_index]) != tolower(pat2[pat2_index])) { /*found different*/
                    break;
                }
            } else {
                if (pat1[pat1_index] != pat2[pat2_index]) { /*found different*/
                    break;
                }
            }

            pat1_index++;
            pat2_index++;
        }

        if (pat1_index == pat1_len) {
            /* pat2 contained pat1 ??? how to do */
        } else if (pat2_index == pat2_len) {
            int j;

            /* 1.pat2 contained some part of pat1 */
            for (j = pat1_index; j < pat1_len; j++) {
                set_GSshift(ptree, pat1, j + 1, offset);
            }
        } else { /* 2.found one substring in pat2 */
            set_GSshift(ptree, pat1, pat1_index + 1, offset);
        }

        i++;
    }

    return 0;
err:
    return -1;
}

/*
 * ACtree_compute_GSshifts
 */
static int ACtree_compute_GSshifts(acbm_pattern_tree *ptree)
{
    int pat_i = 0, pat_j = 0;

    for (pat_i = 0; pat_i < ptree->pattern_count; pat_i++) {
        for (pat_j = 0; pat_j < ptree->pattern_count; pat_j++) {
            unsigned char *ppat_i = (ptree->pattern_list + pat_i)->data;
            int patlen_i = (ptree->pattern_list + pat_i)->len;
            unsigned char *ppat_j = (ptree->pattern_list + pat_j)->data;
            int patlen_j = (ptree->pattern_list + pat_j)->len;
            compute_GSshift(ptree, ppat_i, patlen_i, ppat_j, patlen_j);
        }
    }

    return 0;
}

/*
 * _init_GSshifts
 */
static int _init_GSshifts(acbm_pattern_tree *ptree, int root_index, int shift)
{
    int i;

    if (unlikely(root_index < 0)) {
        return 0;
    }

    acbm_pattern_tree_node *root = GET_ACBM_TREE_NODE(ptree, root_index);

    if (root && root->label != -2) {
        root->GSshift = shift;
    }

    for (i = 0; i < 256; i++) {
        if (root && root->sensitive == ACBM_NO_CASE_SENSI) {
            if (i >= 'A' && i <= 'Z') {
                continue;
            }
        }

        if (root && ACBM_TREE_INDEX_VALID(root->childs_index[i])) {
            _init_GSshifts(ptree, root->childs_index[i], shift);
        }
    }

    return 0;
}

/*
 * ACtree_init_GSshifts
 */
static inline void ACtree_init_GSshifts(acbm_pattern_tree *ptree)
{
    _init_GSshifts(ptree, ptree->root_index, ptree->min_pattern_size);
}

/*
 * ACtree_compute_shifts
 */
static inline void ACtree_compute_shifts(acbm_pattern_tree *ptree)
{
    ACtree_compute_BCshifts(ptree);
    ACtree_init_GSshifts(ptree);
    ACtree_compute_GSshifts(ptree);
}

static int ACtreetail_build(acbm_pattern_tree *ptree, acbm_pattern_data *patterns, int npattern)
{
    int i;
    acbm_pattern_tree_node *root = NULL, *parent = NULL;
    unsigned char ch;
    int max_pattern_len = 0, min_pattern_len = ACBM_PATTERN_LEN;

    if (ptree == NULL || patterns == NULL || npattern < 0) {
        goto err;
    }

    ptree->sensitive = patterns->sensitive;
    /*alloc share mem,rechard*/
    root = _acbm_alloc_node(ptree);
    if (root == NULL) {
        goto err;
    }

    root->label = -2;
    root->depth = 0;
    root->sensitive = patterns->sensitive;
    // add pattern node to tree
    int pat_len;
    int ch_i;

    for (i = 0; i < npattern; i++) {
        pat_len = (patterns + i)->len;

        if (pat_len == 0) {
            continue;
        }/*  else if (pat_len > ACBM_PATTERN_LEN) {
            pat_len = ACBM_PATTERN_LEN;
        } */

        if (pat_len > max_pattern_len) {
            max_pattern_len = pat_len;
        }

        if (pat_len < min_pattern_len) {
            min_pattern_len = pat_len;
        }

        /* search branch adding point */
        parent = root;

        for (ch_i = pat_len - 1; ch_i >= 0; ch_i--) {
            ch = ((patterns + i)->data)[ch_i];

            if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                ch = tolower(ch);
            }

            if (!ACBM_TREE_INDEX_VALID(parent->childs_index[ch])) { /* find adding point */
                break;
            }

            parent = GET_ACBM_TREE_NODE(ptree, parent->childs_index[ch]);
        }

        if (ch_i >= 0) {
            acbm_pattern_tree_node *node = NULL;
            /* add the branch under 'parent' */
            for (; ch_i >= 0; ch_i--) {
                ch = ((patterns + i)->data)[ch_i];

                if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                    ch = tolower(ch);
                }

                /*alloc share mem,rechard*/
                node = _acbm_alloc_node(ptree);
                if (node == NULL) {
                    goto err;
                }

                node->depth = ch_i + 1;  // from 1
                node->ch = ch;
                node->label = -1;
                node->sensitive = patterns->sensitive;
                /* add to tree */
                parent->childs_index[ch] = node->index;

                if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                    /* no case sensitive */
                    if (ch >= 'a' && ch <= 'z') {
                        parent->childs_index[ch - 32] = node->index;
                    }
                }

                parent->nchild++;
                parent->one_child = ch;
                node->parent_index = parent->index;
                parent = node;
            }
        }

        /* last node remember which pattern where it from */
        parent->label = i;
    }

    ptree->pattern_list = patterns;
    ptree->pattern_count = npattern;
    ptree->root_index = root->index;
    ptree->max_depth = max_pattern_len;
    ptree->min_pattern_size = min_pattern_len;
    return 0;
err:

    /* free tree */
    if (ptree && ptree->root_index != -1) {
        _clean_tree(ptree, ptree->root_index);
        ptree->root_index = -1;
    }

    return -1;
}

static int ACtree_build(acbm_pattern_tree *ptree, acbm_pattern_data *patterns, int npattern)
{
    int i;
    acbm_pattern_tree_node *root = NULL, *parent = NULL;
    unsigned char ch = 0;
    int max_pattern_len = 0, min_pattern_len = ACBM_PATTERN_LEN;
    int ret_val = 0;

    if (ptree == NULL || patterns == NULL || npattern < 0) {
        ret_val = 0 - __LINE__;
        goto err;
    }

    ptree->sensitive = patterns->sensitive;
    // create root tree node
    /*alloc share mem,rechard*/
    root = _acbm_alloc_node(ptree);
    if (root == NULL) {
        ret_val = 0 - __LINE__;
        goto err;
    }

    root->label = -2;
    root->depth = 0;
    root->sensitive = ptree->sensitive;

    // add pattern node to tree
    for (i = 0; i < npattern; i++) {
        int pat_len;
        int ch_i;
        pat_len = (patterns + i)->len;

        if (pat_len == 0) {
            continue;
        } /* else if (pat_len > ACBM_PATTERN_LEN) {
            pat_len = ACBM_PATTERN_LEN;
        } */

        if (pat_len > max_pattern_len) {
            max_pattern_len = pat_len;
        }

        if (pat_len < min_pattern_len) {
            min_pattern_len = pat_len;
        }

        /* search branch adding point */
        parent = root;

        for (ch_i = 0; ch_i < pat_len; ch_i++) {
            ch = ((patterns + i)->data)[ch_i];

            if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                ch = tolower(ch);
            }

            if (!ACBM_TREE_INDEX_VALID(parent->childs_index[ch])) {    /* find adding point */
                break;
            }

            parent = GET_ACBM_TREE_NODE(ptree, parent->childs_index[ch]);
        }

        if (ch_i < pat_len) {
            /* add the branch under 'parent' */
            for (; ch_i < pat_len; ch_i++) {
                acbm_pattern_tree_node *node = NULL;
                ch = ((patterns + i)->data)[ch_i];

                if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                    ch = tolower(ch);
                }

                /* alloc node */
                /*alloc share mem,rechard*/
                node = _acbm_alloc_node(ptree);
                if (node == NULL) {
                    ret_val = 0 - __LINE__;
                    goto err;
                }

                node->depth = ch_i + 1;  // from 1
                node->ch = ch;
                node->label = -1;
                node->sensitive = ptree->sensitive;
                /* add to tree */
                parent->childs_index[ch] = node->index;

                if (ptree->sensitive == ACBM_NO_CASE_SENSI) {
                    /* no case sensitive */
                    if (ch >= 'a' && ch <= 'z') {
                        parent->childs_index[ch - 32] = node->index;
                    }
                }

                parent->nchild++;
                parent->one_child = ch;
                node->parent_index = parent->index;
                parent = node;
            }
        }

        /* last node remember which pattern where it from */
        parent->label = i;
    }

    ptree->pattern_list = patterns;
    ptree->pattern_count = npattern;
    ptree->root_index = root->index;
    ptree->max_depth = max_pattern_len;
    ptree->min_pattern_size = min_pattern_len;
    return 0;

err:

    /* free tree */
    if (ptree && ptree->root_index != -1) {
        _clean_tree(ptree, ptree->root_index);
        ptree->root_index = -1;
    }

    return ret_val;
}

static int _acbm_ptree_init(acbm_pattern_tree *ptree)
{
    ptree->root_index = -1;
    ptree->max_alloc_node_num = MAX_ACBM_TREE_NODE_NUM;

    int idx = 0, jdx = 0;
    for (idx = 0; idx < MAX_ACBM_TREE_NODE_NUM; ++idx) {
        for (jdx = 0; jdx < 256; ++jdx) {
            ptree->node_pool[idx].childs_index[jdx] = -1;
        }

        ptree->node_pool[idx].parent_index = -1;
        ptree->node_pool[idx].index = idx;
    }

    return 0;
}

/****************** interface func ********************************************/
static acbm_pattern_tree *acbm_init(acbm_pattern_data *patterns, int npattern, acbm_pattern_tree *ptree_prealloc, bool is_tail_init)
{
    int ret = 0;
    acbm_pattern_tree *ptree = NULL;
    if (PTR_NULL(ptree_prealloc)) {
        ptree = (acbm_pattern_tree *)acbm_smalloc(GET_INITIAL_ACBM_TREE_SIZE());
        if (!ptree) {
            goto err;
        }
        ptree->pre_alloc = 0;
    } else {
        ptree = ptree_prealloc;
        ptree->pre_alloc = 1;
    }

    ret = _acbm_ptree_init(ptree);
    if (unlikely(ret < 0)) {
        ACBM_ERROR("error when _acbm_ptree_init: %d\n", ret);
        goto err;
    }

    if (!is_tail_init) {
        ret = ACtree_build(ptree, patterns, npattern);
    } else {
        ret = ACtreetail_build(ptree, patterns, npattern);
    }
    if (ret < 0) {
        ACBM_ERROR("error when _acbm_ptree_init: %d\n", ret);
        goto err;
    }

    if (!is_tail_init) {
        ACtree_compute_shifts(ptree);
    }

    ptree->pattern_list = NULL;

    if (!ptree->pre_alloc) {
        ptree = _acbm_recondition_node_pool(ptree);
    }

    ptree->init_ok = true;

    return ptree;

err:
    if (ptree && !ptree_prealloc) {
        acbm_sfree(ptree);
    }
    return NULL;
}

/*
 * acbm_clean
 */
static void acbm_clean(acbm_pattern_tree *ptree)
{
    if (ptree == NULL || !(ptree->init_ok)) {
        return;
    }

    if (ACBM_TREE_INDEX_VALID(ptree->root_index)) {
        _clean_tree(ptree, ptree->root_index);
        ptree->root_index = -1;
    }

    if (!ptree->pre_alloc) {
        acbm_sfree(ptree);
    }
    return;
}


acbm_pattern_tree *acbm_search_init(acbm_pattern_data *patterns, int npattern, acbm_pattern_tree *ptree_prealloc)
{
    return acbm_init(patterns, npattern, ptree_prealloc, false);
}

/*
 * acbm_search
 * return value:
 * n: matched n patterns
 * 0: not matched
 * <0: error
 */
int acbm_search(acbm_pattern_tree *ptree, unsigned char *text, int text_len, int matched_indexs[], int nmax_index)
{
    int nmatched = 0;
    register int base_index = 0, cur_index = 0;
    register int real_shift = 0, gs_shift = 0, bc_shift = 0;
    acbm_pattern_tree_node *node = NULL;
    int tmp_node_index = 0;

    if (text_len < ptree->min_pattern_size || ptree->root_index < 0) {
        goto ret;
    }

    base_index = text_len - ptree->min_pattern_size;

    while (base_index >= 0) {
        cur_index = base_index;
        node = GET_ACBM_TREE_NODE(ptree, ptree->root_index);
        //ACBM_DEBUG("Checking pattern tree at %d...\n", base_index);

        while ((tmp_node_index = node->childs_index[text[cur_index]]) >= 0) {
            node = GET_ACBM_TREE_NODE(ptree, tmp_node_index);

            if (node->label >= 0) {
                /* matched a pattern! */
                ACBM_DEBUG("Matched(%d)\n", node->label);
                matched_indexs[nmatched] = node->label;

                if (++nmatched >= nmax_index) {
                    goto ret;
                }
            }

            cur_index++;

            if (cur_index >= text_len) {
                break;
            }
        }

        if (node->nchild > 0) {
            /* match fail */
            acbm_pattern_tree_node *node_child = GET_ACBM_TREE_NODE(ptree, node->childs_index[node->one_child]);
            gs_shift = node_child->GSshift;

            if (cur_index < text_len) {
                bc_shift = ptree->BCshift[text[cur_index]] + base_index - cur_index;
            } else {
                bc_shift = 1;
            }

            real_shift = gs_shift > bc_shift ? gs_shift : bc_shift;
            base_index -= real_shift;
            //ACBM_DEBUG("Failed, GSshift:%d, BCshift:%d Realshift%d\n", gs_shift, bc_shift, real_shift);
        } else {
            /* match successful, ??? to be continued */
            base_index--;
            //ACBM_DEBUG("Matched, shift %d\n", 1);
        }
    }

ret:
    return nmatched;
}



/*
 * acbm_search_pos
 * return value:
 * n: matched n patterns
 * 0: not matched
 * <0: error
 */
/*to avoid truncating pointer,change matched_indexs from unsigned ing to unsigned long,rechard*/
int acbm_search_pos(acbm_pattern_tree *ptree, unsigned char *text, int text_len, int matched_indexs[], int nmax_index, int pos[])
{
    int nmatched = 0;
    register int base_index = 0, cur_index = 0;
    register int real_shift = 0, gs_shift = 0, bc_shift = 0;
    acbm_pattern_tree_node *node = NULL;
    int tmp_node_index = 0;

    if (text_len < ptree->min_pattern_size || ptree->root_index < 0) {
        goto ret;
    }

    base_index = text_len - ptree->min_pattern_size;

    while (base_index >= 0) {
        cur_index = base_index;
        node = GET_ACBM_TREE_NODE(ptree, ptree->root_index);
        //ACBM_DEBUG("Checking pattern tree at %d...", base_index);

        while ((tmp_node_index = node->childs_index[text[cur_index]]) >= 0) {
            node = GET_ACBM_TREE_NODE(ptree, tmp_node_index);

            if (node->label >= 0) {
                /* matched a pattern! */
                ACBM_DEBUG("Matched(%d)\n", node->label);
                matched_indexs[nmatched] = node->label;
                pos[nmatched] = cur_index + 1;
                //key[nmatched] = node->label;

                if (++nmatched >= nmax_index) {
                    goto ret;
                }
            }

            cur_index++;

            if (cur_index >= text_len) {
                break;
            }
        }

        if (node->nchild > 0) {
            /* match fail */
            acbm_pattern_tree_node *node_child = GET_ACBM_TREE_NODE(ptree, node->childs_index[node->one_child]);
            gs_shift = node_child->GSshift;

            if (cur_index < text_len) {
                bc_shift = ptree->BCshift[text[cur_index]] + base_index - cur_index;
            } else {
                bc_shift = 1;
            }

            real_shift = gs_shift > bc_shift ? gs_shift : bc_shift;
            base_index -= real_shift;
            //ACBM_DEBUG("Failed, GSshift:%d, BCshift:%d Realshift%d\n", gs_shift, bc_shift, real_shift);
        } else {
            /* match successful, ??? to be continued */
            base_index--;
            //ACBM_DEBUG("Matched, shift %d\n", 1);
        }
    }

ret:
    return nmatched;
}

void acbm_search_clean(acbm_pattern_tree *ptree)
{
    acbm_clean(ptree);
}

acbm_pattern_tree *acbm_equ_init(acbm_pattern_data *patterns, int npattern, acbm_pattern_tree *ptree_prealloc)
{
    return acbm_init(patterns, npattern, ptree_prealloc, false);
}

int acbm_equ(acbm_pattern_tree *ptree, unsigned char *text, int text_len, int matched_indexs[], int nmax_index, int pos[])
{
    int nmatched = 0;
    register int cur_index = 0;
    acbm_pattern_tree_node *node = NULL;
    int tmp_node_index = 0;

    if (text_len < ptree->min_pattern_size || ptree->root_index < 0) {
        return 0;
    }

    node = GET_ACBM_TREE_NODE(ptree, ptree->root_index);

    while (cur_index < text_len && (tmp_node_index = node->childs_index[text[cur_index]]) >= 0) {
        node = GET_ACBM_TREE_NODE(ptree, tmp_node_index);

        if (node->label >= 0) {
            ACBM_DEBUG("Matched(%d)\n", node->label);
            matched_indexs[nmatched] = node->label;
            pos[nmatched] = cur_index + 1;

            if (++nmatched >= nmax_index) {
                break;
            }
        }

        cur_index++;
    }

    return nmatched;
}

void acbm_equ_clean(acbm_pattern_tree *ptree)
{
    acbm_clean(ptree);
}


acbm_pattern_tree *acbm_tailequ_init(acbm_pattern_data *patterns, int npattern, acbm_pattern_tree *ptree_prealloc)
{
    return acbm_init(patterns, npattern, ptree_prealloc, true);
}

int acbm_tailequ(acbm_pattern_tree *ptree, unsigned char *text, int text_len, int matched_indexs[], int nmax_index, int pos[])
{
    acbm_pattern_tree_node *node = NULL;
    int tmp_node_index = 0;
    int cur_index = 0;
    int index = 0;
    int nmatched = 0;

    if (text_len < ptree->min_pattern_size || ptree->root_index < 0) {
        goto ret;
    }

    cur_index = text_len - 1;
    node = GET_ACBM_TREE_NODE(ptree, ptree->root_index);

    while ((tmp_node_index = node->childs_index[text[cur_index]]) >= 0) {
        node = GET_ACBM_TREE_NODE(ptree, tmp_node_index);

        if (node->label >= 0) {
            matched_indexs[nmatched] = node->label;
            pos[nmatched] = text_len;

            if (++nmatched >= nmax_index) {
                goto ret;
            }
        }

        cur_index--;
        index++;

        if ((cur_index < 0) || (index >= ptree->max_depth)) {
            break;
        }
    }

ret:
    return nmatched;
}

void acbm_tailequ_clean(acbm_pattern_tree *ptree)
{
    acbm_clean(ptree);
}

