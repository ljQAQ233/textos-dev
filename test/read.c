#include <stdio.h>
#include <unistd.h>

int main()
{
    char ch;
    read(0, &ch, 1);
    printf("read : %x\n", ch);

    return 0;
}
