#include <stdio.h>

#define SEPARATOR ';'

void setbg(int x)
{
    printf("\033[48%c5%c%dm ", SEPARATOR, SEPARATOR, x);
}

void rstbg(int end)
{
    printf("\033[0m %03d\n", end);
}

int main()
{
    for (int i = 16 ; i <= 231 ; i++)
    {
        setbg(i);
        if ((i - 15) % 36 == 0)
            rstbg(i);
    }
    printf("\033[0m\n");
    return 0;
}
