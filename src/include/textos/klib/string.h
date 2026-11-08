#pragma once

size_t strlen(const char *str);
size_t strnlen(const char *str, size_t maxlen);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char* str1,const char *str2);
int strncmp(const char* str1, const char *str2, size_t n);
char *strcat(char* dest, const char* src);
char *strchr(const char *str, int c);
char *strrchr(const char *str, int c);
char *strchrnul(const char *str, int c);
char *strrchrnul(const char *str, int c);
char *strstr(const char *haystack, const char *needle);
size_t strspn(const char *str, const char *accept);
size_t strcspn(const char *str, const char *reject);
char *strpbrk(const char *str, const char *accept);
char *strdup(const char *str);
char *strndup(const char *str, size_t n);
void *memset(void *dest, int c, size_t n);
int memcmp(const void *ptr1, const void *ptr2, size_t n);
void *memchr(const void *ptr, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
