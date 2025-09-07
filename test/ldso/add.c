#define EXPORT \
    __attribute__((noinline)) \
    __attribute__((visibility("default")))

extern int quantum_add(int a, int b);

EXPORT int wrap(int a, int b)
{
    return quantum_add(a, b);
}

EXPORT int add(int a, int b)
{
    return wrap(a, b);
}
