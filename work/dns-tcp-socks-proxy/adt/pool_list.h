/************************************************
 *       Filename: pool_list.h
 *    Description: 
 *        Created: 2014-09-29 13:40
 *         Author: dengwei david@stratusee.com
 ************************************************/

#ifndef _POOL_LIST_H
#define _POOL_LIST_H

#include "common/types.h"
#include "common/umemory.h"

typedef struct _pool32_list_t {
    uint32_t head;
    uint32_t tail;
    int32_t length;
#if __WORDSIZE == 64
    uint32_t resv_32;
#endif
} pool32_list_t;   /* -- end of pool_list_t -- */
#define INVALID_OFFSET_32 ((uint32_t)(-1))

typedef struct _pool16_list_t {
    uint16_t head;
    uint16_t tail;
    int16_t length;
    uint16_t resv_16;
} pool16_list_t;   /* -- end of pool16_list_t -- */
#define INVALID_OFFSET_16 ((uint16_t)(-1))

#define PL_UNDEFINE(pool_list)                ((typeof((pool_list).head))(-1))
#define POOL_LIST_IDVALID(pool_list, offset)  (offset == PL_UNDEFINE(pool_list))

#define POOL_LIST_INIT(pool_list)               \
do {                                            \
    (pool_list).head = PL_UNDEFINE(pool_list); \
    (pool_list).tail = PL_UNDEFINE(pool_list); \
    (pool_list).length = 0;                    \
} while (0)

#define POOL_LIST_SIZE(pool_list)   \
    ((uint32_t)((pool_list).length))
#define POOL_LIST_EMPTY(pool_list)  \
    ((pool_list).length == 0)

/*-------------------------------------------
 * get data
 -------------------------------------------*/
#define POOL_LIST_GET_NODE(ptr_ret, node_id, mp_get_from_id_func, pool_head) \
({                                                                                      \
    ptr_ret = (node_id != (typeof(node_id))(-1)                                        \
                ? ((typeof(*(ptr_ret)) *)mp_get_from_id_func(pool_head, node_id))       \
                : NULL);                                                                \
    ptr_ret;                                                                            \
 })

#define POOL_LIST_GET_HEAD(ptr_ret, pool_list, mp_get_from_id_func, pool_head)   \
    POOL_LIST_GET_NODE(ptr_ret, (pool_list).head, mp_get_from_id_func, pool_head)
#define POOL_LIST_GET_TAIL(ptr_ret, pool_list, mp_get_from_id_func, pool_head)   \
    POOL_LIST_GET_NODE(ptr_ret, (pool_list).tail, mp_get_from_id_func, pool_head)

/********************************************
 * COMMON POOL LIST FUNCTION
 ********************************************/

#define COMMON_GET_NEXT(node)   (node)->next

/* add to the tail of pool_list */
#define POOL_LIST_ADD_TAIL(pool_list, insert_id, insert_node, GET_NEXT_OFFSET, mp_get_from_id_func, pool_head) \
({                                                                                                             \
    bool enqueue_ok_##pool_head = true;                                                                        \
    if ((pool_list).length > 0) {                                                                             \
        typeof(*(insert_node)) *__tail_node = mp_get_from_id_func(pool_head, (pool_list).tail);                 \
        if (PTR_NOT_NULL(__tail_node)) {                                                                         \
            (pool_list).tail = GET_NEXT_OFFSET(__tail_node) = insert_id;                                        \
            GET_NEXT_OFFSET(insert_node) = PL_UNDEFINE(pool_list);                                             \
            (pool_list).length++;                                                                             \
        } else {                                                                                               \
            enqueue_ok_##pool_head = false;                                                                    \
        }                                                                                                      \
    } else {                                                                                                   \
        (pool_list).head = (pool_list).tail = insert_id;                                                     \
        GET_NEXT_OFFSET(insert_node) = PL_UNDEFINE(pool_list);                                                 \
        (pool_list).length++;                                                                                 \
    }                                                                                                          \
                                                                                                               \
    enqueue_ok_##pool_head;                                                                                    \
})

