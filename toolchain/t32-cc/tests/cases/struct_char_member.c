struct record {
    char tag;
    int value;
};

int main(void)
{
    struct record r;
    r.tag = 300;
    r.value = 42;
    return r.tag + r.value;
}
