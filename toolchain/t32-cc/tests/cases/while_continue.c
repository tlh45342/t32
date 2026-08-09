int main(void)
{
    int x = 0;
    int sum = 0;

    while (x < 6) {
        x = x + 1;
        if (x == 3)
            continue;
        sum = sum + x;
    }

    return sum;
}