/* remove from the head of pool_list */
#define POOL_LIST_DEL_HEAD(pool_list, dequeue_node, GET_NEXT_OFFSET, mp_get_from_id_func, pool_head)    \
({                                                                                                      \
    bool dequeue_ok_##pool_head = true;                                                                 \
    if ((pool_list).length > 0) {                                                                      \
        dequeue_node = (typeof(*(dequeue_node)) *)mp_get_from_id_func(pool_head, (pool_list).head);    \
        if (PTR_NOT_NULL(dequeue_node)) {                                                               \
            (pool_list).head = GET_NEXT_OFFSET(dequeue_node);                                          \
            (pool_list).length--;                                                                      \
            if ((pool_list).length == 0) {                                                             \
                (pool_list).head = (pool_list).tail = PL_UNDEFINE(pool_list);                         \
            }                                                                                           \
            GET_NEXT_OFFSET(dequeue_node) = PL_UNDEFINE(pool_list);                                     \
        } else {                                                                                        \
            dequeue_ok_##pool_head = false;                                                             \
        }                                                                                               \
    } else {                                                                                            \
        dequeue_ok_##pool_head = false;                                                                 \
        dequeue_node = NULL;                                                                            \
    }                                                                                                   \
                                                                                                        \
    dequeue_ok_##pool_head;                                                                             \
})

/* add to the head of pool_list */
#define POOL_LIST_ADD_HEAD(pool_list, insert_id, insert_node, GET_NEXT_OFFSET) \
do {                                                                           \
    GET_NEXT_OFFSET(insert_node) = (pool_list).head;                          \
    (pool_list).head = insert_id;                                             \
    if ((pool_list).length == 0) {                                            \
        (pool_list).tail = insert_id;                                         \
    }                                                                          \
    (pool_list).length++;                                                     \
} while (0)

#define POOL_LIST_GET_NEXT(ptr_ret, cur_node, GET_NEXT_OFFSET, mp_get_from_id_func, pool_head)  \
    POOL_LIST_GET_NODE(ptr_ret, GET_NEXT_OFFSET(cur_node), mp_get_from_id_func, pool_head)

/*-------------------------------------------
 * doing action
 -------------------------------------------*/
#define POOL_LIST_DELETE_NODE(pool_list, del_ptr, pre_delptr, GET_NEXT_OFFSET, mp_get_id_func, pool_head) \
do {                                                                                                      \
    if (unlikely((pool_list).length <= 0)) {                                                             \
        break;                                                                                            \
    }                                                                                                     \
    if (pre_delptr) {                                                                                     \
        GET_NEXT_OFFSET(pre_delptr) = GET_NEXT_OFFSET(del_ptr);                                           \
        if (GET_NEXT_OFFSET(del_ptr) == PL_UNDEFINE(pool_list)) {                                         \
            /* delete the last node */                                                                    \
            (pool_list).tail = mp_get_id_func(pool_head, pre_delptr);                                    \
        }                                                                                                 \
    } else {                                                                                              \
        /* delete the first node */                                                                       \
        (pool_list).head = GET_NEXT_OFFSET(del_ptr);                                                     \
        if (GET_NEXT_OFFSET(del_ptr) == PL_UNDEFINE(pool_list)) {                                         \
            /* no node now */                                                                             \
            (pool_list).tail = PL_UNDEFINE(pool_list);                                                   \
        }                                                                                                 \
    }                                                                                                     \
                                                                                                          \
    GET_NEXT_OFFSET(del_ptr) = PL_UNDEFINE(pool_list);                                                    \
    (pool_list).length--;                                                                                \
} while (0)

#define POOL_LIST_INSERT_NODE(pool_list, ins_ptr, pre_insptr, GET_NEXT_OFFSET, mp_get_id_func, pool_head) \
do {                                                                                                      \
    uint32_t pool_id = mp_get_id_func(pool_head, ins_ptr);                                                \
    if (pre_insptr) {                                                                                     \
        GET_NEXT_OFFSET(ins_ptr) = GET_NEXT_OFFSET(pre_insptr);                                           \
        if (GET_NEXT_OFFSET(pre_insptr) == PL_UNDEFINE(pool_list)) {                                      \
            (pool_list).tail = pool_id;                                                                  \
        }                                                                                                 \
        GET_NEXT_OFFSET(pre_insptr) = pool_id;                                                            \
    } else {                                                                                              \
        GET_NEXT_OFFSET(ins_ptr) = (pool_list).head;                                                     \
        if ((pool_list).length == 0) {                                                                   \
            (pool_list).tail = pool_id;                                                                  \
        }                                                                                                 \
        (pool_list).head = pool_id;                                                                      \
    }                                                                                                     \
    (pool_list).length++;                                                                                \
} while (0)

/********************************************
 * for queue
 ********************************************/
#define PLIST_ENQUEUE POOL_LIST_ADD_TAIL
#define PLIST_DEQUEUE POOL_LIST_DEL_HEAD

