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

/*
 * on x86 platform, the page table doesn't provide a native method to implement only `PROT_WRITE`,
 * in our OS-implementation, we choose to ignore it because support it may occupy more resource.
 * i.e. page-fault can absolutely do it, cpu would whisper that -> :)
 */
#define PROT_NONE  0
#define PROT_READ  1
#define PROT_WRITE 2
#define PROT_EXEC  4

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
