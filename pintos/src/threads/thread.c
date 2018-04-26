#include "threads/thread.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>
#include "threads/flags.h"
#include "threads/interrupt.h"
#include "threads/intr-stubs.h"
#include "threads/palloc.h"
#include "threads/switch.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

#include "vm/swap_out.h"

#include "devices/timer.h"

#ifdef USERPROG
#include "filesys/file.h"
#include "userprog/process.h"
#endif

/* Random value for struct thread's `magic' member.
   Used to detect stack overflow.  See the big comment at the top
   of thread.h for details. */
#define THREAD_MAGIC 0xcd6abf4b

/* List of processes in THREAD_READY state, that is, processes
   that are ready to run but not actually running. */

// jinu (PROJECT1 thread)
//static struct list ready_list;
static struct list ready_list[PRI_MAX+1];	//jinu3 //64 multi-level queue

/* List of all processes.  Processes are added to this list
   when they are first scheduled and removed when they exit. */
static struct list all_list;

/* Idle thread. */
static struct thread *idle_thread;

/* Initial thread, the thread running init.c:main(). */
static struct thread *initial_thread;

/* Lock used by allocate_tid(). */
static struct lock tid_lock;

/* Stack frame for kernel_thread(). */
struct kernel_thread_frame 
  {
    void *eip;                  /* Return address. */
    thread_func *function;      /* Function to call. */
    void *aux;                  /* Auxiliary data for function. */
  };

/* Statistics. */
static long long idle_ticks;    /* # of timer ticks spent idle. */
static long long kernel_ticks;  /* # of timer ticks in kernel threads. */
static long long user_ticks;    /* # of timer ticks in user programs. */

//(PROJECT2_1 User program basic)
#ifndef USERPROG
bool thread_prior_aging;	//jinu3
#endif


/* Scheduling. */
#define TIME_SLICE 4            /* # of timer ticks to give each thread. */
static unsigned thread_ticks;   /* # of timer ticks since last yield. */

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
bool thread_mlfqs;

static void kernel_thread (thread_func *, void *aux);

static void idle (void *aux UNUSED);
static struct thread *running_thread (void);
static struct thread *next_thread_to_run (void);
static void init_thread (struct thread *, const char *name, int priority);
static bool is_thread (struct thread *) UNUSED;
static void *alloc_frame (struct thread *, size_t size);
static void schedule (void);
void thread_schedule_tail (struct thread *prev);
static tid_t allocate_tid (void);

/* Initializes the threading system by transforming the code
   that's currently running into a thread.  This can't work in
   general and it is possible in this case only because loader.S
   was careful to put the bottom of the stack at a page boundary.

   Also initializes the run queue and the tid lock.

   After calling this function, be sure to initialize the page
   allocator before trying to create any threads with
   thread_create().

   It is not safe to call thread_current() until this function
   finishes. */
void
thread_init (void) 
{
	int i;
	int *tp;
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);

//by jinu (PROJECT1 Thread)
	for(i=PRI_MIN ; i<= PRI_MAX; i++)
	{
		list_init(&ready_list[i]);
	}		


	list_init (&all_list);
//jinu4 vm
//	list_init (&frame_table);

//by jinu (PROJECT1 Thread)
	//initate fixed point
	if(thread_mlfqs)
	{
		fp.fp_format = FP_FORMAT_32BIT;
		fp.fp_op.convert_to_fp = fp_convert_to_fp;
		fp.fp_op.convert_to_int = fp_convert_to_int;
		fp.fp_op.convert_to_int_down = fp_convert_to_int_down;
		fp.fp_op.add = fp_add;
		fp.fp_op.sub = fp_sub;
		fp.fp_op.mult =fp_mult;
		fp.fp_op.devide = fp_devide;
		load_avg = fp.fp_op.convert_to_fp(0);
	}
  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
}

/* Starts preemptive thread scheduling by enabling interrupts.
   Also creates the idle thread. */
void
thread_start (void) 
{
  /* Create the idle thread. */
  struct semaphore start_idle;
  sema_init (&start_idle, 0);
  thread_create ("idle", PRI_MIN, idle, &start_idle);

  /* Start preemptive thread scheduling. */
  intr_enable ();

  /* Wait for the idle thread to initialize idle_thread. */
  sema_down (&start_idle);
}

