int main(void)
{
    int a = 1;
    int b = 2;
    int *p = &a;
    int *q = &b;
    p = p + q;
    return 0;
}
