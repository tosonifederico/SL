#include "./SL.h"


int main() {
    Set *s = New_Set();

    int a = 1;

    s->insert(s, &a, sizeof(int));

    a = 4;

    printf("%d\n", s->lookup(s, &a, sizeof(a)));

    s->delete(s, &a, sizeof(a));

    return 0;
}

