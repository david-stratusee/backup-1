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
    int h_index;        // 该节点在哪个hash_table, 
    // 便于快速的从pool_item中定位到table
    // 该值为-1，表示该节点是free的
    int prev;
    int next;

    hkey_t ht_key;        // 注意这里使用的数据类型
    void *ht_data;        // table of data
} hpool_item_t;

// 下标一律使用int，因为测试表明只有0x6fffffff大小的内存才能申请得到, 所以int足够了
// key 用 uint32_t, 因为可能有 ip 类型
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
 *  Description: 查看当前hash表中的条目的数量
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
 *  Description: hash是否满了
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
 *           接口定义
 ********************************************************************/

/************************************************
 *         Name: choose_table_size
 *  Description: 获得不小于seed的最小素数
 *         
 *       Return: 
 ************************************************/
int choose_table_size(int seed);

/**************************************************************************************
功能:
    初始化hash_t *h

参数:
    int max_items : 调用者希望的hash表的长度
    hash_equal_func eqfunc : 由调用者注册的验证函数, 
                 验证函数返回非0值代表验证成功,返回0值代表验证不成功

返回值:
    hash_t *h : 关于hash表的所有信息
    NULL: 失败
***************************************************************************************/
hash_t *hash_init(int max_items, int need_lock);

/**************************************************************************************
功能:    释放hash表所占的内存空间。
参数:     包含hash表的所有信息的数据结构
**************************************************************************************/
void hash_free(hash_t * h);

/**************************************************************************************
hash_put()

功能:
    往hash表中插入一个条目,调用__hash_put(),__hash_put()中完成了
    插入的核心功能

参数:
    h : hash表的信息
    key : 被插入的key
    data : 被插入的key相对应的数据指针 ，这个指针也存储于hash表的条目中

返回值:
    0  : 往hash表中put一个key,成功
    -2 : 往hash表中put一个key,不成功;
         因为hash表中已经有了该key值,这是为了验证
         调用者传进来的key的唯一性的
    -1 : 其他的可能引起的不成功的put
***************************************************************************************/
int hash_put(hash_t * h, hkey_t key, void *data);

/**************************************************************************************
hash_get()
功能:
    根据提供的key在hash表中找到相应的条目，返回相应的值

参数:
    arg  :需要由调用者指定的验证函数来验证的参数

返回值:
    一个非空地址: 要找的key相对应的数值的指针
    NULL :在表中 没有找到该key
***************************************************************************************/
void *hash_get(hash_t * h, hkey_t key, hash_equal_func equal_func, void *arg);

/************************************************
 *         Name: hash_getput
 *  Description: hash_get和hash_put的统一体, 如果存在则返回, 
 *           否则, 插入
 *         
 *       Return: 0: 插入成功
 *            -1: hash_get成功
 *            -2: 插入失败, 见hash_put
 ************************************************/
int hash_getput(hash_t * h, hkey_t key, hash_equal_func equal_func, void *data);

/************************************************
 *         Name: hash_remove_node
 *  Description: 根据在pool_table中的下标，删除节点
 *         rm_item_index: 在pool_table的下标
 *
 *       Return: 被删除的节点的data; 如该节点为free, 
 *            则返回NULL
 ************************************************/
void *hash_remove_node(hash_t * h, int rm_item_index);

/**************************************************************************************
功能:
    根据key找到相应的条目，将之从hash表中删除，并且重新put被
    删除的元素所在的冲突链上的所有条目。
    其功能由_hash_remove_node实现

参数:
    arg :需要验证的参数

返回值:
    NULL :hash表中没有该key
    !NULL : hash表中该key 所在的位置的数据ht_data
***************************************************************************************/
void *hash_remove(hash_t * h, hkey_t key, hash_equal_func equal_func, void *arg);

/************************************************
 *         Name: hash_change
 *  Description: 将位置为key的数据替换为to_data
 *         
 *       Return: 0: 成功替换
 *            -1: 失败, 未查找到节点
 ************************************************/
int hash_change(hash_t * h, hkey_t key, hash_equal_func equal_func, void *arg, void *to_data);

/************************************************
 *         Name: hash_replace
 *  Description: 随机替换某个元素;
 *         note: 这里不进行key重复性检查
 ************************************************/
void hash_replace(hash_t * h, hkey_t key, void *new_data, int (*get_idx) (int length));

/************************************************
 *         Name: hash_used_iterate
 *  Description: 对使用的hash节点进行遍历
 *         
 *       Return: 0; 如果action_node为NULL, 则返回-1
 ************************************************/
#if 0
typedef void (*action_h_table_t) (htable_item_t * h_table);
#endif
typedef int (*action_node_t) (hkey_t key, void *data, void *arg);
int hash_used_iterate(hash_t * h, action_node_t action_node, void *arg);
int hash_used_iterate_remove(hash_t * h, action_node_t action_node, void *arg, int max_delete_num);

/************************************************
 *         Name: hash_clean_iterate
 *  Description: 删除已经插入的节点中data的内存
 *         note: 调用hash_used_iterate
 ************************************************/
int hash_clean_iterate(hash_t * h);

/************************************************
 *         Name: hash_freenode_print
 *  Description: 打印出空节点的索引
 ************************************************/
int hash_freenode_print(hash_t * h);

#endif
