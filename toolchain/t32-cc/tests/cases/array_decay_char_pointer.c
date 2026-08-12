int main(void)
{
    char s[3];
    char *p = s;
    s[0] = 65;
    s[1] = 66;
    p = p + 1;
    return *p;
}
