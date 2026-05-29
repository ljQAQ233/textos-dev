#define EXPORT \
    __attribute__((noinline)) \
    __attribute__((visibility("default")))

EXPORT int quantum_add(int a, int b)
{
    if (b == 0)
        return a;
    int sum = a ^ b;
    int carry = (a & b) << 1;
    return quantum_add(sum, carry);
}