#define PLIST_GET_NEW POOL_LIST_GET_TAIL
#define PLIST_GET_OLD POOL_LIST_GET_HEAD
/********************************************
 * for stack, only head, no tail
 ********************************************/
typedef struct _pstack32_list_t {
    uint32_t head;
    int32_t length;
} pstack32_list_t;   /* -- end of pool_list_t -- */

typedef struct _pstack16_list_t {
    uint16_t head;
    int16_t length;
#if __WORDSIZE == 64
    uint32_t resv_32;
#endif
} pstack16_list_t;   /* -- end of pstack16_list_t -- */

#define PSTACK_LIST_INIT(pool_list)               \
do {                                            \
    (pool_list).head = PL_UNDEFINE(pool_list); \
    (pool_list).length = 0;                    \
} while (0)

/* remove from the head of pool_list */
#define PSTACK_POP(pool_list, pop_node, GET_NEXT_OFFSET, mp_get_from_id_func, pool_head)     \
({                                                                                           \
    bool pop_ok_##pool_head = true;                                                          \
    if ((pool_list).length > 0) {                                                           \
        pop_node = (typeof(*(pop_node)) *)mp_get_from_id_func(pool_head, (pool_list).head); \
        if (PTR_NOT_NULL(pop_node)) {                                                        \
            (pool_list).head = GET_NEXT_OFFSET(pop_node);                                   \
            (pool_list).length--;                                                           \
            if ((pool_list).length == 0) {                                                  \
                (pool_list).head = PL_UNDEFINE(pool_list);                                  \
            }                                                                                \
            GET_NEXT_OFFSET(pop_node) = PL_UNDEFINE(pool_list);                              \
        } else {                                                                             \
            pop_ok_##pool_head = false;                                                      \
        }                                                                                    \
    } else {                                                                                 \
        pop_ok_##pool_head = false;                                                          \
        pop_node = NULL;                                                                     \
    }                                                                                        \
                                                                                             \
    pop_ok_##pool_head;                                                                      \
})

/* add to the head of pool_list */
#define PSTACK_PUSH(pool_list, insert_id, insert_node, GET_NEXT_OFFSET) \
do {                                                                    \
    GET_NEXT_OFFSET(insert_node) = (pool_list).head;                   \
    (pool_list).head = insert_id;                                      \
    (pool_list).length++;                                              \
} while (0)
#define PSTACK_GET_NEW POOL_LIST_GET_HEAD

/********************************************
 * COMMON POOL DUAL-LIST FUNCTION
 ********************************************/
typedef struct _pool32_dlist_node_t {
    uint32_t prev;
    uint32_t next;
} pool32_dlist_node_t;   /* -- end of pool_dlist_node_t -- */
#define COMMON_GET_DLIST_NODE(node) (node)->dlist_node
#define INVALID_DLIST32_NODE ((pool32_dlist_node_t){INVALID_OFFSET_32, INVALID_OFFSET_32})

typedef struct _pool16_dlist_node_t {
    uint16_t prev;
    uint16_t next;
#if __WORDSIZE == 64
    uint32_t resv_32;
#endif
} pool16_dlist_node_t;   /* -- end of pool16_dlist_node_t -- */
#define INVALID_DLIST16_NODE ((pool16_dlist_node_t){INVALID_OFFSET_16, INVALID_OFFSET_16, 0})

/* add to the tail of pool_list */
#define POOL_DLIST_ADD_TAIL(pool_list, insert_id, insert_node, GET_DLIST_NODE, mp_get_from_id_func, pool_head)   \
({                                                                                                               \
    bool enqueue_ok_##pool_head = true;                                                                          \
    if ((pool_list).length > 0) {                                                                               \
        typeof(*(insert_node)) *__tail_node = mp_get_from_id_func(pool_head, (pool_list).tail);                 \
        if (PTR_NOT_NULL(__tail_node)) {                                                                         \
            GET_DLIST_NODE(insert_node).next = PL_UNDEFINE(pool_list);                                           \
            GET_DLIST_NODE(insert_node).prev = (pool_list).tail;                                                \
            (pool_list).tail = GET_DLIST_NODE(__tail_node).next = insert_id;                                    \
            (pool_list).length++;                                                                               \
        } else {                                                                                                 \
            enqueue_ok_##pool_head = false;                                                                      \
        }                                                                                                        \
    } else {                                                                                                     \
        (pool_list).head = (pool_list).tail = insert_id;                                                       \
        GET_DLIST_NODE(insert_node).next = GET_DLIST_NODE(insert_node).prev = PL_UNDEFINE(pool_list);            \
        (pool_list).length++;                                                                                   \
    }                                                                                                            \
                                                                                                                 \
    enqueue_ok_##pool_head;                                                                                      \
})

