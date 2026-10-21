#pragma once

#define __CMDLINE_USED(key)

void cmdline_get_raw(const char **raw, size_t *size);
bool cmdline_search(const char *key, const char **v, const char **vend);

void cmdline_free_str(char *str);

int cmdline_get_buf(const char *key, char *buf, size_t *size);
int cmdline_get_str(const char *key, char **str);
int cmdline_get_u64(const char *key, uint64_t *val);
int cmdline_get_bool(const char *key, bool *val);

char *cmdline_getdef_str(const char *key, char *def);
uint64_t cmdline_getdef_u64(const char *key, uint64_t def);
bool cmdline_getdef_bool(const char *key, bool def);
