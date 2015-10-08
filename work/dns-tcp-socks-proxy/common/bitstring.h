
#ifndef _BITSTRING_H
#define	_BITSTRING_H

#include "common/umemory.h"

typedef	unsigned char bitstr_t;

/* internal macros */
/* byte of the bitstring bit is in */
#define	_bit_byte(bit) \
    ((bit) >> 3)

/* mask for the bit within its byte */
#define	_bit_mask(bit) \
    (1 << ((bit)&0x7))

/* external macros */
/* bytes in a bitstring of nbits bits */
#define	bitstr_size(nbits) \
    ((((nbits) - 1) >> 3) + 1)

/* allocate a bitstring */
#define	bit_alloc(nbits) \
    (bitstr_t *)ucalloc(1, \
            (uint32_t)bitstr_size(nbits) * sizeof(bitstr_t))

#define	bit_free(ptr) \
    ufree(ptr)

/* allocate a bitstring on the stack */
#define	bit_decl(name, nbits) \
    (name)[bitstr_size(nbits)]

#define bit_init(name, nbits)   \
    memset(name, 0, bitstr_size(nbits) * sizeof(bitstr_t))

/* is bit N of bitstring name set? */
#define	bit_test(name, bit) \
    ((name)[_bit_byte(bit)] & _bit_mask(bit))

/* set bit N of bitstring name */
#define	bit_set(name, bit) \
    (name)[_bit_byte(bit)] |= _bit_mask(bit)

/* clear bit N of bitstring name */
#define	bit_clear(name, bit) \
    (name)[_bit_byte(bit)] &= ~_bit_mask(bit)

/* clear bits start ... stop in bitstring */
static inline void bit_nclear(bitstr_t *name, int start, int stop)
{
    register bitstr_t *_name = name;
    register int _start = start, _stop = stop;
    register int _startbyte = _bit_byte(_start);
    register int _stopbyte = _bit_byte(_stop);

    if (_startbyte == _stopbyte) {
        _name[_startbyte] &= ((0xff >> (8 - (_start&0x7))) |
                (0xff << ((_stop&0x7) + 1)));
    } else {
        _name[_startbyte] &= 0xff >> (8 - (_start&0x7));
        while (++_startbyte < _stopbyte) _name[_startbyte] = 0;
        _name[_stopbyte] &= 0xff << ((_stop&0x7) + 1);
    }
}

/* set bits start ... stop in bitstring */
static inline void bit_nset(bitstr_t *name, int start, int stop)
{
    register bitstr_t *_name = name;
    register int _start = start, _stop = stop;
    register int _startbyte = _bit_byte(_start);
    register int _stopbyte = _bit_byte(_stop);

    if (_startbyte == _stopbyte) {
        _name[_startbyte] |= ((0xff << (_start&0x7)) &
                (0xff >> (7 - (_stop&0x7))));
    } else {
        _name[_startbyte] |= 0xff << ((_start)&0x7);
        while (++_startbyte < _stopbyte) _name[_startbyte] = 0xff;
        _name[_stopbyte] |= 0xff >> (7 - (_stop&0x7));
    }
}

/* find first bit clear in name */
static inline int bit_ffc(bitstr_t *name, int nbits)
{
    register bitstr_t *_name = name;
    register int _byte, _nbits = nbits;
    register int _stopbyte = _bit_byte(_nbits), _value = -1;

    for (_byte = 0; _byte < _stopbyte; ++_byte) {
        if (_name[_byte] != 0xff) {
            _value = _byte << 3;
            for (_stopbyte = _name[_byte]; (_stopbyte&0x1);
                    ++_value, _stopbyte >>= 1);
            break;
        }
    }

    return _value;
}

/* find first bit set in name */
static inline int bit_ffs(bitstr_t *name, int nbits)
{
    register bitstr_t *_name = name;
    register int _byte, _nbits = nbits;
    register int _stopbyte = _bit_byte(_nbits), _value = -1;

    for (_byte = 0; _byte < _stopbyte; ++_byte) {
        if (_name[_byte]) {
            _value = _byte << 3;
            for (_stopbyte = _name[_byte]; !(_stopbyte&0x1);
                    ++_value, _stopbyte >>= 1);
            break;
        }
    }

    return _value;
}

/* find first testn bit set in name */
static inline int bit_testn(bitstr_t *name, int nbits, int testn)
{
    register bitstr_t *_name = name;
    register int _byte, _nbits = nbits, _setn = 0;
    register int _stopbyte = _bit_byte(_nbits), _value;
    register unsigned int _one_value;

    for (_byte = 0; _byte < _stopbyte; ++_byte) {
        if (_name[_byte]) {
            _value = _byte << 3;
            for (_one_value = _name[_byte]; _one_value != 0; ++_value, _one_value >>= 1) {
                if ((_one_value & 0x1)) {
                    if (++_setn == testn) {
                        return (_value - testn + 1);
                    }
                } else if (_setn) {
                    _setn = 0;
                }
            }
        } else if (_setn) {
            _setn = 0;
        }
    }

    return -1;
}

#endif /* !_BITSTRING_H */
