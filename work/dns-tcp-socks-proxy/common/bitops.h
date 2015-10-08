/************************************************
 *       Filename: bit_op.h
 *    Description: vshare bit operation
 *
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _BIT_OP_H
#define _BIT_OP_H

/********************************************
 * value should be (unsigned long)
 ********************************************/
#define test_bit(value, nr)    ((value) & (((typeof(value))1) << (nr)))
#define set_bit(value, nr)    value |= (((typeof(value))1) << (nr))
#define clear_bit(value, nr)    value &= (~(((typeof(value))1) << (nr)))
#define change_bit(value, nr) value ^= (((typeof(value))1) << (nr))
#define reset_bit(value, nr)    value = (((typeof(value))1) << (nr))

#define test_bit_uint(value, nr)    ((value) & (1U << (nr)))
#define set_bit_uint(value, nr)   value |= (1U << (nr))
#define clear_bit_uint(value, nr)    value &= (~(1U << (nr)))
#define change_bit_uint(value, nr) value ^= (1U << (nr))
#define reset_bit_uint(value, nr)    value = (1U << (nr))

#define test_bitmask(value, mask)   (((value) & (mask)) == (mask))

/********************************************
 * set bit, and return the former set of this bit
 ********************************************/
#define test_and_set_bit(value, nr)        \
({    \
     typeof(value) mask1_##value##_mask = (((typeof(value))1) << (nr));    \
     if (!((value) & mask1_##value##_mask)) {    \
         value |= (mask1_value##_mask);    \
         0;                \
    } else {                \
         1;                \
     }                    \
 })

/********************************************
 * clear bit, and return the former set of this bit
 ********************************************/
#define test_and_clear_bit(value, nr)    \
({    \
     typeof(value) mask2_##value##_mask = (((typeof(value))1) << (nr));    \
     if (((value) & mask2_##value##_mask)) {    \
         value &= ~(mask2_value##_mask);    \
         1;            \
    } else {            \
         0;            \
     }    \
 })

#endif    // _BIT_OP_H

