#define R_ADDR 0xCF8
#define R_DATA 0xCFC

#include <io.h>

// 0x__80 -> other
struct {
    const u16 code;
    const char *name;
} pci_class[] = {
    { 0x0000, "Non-VGA unclassified device"              },
    { 0x0001, "VGA compatible unclassified device"       },
    { 0x0100, "SCSI storage controller"                  },
    { 0x0101, "IDE interface"                            },
    { 0x0102, "Floppy disk controller"                   },
    { 0x0103, "IPI bus controller"                       },
    { 0x0104, "RAID bus controller"                      },
    { 0x0180, "Mass storage controller"                  },
    { 0x0200, "Ethernet controller"                      },
    { 0x0201, "Token ring network controller"            },
    { 0x0202, "FDDI network controller"                  },
    { 0x0203, "ATM network controller"                   },
    { 0x0204, "ISDN controller"                          },
    { 0x0280, "Network controller"                       },
    { 0x0300, "VGA controller"                           },
    { 0x0301, "XGA controller"                           },
    { 0x0302, "3D controller"                            },
    { 0x0380, "Display controller"                       },
    { 0x0400, "Multimedia video controller"              },
    { 0x0401, "Multimedia audio controller"              },
    { 0x0402, "Computer telephony device"                },
    { 0x0480, "Multimedia controller"                    },
    { 0x0500, "RAM memory"                               },
    { 0x0501, "FLASH memory"                             },
    { 0x0580, "Memory controller"                        },
    { 0x0600, "Host bridge"                              },
    { 0x0601, "ISA bridge"                               },
    { 0x0602, "EISA bridge"                              },
    { 0x0603, "MicroChannel bridge"                      },
    { 0x0604, "PCI bridge"                               },
    { 0x0605, "PCMCIA bridge"                            },
    { 0x0606, "NuBus bridge"                             },
    { 0x0607, "CardBus bridge"                           },
    { 0x0608, "RACEway bridge"                           },
    { 0x0609, "Semi-transparent PCI-to-PCI bridge"       },
    { 0x060A, "InfiniBand to PCI host bridge"            },
    { 0x0680, "Bridge"                                   },
    { 0x0700, "Serial controller"                        },
    { 0x0701, "Parallel controller"                      },
    { 0x0702, "Multiport serial controller"              },
    { 0x0703, "Modem"                                    },
    { 0x0780, "Communication controller"                 },
    { 0x0800, "PIC"                                      },
    { 0x0801, "DMA controller"                           },
    { 0x0802, "Timer"                                    },
    { 0x0803, "RTC"                                      },
    { 0x0804, "PCI Hot-plug controller"                  },
    { 0x0880, "System peripheral"                        },
    { 0x0900, "Keyboard controller"                      },
    { 0x0901, "Digitizer Pen"                            },
    { 0x0902, "Mouse controller"                         },
    { 0x0903, "Scanner controller"                       },
    { 0x0904, "Gameport controller"                      },
    { 0x0980, "Input device controller"                  },
    { 0x0A00, "Generic Docking Station"                  },
    { 0x0A80, "Docking Station"                          },
    { 0x0B00, "386"                                      },
    { 0x0B01, "486"                                      },
    { 0x0B02, "Pentium"                                  },
    { 0x0B10, "Alpha"                                    },
    { 0x0B20, "Power PC"                                 },
    { 0x0B30, "MIPS"                                     },
    { 0x0B40, "Co-processor"                             },
    { 0x0C00, "FireWire (IEEE 1394)"                     },
    { 0x0C01, "ACCESS Bus"                               },
    { 0x0C02, "SSA"                                      },
    { 0x0C03, "USB Controller"                           },
    { 0x0C04, "Fiber Channel"                            },
    { 0x0C05, "SMBus"                                    },
    { 0x0C06, "InfiniBand"                               },
    { 0x0D00, "IRDA controller"                          },
    { 0x0D01, "Consumer IR controller"                   },
    { 0x0D10, "RF controller"                            },
    { 0x0D80, "Wireless controller"                      },
    { 0x0E00, "I2O"                                      },
    { 0x0F00, "Satellite TV controller"                  },
    { 0x0F01, "Satellite audio communication controller" },
    { 0x0F03, "Satellite voice communication controller" },
    { 0x0F04, "Satellite data communication controller"  },
    { 0x1000, "Network and computing encryption device"  },
    { 0x1010, "Entertainment encryption device"          },
    { 0x1080, "Encryption controller"                    },
    { 0x1100, "DPIO module"                              },
    { 0x1101, "Performance counters"                     },
    { 0x1110, "Communication synchronizer"               },
    { 0x1180, "Signal processing controller"             }
};

