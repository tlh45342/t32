struct point {
    int x;
    int y;
};

int set_point(struct point *p)
{
    p->x = 40;
    p->y = 2;
    return 0;
}

int main(void)
{
    struct point a;

    a.x = 0;
    a.y = 0;
    set_point(&a);
    return a.x + a.y;
}
