#ifndef _STDDEF1_H_
#define _STDDEF1_H_
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

#ifndef container_of
#define container_of(ptr, type, member) ({			\
      const typeof(((type *) NULL)->member ) *__mptr = (ptr);	\
      (type *) ((char *) __mptr - offsetof(type,member));	\
    })
#endif

#define MIN(x, y) ({                            \
      typeof(x) _min1 = (x);                    \
      typeof(y) _min2 = (y);                    \
      (void) (&_min1 == &_min2);                \
      _min1 < _min2 ? _min1 : _min2; })

#define MAX(x, y) ({                            \
      typeof(x) _max1 = (x);                    \
      typeof(y) _max2 = (y);                    \
      (void) (&_max1 == &_max2);                \
      _max1 > _max2 ? _max1 : _max2; })

#define min_t(type, x, y) ({                    \
      type __min1 = (x);                        \
      type __min2 = (y);                        \
      __min1 < __min2 ? __min1: __min2; })

#define max_t(type, x, y) ({                    \
      type __max1 = (x);                        \
      type __max2 = (y);                        \
      __max1 > __max2 ? __max1: __max2; })

#endif
