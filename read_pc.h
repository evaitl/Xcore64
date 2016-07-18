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
	int	si_signo;			/* signal number */
	int	si_code;			/* extra code */
	int	si_errno;			/* errno */
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
typedef struct elf_prstatus
{
#if 0
	long	pr_flags;	/* XXX Process flags */
	short	pr_why;		/* XXX Reason for process halt */
	short	pr_what;	/* XXX More detailed reason */
#endif
	struct elf_siginfo pr_info;	/* Info associated with signal */
	short	pr_cursig;		/* Current signal */
	unsigned long pr_sigpend;	/* Set of pending signals */
	unsigned long pr_sighold;	/* Set of held signals */
#if 0
	struct sigaltstack pr_altstack;	/* Alternate stack info */
	struct sigaction pr_action;	/* Signal action for current sig */
#endif
	pid_t	pr_pid;
	pid_t	pr_ppid;
	pid_t	pr_pgrp;
	pid_t	pr_sid;
	struct timeval pr_utime;	/* User time */
	struct timeval pr_stime;	/* System time */
	struct timeval pr_cutime;	/* Cumulative user time */
	struct timeval pr_cstime;	/* Cumulative system time */
#if 0
	long	pr_instr;		/* Current instruction */
#endif
//	elf_gregset_t pr_reg;	/* GP registers */
    struct user_regs_struct pr_reg;
#ifdef CONFIG_BINFMT_ELF_FDPIC
	/* When using FDPIC, the loadmap addresses need to be communicated
	 * to GDB in order for GDB to do the necessary relocations.  The
	 * fields (below) used to communicate this information are placed
	 * immediately after ``pr_reg'', so that the loadmap addresses may
	 * be viewed as part of the register set if so desired.
	 */
	unsigned long pr_exec_fdpic_loadmap;
	unsigned long pr_interp_fdpic_loadmap;
#endif
	int pr_fpvalid;		/* True if math co-processor being used.  */
} elf_prstatus;

#endif // READ_PC_h