static const char *get_classname(u16 code)
{
    int s = 0, mid, cmp,
        t = sizeof(pci_class) / sizeof(pci_class[0]);
    while (s <= t) {
        mid = (s + t) / 2;
        cmp = pci_class[mid].code;
        if (code > cmp)
            s = mid + 1;
        else if (code < cmp)
            t = mid - 1;
        else
            return pci_class[mid].name;
    }

    return "Unknown device";
}

static inline void pci_set_addr(u8 bus, u8 slot, u8 func, u8 offset)
{
    u32 addr = 0;
    addr |= ((u32)bus << 16);
    addr |= ((u32)slot << 11);
    addr |= ((u32)func << 8);
    addr |= ((u32)offset & 0xFC);
    addr |= (1u << 31);
    outdw(R_ADDR, addr);
}

u8 pci_read_byte(u8 bus, u8 slot, u8 func, u8 offset)
{
    pci_set_addr(bus, slot, func, offset);
    int low = offset & 0b11;
    u32 tmp = indw(R_DATA);
    return ((u8 *)&tmp)[low];
}

u16 pci_read_word(u8 bus, u8 slot, u8 func, u8 offset)
{
    pci_set_addr(bus, slot, func, offset);
    int low = offset & 0b11;
    u32 tmp = indw(R_DATA);
    return ((u16 *)&tmp)[low / 2];
}

u32 pci_read_dword(u8 bus, u8 slot, u8 func, u8 offset)
{
    pci_set_addr(bus, slot, func, offset);
    int low = offset & 0b11;
    u32 tmp = indw(R_DATA);
    return tmp;
}

static inline u16 get_vendor(u8 bus, u8 slot, u8 func)
{
    /* vendor 是第0个字 */
    return pci_read_word(bus, slot, func, 0);
}

static inline u16 get_hdrtype(u8 bus, u8 slot, u8 func)
{
    return pci_read_byte(bus, slot, func, 14);
}

static inline u16 get_code(u8 bus, u8 slot, u8 func)
{
    return pci_read_word(bus, slot, func, 10);
}

static void scan_dev(u8 bus, u8 dev)
{
    u16 vendor;
    for (int func = 0 ; func < 8 ; func++) {
        vendor = get_vendor(bus, dev, func);
        if (vendor == 0 || vendor == 0xFFFF)
            break;

        u16 class = get_code(bus, dev, func);

        dprintk(K_PCI, " [%u/%u/%d] %x %s\n", bus, dev, func, vendor, get_classname(class));
    }
}

static void scan_bus(u8 bus)
{
    for (int dev = 0 ; dev < 32 ; dev++)
        scan_dev(bus, dev);
}

static void scan_all()
{
    DEBUGK(K_PCI, "scanning pci...\n");
    u8 type = get_hdrtype(0, 0, 0);
    if ((type & 0x80) == 0)
        scan_bus(0);
    else {
        for (u8 func = 0 ; func < 8 ; func++) {
            if (get_vendor(0, 0, func) == 0xFFFF)
                break;
            scan_bus(func);
        }
    }
}

void pci_init()
{
    scan_all();
}
