/************************************************
 *       Filename: hash_index_func.h
 *    Description: hash index function
 *
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _HASH_INDEX_FUNC_H
#define _HASH_INDEX_FUNC_H

#include "common/misc.h"

/* recommand: djb hash function */
static inline uint32_t djb_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = 5381;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = ((hash << 5) + hash) + (*str);
    }

    return hash;
}

static inline uint32_t djb_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t hash = 5381;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = ((hash << 5) + hash) + tolower(*str);
    }

    return hash;
}

/* rs hash function */
static inline uint32_t rs_hash(unsigned char *str, uint32_t len)
{
    uint32_t b = 378551;
    uint32_t a = 63689;
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = hash * a + (*str);
        a = a * b;
    }

    return hash;
}

static inline uint32_t rs_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t b = 378551;
    uint32_t a = 63689;
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = hash * a + tolower(*str);
        a = a * b;
    }

    return hash;
}

/* js hash function */
static inline uint32_t js_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = 1315423911;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash ^= ((hash << 5) + (*str) + (hash >> 2));
    }

    return hash;
}

static inline uint32_t js_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t hash = 1315423911;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash ^= ((hash << 5) + tolower(*str) + (hash >> 2));
    }

    return hash;
}

/*  p. j. weinberger hash function */
static inline uint32_t pjw_hash(unsigned char *str, uint32_t len)
{
    const uint32_t bitsinunsignedint = (uint32_t)(sizeof(uint32_t) * 8);
    const uint32_t threequarters = (uint32_t)((bitsinunsignedint * 3) / 4);
    const uint32_t oneeighth = (uint32_t)(bitsinunsignedint / 8);
    const uint32_t highbits = (uint32_t)(0xffffffff) << (bitsinunsignedint - oneeighth);
    uint32_t hash = 0;
    uint32_t test = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash << oneeighth) + (*str);

        if ((test = hash & highbits) != 0) {
            hash = ((hash ^ (test >> threequarters)) & (~highbits));
        }
    }

    return hash;
}

static inline uint32_t pjw_hash_case(unsigned char *str, uint32_t len)
{
    const uint32_t bitsinunsignedint = (uint32_t)(sizeof(uint32_t) * 8);
    const uint32_t threequarters = (uint32_t)((bitsinunsignedint * 3) / 4);
    const uint32_t oneeighth = (uint32_t)(bitsinunsignedint / 8);
    const uint32_t highbits = (uint32_t)(0xffffffff) << (bitsinunsignedint - oneeighth);
    uint32_t hash = 0;
    uint32_t test = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash << oneeighth) + tolower(*str);

        if ((test = hash & highbits) != 0) {
            hash = ((hash ^ (test >> threequarters)) & (~highbits));
        }
    }

    return hash;
}

/* elf hash function */
static inline uint32_t elf_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0;
    uint32_t x = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash << 4) + (*str);

        if ((x = hash & 0xf0000000l) != 0) {
            hash ^= (x >> 24);
        }

        hash &= ~x;
    }

    return hash;
}

static inline uint32_t elf_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0;
    uint32_t x = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash << 4) + tolower(*str);

        if ((x = hash & 0xf0000000l) != 0) {
            hash ^= (x >> 24);
        }

        hash &= ~x;
    }

    return hash;
}

/* bkdr hash function */
static inline uint32_t bkdr_hash(unsigned char *str, uint32_t len)
{
    uint32_t seed = 131;    /* 31 131 1313 13131 131313 etc.. */
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash * seed) + (*str);
    }

    return hash;
}

static inline uint32_t bkdr_hash_tolower(unsigned char *str, uint32_t len)
{
    uint32_t seed = 131;    /* 31 131 1313 13131 131313 etc.. */
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (hash * seed) + tolower(*str);
    }

    return hash;
}

/* sdbm hash function */
static inline uint32_t sdbm_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = (*str) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

static inline uint32_t sdbm_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = tolower(*str) + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

/* dek hash function */
static inline uint32_t dek_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = len;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
    }

    return hash;
}

static inline uint32_t dek_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t hash = len;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = ((hash << 5) ^ (hash >> 27)) ^ tolower(*str);
    }

    return hash;
}

/* bp hash function */
static inline uint32_t bp_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = hash << 7 ^ (*str);
    }

    return hash;
}

static inline uint32_t bp_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash = hash << 7 ^ tolower(*str);
    }

    return hash;
}

/* fnv hash function */
static inline uint32_t fnv_hash(unsigned char *str, uint32_t len)
{
    const uint32_t fnv_prime = 0x811c9dc5;
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash *= fnv_prime;
        hash ^= (*str);
    }

    return hash;
}

static inline uint32_t fnv_hash_case(unsigned char *str, uint32_t len)
{
    const uint32_t fnv_prime = 0x811c9dc5;
    uint32_t hash = 0;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash *= fnv_prime;
        hash ^= tolower(*str);
    }

    return hash;
}

/* ap hash function */
static inline uint32_t ap_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0xaaaaaaaa;
    uint32_t i = 0;

    for (i = 0; i < len; str++, i++) {
        hash ^= ((i & 1) == 0) ? ((hash << 7) ^ (*str) ^ (hash >> 3)) : (~((hash << 11) ^ (*str) ^ (hash >> 5)));
    }

    return hash;
}

static inline uint32_t ap_hash_case(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0xaaaaaaaa;
    uint32_t i = 0;
    char value = 0;

    for (i = 0; i < len; str++, i++) {
        value = tolower(*str);
        hash ^= ((i & 1) == 0) ? ((hash << 7) ^ value ^ (hash >> 3)) : (~((hash << 11) ^ value ^ (hash >> 5)));
    }

    return hash;
}

// we are using super fast hash, len should be the 
#define hash_get16bits(d) (*((uint16_t *)(d)))
static inline uint32_t _super_fast_hash(unsigned char *str, uint32_t len)
{
    uint32_t hash = 0;
    uint32_t rem = 0;
    uint32_t tmp = 0;

    if (!str || (int32_t)len <= 0) {
        return 0;
    }

    rem = len & 3;
    len >>= 2;

    /*  Main loop */
    for (; len > 0; len--) {
        hash += hash_get16bits(str);
        tmp = (hash_get16bits(str + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        str += (sizeof(uint16_t) << 1);
        hash += hash >> 11;
    }

    /*  Handle end cases */
    switch (rem) {
        case 3:
            hash += hash_get16bits(str);
            hash ^= hash << 16;
            hash ^= ((char)str[sizeof(uint16_t)]) << 18;
            hash += hash >> 11;
            break;

        case 2:
            hash += hash_get16bits(str);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;

        case 1:
            hash += (char)(*str);
            hash ^= hash << 10;
            hash += hash >> 1;
            break;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}
#define super_fast_hash(str, len)   _super_fast_hash((unsigned char *)(str), (uint32_t)(len))


#endif    // _HASH_INDEX_FUNC_H

