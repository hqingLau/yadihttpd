#include <stdio.h>
#include <stdlib.h>

int main2()
{
    int *n = (int *)malloc(sizeof(int));
    *n = 1;
    int **p = &n;
    int *n2 = *p;
    *n2 = 2;
    printf("%d %d\n",*n, *n2);
}