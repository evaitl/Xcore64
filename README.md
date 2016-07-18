# read_pc

Simple program that mmaps a core file named on the command line and
dumps the registers.  Only for an x86_64 linux system.

Very early stage, I'm gradually adding stuff that it dumps. Here is
the current output:

```
evaitl@bb ~/se/read_pc $ cat foo.c 
#include <stdio.h>
int main(){
    for(int i=0; i<10; ++i){
        printf("%d\n",i);
        if(i==3){
            *(int *)i=3;
        }
    }
    return 0;
}
evaitl@bb ~/se/read_pc $ ./read_pc 
usage: read_pc [-b] [-i] [-r] [-s] [-t] core
    -b backtrace
    -i program info
    -r general registers
    -s signal info
    -t program status

evaitl@bb ~/se/read_pc $ ./read_pc -ristb core
General Registers: 
r15     0x000000000000000000  r14     0x000000000000000000  
r13     0x0000007ffdc41a3ae0  r12     0x000000000000400430  
rbp     0x0000007ffdc41a3a00  rbx     0x000000000000000000  
r11     0x000000000000000246  r10     0x000000000000000000  
r9      0x000000000000000002  r8      0x000000000000000000  
rax     0x000000000000000003  rcx     0x00000000007ffffffe  
rdx     0x0000007fe9e5afa780  rsi     0x000000000000000001  
rdi     0x000000000000000001  ss      0x00000000000000002b  
rip     0x000000000000400556  cs      0x000000000000000033  
eflags  0x000000000000010246  rsp     0x0000007ffdc41a39f0  
fs_base 0x0000007fe9e5cfa700  gs_base 0x000000000000000000  
ds      0x000000000000000000  es      0x000000000000000000  
fs      0x000000000000000000  gs      0x000000000000000000  
orig_rax 0x00ffffffffffffffff  

Program status: 
signo 11 signal code 0 errno 0
cursig 11 sigpend 000000000000000000 sigheld 000000000000000000
pid 4154 ppid 2932 pgrp 4154 sid 2932
utime: 0.000000 stime 0.000000
cutime: 0.000000 cstime 0.000000
fpvalid: 1


Signal Information: 
signo: 11 errno 0 code 1
addr 0x3 addr_lsb 0 addr_bnd ((nil), (nil))


Process Information:
state 0 (R) zombie 0 nice 0 flags 0x400600
uid 1000 gid 1000 pid 4154 ppid 2932 pgrp 4154 sid 2932
fname: foo
args: ./foo 


All worked
```
