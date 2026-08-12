struct point {
    int x;
    int y;
};

int bad(struct point p)
{
    return p.x;
}

int main(void)
{
    return 0;
}
