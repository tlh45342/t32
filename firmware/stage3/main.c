/*
 * T32 C STAGE3 0.0.4
 *
 * First C source using a string literal and libt32 puts().
 */
int main(void)
{
    int rc;

    rc = puts("Hello from C via puts()");
    return 42 + rc;
}
