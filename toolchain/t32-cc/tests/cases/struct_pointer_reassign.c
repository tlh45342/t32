struct pair {
    int a;
    int b;
};

int main(void)
{
    struct pair left;
    struct pair right;
    struct pair *p = &left;
    p->a = 11;
    p = &right;
    p->b = 31;
    return left.a + right.b;
}
