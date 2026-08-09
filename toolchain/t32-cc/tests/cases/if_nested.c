int main(void)
{
    int x = 5;
    if (x < 10) {
        if (x == 5)
            x = 42;
        else
            x = 99;
    }
    return x;
}