/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void) 
{
  struct thread *t = thread_current ();

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;
#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;

  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return ();

//by jinu (PROJECT1 Thread)
//BSD scheduler
	if(thread_mlfqs == true)
		BSD_schedule();

//by jinu (PROJECT1 Thread)
//thrread aging
#ifndef USERPROG
	if(thread_prior_aging == true)
		thread_aging();
#endif



}

/* Prints thread statistics. */
void
thread_print_stats (void) 
{
  printf ("Thread: %lld idle ticks, %lld kernel ticks, %lld user ticks\n",
          idle_ticks, kernel_ticks, user_ticks);
}

/* Creates a new kernel thread named NAME with the given initial
   PRIORITY, which executes FUNCTION passing AUX as the argument,
   and adds it to the ready queue.  Returns the thread identifier
   for the new thread, or TID_ERROR if creation fails.

   If thread_start() has been called, then the new thread may be
   scheduled before thread_create() returns.  It could even exit
   before thread_create() returns.  Contrariwise, the original
   thread may run for any amount of time before the new thread is
   scheduled.  Use a semaphore or some other form of
   synchronization if you need to ensure ordering.

   The code provided sets the new thread's `priority' member to
   PRIORITY, but no actual priority scheduling is implemented.
   Priority scheduling is the goal of Problem 1-3. */
tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux) 
{
  struct thread *t;
  struct kernel_thread_frame *kf;
  struct switch_entry_frame *ef;
  struct switch_threads_frame *sf;
  tid_t tid;
  enum intr_level old_level;
	char *fname;
	char **ptr;

  ASSERT (function != NULL);

  /* Allocate thread. */
  t = palloc_get_page (PAL_ZERO);
  if (t == NULL)
    return TID_ERROR;

  /* Initialize thread. */

/* by jinu (PROJECT2_1 User program basic) 
	this is for subtract file name from name because name also has arguments list, 
	not only file name.	
												jinu 2017-10-15*/
//by jinu (PROJECT1 Thread)
//I changed the parsing location in thread_create to process_execute
/*	fname = (char*) malloc (sizeof(char)* (strlen(name)+1));
	memcpy(fname, name, strlen(name)+1);
	fname = strtok_r(fname, " ", &ptr);	
	
	init_thread (t, fname, priority);

	free(fname);//jinu2 multioom
*/
	init_thread(t, name, priority);
  tid = t->tid = allocate_tid ();

  /* Prepare thread for first run by initializing its stack.
     Do this atomically so intermediate values for the 'stack' 
     member cannot be observed. */
  old_level = intr_disable ();

  /* Stack frame for kernel_thread(). */
  kf = alloc_frame (t, sizeof *kf);
  kf->eip = NULL;
  kf->function = function;
  kf->aux = aux;

  /* Stack frame for switch_entry(). */
  ef = alloc_frame (t, sizeof *ef);
  ef->eip = (void (*) (void)) kernel_thread;

  /* Stack frame for switch_threads(). */
  sf = alloc_frame (t, sizeof *sf);
  sf->eip = switch_entry;
  sf->ebp = 0;

  intr_set_level (old_level);

  /* Add to run queue. */
  thread_unblock (t);
	
  return tid;
}

/* Puts the current thread to sleep.  It will not be scheduled
   again until awoken by thread_unblock().

   This function must be called with interrupts turned off.  It
   is usually a better idea to use one of the synchronization
   primitives in synch.h. */
void
thread_block (void) 
{
  ASSERT (!intr_context ());
  ASSERT (intr_get_level () == INTR_OFF);

  thread_current ()->status = THREAD_BLOCKED;
  schedule ();
}

/* Transitions a blocked thread T to the ready-to-run state.
   This is an error if T is not blocked.  (Use thread_yield() to
   make the running thread ready.)

   This function does not preempt the running thread.  This can
   be important: if the caller had disabled interrupts itself,
   it may expect that it can atomically unblock a thread and
   update other data. */
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;
	int top_pri;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);

//by jinu (PROJECT1 Thread)
	//push renning thread to priority queue
	top_pri = thread_top_priority();
	list_push_back(&ready_list[t->priority], &t->elem);	//jinu3
  t->status = THREAD_READY;
  intr_set_level (old_level);

	//if the priority changed, thread_yield()
	if(top_pri < t->priority && intr_get_level()==INTR_ON)	//interrupt should be enabled for thread_yield()
	{
		thread_yield();
	}


}


