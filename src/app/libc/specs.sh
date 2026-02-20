incdir=$1
crtdir=$2
libdir=$3
ldso=$4

# NOTE: 如果置空一个规则, 需要两个空行!!!

cat <<EOF
*cc1:
%(cc1_cpu) -nostdinc $(for d in $incdir; do printf -- "-isystem %s " "$d"; done)

*lib:


*link_libgcc:
$(for d in $libdir; do printf -- "-L %s " "$d"; done) -lc

*libgcc:
libgcc.a%s

*startfile:
$crtdir/crt1.c.o $crtdir/crti.c.o

*endfile:
$crtdir/crtn.c.o

*link:
-dynamic-linker $ldso -nostdlib %{shared:-shared} %{static:-static} %{rdynamic:-export-dynamic}

*esp_link:
*esp_options:

EOF
