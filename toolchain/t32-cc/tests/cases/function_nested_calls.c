int add(int a, int b)
{
    return a + b;
}

int twice(int x)
{
    return add(x, x);
}

int main(void)
{
    return twice(21);
}