/* Returns the name of the running thread. */
const char *
thread_name (void) 
{
  return thread_current ()->name;
}

/* Returns the running thread.
   This is running_thread() plus a couple of sanity checks.
   See the big comment at the top of thread.h for details. */
struct thread *
thread_current (void) 
{
  struct thread *t = running_thread ();
  
  /* Make sure T is really a thread.
     If either of these assertions fire, then your thread may
     have overflowed its stack.  Each thread has less than 4 kB
     of stack, so a few big automatic arrays or moderate
     recursion can cause stack overflow. */
  ASSERT (is_thread (t));
  ASSERT (t->status == THREAD_RUNNING);

  return t;
}

/* Returns the running thread's tid. */
tid_t
thread_tid (void) 
{
  return thread_current ()->tid;
}

/* Deschedules the current thread and destroys it.  Never
   returns to the caller. */
void
thread_exit (void) 
{
	struct file* file;
	int i;
	ASSERT (!intr_context ());

//	sema_up(&thread_current()->sema_exec);	//jinu2
	if(lock_held_by_current_thread(&thread_current()->lock_exec))
		lock_release(&thread_current()->lock_exec);

#ifdef USERPROG
  process_exit ();
#endif
  /* Remove thread from all threads list, set our status to dying,
     and schedule another process.  That process will destroy us
     when it calls thread_schedule_tail(). */
	//by jinu (PROJECT2_2 User program file system call)
	/* multi-oom
		before exit thread, close all files in thread structure and free 
	*/ 
#ifdef USERPROG
	file = thread_current()->file_cur;
	file_allow_write(file);
	file_close(file);
	for(i=0;i<=(thread_current()->fd)-2;i++)
	{
		if(thread_current()->files_table[i])
		{
			file_close(thread_current()->files_table[i]);
			thread_current()->files_table[i]=NULL;
		}
	}
	free(thread_current()->files_table);
#endif  
/* by jinu (PROJECT2_1 User program basic)
  this is for synchronizing with parent process which is in process_wait()
	it will send signal to wake up parent process because it is ready to die.*/
	
//	sema_up(&thread_current()->sema_wait); //wake up the parent process
	lock_release(&thread_current()->lock_wait);

	if(thread_current()->exit_soon != true)
	//	sema_down(&thread_current()->sema_exit); //wait until parent process takes exit_status from struct thread
	{
		lock_acquire(&thread_current()->lock_exit);
	}
	intr_disable ();
//	sema_up(&thread_current()->sema_wait);
//	lock_release(&thread_current()->lock_wait);

	lock_release(&thread_current()->lock_exit2);

	list_remove (&thread_current()->allelem);

	thread_current ()->status = THREAD_DYING;
	schedule ();
  NOT_REACHED ();
}

/* Yields the CPU.  The current thread is not put to sleep and
   may be scheduled again immediately at the scheduler's whim. */
void
thread_yield (void) 
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;

  ASSERT (!intr_context ());

  old_level = intr_disable ();
  if (cur != idle_thread) 
	{

		list_push_back(&ready_list[cur->priority], &cur->elem);
		cur->recent_unused = timer_ticks();	//jinu (PROJECT2_1 User program basic) for priority queue
	}

  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}


/* Invoke function 'func' on all threads, passing along 'aux'.
   This function must be called with interrupts off. */
void
thread_foreach (thread_action_func *func, void *aux)
{
  struct list_elem *e;

  ASSERT (intr_get_level () == INTR_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
       e = list_next (e))
    {
      struct thread *t = list_entry (e, struct thread, allelem);
      func (t, aux);
    }
}

/* Sets the current thread's priority to NEW_PRIORITY. */
void
thread_set_priority (int new_priority) 
{
  thread_current ()->priority = new_priority;
		
//by jinu (PROJECT1 Thread)
	//if new priority is lower, then do the context switch
	if(new_priority < thread_top_priority() && intr_get_level()==INTR_ON)
	{
		thread_yield();
	}
}

/* Returns the current thread's priority. */
int
thread_get_priority (void) 
{
  return thread_current ()->priority;
}

//by jinu (PROJECT1 Thread)
/* get top priority */
int
thread_top_priority(void)
{
	int i;

	for(i=PRI_MAX; i>=PRI_MIN; i--)
	{
		if(!list_empty(&ready_list[i]))
			return i;
	}

	return 0;
}

