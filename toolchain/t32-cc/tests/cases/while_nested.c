int main(void)
{
    int x = 1;
    while (x < 16) {
        while (x < 4)
            x = x + 1;
        x = x * 2;
    }
    return x;
}
