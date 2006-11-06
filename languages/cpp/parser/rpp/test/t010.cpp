#include "test.h"

#define TEST_H

#if defined(TEST_H)
OK
#else
ERROR
#endif

#if !defined(TEST_H)
ERROR
#else
OK
#endif
