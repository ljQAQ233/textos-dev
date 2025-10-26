#include <setjmp.h>
#include <stdio.h>

jmp_buf a, b;
void A();
void B();
#define log(x, ARGS...) printf("- %d : " x, __LINE__, ##ARGS)

void A()
{
    int r;
    log("(A1)\n");
    r = setjmp(a);
    if (r == 0)
        B();

    log("(A2) r=%d\n", r);
    r = setjmp(a);
    if (r == 0)
        longjmp(b, 20001);

    log("(A3) r=%d\n", r);
    r = setjmp(a);
    if (r == 0)
        longjmp(b, 20002);

    log("(A4) r=%d\n", r);
}

void B()
{
    int r;
    log("(B1)\n");
    r = setjmp(b);
    if (r == 0)
        longjmp(a, 10001);

    log("(B2) r=%d\n", r);
    r = setjmp(b);
    if (r == 0)
        longjmp(a, 10002);

    log("(B3) r=%d\n", r);
    r = setjmp(b);
    if (r == 0)
        longjmp(a, 10003);
}

int main()
{
    A();
    return 0;
}

// result should be:
// - 12 : (A1)
// - 33 : (B1)
// - 17 : (A2) r=10001
// - 38 : (B2) r=20001
// - 22 : (A3) r=10002
// - 43 : (B3) r=20002
// - 27 : (A4) r=10003