/* remove from the head of pool_list */
#define POOL_DLIST_DEL_HEAD(pool_list, dequeue_node, GET_DLIST_NODE, mp_get_from_id_func, pool_head)                \
({                                                                                                                  \
    bool dequeue_ok_##pool_head = true;                                                                             \
    if ((pool_list).length > 0) {                                                                                  \
        POOL_LIST_GET_HEAD(dequeue_node, pool_list, mp_get_from_id_func, pool_head);                                \
        if (dequeue_node) {                                                                                         \
            typeof(*(dequeue_node)) *_sechead_node;                                                                 \
            (pool_list).head = GET_DLIST_NODE(dequeue_node).next;                                                  \
            POOL_LIST_GET_HEAD(_sechead_node, pool_list, mp_get_from_id_func, pool_head);                           \
            (pool_list).length--;                                                                                  \
            if ((pool_list).length == 0) {                                                                         \
                (pool_list).head = (pool_list).tail = PL_UNDEFINE(pool_list);                                     \
            } else {                                                                                                \
                GET_DLIST_NODE(_sechead_node).prev = PL_UNDEFINE(pool_list);                                        \
            }                                                                                                       \
            GET_DLIST_NODE(dequeue_node).next = GET_DLIST_NODE(dequeue_node).prev = PL_UNDEFINE(pool_list);         \
        } else {                                                                                                    \
            dequeue_ok_##pool_head = false;                                                                         \
        }                                                                                                           \
    } else {                                                                                                        \
        dequeue_ok_##pool_head = false;                                                                             \
        dequeue_node = NULL;                                                                                        \
    }                                                                                                               \
                                                                                                                    \
    dequeue_ok_##pool_head;                                                                                         \
})

/* add to the head of pool_list */
#define POOL_DLIST_ADD_HEAD(pool_list, insert_id, insert_node, GET_DLIST_NODE, mp_get_from_id_func, pool_head) \
do {                                                                                                           \
    typeof(*(insert_node)) *_head_node = NULL;                                                                 \
    POOL_LIST_GET_HEAD(_head_node, pool_list, mp_get_from_id_func, pool_head);                                 \
    GET_DLIST_NODE(insert_node).prev = PL_UNDEFINE(pool_list);                                                 \
    GET_DLIST_NODE(insert_node).next = (pool_list).head;                                                      \
    if (_head_node) {                                                                                          \
        GET_DLIST_NODE(_head_node).prev = insert_id;                                                          \
    }                                                                                                          \
    (pool_list).head = insert_id;                                                                             \
    if ((pool_list).length == 0) {                                                                            \
        (pool_list).tail = insert_id;                                                                         \
    }                                                                                                          \
    (pool_list).length++;                                                                                     \
} while (0)

/********************************************
 * for queue
 ********************************************/
#define PDLIST_ENQUEUE POOL_DLIST_ADD_TAIL
#define PDLIST_DEQUEUE POOL_DLIST_DEL_HEAD

#define PDLIST_GET_NEW POOL_LIST_GET_TAIL
#define PDLIST_GET_OLD POOL_LIST_GET_HEAD
/********************************************
 * for stack
 ********************************************/
#define PDSTACK_POP  POOL_DLIST_DEL_HEAD
#define PDSTACK_PUSH POOL_DLIST_ADD_HEAD

#define PDSTACK_GET_NEW POOL_DLIST_GET_HEAD

#define POOL_DLIST_GET_NEXT(ptr_ret, cur_node, GET_DLIST_NODE, mp_get_from_id_func, pool_head)  \
    POOL_LIST_GET_NODE(ptr_ret, GET_DLIST_NODE(cur_node).next, mp_get_from_id_func, pool_head)
#define POOL_DLIST_GET_PREV(ptr_ret, cur_node, GET_DLIST_NODE, mp_get_from_id_func, pool_head)  \
    POOL_LIST_GET_NODE(ptr_ret, GET_DLIST_NODE(cur_node).prev, mp_get_from_id_func, pool_head)

/*-------------------------------------------
 * doing action
 -------------------------------------------*/
