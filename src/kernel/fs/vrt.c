#include <textos/dev/buffer.h>
#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/fs/internal.h>
#include <textos/klib/string.h>
#include <textos/mm.h>
#include <textos/printk.h>

#include <textos/dev.h>

// clang-format on

#include <textos/args.h>
#include <textos/klib/vsprintf.h>

// todo: fix fat32_truncate

extern void __vfs_pipe_init();

void fs_init()
{
    // abstract
    __vfs_pipe_init();
    printk("file system initialized!\n");
}
