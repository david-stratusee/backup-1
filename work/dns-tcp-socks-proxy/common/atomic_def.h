/************************************************
 *       Filename: atomic_plat_def.h
 *    Description: define atomic platformless fuction
 *
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _ATOMIC_DEF_H
#define _ATOMIC_DEF_H

/********************************************
 * in x86, only kernel has atomic operation, 
 * in user-space, we only set it to int, thread-relative
 ********************************************/

#if 0
type __sync_fetch_and_add (type *ptr, type value, ...);
type __sync_fetch_and_sub (type *ptr, type value, ...);
type __sync_fetch_and_or (type *ptr, type value, ...);
type __sync_fetch_and_and (type *ptr, type value, ...);
type __sync_fetch_and_xor (type *ptr, type value, ...);
type __sync_fetch_and_nand (type *ptr, type value, ...);
//{ tmp = *ptr; *ptr op= value; return tmp; }
//{ tmp = *ptr; *ptr = ~tmp & value; return tmp; }   nand

type __sync_add_and_fetch (type *ptr, type value, ...);
type __sync_sub_and_fetch (type *ptr, type value, ...);
type __sync_or_and_fetch (type *ptr, type value, ...);
type __sync_and_and_fetch (type *ptr, type value, ...);
type __sync_xor_and_fetch (type *ptr, type value, ...);
type __sync_nand_and_fetch (type *ptr, type value, ...);
//{ *ptr op= value; return *ptr; }
//{ *ptr = ~*ptr & value; return *ptr; }   nand

bool __sync_bool_compare_and_swap (type *ptr, type oldval, type newval, ...);
type __sync_val_compare_and_swap (type *ptr, type oldval, type newval, ...);
These builtins perform an atomic compare and swap. That is, if the current value of *ptr is oldval, then write newval into *ptr.
#endif

#define atomic_get(v)               (v)
#define atomic_set(v, i)            (v) = (typeof(v))(i)
#define atomic_add(v, i)            __sync_fetch_and_add(&(v), (typeof(v))(i))
#define atomic_sub(v, i)            __sync_fetch_and_sub(&(v), (typeof(v))(i))
#define atomic_add_and_return(v, i) __sync_add_and_fetch(&(v), (typeof(v))(i))
#define atomic_sub_and_return(v, i) __sync_sub_and_fetch(&(v), (typeof(v))(i))
#define atomic_add_and_test(v, i)   (atomic_add_and_return(v, i) == 0)
#define atomic_sub_and_test(v, i)   (atomic_sub_and_return(v, i) == 0)
#define atomic_test_and_set(v, old_val, new_value)    __sync_bool_compare_and_swap(&(v), (typeof(v))(old_val), (typeof(v))(new_value))

#define atomic_init(v)              atomic_set(v, 0)
#define atomic_inc(v)               atomic_add(v, 1)
#define atomic_inc_and_return(v)    atomic_add_and_return(v, 1)
#define atomic_dec(v)               atomic_sub(v, 1)
#define atomic_dec_and_return(v)    atomic_sub_and_return(v, 1)
#define atomic_inc_and_test(v)      atomic_add_and_test(v, 1)
#define atomic_dec_and_test(v)      (v <= 0 || atomic_sub_and_test(v, 1))

#endif    // _ATOMIC_DEF_H

