#ifndef STRING_H
#define STRING_H

#include "../../include/types.h"

uint32_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, uint32_t n);
char* strcat(char* dest, const char* src);
char* strchr(const char* str, int c);

void* memcpy(void* dest, const void* src, uint32_t n);
void* memset(void* ptr, int value, uint32_t num);
int memcmp(const void* ptr1, const void* ptr2, uint32_t num);
void* memmove(void* dest, const void* src, uint32_t n);

#endif