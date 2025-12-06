#ifndef STRING_H
#define STRING_H

#include "../../include/types.h"

uint32_t strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, uint32_t n);
void strcpy(char* dest, const char* src);
void strncpy(char* dest, const char* src, uint32_t n);
char* strchr(const char* str, int c);
void* memset(void* ptr, int value, uint32_t num);
void* memcpy(void* dest, const void* src, uint32_t n);

#endif