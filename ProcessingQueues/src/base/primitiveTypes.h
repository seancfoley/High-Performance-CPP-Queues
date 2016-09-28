/*
 * primitiveTypes.h
 *
 *  Created on: Sep 8, 2010
 *      Author: sfoley
 */

#ifndef PRIMITIVETYPES_H_
#define PRIMITIVETYPES_H_

#include <climits>
#include <stdint.h>

#undef UINT_64
#undef UINT_32
#undef UINT_16
#undef UINT_8
#undef INT_64
#undef INT_32
#undef INT_16
#undef INT_8

typedef uint32_t UINT_32;
typedef uint16_t UINT_16;
typedef unsigned char UINT_8;

typedef int32_t INT_32;
typedef int16_t INT_16;
typedef int8_t  INT_8;

typedef int64_t INT_64;
typedef uint64_t UINT_64;

#endif /* PRIMITIVETYPES_H_ */
