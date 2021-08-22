#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "klee/klee.h"
int add(int x){
    return x + 1;
}
int main(){
    /*
    struct t{
        int x;
        int y;
    } kk;
    printf("%d \n", sizeof(kk));
    klee_make_symbolic(&kk, sizeof(struct t), "kk");
    klee_print_expr("kk= ", kk);
    */
    //int x = klee_range(10, 20, "a");
    int x;
    klee_make_symbolic(&x, sizeof(int), "xx");
    char *data =(char*) malloc(100);
    klee_make_symbolic(data, 100, "data");
    strcpy(data, "hello world");
    strcat(data, "what??/");
    klee_print_expr("data", *data);

    if( x > 100){
        klee_print_expr("myx1", x);
    }
    else if (x > 90) {
        klee_print_expr("myx2", x);

    }
    else if(x > 50){
        klee_print_expr("myx3", x);

    }
    else{
        klee_print_expr("myx4", x);
    }

}
