#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>

int main() {
    int a;
    fscanf(stdin, "%d", &a);
    fprintf(stdout, "%d\n", a + 1);
    return 0;
}