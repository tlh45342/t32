int add(int a, int b)
{
    return a + b;
}

int twice(int x)
{
    return x * 2;
}

int main(void)
{
    return add(twice(5), twice(7));
}
