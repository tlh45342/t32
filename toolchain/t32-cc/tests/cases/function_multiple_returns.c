int signish(int x)
{
    if (x < 0)
        return 1;
    if (x == 0)
        return 2;
    return 3;
}

int main(void)
{
    return signish(0);
}
