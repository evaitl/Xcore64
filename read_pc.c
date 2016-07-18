#include <stdarg.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

//#include <linux/elfcore.h>
#include "read_pc.h"

#define assert(x) assert_helper(__FILE__, __LINE__, #x, (int)(x))

static void usage(void){
    printf("usage: read_pc core");
    exit(1);
}
static void die(const char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr,"\nerrno: %m\n");
    va_end(ap);
    exit(1);
}
static void assert_helper(const char *file, int line, const char *str, int x){
    if(!x){
        die("Assert fail:%s:%d: %s", file, line, str);
    }
}
static inline int Open(const char *fname, int flags, ...){
    va_list ap;
    va_start(ap,flags);
    int mode=va_arg(ap,int);
    int ret=open(fname, flags, mode);
    if (ret==-1){
        die("open error");
    }
    return ret;
}
static size_t get_len(int fd){
    struct stat stat_buf;
    if(fstat(fd,&stat_buf)){
        die("fstat");
    }
    return stat_buf.st_size;
}

void hexdump(void *p){
    unsigned char *cp=p;
    for(int i=0;i<0xb80; ++i){
        if( *(uint64_t *) &cp[i] == 0x400556){
            printf("rip offset %d\n",i);
        }
    }
    for(int i=0; i<60; ++i){
        printf("%08lx %02x %02x %02x %02x  %02x %02x %02x %02x "
               "%02x %02x %02x %02x  %02x %02x %02x %02x\n",
               (unsigned long)((intptr_t)cp-(intptr_t)p),
               cp[0], cp[1], cp[2], cp[3], 
               cp[4], cp[5], cp[6], cp[7],
               cp[8], cp[9], cp[10], cp[11],
               cp[12], cp[13], cp[14], cp[15]);
        cp+=16;
    }
}
static void *Mmap(void *addr, size_t len, int prot,
                  int flags, int fd, off_t offset){
    void *ret=mmap(addr,len,prot,flags,fd,offset);
    if(ret==MAP_FAILED){
        die("mmap");
    }
    return ret;
}
static void Munmap(void *addr, size_t len){
    if(munmap(addr,len)){
        die("munmap");
    }
}
static void Fstat(int fd, struct stat *buf){
    if(fstat(fd,buf)){
        die("fstat");
    }
}

/*
  Returns p rounded up to next 8.
*/
static uint64_t roundup8(uint64_t p){
    if(p%8){
        return p+8-p%8;
    }
    return p;
}


/** 

    \param vp  Points at memory mapped elf file. 
    \return A pointer to an elf_prstatus structure, if found, else 0.

    Verify this is an elf file. 

    Walk the program headers looking for ones of type PT_NOTE.
    In each PT_NOTE section, look for a note of type NT_PRSTATUS

    n_descsz in that note will point at the file offset of an
    elf_prstatus64, which contains the saved GP registers.

    FP, SSE, MMX, and such registers are saved in a NT_X86_XSTATE
    note from the XSAVE instruction, so aren't found in this note. 

    Refs:

    [x86_64 System V ABI] (http://www.x86-64.org/documentation/abi.pdf) 
    [Elf 64 object file format] (https://www.uclibc.org/docs/elf-64-gen.pdf)

*/
elf_prstatus *get_prstatus(void *vp){
    // magic for 64 bit little endian elf. 
    char magic_ident[]="\x7f""ELF\x02\x01\x01";
    if(memcmp(magic_ident, vp, 7)){
        die("Not correct elf. ");
    }
    Elf64_Ehdr *eh=vp;
    for(int i=0; i<eh->e_phnum; ++i){
        Elf64_Phdr *ph=(vp+eh->e_phoff+i*eh->e_phentsize);
        if(ph->p_type!=PT_NOTE){
            continue;
        }
        void *note_table=(vp + ph->p_offset);
        void *note_table_end=(note_table+ph->p_filesz);
        Elf64_Nhdr *current_note=note_table;
        while(current_note<(Elf64_Nhdr *)note_table_end){
            void *note_end=current_note;
            note_end += 3*sizeof(Elf64_Word);
            note_end += roundup8(current_note->n_namesz);
            if(current_note->n_type == NT_PRSTATUS){
                return note_end;
            }
            note_end += roundup8(current_note->n_descsz);
            current_note=note_end;          
        }
    }
    return 0;
}

/*
  Get and print the pc of a core dump. Assumes an x86_64 linux core file. 
*/
int main(int argc, char **argv){
    if(argc !=2){
        usage();
    }    
    int fd=Open(argv[1],O_RDONLY);
    struct stat stat_buf;
    Fstat(fd,&stat_buf);
    size_t len=get_len(fd);
    void *mp=Mmap(0,len,PROT_READ,MAP_PRIVATE,fd,0);

    elf_prstatus * prs = get_prstatus(mp);
    if(!prs){
        die("No prstatus note found. ");
    }
    assert(prs!=0);
        printf("rip %010llx rsp %010llx rbp %010llx eflags %010llx\n"
	       "rax %010llx rbx %010llx rcx %010llx rdx %010llx\n",
	       prs->pr_reg.rip, prs->pr_reg.rsp,
	       prs->pr_reg.rbp, prs->pr_reg.eflags,
	       prs->pr_reg.rax, prs->pr_reg.rbx,
	       prs->pr_reg.rcx, prs->pr_reg.rdx);
//    hexdump(&prstatus->pr_reg);
    
    Munmap(mp,len);
    close(fd);
    printf("All worked\n");
    return 0;
}
