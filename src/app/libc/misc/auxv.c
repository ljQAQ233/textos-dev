const void **__auxv;

unsigned long getauxval(unsigned long type)
{
    const unsigned long *aux = (void *)__auxv;
    for ( ; aux[0] ; aux += 2)
        if (aux[0] == type)
            return aux[1];
    return 0;
}
