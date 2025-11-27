/*
 * KaiOS - Basic type definitions
 * Lightweight OS kernel types
 */

#ifndef KAIOS_TYPES_H
#define KAIOS_TYPES_H

// Basic integer types
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

// Size types
typedef uint32_t size_t;
typedef int32_t  ssize_t;

// Null pointer
#define NULL 0
#define nullptr 0

// Inline and attribute macros
#define PACKED __attribute__((packed))
#define ALIGNED(x) __attribute__((aligned(x)))
#define UNUSED __attribute__((unused))

#endif // KAIOS_TYPES_H
