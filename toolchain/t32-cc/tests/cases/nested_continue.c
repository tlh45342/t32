int main(void)
{
    int outer = 0;
    int inner = 0;
    int hits = 0;

    for (outer = 0; outer < 3; outer = outer + 1) {
        for (inner = 0; inner < 4; inner = inner + 1) {
            if (inner == 1)
                continue;
            hits = hits + 1;
        }
    }

    return hits;
}
