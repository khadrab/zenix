#ifndef TYPES_H
#define TYPES_H

/* Basic integer types - defined directly without stdint.h dependency */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;

/* Size types */
typedef uint32_t size_t;
typedef int32_t  ssize_t;

/* Boolean type */
typedef uint8_t bool;
#define true  1
#define false 0

/* Pointer types */
typedef uint32_t uintptr_t;
typedef int32_t  intptr_t;

/* NULL definition */
#ifndef NULL
#define NULL ((void*)0)
#endif

#endif /* TYPES_H */