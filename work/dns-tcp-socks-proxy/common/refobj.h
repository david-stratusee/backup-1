/******************************************************************
  Copyright (C), 1996-2012, Venus Info Tech Inc.
  Program name:
  File name: refobj.h
  Author: wang_wenyin Version: v 1.0 Date: 03/02/2012 17:53:42
  Description:  <+description+>

*******************************************************************/
#ifndef __REFOBJ_H__
#define __REFOBJ_H__

#include "common/atomic_def.h"

#define REFOBJ_INIT(refobj) \
    atomic_set(refobj, 1)

#define REFOBJ_HOLD(refobj) \
    atomic_inc(refobj)

#define REFOBJ_RELEASE(refobj)  \
    atomic_dec_and_test(refobj)

#define REFOBJ_GET_USERS(refobj)  \
    atomic_get(refobj)

#endif /* !__REFOBJ_H__ */

