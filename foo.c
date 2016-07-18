#include <stdio.h>
#include <stdint.h>
volatile int i;
int bar(){
    for(i=0; i<10; ++i){
        printf("%d\n",i);
        if(i==3){
            *(int *)(intptr_t)i=3;
        }
    }
}
int foo(){
    return bar();
}
int main(){
    return foo();
}
