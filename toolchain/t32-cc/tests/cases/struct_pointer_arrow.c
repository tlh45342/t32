struct point {
    int x;
    int y;
};

int main(void)
{
    struct point p;
    struct point *q = &p;
    q->x = 40;
    q->y = 2;
    return q->x + q->y;
}
