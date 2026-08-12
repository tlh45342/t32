int main(void)
{
    int x = 42;
    int *p;
    p = &x;
    *p = 73;
    return *p;
}
