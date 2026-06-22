extern int __dlm_mallopt(int, int);

int mallopt(int param, int value)
{
    return __dlm_mallopt(param, value);
}
