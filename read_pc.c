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
#include <signal.h>

#include "read_pc.h"


static options_t options;

static void die(const char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr,"\nerrno: %m\n");
    va_end(ap);
    exit(1);
}
static inline void assert_helper(const char *file, int line, const char *str, int x){
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
        die("Couldn't open file '%s':",fname);
    }
    return ret;
}
static inline size_t get_len(int fd){
    struct stat stat_buf;
    if(fstat(fd,&stat_buf)){
        die("fstat");
    }
    return stat_buf.st_size;
}
#if 0
static void hexdump(void *p){
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
#endif
static inline void *Mmap(void *addr, size_t len, int prot,
                         int flags, int fd, off_t offset){
    void *ret=mmap(addr,len,prot,flags,fd,offset);
    if(ret==MAP_FAILED){
        die("mmap");
    }
    return ret;
}
static inline void Munmap(void *addr, size_t len){
    if(munmap(addr,len)){
        die("munmap");
    }
}
static inline void Fstat(int fd, struct stat *buf){
    if(fstat(fd,buf)){
        die("fstat");
    }
}

/*
  Returns p rounded up to next 8.
*/
static inline uint64_t roundup8(uint64_t p){
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
static void *get_note(void *vp, int nt_type){
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
            if(current_note->n_type == nt_type){
                return note_end;
                
            }
            note_end += roundup8(current_note->n_descsz);
            current_note=note_end;          
        }
    }
    return 0;
}

static void usage(void){
    die("usage: read_pc [-b] [-i] [-r] [-s] [-t] core\n"
	"    -f file headers\n"
        "    -b backtrace\n"
	"    -p program headers\n"
        "    -i program info\n"
        "    -r general registers\n"
        "    -s signal info\n"
        "    -t program status\n"
        );
}
static void parse_options(int argc, char **argv){
    int opt;
    while((opt=getopt(argc, argv, "bfiprst"))!=-1){
        switch(opt){
        case 'b':
            options.backtrace=1;
            break;
	case 'f':
	    options.file_headers=1;
	    break;
        case 'i':
            options.prog_info=1;
            break;
        case 'p':
            options.segments = 1;
            break;
        case 'r':
            options.general_registers=1;
            break;
        case 's':
            options.signal_info=1;
            break;
        case 't':
            options.prstatus=1;
            break;
        default:
            usage();
        }
    }
    if(argc-optind !=1){
        usage();
    }
}
static void print_prstatus(elf_prstatus_t *prs){
    assert(prs);
    printf("Program status: \n");
    printf("signo %d signal code %d errno %d\n",
           prs->pr_info.si_signo, 
           prs->pr_info.si_code, 
           prs->pr_info.si_errno);
    printf("cursig %d sigpend %#018lx sigheld %#018lx\n",
           prs->pr_cursig,
           prs->pr_sigpend,
           prs->pr_sighold);
    printf("pid %d ppid %d pgrp %d sid %d\n",
           prs->pr_pid,prs->pr_ppid,
           prs->pr_pgrp,prs->pr_sid);
    // times...
    printf("utime: %ld.%06ld stime %ld.%06ld\n",
           prs->pr_utime.tv_sec,
           prs->pr_utime.tv_usec,
           prs->pr_stime.tv_sec,
           prs->pr_stime.tv_usec);
    printf("cutime: %ld.%06ld cstime %ld.%06ld\n",
           prs->pr_cutime.tv_sec,
           prs->pr_cutime.tv_usec,
           prs->pr_cstime.tv_sec,
           prs->pr_cstime.tv_usec);
    printf("fpvalid: %d\n",prs->pr_fpvalid);
    printf("\n\n");
}

