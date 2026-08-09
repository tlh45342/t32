int main(void)
{
    int x = 12;
    if (x < 10) {
        x = x + 100;
    } else {
        x = x - 2;
        x = x * 3;
    }
    return x;
}
