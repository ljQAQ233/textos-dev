double sin(double x)
{
    asm volatile(
        "fldl %0\n"
        "fsin\n"
        "fstpl %0\n"
        : "+m"(x));
    return x;
}

double cos(double x)
{
    asm volatile(
        "fldl %0\n"
        "fcos\n"
        "fstpl %0\n"
        : "+m"(x));
    return x;
}

double tan(double x)
{
    asm volatile(
        "fldl %0\n"
        "fptan\n"
        "fstpl %0\n" // 1
        "fstpl %0\n" // tan(x)
        : "+m"(x));
    return x;
}

// arctan(y / x)
double atan2(double y, double x)
{
    asm volatile(
        "fldl %0\n"
        "fldl %1\n"
        "fpatan\n"
        "fstpl %0\n" // res
        "fstpl %1\n" // 
        : "+m"(x), "+m"(y));
    return x;
}
