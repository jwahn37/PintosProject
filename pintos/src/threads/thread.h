#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include <threads/synch.h>
//#include "threads/synch.h"
//#include "device/timer.h"
/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* jinu3*/
#ifndef USERPROG
extern bool thread_prior_aging;
#endif
extern bool thread_mlfqs;	//jinu3
/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

typedef int fp_t;

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    int priority;                       /* Priority. */
    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */

		//struct list list_SPT; 	//jinu4 vm
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
/* by jinu (PROJECT2_1 User program basic)		
 wait lock to wait dying by jinu 2017/10/10*/
//		struct semaphore sema_wait;
//		struct semaphore sema_exit;	
//		struct semaphore sema_exec;

		struct lock lock_wait;
		struct lock lock_exit;
		struct lock lock_exec;
		struct lock lock_exit2;
/* exit status of child process by jinu 2017/10/15*/
		uint32_t exit_status;
		bool exit_soon;
	//by jinu (PROJECT2_2 User program file system call)
		//struct file *file_table[128];
		struct file **files_table;	//file descriptor table
		int fd;;							//file descriptor
		struct file *file_cur; //cureent file //jinu
		int64_t recent_unused;	//jinu3

		int nice;	//jinu3
		fp_t recent_cpu; //fp type

		
  };

/*fixed point jinu3*/
#define FP_FORMAT_32BIT 16384
#define FP_REAR true
#define INT_REAR false

struct fp_operation{
	int (*convert_to_fp)(int n);
	int (*convert_to_int) (int n);
	int (*convert_to_int_down) (int n);
	int (*add) (int a, int b, bool fp_rear);
	int (*sub)	(int a, int b, bool fp_rear);
	int (*mult) (int a, int b, bool fp_rear);
	int (*devide) (int a, int b, bool fp_rear);
};

struct fixed_point{
	struct fp_operation fp_op;
	int fp_format;
};

struct fixed_point fp;
fp_t load_avg;

//int a;

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);
int thread_top_priority(void);	//jinu3


int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

/* get thread from tid by jinu 2017-10-10*/
struct thread *thread_from_tid (tid_t tid);
tid_t tid_range(bool, tid_t); //2017-10-15 jinu

void thread_aging(void);
//jinu3 fixed point
int fp_convert_to_fp (int n);
int fp_convert_to_int (int n);
int fp_convert_to_int_down (int n);
int fp_add (int a, int b, bool fp_rear);
int fp_sub (int a, int b, bool fp_rear);
int fp_mult (int a, int b, bool fp_rear);
int fp_devide (int a, int b, bool fp_rear);
void fp_print(fp_t val, char *msg);
//jinu3 BSD schedule
void BSD_schedule(void);
void BSD_calc_priority(void);
void BSD_calc_load_average(void);
void BSD_calc_recent_cpt(void);
int BSD_calc_priority_this(fp_t recent_cpu, int nice);

#endif /* threads/thread.h */