//by jinu (PROJECT1 Thread)
/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice ) 
{
  /* Not yet implemented. */
	ASSERT(-20<=nice && nice<=20);
	thread_current()->nice = nice;
	//recalculate priority

	thread_current()->priority = BSD_calc_priority_this(thread_current()->recent_cpu, thread_current()->nice);

	//prioirty changed then thread_yield
	if(thread_current()->priority < thread_top_priority() && intr_get_level()==INTR_ON)
	{
		thread_yield();
	}
}



/* Returns the current thread's nice value. */
int
thread_get_nice (void) 
{
  /* Not yet implemented. */
	return thread_current()->nice;	
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg (void) 
{
  /* Not yet implemented. */
	
  return fp.fp_op.convert_to_int(fp.fp_op.mult(load_avg, 100, INT_REAR));
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu (void) 
{
  /* Not yet implemented. */
  return fp.fp_op.convert_to_int(fp.fp_op.mult(thread_current()->recent_cpu, 100, INT_REAR));
}

/* Idle thread.  Executes when no other thread is ready to run.

   The idle thread is initially put on the ready list by
   thread_start().  It will be scheduled once initially, at which
   point it initializes idle_thread, "up"s the semaphore passed
   to it to enable thread_start() to continue, and immediately
   blocks.  After that, the idle thread never appears in the
   ready list.  It is returned by next_thread_to_run() as a
   special case when the ready list is empty. */
static void
idle (void *idle_started_ UNUSED) 
{
  struct semaphore *idle_started = idle_started_;
  idle_thread = thread_current ();
  sema_up (idle_started);

  for (;;) 
    {
      /* Let someone else run. */
      intr_disable ();
      thread_block ();

      /* Re-enable interrupts and wait for the next one.

         The `sti' instruction disables interrupts until the
         completion of the next instruction, so these two
         instructions are executed atomically.  This atomicity is
         important; otherwise, an interrupt could be handled
         between re-enabling interrupts and waiting for the next
         one to occur, wasting as much as one clock tick worth of
         time.

         See [IA32-v2a] "HLT", [IA32-v2b] "STI", and [IA32-v3a]
         7.11.1 "HLT Instruction". */
      asm volatile ("sti; hlt" : : : "memory");
    }
}

/* Function used as the basis for a kernel thread. */
static void
kernel_thread (thread_func *function, void *aux) 
{
  ASSERT (function != NULL);

  intr_enable ();       /* The scheduler runs with interrupts off. */
  function (aux);       /* Execute the thread function. */
  thread_exit ();       /* If function() returns, kill the thread. */
}

/* Returns the running thread. */
struct thread *
running_thread (void) 
{
  uint32_t *esp;

  /* Copy the CPU's stack pointer into `esp', and then round that
     down to the start of a page.  Because `struct thread' is
     always at the beginning of a page and the stack pointer is
     somewhere in the middle, this locates the curent thread. */
  asm ("mov %%esp, %0" : "=g" (esp));
  return pg_round_down (esp);
}

/* Returns true if T appears to point to a valid thread. */
static bool
is_thread (struct thread *t)
{
  return t != NULL && t->magic == THREAD_MAGIC;
}

/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t,const char *name, int priority)
{
	char* fname;
	char* ptr;
	int *tp;
  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
	t->status = THREAD_BLOCKED;

  strlcpy (t->name, name, sizeof t->name);
  
	t->stack = (uint8_t *) t + PGSIZE;
  t->priority = priority;
  t->magic = THREAD_MAGIC;
	//jinu4 vm
	//list_init(&t->list_SPT);

	//by jinu (PROJECT2_2 User program file system call)
	//add additional thread struct variable
	
	t->fd = 1;		//max file descriptor number
	t->files_table = NULL;	//it contains fd table
	t->exit_soon = false;		//communicate between process_execute and start_process
	
	//by jinu (PROJECT1 Thread)
	//initiate nice and recent_cpu value
	if(thread_mlfqs)
	{
		
		if(!strcmp("main", name))
		{
			t->nice = 0;	//init thread
			t->recent_cpu = fp.fp_op.convert_to_fp(0);
		}
		else
		{
			t->nice = thread_current()->nice;	//inherit parent's nice
			t->recent_cpu = thread_current()->recent_cpu; //inherit parent's recent_cpu
		}
	}
	t->recent_unused = timer_ticks(); 
	
	//sema_init(&t->sema_exit, 0); //jinu2
  //sema_init(&t->sema_wait, 0);
	//sema_init(&t->sema_exec, 0);
	lock_init(&t->lock_exit);
	lock_init(&t->lock_wait);
	lock_init(&t->lock_exec);
	lock_init(&t->lock_exit2);

	lock_acquire_other(&t->lock_exec, t);
	lock_acquire_other(&t->lock_wait, t);
	lock_acquire_other(&t->lock_exit2, t);
//lock_acquire(&t->lock_exit);
	//lock_acquire(&t->lock_wait);	//aqcuire owner
//	lock_acquire(&t->lock_exec);

	list_push_back (&all_list, &t->allelem);
}

/* Allocates a SIZE-byte frame at the top of thread T's stack and
   returns a pointer to the frame's base. */
static void *
alloc_frame (struct thread *t, size_t size) 
{
  /* Stack data is always allocated in word-size units. */
  ASSERT (is_thread (t));
  ASSERT (size % sizeof (uint32_t) == 0);

  t->stack -= size;
  return t->stack;
}

/* Chooses and returns the next thread to be scheduled.  Should
   return a thread from the run queue, unless the run queue is
   empty.  (If the running thread can continue running, then it
   will be in the run queue.)  If the run queue is empty, return
   idle_thread. */
static struct thread *
next_thread_to_run (void) 
{
/*
  if (list_empty (&ready_list))
    return idle_thread;
  else
    return list_entry (list_pop_front (&ready_list), struct thread, elem);
*/
//by jinu (PROJECT1 Thread)
	int i;
	for(i=PRI_MAX; i>=PRI_MIN; i--)
	{
		if (!list_empty (&ready_list[i]))
			return list_entry (list_pop_front (&ready_list[i]), struct thread, elem);
	}

	return idle_thread;

}

/* Completes a thread switch by activating the new thread's page
   tables, and, if the previous thread is dying, destroying it.

   At this function's invocation, we just switched from thread
   PREV, the new thread is already running, and interrupts are
   still disabled.  This function is normally invoked by
   thread_schedule() as its final action before returning, but
   the first time a thread is scheduled it is called by
   switch_entry() (see switch.S).

   It's not safe to call printf() until the thread switch is
   complete.  In practice that means that printf()s should be
   added at the end of the function.

   After this function and its caller returns, the thread switch
   is complete. */
void
thread_schedule_tail (struct thread *prev)
{
  struct thread *cur = running_thread ();
  
  ASSERT (intr_get_level () == INTR_OFF);

  /* Mark us as running. */
  cur->status = THREAD_RUNNING;

  /* Start new time slice. */
  thread_ticks = 0;

#ifdef USERPROG
  /* Activate the new address space. */
  process_activate ();
#endif

  /* If the thread we switched from is dying, destroy its struct
     thread.  This must happen late so that thread_exit() doesn't
     pull out the rug under itself.  (We don't free
     initial_thread because its memory was not obtained via
     palloc().) */
  if (prev != NULL && prev->status == THREAD_DYING && prev != initial_thread) 
    {
      ASSERT (prev != cur);
			palloc_free_page (prev);
    }
}

/* Schedules a new process.  At entry, interrupts must be off and
   the running process's state must have been changed from
   running to some other state.  This function finds another
   thread to run and switches to it.

   It's not safe to call printf() until thread_schedule_tail()
   has completed. */
static void
schedule (void) 
{
  struct thread *cur = running_thread ();
  struct thread *next = next_thread_to_run ();
  struct thread *prev = NULL;

  ASSERT (intr_get_level () == INTR_OFF);
  ASSERT (cur->status != THREAD_RUNNING);
  ASSERT (is_thread (next));

  if (cur != next)
    prev = switch_threads (cur, next);
  thread_schedule_tail (prev);
}

/* Returns a tid to use for a new thread. */
static tid_t
allocate_tid (void) 
{
  static tid_t next_tid = 1;
  tid_t tid;

  lock_acquire (&tid_lock);
  tid = next_tid++;
	/* by jinu (PROJECT2_1 User program basic)
	this function is for update max tid value
	it is necessary to check error exceptino in process_wait whther it gets invalid tid from child process */
	tid_range(true, tid);	//2017-10-15 jinu
  lock_release (&tid_lock);

  return tid;
}
/* by jinu (PROJECT2_1 User program basic)
												2017-10-15 
	if put_or_pull is true then allocate max tid
	else, then return the range of tid

	the max_tid is the maximum tid. So the tid range is from 0 to max_tid	
*/
tid_t
tid_range(bool put_or_pull, tid_t new_tid)
{
	static tid_t max_tid = 0;
	
	if(put_or_pull == true)
	{
		max_tid = new_tid;
		return NULL;
	}
	else
		return max_tid;
}

/* Offset of `stack' member within `struct thread'.
   Used by switch.S, which can't figure it out on its own. */
uint32_t thread_stack_ofs = offsetof (struct thread, stack);

/* by jinu (PROJECT2_1 User program basic)
	get struct thread from tid  2017-10-10*/
struct thread *thread_from_tid(tid_t tid)
{
	struct list_elem *cur_elem;
	struct thread *cur_thread;
	
	for(cur_elem = list_begin(&all_list);
			cur_elem != list_end(&all_list);
			cur_elem = list_next(cur_elem))
	{
		if((cur_thread=list_entry(cur_elem, struct thread, allelem))->tid == tid)
		{
			return cur_thread;
		}
	}
	return NULL;
}
//by jinu (PROJECT1 Thread)
void thread_aging()
{
	struct list_elem *e, *e_tp;
	struct thread *th_cur;
	bool yield;
	int i;
	
	yield = false;
	for( i=PRI_MAX-1; i>=PRI_MIN; i--)
	{
		for( e=list_begin(&ready_list[i]);	
				e != list_end(&ready_list[i]);
				e = list_next(e)	)
		{
			th_cur = list_entry(e, struct thread, elem);

			if(timer_elapsed(th_cur->recent_unused) % 100 == 0)
			{
/*
printf("thread-aging\n");
printf("rc :%lld\n",th_cur->recent_cpu);
printf("time : %lld\n",timer_ticks());
printf("timer_elasp : %lld\n", timer_elapsed(th_cur->recent_cpu));
*/
				th_cur->priority = th_cur->priority+1;
				e_tp = list_remove(e);
				list_push_back(&ready_list[th_cur->priority], e);
				e = list_prev(e_tp);

				if(th_cur->priority > thread_current()->priority)
					yield = true;
			}
		}
	}
	if(yield)
	{
		intr_yield_on_return();
	}
}

//by jinu (PROJECT1 Thread)
//BSD scheduler
void BSD_schedule()
{
  enum intr_level old_level;
	//char yield=false;
	int64_t tick = timer_ticks();

	if(strcmp(thread_current()->name, "idle"))
		thread_current()->recent_cpu = fp.fp_op.add(thread_current()->recent_cpu, 1, INT_REAR);

	if(tick % TIMER_FREQ == 0)	//per second
	{
		BSD_calc_load_average();
		BSD_calc_recent_cpu();
	}
	if(tick%4==0)	
	{	
		 BSD_calc_priority();	
	}
}
//by jinu (PROJECT1 Thread)
int BSD_calc_priority_this(fp_t recent_cpu, int nice)
{
	fp_t priority;
	fp_t rcpu_4;

	rcpu_4 = fp.fp_op.devide(recent_cpu, 4, INT_REAR);
	
	priority = fp.fp_op.sub(fp.fp_op.convert_to_fp(PRI_MAX-(nice*2)), rcpu_4, FP_REAR);
	priority = fp.fp_op.convert_to_int_down(priority);

	if(priority>PRI_MAX)	priority = PRI_MAX;
	if(priority<PRI_MIN)	priority = PRI_MIN;

	return priority;
}
//by jinu (PROJECT1 Thread)
void BSD_calc_priority()
{		
	struct list_elem *e;
	struct list_elem *e_ready;
	struct thread *th_cur;
	bool yield;
	int i;
	int pri;
	fp_t rcpu_4;

	yield = false;
	
	for(e = list_begin(&all_list);
			e != list_end(&all_list);
			e = list_next(e))
	{
		th_cur = list_entry(e, struct thread, allelem);

		if(strcmp(th_cur->name, "idle"))
		{
	//calculate priority, result should be rounded down to nearest integer	

			rcpu_4 = fp.fp_op.devide(th_cur->recent_cpu, 4, INT_REAR);
		
			pri = th_cur->priority;
			th_cur->priority = fp.fp_op.sub(fp.fp_op.convert_to_fp(PRI_MAX-(th_cur->nice*2)), rcpu_4, FP_REAR);
			th_cur->priority = fp.fp_op.convert_to_int_down(th_cur->priority);
			
			if(th_cur->priority>PRI_MAX)	th_cur->priority = PRI_MAX;
			if(th_cur->priority<PRI_MIN)	th_cur->priority = PRI_MIN;
		
			// thread in ready list			
			if(th_cur->status == THREAD_READY && pri != th_cur->priority)
			{
				e_ready = &th_cur->elem;
				list_remove(e_ready);
				list_push_back(&ready_list[th_cur->priority], e_ready);		
			
				if(th_cur->priority > thread_current()->priority)
					yield = true;
			}
			// thread is running	
		
			if(th_cur->status == THREAD_RUNNING)
			{
				if(th_cur->priority < thread_top_priority())
					yield = true;
			}

		}
	}
	//thread_yield()
	if(yield && intr_context())
	{
		intr_yield_on_return();
	}
}
//by jinu (PROJECT1 Thread)
void BSD_calc_load_average()
{
	int ready_threads;
	int i;
	fp_t d59_60;
	fp_t d1_60;	
	
	ready_threads = 0;
	for(i=PRI_MIN; i<=PRI_MAX; i++)
	{
		ready_threads += list_size(&ready_list[i]);
	}
	if(strcmp(thread_current()->name, "idle"))
	{
		ready_threads++;
	}

	d59_60 = fp.fp_op.devide(fp.fp_op.convert_to_fp(59), 60, INT_REAR);
	d1_60 = fp.fp_op.devide(fp.fp_op.convert_to_fp(1), 60, INT_REAR);
	load_avg = fp.fp_op.mult(load_avg, d59_60, FP_REAR);
	load_avg = fp.fp_op.add(load_avg,  fp.fp_op.mult(d1_60, ready_threads, INT_REAR), FP_REAR);
}
//by jinu (PROJECT1 Thread)
void BSD_calc_recent_cpu()
{
	fp_t coef;
  struct list_elem *e;
	struct thread *th_cur;

	coef = fp.fp_op.mult(load_avg, 2, INT_REAR);
	coef = fp.fp_op.devide(coef, fp.fp_op.add(coef, 1, INT_REAR), FP_REAR);

	for(e=list_begin(&all_list);
			e!=list_end(&all_list);
			e = list_next(e)	)
	{
		th_cur = list_entry(e, struct thread, allelem);	

	if(strcmp(th_cur->name, "idle")){
		th_cur->recent_cpu = fp.fp_op.mult(coef, th_cur->recent_cpu, FP_REAR);
		th_cur->recent_cpu = fp.fp_op.add(th_cur->recent_cpu,  th_cur->nice, INT_REAR);
	}	
	}
}

//by jinu (PROJECT1 Thread)
//fp_operation

int fp_convert_to_fp (int n)
{
	return n*fp.fp_format; 
}
int fp_convert_to_int (int x)
{
	//x>=0 or x<0
	if(x&1024*1024*1024*2==0)	return (x + fp.fp_format/2) / fp.fp_format;
	else										return (x - fp.fp_format/2) / fp.fp_format;
}
int fp_convert_to_int_down (int x)
{
	return x / fp.fp_format;
}
int fp_add (int a, int b, bool fp_rear)
{
	if(fp_rear)	 return a+b;
	else				 return a+b*fp.fp_format;
}
int fp_sub (int a, int b, bool fp_rear)
{
	if(fp_rear)  return a-b;
	else				 return a-b*fp.fp_format;
}

int fp_mult (int a, int b, bool fp_rear)
{
	if(fp_rear)	return ((int64_t) a) * b / fp.fp_format;
	else				return a*b;
}
int fp_devide (int a, int b, bool fp_rear)
{
	if(fp_rear)	return ((int64_t) a) * fp.fp_format / b;
	else				return a/b;
}
void fp_print (fp_t val, char *msg)
{
	printf("%s : %d\n",val, fp.fp_op.convert_to_int(fp.fp_op.mult(msg, 100, INT_REAR)));
}

