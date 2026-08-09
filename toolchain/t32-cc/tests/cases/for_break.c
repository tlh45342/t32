int main(void)
{
    int x = 0;

    for (x = 0; x < 10; x = x + 1) {
        if (x == 4)
            break;
    }

    return x;
}