static void print_regs(elf_prstatus_t *prs){
    assert(prs);
    printf("General Registers: \n");
    printf("r15     " REGFMT "  ",prs->regs.r15);
    printf("r14     " REGFMT "  ",prs->regs.r14);
    putchar('\n');
    printf("r13     " REGFMT "  ",prs->regs.r13);
    printf("r12     " REGFMT "  ",prs->regs.r12);
    putchar('\n');
    printf("rbp     " REGFMT "  ",prs->regs.rbp);
    printf("rbx     " REGFMT "  ",prs->regs.rbx);
    putchar('\n');
    printf("r11     " REGFMT "  ",prs->regs.r11);
    printf("r10     " REGFMT "  ",prs->regs.r10);
    putchar('\n');
    printf("r9      " REGFMT "  ",prs->regs.r9);
    printf("r8      " REGFMT "  ",prs->regs.r8);
    putchar('\n');
    printf("rax     " REGFMT "  ",prs->regs.rax);
    printf("rcx     " REGFMT "  ",prs->regs.rcx);
    putchar('\n');
    printf("rdx     " REGFMT "  ",prs->regs.rdx);
    printf("rsi     " REGFMT "  ",prs->regs.rsi);
    putchar('\n');
    printf("rdi     " REGFMT "  ",prs->regs.rdi);
    printf("ss      " REGFMT "  ",prs->regs.ss);
    putchar('\n');
    printf("rip     " REGFMT "  ",prs->regs.rip);
    printf("cs      " REGFMT "  ",prs->regs.cs);
    putchar('\n');
    printf("eflags  " REGFMT "  ",prs->regs.eflags);
    printf("rsp     " REGFMT "  ",prs->regs.rsp);
    putchar('\n');
    printf("fs_base " REGFMT "  ",prs->regs.fs_base);
    printf("gs_base " REGFMT "  ",prs->regs.gs_base);
    putchar('\n');
    printf("ds      " REGFMT "  ",prs->regs.ds);
    printf("es      " REGFMT "  ",prs->regs.es);
    putchar('\n');
    printf("fs      " REGFMT "  ",prs->regs.fs);
    printf("gs      " REGFMT "  ",prs->regs.gs);
    putchar('\n');
    printf("orig_rax " REGFMT "  ",prs->regs.orig_rax);

    printf("\n\n");
}

/*
  \param addr The address we are interested in.

  \return Phdr for loadable segment that contains addr
 */
static Elf64_Phdr *get_loaded_segment(void *vp,uint64_t addr){
    Elf64_Ehdr *eh=vp;
    for(int i=0; i<eh->e_phnum; ++i){
        Elf64_Phdr *ph=(vp+eh->e_phoff+i*eh->e_phentsize);
        if(ph->p_type!=PT_LOAD){
            continue;
        }
	uint64_t lower_bound=ph->p_vaddr;
	uint64_t upper_bound=ph->p_vaddr + ph->p_filesz;
	if(addr >= lower_bound && addr<=upper_bound){
	    return ph;
	}
    }
    return 0;
}

static void print_backtrace(void *mp){
    assert(mp);
    elf_prstatus_t *prs = get_note(mp,NT_PRSTATUS);
    assert(prs);
    Elf64_Phdr *ph=get_loaded_segment(mp,
				      prs->regs.rbp);
    printf("Backtrace: \n");
    if(!ph){
	printf("Can't backtrace from addr "REGFMT"\n",
	       prs->regs.rbp);
    }
    printf("offset %lx vaddr %lx\n",
	   ph->p_offset, ph->p_vaddr);
    printf("Working on this still....Come back soon.\n");
    // XXX
}
/*
  This one is problematic. I'm guessing from the sigaction manpage,
  but I haven't looked at the kernel source to see which fields in the
  sigaction_t union are actually being filled out. 

  Use at your own risk. 
 */
