/************************************************
 *       Filename: cplus_define.h
 *    Description: define cplus header
 *        Created:
 *         Author: dengwei
 ************************************************/
#ifndef _CPLUS_DEFINE_H
#define _CPLUS_DEFINE_H

#ifdef __cplusplus
#define CPLUS_BEGIN extern "C" {
#else
#define CPLUS_BEGIN
#endif

#ifdef __cplusplus
#define CPLUS_END }
#else
#define CPLUS_END
#endif

#endif    // _CPLUS_DEFINE_H

