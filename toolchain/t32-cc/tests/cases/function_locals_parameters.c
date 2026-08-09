int mix(int a, int b)
{
    int x = a + 1;
    int y = b + 2;
    x = x * y;
    return x + a;
}

int main(void)
{
    return mix(3, 4);
}
