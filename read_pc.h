#ifndef READ_PC_h
#define READ_PC_h

// Snagged from linux/elfcore.h, user.h, procfs.h, ...

struct user_regs_struct{
    unsigned long long int r15;
    unsigned long long int r14;
    unsigned long long int r13;
    unsigned long long int r12;
    unsigned long long int rbp;
    unsigned long long int rbx;
    unsigned long long int r11;
    unsigned long long int r10;
    unsigned long long int r9;
    unsigned long long int r8;
    unsigned long long int rax;
    unsigned long long int rcx;
    unsigned long long int rdx;
    unsigned long long int rsi;
    unsigned long long int rdi;
    unsigned long long int orig_rax;
    unsigned long long int rip;
    unsigned long long int cs;
    unsigned long long int eflags;
    unsigned long long int rsp;
    unsigned long long int ss;
    unsigned long long int fs_base;
    unsigned long long int gs_base;
    unsigned long long int ds;
    unsigned long long int es;
    unsigned long long int fs;
    unsigned long long int gs;
};


typedef uint64_t elf_greg_t;
#define ELF_NGREG (sizeof (struct user_regs_struct) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];


struct elf_siginfo
{
    int si_signo;                       /* signal number */
    int si_code;                        /* extra code */
    int si_errno;                       /* errno */
};
/*
 * Definitions to generate Intel SVR4-like core files.
 * These mostly have the same names as the SVR4 types with "elf_"
 * tacked on the front to prevent clashes with linux definitions,
 * and the typedef forms have been avoided.  This is mostly like
 * the SVR4 structure, but more Linuxy, with things that Linux does
 * not support and which gdb doesn't really use excluded.
 * Fields present but not used are marked with "XXX".
 */
typedef struct elf_prstatus{
    struct elf_siginfo pr_info; /* Info associated with signal */
    short       pr_cursig;              /* Current signal */
    unsigned long pr_sigpend;   /* Set of pending signals */
    unsigned long pr_sighold;   /* Set of held signals */
    pid_t       pr_pid;
    pid_t       pr_ppid;
    pid_t       pr_pgrp;
    pid_t       pr_sid;
    struct timeval pr_utime;    /* User time */
    struct timeval pr_stime;    /* System time */
    struct timeval pr_cutime;   /* Cumulative user time */
    struct timeval pr_cstime;   /* Cumulative system time */
    struct user_regs_struct regs;
    int pr_fpvalid;             /* True if math co-processor being used.  */
} elf_prstatus_t;

#define ELF_PRARGSZ	(80)	/* Number of chars for args */
typedef struct elf_prpsinfo
{
	char	pr_state;	/* numeric process state */
	char	pr_sname;	/* char for pr_state */
	char	pr_zomb;	/* zombie */
	char	pr_nice;	/* nice val */
	unsigned long pr_flag;	/* flags */
        unsigned int pr_uid;
	unsigned int pr_gid;
	pid_t	pr_pid, pr_ppid, pr_pgrp, pr_sid;
	/* Lots missing */
	char	pr_fname[16];	/* filename of executable */
	char	pr_psargs[ELF_PRARGSZ];	/* initial part of arg list */
} elf_prpsinfo_t;


typedef struct options_t {
    int general_registers;
    int prstatus;
    int signal_info;
    int prog_info;
    int backtrace;
}options_t;
#define assert(x) assert_helper(__FILE__, __LINE__, #x, ((int)(intptr_t)(x)))
#define REGFMT "0x%018llx"

#endif // READ_PC_h