#define POOL_DLIST_DELETE_NODE(pool_list, del_ptr, GET_DLIST_NODE, mp_get_from_id_func, pool_head)   \
do {                                                                                                 \
    if (unlikely((pool_list).length <= 0)) {                                                        \
        break;                                                                                       \
    }                                                                                                \
    typeof(*(del_ptr)) *pre_delptr = NULL, *sub_delptr = NULL;                                       \
    if (GET_DLIST_NODE(del_ptr).prev != PL_UNDEFINE(pool_list)) {                                    \
        POOL_DLIST_GET_PREV(pre_delptr, del_ptr, GET_DLIST_NODE, mp_get_from_id_func, pool_head);    \
    }                                                                                                \
                                                                                                     \
    if (GET_DLIST_NODE(del_ptr).next != PL_UNDEFINE(pool_list)) {                                    \
        POOL_DLIST_GET_NEXT(sub_delptr, del_ptr, GET_DLIST_NODE, mp_get_from_id_func, pool_head);    \
    }                                                                                                \
                                                                                                     \
    if (pre_delptr) {                                                                                \
        GET_DLIST_NODE(pre_delptr).next = GET_DLIST_NODE(del_ptr).next;                              \
    } else {                                                                                         \
        /* delete the first node */                                                                  \
        (pool_list).head = GET_DLIST_NODE(del_ptr).next;                                            \
    }                                                                                                \
                                                                                                     \
    if (sub_delptr) {                                                                                \
        GET_DLIST_NODE(sub_delptr).prev = GET_DLIST_NODE(del_ptr).prev;                              \
    } else {                                                                                         \
        /* no node now */                                                                            \
        (pool_list).tail = GET_DLIST_NODE(del_ptr).prev;                                            \
    }                                                                                                \
                                                                                                     \
	                                                                                                 \
    GET_DLIST_NODE(del_ptr).prev = GET_DLIST_NODE(del_ptr).next = PL_UNDEFINE(pool_list);            \
    (pool_list).length--;                                                                           \
} while (0)

#define POOL_DLIST_INSERT_AFTER(pool_list, ins_ptr, pre_insptr, GET_DLIST_NODE, mp_get_from_id_func, mp_get_id_func, pool_head) \
do {                                                                                                                            \
    typeof(*(ins_ptr)) *sub_insptr;                                                                                             \
    uint32_t pool_id = mp_get_id_func(pool_head, ins_ptr);                                                                      \
    if (pre_insptr) {                                                                                                           \
        GET_DLIST_NODE(ins_ptr).prev = mp_get_id_func(pool_head, pre_insptr);                                                   \
        GET_DLIST_NODE(ins_ptr).next = GET_DLIST_NODE(pre_insptr).next;                                                         \
        GET_DLIST_NODE(pre_insptr).next = pool_id;                                                                              \
        if (GET_DLIST_NODE(pre_insptr).next != PL_UNDEFINE(pool_list)) {                                                        \
            /* not last node */                                                                                                 \
            sub_insptr = ((typeof(*(ins_ptr)) *)mp_get_from_id_func(pool_head, GET_DLIST_NODE(pre_insptr).next));               \
            GET_DLIST_NODE(sub_insptr).prev = pool_id;                                                                          \
        } else {                                                                                                                \
            (pool_list).tail = pool_id;                                                                                        \
        }                                                                                                                       \
    } else {                                                                                                                    \
        GET_DLIST_NODE(ins_ptr).prev = PL_UNDEFINE(pool_list);                                                                  \
        GET_DLIST_NODE(ins_ptr).next = (pool_list).head;                                                                       \
        POOL_LIST_GET_HEAD(sub_insptr, pool_list, mp_get_from_id_func, pool_head);                                              \
        (pool_list).head = pool_id;                                                                                            \
        if ((pool_list).length > 0) {                                                                                          \
            GET_DLIST_NODE(sub_insptr).prev = pool_id;                                                                          \
        } else {                                                                                                                \
            (pool_list).tail = pool_id;                                                                                        \
        }                                                                                                                       \
    }                                                                                                                           \
    (pool_list).length++;                                                                                                      \
} while (0)

/********************************************
 * for consume and produce
 ********************************************/
#define CAN_PRODUCE(produce_index, consume_index, list_len) \
    UINT_LT(produce_index, (consume_index) + (list_len))

#define CAN_CONSUME(produce_index, consume_index) \
    UINT_LT(consume_index, produce_index)

#endif   /* -- #ifndef _POOL_LIST_H -- */
