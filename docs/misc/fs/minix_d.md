# minix history - dir_struct

## v1

```c
typedef struct __packed {
  u16_t d_inum;
  char d_name[14];
} dir_struct;
```

## v2

```c
struct direct {
  u16_t d_ino;
  char d_name[14];
} __packed;
```

## v3

```c
struct direct {
  u32_t d_ino;
  char d_name[60];
} __packed;
```

# diff

- v1 -> v2 - 不变
