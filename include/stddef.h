#ifndef STDDEF_H
#define STDDEF_H

/* Size type */
typedef unsigned int size_t;
typedef int ptrdiff_t;

/* NULL pointer */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* Offset of member in structure */
#define offsetof(type, member) ((size_t)&(((type*)0)->member))

#endif /* STDDEF_H */