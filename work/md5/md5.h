/************************************************
 *       Filename: md5.h
 *    Description: 
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _MD5_H
#define _MD5_H

#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <inttypes.h>
#include <stdbool.h>
#include <errno.h>

/* MD5 context. */
typedef struct {
    uint32_t state[4];    /* state (ABCD) */
    uint32_t count[2];    /* number of bits, modulo 2^64 (lsb first) */
    uint8_t buffer[64];   /* input buffer */
} MD5_CONTEXT;

void MD5Init(MD5_CONTEXT *context);
void MD5Update(MD5_CONTEXT *context, uint8_t *input, uint32_t inputLen);
void MD5Final(uint8_t digest[16], MD5_CONTEXT * context);
void MessageDigest(uint8_t *szInput, uint32_t inputLen, uint8_t szOutput[16]);
int md5_file(const char *path, uint8_t checksum[16]);

#endif    // _MD5_H
