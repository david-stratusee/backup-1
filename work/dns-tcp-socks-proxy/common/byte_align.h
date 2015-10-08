#ifndef _BYTE_ALIGN_H
#define _BYTE_ALIGN_H

#include "common/types.h"

#define _GET_BIG_DATA(data, val_type)                     \
({                                                             \
     uint32_t idx     = 0;                                     \
     val_type ret_val = 0;                                     \
     for (idx = 0; idx < sizeof(val_type); ++idx) {                         \
        ret_val = ((ret_val << 8) | ((uint8_t *)(data))[idx]); \
     }                                                         \
     ret_val;                                                  \
})

#define _GET_LITTLE_DATA(data, val_type)     \
({                                                \
     uint32_t idx     = 0;                        \
     val_type ret_val = 0;                        \
     for (idx = sizeof(val_type) - 1; idx >= 0; idx--) {       \
        ret_val = ((ret_val << 8) | ((uint8_t *)(data))[idx]); \
     }                                            \
     ret_val;                                     \
})

#define GET_BIG_U16(data)    _GET_BIG_DATA(data, uint16_t)
#define GET_LITTLE_U16(data) _GET_LITTLE_DATA(data, uint16_t)
#define GET_BIG_16(data)     (int16_t)GET_BIG_U16(data)
#define GET_LITTLE_16(data)  (int16_t)GET_LITTLE_U16(data)

#define GET_BIG_U32(data)    _GET_BIG_DATA(data, uint32_t)
#define GET_LITTLE_U32(data) _GET_LITTLE_DATA(data, uint32_t)
#define GET_BIG_32(data)     (int32_t)GET_BIG_U32(data)
#define GET_LITTLE_32(data)  (int32_t)GET_LITTLE_U32(data)

#define GET_BIG_U64(data)    _GET_BIG_DATA(data, uint64_t)
#define GET_LITTLE_U64(data) _GET_LITTLE_DATA(data, uint64_t)
#define GET_BIG_64(data)     (int64_t)GET_BIG_U64(data)
#define GET_LITTLE_64(data)  (int64_t)GET_LITTLE_U64(data)

#endif

