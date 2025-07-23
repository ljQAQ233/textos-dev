// NIC request

#define SIOCADDRT          0x890B
#define SIOCDELRT          0x890C
#define SIOCRTMSG          0x890D

#define SIOCGIFNAME        0x8910
#define SIOCSIFLINK        0x8911
#define SIOCGIFCONF        0x8912
#define SIOCGIFFLAGS       0x8913
#define SIOCSIFFLAGS       0x8914
#define SIOCGIFADDR        0x8915 // get addr
#define SIOCSIFADDR        0x8916 // set addr
#define SIOCGIFDSTADDR     0x8917
#define SIOCSIFDSTADDR     0x8918
#define SIOCGIFBRDADDR     0x8919
#define SIOCSIFBRDADDR     0x891a
#define SIOCGIFNETMASK     0x891b
#define SIOCSIFNETMASK     0x891c
#define SIOCGIFMETRIC      0x891d
#define SIOCSIFMETRIC      0x891e
#define SIOCGIFMEM         0x891f
#define SIOCSIFMEM         0x8920
#define SIOCGIFMTU         0x8921
#define SIOCSIFMTU         0x8922
#define SIOCSIFNAME        0x8923
#define SIOCSIFHWADDR      0x8924 // hardware addr
#define SIOCGIFENCAP       0x8925
#define SIOCSIFENCAP       0x8926
#define SIOCGIFHWADDR      0x8927 // hardware addr
#define SIOCGIFSLAVE       0x8929
#define SIOCSIFSLAVE       0x8930
#define SIOCADDMULTI       0x8931
#define SIOCDELMULTI       0x8932
#define SIOCGIFINDEX       0x8933
#define SIOGIFINDEX        SIOCGIFINDEX
#define SIOCSIFPFLAGS      0x8934
#define SIOCGIFPFLAGS      0x8935
#define SIOCDIFADDR        0x8936
#define SIOCSIFHWBROADCAST 0x8937
#define SIOCGIFCOUNT       0x8938

#define SIOCGIFBR          0x8940
#define SIOCSIFBR          0x8941

#define SIOCGIFTXQLEN      0x8942
#define SIOCSIFTXQLEN      0x8943

#define SIOCDARP           0x8953
#define SIOCGARP           0x8954
#define SIOCSARP           0x8955

#define SIOCDRARP          0x8960
#define SIOCGRARP          0x8961
#define SIOCSRARP          0x8962

#define SIOCGIFMAP         0x8970
#define SIOCSIFMAP         0x8971

#define SIOCADDDLCI        0x8980
#define SIOCDELDLCI        0x8981

#define SIOCDEVPRIVATE     0x89F0
#define SIOCPROTOPRIVATE   0x89E0

// Terminal
#define TCGETS             0x5401
#define TCSETS             0x5402

#define TIOCSPGRP          0x5410
#define TIOCGPGRP          0x540f
