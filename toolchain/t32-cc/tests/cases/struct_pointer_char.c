struct record {
    char tag;
    int value;
};

int main(void)
{
    struct record r;
    struct record *p = &r;
    p->tag = 300;
    p->value = 42;
    return p->tag + p->value;
}
