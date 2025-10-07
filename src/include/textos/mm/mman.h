#pragma once

/*
 * map flags (POSIX):
 *  - MAP_SHARED
 *    changes would be shared and be applied to underlying object
 *  - MAP_PRIVATE
 *    changed would be only visible to the current proc and not applied
 *  - MAP_TYPE
 *    only a mask for internal using
 *  - MAP_FIXED
 *    parse addr exactly, `addr` % PAGE_SIZE == off % PAGE_SIZE
 *  - MAP_ANON
 *    anonymous memory region, mapping is not backed by any file, the contents
 *    are initialized to 0. make sure `fd` is -1 with `off` set to 0!!!
 */
#define MAP_SHARED    0x1  // TODO
#define MAP_PRIVATE   0x2
#define MAP_TYPE      0xf
#define MAP_FIXED     0x10
#define MAP_ANON      0x20
#define MAP_ANONYMOUS MAP_ANON

#define MAPL_USER 0
#define MAPL_FILE 1
#define MAPL_STACK 2
#define MAPL_HEAP 3

/*
 * on x86 platform, the page table doesn't provide a native method to implement only `PROT_WRITE`,
 * in our OS-implementation, we choose to ignore it because support it may occupy more resource.
 * i.e. page-fault can absolutely do it, cpu would whisper that -> :)
 */
#define PROT_NONE  0
#define PROT_READ  1
#define PROT_WRITE 2
#define PROT_EXEC  4

#define mapprot(p) \
    (((p) & PROT_READ  ? 0 : 0) \
     | ((p) & PROT_WRITE ? PE_RW : 0) \
     | ((p) & PROT_EXEC ? 0 : PE_NX))

#define MAP_FAILED ((void *)-1)

/*
 * POSIX MANUAL:
 *  The mapping established by mmap() shall replace any previous mappings for those whole pages containing  any
 *  part of the address space of the process starting at pa and continuing for len bytes.
 *
 *  If  the size of the mapped file changes after the call to mmap() as a result of some other operation on the
 *  mapped file, the effect of references to portions of the mapped region that correspond to added or  removed
 *  portions of the file is unspecified.
 */

#ifdef __TEXT_OS__

/*
 * it holds internal mmap argument
 */
typedef struct
{
    addr_t va;
    size_t num;
    int prot;
    int flgs;
    size_t foff;
    void *fnode;
    addr_t *ppgs;
} vm_region_t;

#include <textos/file.h>
#include <textos/klib/list.h>
#include <textos/klib/rbtree.h>

typedef struct
{
    addr_t s, t;
    int flgs;
    int prot;
    char label;
    union
    {
        struct
        {
            size_t foff;
            node_t *node;
        };
    } obj;
    list_t list;
    rbnode_t node;
} vm_area_t;

typedef struct
{
    list_t list;
    rbtree_t tree;
} vm_space_t;

#define MRET(x) ((void *)x)

void *mmap_file(vm_region_t *vm);
void *mmap_anon(vm_region_t *vm);

vm_space_t *mm_new_space(vm_space_t *old);
vm_area_t *mmap_new_vma(vm_area_t *old);

void mmap_free_vma(vm_area_t *vma);
void mmap_regst(vm_space_t *sp, vm_area_t *vma);
void mmap_display(vm_space_t *sp);
vm_area_t *mmap_lowerbound(vm_space_t *sp, addr_t addr);
vm_area_t *mmap_upperbound(vm_space_t *sp, addr_t addr);
vm_area_t *mmap_containing(vm_space_t *sp, addr_t addr);

#endif

void *mmap(void *addr, size_t len, int prot, int flgs, int fd, size_t off);

int mprotect(void *addr, size_t len, int prot);

int munmap(void *addr, size_t len);
