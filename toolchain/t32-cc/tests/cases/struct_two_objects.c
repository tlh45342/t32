struct pair {
    int a;
    int b;
};

int main(void)
{
    struct pair left;
    struct pair right;
    left.a = 11;
    left.b = 22;
    right.a = 33;
    right.b = 44;
    return left.b + right.a;
}
