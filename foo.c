#include <stdio.h>
#include <stdlib.h>

void foo0() {
    printf("foo0 is called\n");
    return;
}

int foo1(int a) {
    printf("foo1 is called(a=%d)\n", a);
    return a;
}

int foo3(int a, int b, int c) {
    printf("foo3 is called(a=%d, b=%d, c=%d)\n", a, b, c);
    return a + b + c;
}

int foo7(int a, int b, int c, int d, int e, int f, int g) {
    printf("foo7 is called(a=%d, b=%d, ...)\n", a, b);
    return a + b + c + d + e + f + g;
}

void print(int n) {
    printf("%d\n", n);
    return;
}