static void print_signal_info(void *mp){
    assert(mp);
    printf("Signal Information: \n");
    siginfo_t *si=get_note(mp,NT_SIGINFO);
    assert(si);
    printf("signo: %d errno %d code %d\n",
           si->si_signo, si->si_errno, si->si_code);
    switch(si->si_code){
    case SI_TKILL:
    case SI_USER: // kill(2)
        printf("pid %d uid %d\n", si->si_pid, si->si_uid);
        return;
    case SI_TIMER: // posix timer
        printf("tid %d overrun %d sigval %d\n",
               si->si_timerid, si->si_overrun,
               si->_sifields._timer.si_sigval.sival_int);
        return;
    default:
        break;
    }
    switch(si->si_signo){
    case SIGCHLD: // pid, uid , status, utime, stime.
        printf("pid %d uid %d status %d utime %ld stime %ld\n",
               si->si_pid,si->si_uid,
               si->si_status,
               si->si_utime,
               si->si_stime
            );
        break;
    case SIGILL:
    case SIGFPE:
    case SIGSEGV:
    case SIGBUS: //  addr, addr_lsb, addr_bnd
        printf("addr %p addr_lsb %#x addr_bnd (%p, %p)\n",
               si->si_addr,
               si->si_addr_lsb,
               si->si_lower,
               si->si_upper
            );
        break;
    case SIGPOLL: // band, fd
        printf("band %ld fd %d\n",si->si_band, si->si_fd);
        break;
    case SIGSYS: // calladdr, syscall, arch
        printf("call_addr %p syscall %d arch %x\n",
               si->si_call_addr, si->si_syscall,
               si->si_arch);
        break;
    }
    printf("\n\n");
}
static void print_prog_info(void *mp){
    elf_prpsinfo_t *pi=get_note(mp,NT_PRPSINFO);
    if(!pi){
        die("no propsinfo");
    }
    printf("Process Information:\n");
    printf("state %d (%c) zombie %d nice %d flags %#lx\n"
           "uid %d gid %d pid %d ppid %d pgrp %d sid %d\n"
           "fname: %s\n"
           "args: %s\n",
           pi->pr_state,
           pi->pr_sname,
           pi->pr_zomb,
           pi->pr_nice,
           pi->pr_flag,
           pi->pr_uid,
           pi->pr_gid,
           pi->pr_pid, pi->pr_ppid, pi->pr_pgrp, pi->pr_sid,
           pi->pr_fname,
           pi->pr_psargs);

    printf("\n\n");
}

static char *ph_type_str(int ptype){
    static char buf[50];
    switch(ptype){
    case PT_NULL: return "NULL";
    case PT_LOAD: return "LOAD";
    case PT_DYNAMIC: return "DYNAMIC";
    case PT_INTERP: return "INTERP";
    case PT_NOTE: return "NOTE";
    case PT_SHLIB: return "SHLIB";
    case PT_PHDR: return "PHDR";
    case PT_TLS: return "TLS";
    case	PT_NUM: return "NUM";
    case PT_LOOS: return "LOOS";
    case PT_GNU_EH_FRAME: return "EH_FRAME";
    case PT_GNU_STACK: return "STACK";
    case PT_GNU_RELRO: return "RELRO";
    case PT_HIOS: return "HIOS";
    default:
	snprintf(buf,sizeof(buf),"%#x",ptype);
    }
    return buf;
}
static char *ph_flags(int flags){
    static char buf[50];
    snprintf(buf,sizeof(buf),"%c%c%c",
	     (flags & 0x01 ? 'R': ' '),
	     (flags & 0x02 ? 'W': ' '),
	     (flags & 0x04 ? 'X': ' '));
    return buf;
}
static void print_ph(Elf64_Phdr *ph){
    printf(" %-10s0x%016lx 0x%016lx %016lx\n"
	   "           0x%016lx 0x%016lx %-6s"
	   "  0x%06lx\n",
	   ph_type_str(ph->p_type),
	   ph->p_offset, ph->p_vaddr, ph->p_paddr,
	   ph->p_filesz, ph->p_memsz, 
	   ph_flags(ph->p_flags),ph->p_align);
}

