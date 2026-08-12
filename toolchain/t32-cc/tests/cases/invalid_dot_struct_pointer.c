struct point { int x; };
int main(void)
{
    struct point p;
    struct point *q = &p;
    return q.x;
}
