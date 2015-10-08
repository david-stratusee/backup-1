#ifndef _HASH_H
#define _HASH_H

#include <stdlib.h>
#include "common/types.h"
#include "common/cmlib_lock.h"
#include "common/optimize.h"

typedef uint32_t hkey_t;

/* type definition */
typedef int (*hash_equal_func) (hkey_t key, void *data_ht, void *arg);

typedef struct {
    int collisiontimes_find;
    int collisiontimes;
    int collisiontimes_put;
} hash_stat_t;

// hash table head
typedef struct _htable_item_t {
    int head_index;        // idx
    int element_counter;    // counter of collision nodes
} htable_item_t;

// data node
typedef struct _hpool_item_t {
    int h_index;        // �ýڵ����ĸ�hash_table, 
    // ���ڿ��ٵĴ�pool_item�ж�λ��table
    // ��ֵΪ-1����ʾ�ýڵ���free��
    int prev;
    int next;

    hkey_t ht_key;        // ע������ʹ�õ���������
    void *ht_data;        // table of data
} hpool_item_t;

// �±�һ��ʹ��int����Ϊ���Ա���ֻ��0x6fffffff��С���ڴ��������õ�, ����int�㹻��
// key �� uint32_t, ��Ϊ������ ip ����
/* structures */
typedef struct _hash_t {
    int h_items;
    int h_max_items;
    int need_lock;
    int avail_num;        // free number

    htable_item_t *h_table;
    hpool_item_t *pool_table;    // the type of each node is pool_item

    int avail_head;        // free head

    rwlock_t h_lock;

#ifdef DEBUG_HASH_STAT
    hash_stat_t stat;
#endif
} hash_t;

/********************************************************************
 *               inline function
 ********************************************************************/

/*********************
 * lock function
 *********************/
static inline void hash_lock_init(hash_t * h)
{
    if (h->need_lock) {
        rwlock_init(h->h_lock);
    }
}

static inline void hash_read_lock(hash_t * h)
{
    if (likely(h->need_lock)) {
        rw_read_lock(h->h_lock);
    }
}
static inline void hash_read_unlock(hash_t * h)
{
    if (likely(h->need_lock)) {
        rw_read_unlock(h->h_lock);
    }
}

static inline void hash_write_lock(hash_t * h)
{
    if (likely(h->need_lock)) {
        rw_write_lock(h->h_lock);
    }
}
static inline void hash_write_unlock(hash_t * h)
{
    if (likely(h->need_lock)) {
        rw_write_unlock(h->h_lock);
    }
}

/*****************************
 * hash information function
 *****************************/
/************************************************
 *         Name: hash_count
 *  Description: �鿴��ǰhash���е���Ŀ������
 *         
 *       Return: item_num
 ************************************************/
static inline int hash_count(hash_t * h)
{
    int retval = 0;

    hash_read_lock(h);
    retval = h->h_items;
    hash_read_unlock(h);
    return retval;
}

/************************************************
 *         Name: hash_full
 *  Description: hash�Ƿ�����
 *         
 *       Return: 1: full; 0: not full
 ************************************************/
static inline int hash_full(hash_t * h)
{
    int retval = 0;

    hash_read_lock(h);
    retval = (h->h_items >= h->h_max_items);
    hash_read_unlock(h);
    return retval;
}

/********************************************************************
 *           �ӿڶ���
 ********************************************************************/

/************************************************
 *         Name: choose_table_size
 *  Description: ��ò�С��seed����С����
 *         
 *       Return: 
 ************************************************/
int choose_table_size(int seed);

/**************************************************************************************
����:
    ��ʼ��hash_t *h

����:
    int max_items : ������ϣ����hash��ĳ���
    hash_equal_func eqfunc : �ɵ�����ע�����֤����, 
                 ��֤�������ط�0ֵ������֤�ɹ�,����0ֵ������֤���ɹ�

����ֵ:
    hash_t *h : ����hash���������Ϣ
    NULL: ʧ��
***************************************************************************************/
hash_t *hash_init(int max_items, int need_lock);

/**************************************************************************************
����:    �ͷ�hash����ռ���ڴ�ռ䡣
����:     ����hash���������Ϣ�����ݽṹ
**************************************************************************************/
void hash_free(hash_t * h);

