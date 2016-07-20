#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
void sighandler(int sig){
    char *s="In signal handler\n";
    write(1, s,strlen(s));
//    abort();
}
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
struct sigaction sa;
int main(){
    sa.sa_handler=sighandler;
    sigaction(SIGSEGV, &sa, 0);
    return foo();
}
