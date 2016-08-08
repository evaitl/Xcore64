## Xcore64

This is an [answer](http://stackoverflow.com/questions/38330622/get-the-address-that-caused-segmentation-fault-from-core-dump-using-c) to a stack overflow question about how to get the address that caused a core dump from C code. 

This is a simple C program that mmaps a core file named on the command line and
dumps a bunch of debug information.  Only for an x86_64 linux system.

Very early stage, I'm gradually adding stuff that it dumps.

Planning to add:

* cleanup elf header output
* walk/display rest of the notes sections


Here is the current output:

```
evaitl@bb ~/se/read_pc $ cat foo.c 
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
evaitl@bb ~/se/read_pc $ make
gcc -Wall -O3 -Werror -c read_pc.c
gcc  -o read_pc read_pc.o -lelf
gcc -O0 -o foo foo.c
evaitl@bb ~/se/read_pc $ ./foo 
0
1
2
3
Segmentation fault (core dumped)
evaitl@bb ~/se/read_pc $ ./read_pc 
usage: read_pc [-b] [-i] [-f] [-p] [-r] [-s] [-t] core
    -f file headers
    -b backtrace
    -p program headers
    -i program info
    -r general registers
    -s signal info
    -t program status

errno: Success
evaitl@bb ~/se/read_pc $ ./read_pc -bfiprst core 
General Registers: 
r15     0x000000000000000000  r14     0x000000000000000000  
r13     0x0000007ffc57895c30  r12     0x000000000000400430  
rbp     0x0000007ffc57895b30  rbx     0x000000000000000000  
r11     0x000000000000000246  r10     0x000000000000000000  
r9      0x000000000000000002  r8      0x000000000000000000  
rax     0x000000000000000003  rcx     0x00000000007ffffffe  
rdx     0x0000007f5cca175780  rsi     0x000000000000000001  
rdi     0x000000000000000001  ss      0x00000000000000002b  
rip     0x000000000000400560  cs      0x000000000000000033  
eflags  0x000000000000010246  rsp     0x0000007ffc57895b30  
fs_base 0x0000007f5cca375700  gs_base 0x000000000000000000  
ds      0x000000000000000000  es      0x000000000000000000  
fs      0x000000000000000000  gs      0x000000000000000000  
orig_rax 0x00ffffffffffffffff  

Program status: 
signo 11 signal code 0 errno 0
cursig 11 sigpend 000000000000000000 sigheld 000000000000000000
pid 27927 ppid 27891 pgrp 27927 sid 27891
utime: 0.000000 stime 0.004000
cutime: 0.000000 cstime 0.000000
fpvalid: 1


Signal Information: 
signo: 11 errno 0 code 1
addr 0x3 addr_lsb 0 addr_bnd ((nil), (nil))


Process Information:
state 0 (R) zombie 0 nice 0 flags 0x400600
uid 1000 gid 1000 pid 27927 ppid 27891 pgrp 27927 sid 27891
fname: foo
args: ./foo 


Backtrace: 
rip = 0x000000000000400560
rip = 0x000000000000400591
rip = 0x0000000000004005a1


File header:
Magic:     7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
Type:                          4
Machine:                       62
Version:                       1
Entry:                         0
phoff:                         64
shoff:                         0
Flags:                         0
Elf Header Size:               64
phentsize:                     56
phnum:                         20
shentsize:                     0
shnum:                         0
shstrndx:                      0


Program Headers:
   Type      Offset             Virt Addr          PhysAddr          
             FileSiz            MemSize              Flags  Align    
 NOTE      0x00000000000004a0 0x0000000000000000 0000000000000000
           0x0000000000000b98 0x0000000000000000         0x000000
 LOAD      0x0000000000002000 0x0000000000400000 0000000000000000
           0x0000000000001000 0x0000000000001000 R X     0x001000
 LOAD      0x0000000000003000 0x0000000000600000 0000000000000000
           0x0000000000001000 0x0000000000001000   X     0x001000
 LOAD      0x0000000000004000 0x0000000000601000 0000000000000000
           0x0000000000001000 0x0000000000001000  WX     0x001000
 LOAD      0x0000000000005000 0x0000000000c8c000 0000000000000000
           0x0000000000021000 0x0000000000021000  WX     0x001000
 LOAD      0x0000000000026000 0x00007f5cc9db0000 0000000000000000
           0x0000000000001000 0x00000000001c0000 R X     0x001000
 LOAD      0x0000000000027000 0x00007f5cc9f70000 0000000000000000
           0x0000000000000000 0x00000000001ff000         0x001000
 LOAD      0x0000000000027000 0x00007f5cca16f000 0000000000000000
           0x0000000000004000 0x0000000000004000   X     0x001000
 LOAD      0x000000000002b000 0x00007f5cca173000 0000000000000000
           0x0000000000002000 0x0000000000002000  WX     0x001000
 LOAD      0x000000000002d000 0x00007f5cca175000 0000000000000000
           0x0000000000004000 0x0000000000004000  WX     0x001000
 LOAD      0x0000000000031000 0x00007f5cca179000 0000000000000000
           0x0000000000001000 0x0000000000026000 R X     0x001000
 LOAD      0x0000000000032000 0x00007f5cca374000 0000000000000000
           0x0000000000003000 0x0000000000003000  WX     0x001000
 LOAD      0x0000000000035000 0x00007f5cca39c000 0000000000000000
           0x0000000000002000 0x0000000000002000  WX     0x001000
 LOAD      0x0000000000037000 0x00007f5cca39e000 0000000000000000
           0x0000000000001000 0x0000000000001000   X     0x001000
 LOAD      0x0000000000038000 0x00007f5cca39f000 0000000000000000
           0x0000000000001000 0x0000000000001000  WX     0x001000
 LOAD      0x0000000000039000 0x00007f5cca3a0000 0000000000000000
           0x0000000000001000 0x0000000000001000  WX     0x001000
 LOAD      0x000000000003a000 0x00007ffc57876000 0000000000000000
           0x0000000000022000 0x0000000000022000  WX     0x001000
 LOAD      0x000000000005c000 0x00007ffc578da000 0000000000000000
           0x0000000000002000 0x0000000000002000   X     0x001000
 LOAD      0x000000000005e000 0x00007ffc578dc000 0000000000000000
           0x0000000000002000 0x0000000000002000 R X     0x001000
 LOAD      0x0000000000060000 0xffffffffff600000 0000000000000000
           0x0000000000001000 0x0000000000001000 R X     0x001000
All worked
```
