## read_pc

Simple program that mmaps a core file named on the command line and
dumps a bunch of debug information.  Only for an x86_64 linux system.

Very early stage, I'm gradually adding stuff that it dumps.

Planning to add:

* backtrace
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
evaitl@bb ~/se/read_pc $ ./read_pc 
usage: read_pc [-b] [-i] [-r] [-s] [-t] core
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
r13     0x0000007fffe05c9690  r12     0x000000000000400430  
rbp     0x0000007fffe05c9590  rbx     0x000000000000000000  
r11     0x000000000000000246  r10     0x000000000000000000  
r9      0x000000000000000002  r8      0x000000000000000000  
rax     0x000000000000000003  rcx     0x00000000007ffffffe  
rdx     0x0000007fd1f6473780  rsi     0x000000000000000001  
rdi     0x000000000000000001  ss      0x00000000000000002b  
rip     0x000000000000400560  cs      0x000000000000000033  
eflags  0x000000000000010246  rsp     0x0000007fffe05c9590  
fs_base 0x0000007fd1f6673700  gs_base 0x000000000000000000  
ds      0x000000000000000000  es      0x000000000000000000  
fs      0x000000000000000000  gs      0x000000000000000000  
orig_rax 0x00ffffffffffffffff  

Program status: 
signo 11 signal code 0 errno 0
cursig 11 sigpend 000000000000000000 sigheld 000000000000000000
pid 22438 ppid 19375 pgrp 22438 sid 19375
utime: 0.000000 stime 0.000000
cutime: 0.000000 cstime 0.000000
fpvalid: 1


Signal Information: 
signo: 11 errno 0 code 1
addr 0x3 addr_lsb 0 addr_bnd ((nil), (nil))


Process Information:
state 0 (R) zombie 0 nice 0 flags 0x400600
uid 1000 gid 1000 pid 22438 ppid 19375 pgrp 22438 sid 19375
fname: foo
args: ./foo bar bash no not bash pythonsh 


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
 LOAD      0x0000000000005000 0x0000000001dfb000 0000000000000000
           0x0000000000021000 0x0000000000021000  WX     0x001000
 LOAD      0x0000000000026000 0x00007fd1f60ae000 0000000000000000
           0x0000000000001000 0x00000000001c0000 R X     0x001000
 LOAD      0x0000000000027000 0x00007fd1f626e000 0000000000000000
           0x0000000000000000 0x00000000001ff000         0x001000
 LOAD      0x0000000000027000 0x00007fd1f646d000 0000000000000000
           0x0000000000004000 0x0000000000004000   X     0x001000
 LOAD      0x000000000002b000 0x00007fd1f6471000 0000000000000000
           0x0000000000002000 0x0000000000002000  WX     0x001000
 LOAD      0x000000000002d000 0x00007fd1f6473000 0000000000000000
           0x0000000000004000 0x0000000000004000  WX     0x001000
 LOAD      0x0000000000031000 0x00007fd1f6477000 0000000000000000
           0x0000000000001000 0x0000000000026000 R X     0x001000
 LOAD      0x0000000000032000 0x00007fd1f6672000 0000000000000000
           0x0000000000003000 0x0000000000003000  WX     0x001000
 LOAD      0x0000000000035000 0x00007fd1f669a000 0000000000000000
           0x0000000000002000 0x0000000000002000  WX     0x001000
 LOAD      0x0000000000037000 0x00007fd1f669c000 0000000000000000
           0x0000000000001000 0x0000000000001000   X     0x001000
 LOAD      0x0000000000038000 0x00007fd1f669d000 0000000000000000
           0x0000000000001000 0x0000000000001000  WX     0x001000
 LOAD      0x0000000000039000 0x00007fd1f669e000 0000000000000000
           0x0000000000001000 0x0000000000001000  WX     0x001000
 LOAD      0x000000000003a000 0x00007fffe05a9000 0000000000000000
           0x0000000000022000 0x0000000000022000  WX     0x001000
 LOAD      0x000000000005c000 0x00007fffe05eb000 0000000000000000
           0x0000000000002000 0x0000000000002000   X     0x001000
 LOAD      0x000000000005e000 0x00007fffe05ed000 0000000000000000
           0x0000000000002000 0x0000000000002000 R X     0x001000
 LOAD      0x0000000000060000 0xffffffffff600000 0000000000000000
           0x0000000000001000 0x0000000000001000 R X     0x001000
All worked
```