static void print_segments(void *vp){
    printf("Program Headers:\n");
     Elf64_Ehdr *eh=vp;
     printf("%-10s   %-18s %-18s %-18s\n",
	    "   Type", "Offset", "Virt Addr", "PhysAddr");
     printf("%-10s   %-18s %-18s %-18s\n",
	    "  ", "FileSiz", "MemSize", "  Flags  Align");
     for(int i=0; i<eh->e_phnum; ++i){
        Elf64_Phdr *ph=(vp+eh->e_phoff+i*eh->e_phentsize);
	print_ph(ph);
    }
}
static char *hexstr(const unsigned char *cp){
    static char buf[80];
    snprintf(buf,sizeof(buf),
	     "%02x %02x %02x %02x "
	     "%02x %02x %02x %02x "
	     "%02x %02x %02x %02x "
	     "%02x %02x %02x %02x",
	     cp[0],cp[1],cp[2],cp[3],
	     cp[4],cp[5],cp[6],cp[7],
	     cp[8],cp[9],cp[10],cp[11],
	     cp[12],cp[13],cp[14],cp[15]);
    return buf;
}

/*
  Unfinished. Prints the elf file header. 

  TODO: Make this look pretty. 
 */
static void print_file(void *vp){
    Elf64_Ehdr *eh=vp;
    
    printf("File header:\n");
    printf("%-10s %s\n",
	   "Magic:", hexstr(eh->e_ident));
    printf("%-30s %d\n",
	   "Type:",eh->e_type);
    printf("%-30s %d\n",
	   "Machine:",eh->e_machine);
    printf("%-30s %d\n",
	   "Version:",eh->e_version);
    printf("%-30s %ld\n",
	   "Entry:",eh->e_entry);
    printf("%-30s %ld\n",
	   "phoff:",eh->e_phoff);
    printf("%-30s %ld\n",
	   "shoff:",eh->e_shoff);
    printf("%-30s %d\n",
	   "Flags:",eh->e_flags);
    printf("%-30s %d\n",
	   "Elf Header Size:",eh->e_ehsize);
    printf("%-30s %d\n",
	   "phentsize:",eh->e_phentsize);
    printf("%-30s %d\n",
	   "phnum:",eh->e_phnum);
    printf("%-30s %d\n",
	   "shentsize:",eh->e_shentsize);
    printf("%-30s %d\n",
	   "shnum:",eh->e_shnum);
    printf("%-30s %d\n",
	   "shstrndx:",eh->e_shstrndx);
    printf("\n\n");
}
/*
  Get and print the pc of a core dump. Assumes an x86_64 linux core file. 
*/
int main(int argc, char **argv){
    parse_options(argc, argv);
    int fd=Open(argv[optind],O_RDONLY);
    struct stat stat_buf;
    Fstat(fd,&stat_buf);
    size_t len=get_len(fd);
    void *mp=Mmap(0,len,PROT_READ,MAP_PRIVATE,fd,0);
    // magic for 64 bit little endian elf. 
    static char magic_ident[]="\x7f""ELF\x02\x01\x01";
    if(memcmp(magic_ident, mp, 7)){
        die("Not a x86_64 elf file.");
    }
    elf_prstatus_t * prs = get_note(mp,NT_PRSTATUS);
    if(!prs){
        die("No prstatus note found. ");
    }
    if(options.general_registers){
        print_regs(prs);
    }else{
        printf("rip "REGFMT"\n",prs->regs.rip);
    }
    if(options.prstatus){
        print_prstatus(prs);
    }
    if(options.signal_info){
        print_signal_info(mp);
    }
    if(options.prog_info){
        print_prog_info(mp);
    }
    if(options.backtrace){
        print_backtrace(mp);
    }
    if(options.file_headers){
	print_file(mp);
    }
    if(options.segments){
	print_segments(mp);
    }
    assert(prs!=0);
    Munmap(mp,len);
    close(fd);
    printf("All worked\n");
    return 0;
}
