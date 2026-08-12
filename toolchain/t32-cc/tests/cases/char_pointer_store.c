int main(void)
{
    char c = 0;
    char *p = &c;
    *p = 66;
    return c;
}
