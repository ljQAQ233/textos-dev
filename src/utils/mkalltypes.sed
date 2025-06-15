# __typedef x y; -> ...;
/^__typedef/s/__typedef \(.*\) \([^ ]*\);$/#if defined(__NEED_\2) \&\& !defined(__DEFINED_\2)\
typedef \1 \2;\
#define __DEFINED_\2\
#endif\
/

# __typeaka x; -> typedef __x x;
/^__typeaka \(.*\);$/s//\
#if defined(__NEED_\1) \&\& !defined(__DEFINED_\1)\
typedef __\1 \1;\
#define __DEFINED_\1\
#endif\
/

/^__struct/s/__struct * \([^ ]*\) \(.*\);$/#if defined(__NEED_struct_\1) \&\& !defined(__DEFINED_struct_\1)\
struct \1 \2;\
#define __DEFINED_struct_\1\
#endif\
/

