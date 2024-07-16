void _start ()
{
    __asm__ volatile (
        "int $0x80"
        :
        : "D"(0x233), "S"(0x234), "d"(0x235), "c"(0x236), "a"(0)
        );

    while (1);
}
