int main(void)
{
    int a[3];
    int *p = a;
    a[0] = 11;
    a[1] = 22;
    p = p + 1;
    return *p;
}
