#ifndef USERPROG_EXCEPTION_H
#define USERPROG_EXCEPTION_H

/* Page fault error code bits that describe the cause of the exception.  */
#define PF_P 0x1    /* 0: not-present page. 1: access rights violation. */
#define PF_W 0x2    /* 0: read, 1: write. */
#define PF_U 0x4    /* 0: kernel, 1: user process. */
#define PF_A 0x20		/* 0: not-access. 1: access //jinu4 vm*/
#define PF_D 0x40		/* 0: not-dirty. 1: dirty //jinu4 vm*/
void exception_init (void);
void exception_print_stats (void);

#endif /* userprog/exception.h */
