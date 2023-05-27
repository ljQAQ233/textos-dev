#ifndef __ELF_H__
#define __ELF_H__

#define ELF_MAGIC 0x464C457F

/* Elf Classes
 * Like Cpu Bits or Word Length
 */
#define ELFCLASSNONE   0
#define ELFCLASS32     1
#define ELFCLASS64     2

/* Elf Data 
 * Data Encodeing Type
 */
#define ELFDATANONE    0
#define ELFDATA_LSB    1
#define ELFDATA_MSB    2

/*
 * Elf Version 
 * The Current Version is 1
 */
#define EV_NONE        0
#define EV_CURRENT     1

/* Elf for which OS - OSABI 
 * We mustn't use it now,so only define the valid Macro 'ELFOSABI_LINUX'
 */
#define ELFOSABI_NONE  0
#define ELFOSABI_LINUX 3

/* Elf Pad Area 
 * Fill the Pad Area
 */
#define ELFRESERVED    0

/* Elf Machines
 * The Bootloader Supports the following types
 */
#define EM_386         3
#define EM_X86_64      62
#define EM_ARM         40
#define EM_AARCH64     183

/* Elf File Type 
 * There are more types But we use only the following
 */
#define ET_NONE        0 // Invalid Type
#define ET_REL         1 // Relocatable File
#define ET_EXEC        2 // Executable File
#define ET_DYN         3 // Shared Obj File

/* Program Header Flags 
 * Segment flags - ELF_PHEADER.Flgs
 */
#define PF_X        (1 << 0)    // Segment is executable 
#define PF_W        (1 << 1)    // Segment is writable
#define PF_R        (1 << 2)    // Segment is readable


/* Define something for boot in different platforms */
#if defined(MDE_CPU_X64)
  #define ELF_SUPPORTED_ARCH  EM_X86_64
  #define ELF_SUPPORTED_CLASS ELFCLASS64
#elif defined(MDE_CPU_IA32)
  #define ELF_SUPPORTED_ARCH  EM_386
  #define ELF_SUPPORTED_CLASS ELFCLASS32
#elif defined(MDE_CPU_AARCH64)
  #define ELF_SUPPORTED_ARCH  EM_AARCH64
  #define ELF_SUPPORTED_CLASS ELFCLASS64
#elif defined(MDE_CPU_ARM)
  #define ELF_SUPPORTED_ARCH  EM_ARM
  #define ELF_SUPPORTED_CLASS ELFCLASS32
#endif

#pragma pack(1)

typedef struct {
    UINT32 Magic;
    UINT8  Class;
    UINT8  Data;
    UINT8  Version;
    UINT8  OSABI;
    UINT8  ABIVersion;
    UINT8  Pad[6];
    UINT8  Nident;
    UINT16 Type;
    UINT16 Machine;
    UINT32 ObjFileVersion;
    UINT64 Entry;
    UINT64 PhOffset;
    UINT64 ShOffset;
    UINT32 Flgs;
    UINT16 EhSiz;
    UINT16 PhentSiz;
    UINT16 PhNum;
    UINT16 ShentSiz;
    UINT16 ShNum;
    UINT16 ShStrIdx;
} ELF_HEADER_64;

typedef struct {
    UINT32 Magic;
    UINT8  Class;
    UINT8  Data;
    UINT8  Version;
    UINT8  OSABI;
    UINT8  ABIVersion;
    UINT8  Pad[6];
    UINT8  Nident;
    UINT16 Type;
    UINT16 Machine;
    UINT32 ObjFileVersion;
    UINT32 Entry;
    UINT32 PhOffset;
    UINT32 ShOffset;
    UINT32 Flgs;
    UINT16 EhSiz;
    UINT16 PhentSiz;
    UINT16 PhNum;
    UINT16 ShentSiz;
    UINT16 ShNum;
    UINT16 ShStrIdx;
} ELF_HEADER_32;

/* Use UINTN instead of UINT32 or others to construct different sturctures in different platforms */

typedef struct {
    UINT32 Magic;            // String Must be {0x7F,'E','L','F'},Type UINT32 must be 0x464C457F
    UINT8  Class;           // Supported Bits
    UINT8  Data;            // Way to Encode
    UINT8  Version;         // Elf Version, 1
    UINT8  OSABI;           // Describes the file is for which OS to run
    UINT8  ABIVersion;      // Application Binary Interface Version
    UINT8  Pad[6];          // Zero to fill
    UINT8  Nident;          // Size of Ident
    UINT16 Type;            // Which Elf Type,Exec or Others
    UINT16 Machine;         // Arch / Machine
    UINT32 ObjFileVersion;  // This Member identifies the Object file Version
    UINTN  Entry;           // Addr
    UINTN  PhOffset;        // Programme header offset
    UINTN  ShOffset;        // Section header offset
    UINT32 Flgs;            // In standardization,macros start with EF_
    UINT16 EhSiz;           // ELF header's size(bytes)
    UINT16 PhentSiz;        // Programme header table entry size
    UINT16 PhNum;           // the num of programme header table entry
    UINT16 ShentSiz;        // Section header table entry size
    UINT16 ShNum;           // the num of section header table entry
    UINT16 ShStrIdx;        // Section header string table index
} ELF_HEADER;

#pragma pack()

#define PT_NONE 0
#define PT_LOAD 1

#pragma pack(1)

typedef struct {
  UINT32 Type;          // Type of the segment,e.g. PT_LOAD
  UINT32 Flgs;          // Segment flags
  UINT64 Offset;        // The segment's offset in the file
  UINT64 VirtualAddr;
  UINT64 PhysicalAddr;
  UINT64 FileSiz;       // The size of the segment in the file
  UINT64 MemSiz;        // The size of the segment that used in memory space
  UINT64 Align;
} ELF_PHEADER_64;

typedef struct {
  UINT32 Type;
  UINT32 Offset;
  UINT32 VirtualAddr;
  UINT32 PhysicalAddr;
  UINT32 FileSiz;
  UINT32 MemSiz;
  UINT32 Flgs;
  UINT32 Align;
} ELF_PHEADER_32;

#pragma pack()

/* Mutiple arch but not used now */
#if defined(MDE_CPU_X64) || defined(MDE_CPU_AARCH64)
  typedef ELF_PHEADER_64 ELF_PHEADER;
#elif defined(MDE_CPU_IA32) || defined(MDE_CPU_ARM)
  typedef ELF_PHEADER_32 ELF_PHEADER;
#endif

EFI_STATUS
ElfLoad (
  VOID             *Buffer,
  PHYSICAL_ADDRESS *Entry
  );

#endif

