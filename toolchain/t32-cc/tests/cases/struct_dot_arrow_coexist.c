struct point {
    int x;
    int y;
};

int main(void)
{
    struct point p;
    struct point *q;

    p.x = 10;
    p.y = 20;
    q = &p;
    q->y = 32;

    return p.x + q->y;
}
