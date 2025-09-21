/*
 * original version from iTerm2
 * adapted from that shell script
 */
#include <stdio.h>

#define SEPARATOR ';'

void setbg(int r, int g, int b)
{
    printf("\033[48%c2%c%d%c%d%c%dm", SEPARATOR, SEPARATOR, r, SEPARATOR, g, SEPARATOR, b);
}

void rstbg()
{
    printf("\033[0m\n");
}

void rainbow(int i, int *r, int *g, int *b)
{
    int h = i / 43;
    int f = i - 43 * h;
    int t = f * 255 / 43;
    int q = 255 - t;

    if (h == 0)
        *r = 255, *g = t, *b = 0;
    else if (h == 1)
        *r = q, *g = 255, *b = 0;
    else if (h == 2)
        *r = 0, *g = 255, *b = t;
    else if (h == 3)
        *r = 0, *g = q, *b = 255;
    else if (h == 4)
        *r = t, *g = 0, *b = 255;
    else if (h == 5)
        *r = 255, *g = 0, *b = q;
}

int main()
{
    // Display background gradients
    for (int i = 0; i <= 127; i++)
    {
        setbg(i, 0, 0);
        printf(" ");
    }
    rstbg();

    for (int i = 255; i >= 128; i--)
    {
        setbg(i, 0, 0);
        printf(" ");
    }
    rstbg();

    for (int i = 0; i <= 127; i++)
    {
        setbg(0, i, 0);
        printf(" ");
    }
    rstbg();

    for (int i = 255; i >= 128; i--)
    {
        setbg(0, i, 0);
        printf(" ");
    }
    rstbg();

    for (int i = 0; i <= 127; i++)
    {
        setbg(0, 0, i);
        printf(" ");
    }
    rstbg();

    for (int i = 255; i >= 128; i--)
    {
        setbg(0, 0, i);
        printf(" ");
    }
    rstbg();

    // Display rainbow gradient
    for (int i = 0; i <= 127; i++)
    {
        int r, g, b;
        rainbow(i, &r, &g, &b);
        setbg(r, g, b);
        printf(" ");
    }
    rstbg();

    for (int i = 255; i >= 128; i--)
    {
        int r, g, b;
        rainbow(i, &r, &g, &b);
        setbg(r, g, b);
        printf(" ");
    }
    rstbg();

    return 0;
}
