int main(void)
{
    int a = 11;
    int b = 22;
    int *p = &a;
    p = p + 1;
    return *p;
}
