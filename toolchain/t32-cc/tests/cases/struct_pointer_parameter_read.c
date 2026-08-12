struct point {
    int x;
    int y;
};

int sum_point(struct point *p)
{
    return p->x + p->y;
}

int main(void)
{
    struct point a;

    a.x = 17;
    a.y = 25;
    return sum_point(&a);
}
