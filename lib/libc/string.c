// lib/libc/string.c - مكتبة معالجة النصوص الكاملة

#include "string.h"

// حساب طول النص
uint32_t strlen(const char* str) {
    uint32_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

// مقارنة نصين
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// نسخ نص
char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

// نسخ نص بحد أقصى
char* strncpy(char* dest, const char* src, uint32_t n) {
    uint32_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

// دمج نصين
char* strcat(char* dest, const char* src) {
    char* ret = dest;
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++));
    return ret;
}

// البحث عن حرف
char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

// نسخ ذاكرة
void* memcpy(void* dest, const void* src, uint32_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    for (uint32_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    
    return dest;
}

// تعيين قيمة للذاكرة
void* memset(void* ptr, int value, uint32_t num) {
    unsigned char* p = (unsigned char*)ptr;
    
    for (uint32_t i = 0; i < num; i++) {
        p[i] = (unsigned char)value;
    }
    
    return ptr;
}

// مقارنة ذاكرة
int memcmp(const void* ptr1, const void* ptr2, uint32_t num) {
    const unsigned char* p1 = (const unsigned char*)ptr1;
    const unsigned char* p2 = (const unsigned char*)ptr2;
    
    for (uint32_t i = 0; i < num; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    
    return 0;
}

// نقل ذاكرة (يدعم التداخل)
void* memmove(void* dest, const void* src, uint32_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    
    if (d < s) {
        for (uint32_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        for (uint32_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    
    return dest;
}