/**************************************************************************************
hash_put()

����:
    ��hash���в���һ����Ŀ,����__hash_put(),__hash_put()�������
    ����ĺ��Ĺ���

����:
    h : hash�����Ϣ
    key : �������key
    data : �������key���Ӧ������ָ�� �����ָ��Ҳ�洢��hash�����Ŀ��

����ֵ:
    0  : ��hash����putһ��key,�ɹ�
    -2 : ��hash����putһ��key,���ɹ�;
         ��Ϊhash�����Ѿ����˸�keyֵ,����Ϊ����֤
         �����ߴ�������key��Ψһ�Ե�
    -1 : �����Ŀ�������Ĳ��ɹ���put
***************************************************************************************/
int hash_put(hash_t * h, hkey_t key, void *data);

/**************************************************************************************
hash_get()
����:
    �����ṩ��key��hash�����ҵ���Ӧ����Ŀ��������Ӧ��ֵ

����:
    arg  :��Ҫ�ɵ�����ָ������֤��������֤�Ĳ���

����ֵ:
    һ���ǿյ�ַ: Ҫ�ҵ�key���Ӧ����ֵ��ָ��
    NULL :�ڱ��� û���ҵ���key
***************************************************************************************/
void *hash_get(hash_t * h, hkey_t key, hash_equal_func equal_func, void *arg);

/************************************************
 *         Name: hash_getput
 *  Description: hash_get��hash_put��ͳһ��, ��������򷵻�, 
 *           ����, ����
 *         
 *       Return: 0: ����ɹ�
 *            -1: hash_get�ɹ�
 *            -2: ����ʧ��, ��hash_put
 ************************************************/
int hash_getput(hash_t * h, hkey_t key, hash_equal_func equal_func, void *data);

/************************************************
 *         Name: hash_remove_node
 *  Description: ������pool_table�е��±꣬ɾ���ڵ�
 *         rm_item_index: ��pool_table���±�
 *
 *       Return: ��ɾ���Ľڵ��data; ��ýڵ�Ϊfree, 
 *            �򷵻�NULL
 ************************************************/
void *hash_remove_node(hash_t * h, int rm_item_index);

/**************************************************************************************
����:
    ����key�ҵ���Ӧ����Ŀ����֮��hash����ɾ������������put��
    ɾ����Ԫ�����ڵĳ�ͻ���ϵ�������Ŀ��
    �书����_hash_remove_nodeʵ��

����:
    arg :��Ҫ��֤�Ĳ���

����ֵ:
    NULL :hash����û�и�key
    !NULL : hash���и�key ���ڵ�λ�õ�����ht_data
***************************************************************************************/
void *hash_remove(hash_t * h, hkey_t key, hash_equal_func equal_func, void *arg);

/************************************************
 *         Name: hash_change
 *  Description: ��λ��Ϊkey�������滻Ϊto_data
 *         
 *       Return: 0: �ɹ��滻
 *            -1: ʧ��, δ���ҵ��ڵ�
 ************************************************/
int hash_change(hash_t * h, hkey_t key, hash_equal_func equal_func, void *arg, void *to_data);

/************************************************
 *         Name: hash_replace
 *  Description: ����滻ĳ��Ԫ��;
 *         note: ���ﲻ����key�ظ��Լ��
 ************************************************/
void hash_replace(hash_t * h, hkey_t key, void *new_data, int (*get_idx) (int length));

/************************************************
 *         Name: hash_used_iterate
 *  Description: ��ʹ�õ�hash�ڵ���б���
 *         
 *       Return: 0; ���action_nodeΪNULL, �򷵻�-1
 ************************************************/
#if 0
typedef void (*action_h_table_t) (htable_item_t * h_table);
#endif
typedef int (*action_node_t) (hkey_t key, void *data, void *arg);
int hash_used_iterate(hash_t * h, action_node_t action_node, void *arg);
int hash_used_iterate_remove(hash_t * h, action_node_t action_node, void *arg, int max_delete_num);

/************************************************
 *         Name: hash_clean_iterate
 *  Description: ɾ���Ѿ�����Ľڵ���data���ڴ�
 *         note: ����hash_used_iterate
 ************************************************/
int hash_clean_iterate(hash_t * h);

/************************************************
 *         Name: hash_freenode_print
 *  Description: ��ӡ���սڵ������
 ************************************************/
int hash_freenode_print(hash_t * h);

#endif
