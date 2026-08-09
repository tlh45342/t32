int main(void)
{
    int x = 5;
    int y = 10;

    if (x < y)
        x = x + y;
    else
        y = y + x;

    return x + y;
}
