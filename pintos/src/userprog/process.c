#include "userprog/process.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/gdt.h"
#include "userprog/pagedir.h"
#include "userprog/tss.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/flags.h"
#include "threads/init.h"
#include "threads/interrupt.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/synch.h"

#include "vm/swap_out.h"

//struct block *fs_device; //jinu4

static thread_func start_process NO_RETURN;
static bool load (const char *cmdline, void (**eip) (void), void **esp);

/* Starts a new thread running a user program loaded from
   FILENAME.  The new thread may be scheduled (and may even exit)
   before process_execute() returns.  Returns the new process's
   thread id, or TID_ERROR if the thread cannot be created. */

/* the file_name is not only file name but also has arguments. (jinu)*/
tid_t
process_execute (const char *file_name) 
{
  char *fn_copy;
	char *fname;
	char **ptr;
  tid_t tid;
  /* Make a copy of FILE_NAME.
		
     Otherwise there's a race between the caller and load(). */
  fn_copy = palloc_get_page (0);
 if (fn_copy == NULL)
    return TID_ERROR;
 
	strlcpy (fn_copy, file_name, PGSIZE);
  /* Create a new thread to execute FILE_NAME. */

//by jinu (PROJECT1 Thread) ,(PROJECT2_1 User program basic)
//I changed the parsing location from thread_create to process_execute 
	fname = (char*) malloc (sizeof(char)*(strlen(file_name)+1));
	memcpy(fname, file_name, strlen(file_name)+1);
	fname = strtok_r(fname, " ",&ptr);
  tid = thread_create (fname, PRI_DEFAULT, start_process, fn_copy);
	free(fname); //jinu3

	/* by jinu (PROJECT2_1 User program basic) 
	this is for checking exception in process_wait 
	exit_soon means that this process exit recently. here, initiate the data*/	
	
	thread_from_tid(tid)->exit_soon =false;

	/*by jinu (PROJECT2_2 User program file system call)	
	process execute should wait until child process is loaded successfully
	if child loads fails, then it should return -1*/

	//sema_down(&thread_from_tid(tid)->sema_exec);//lock between start_process(load) and process_execute
	//lock_acquire(&thread_from_tid(tid)->lock_exec);			//jinu3 init nilock(acquire owner)
	lock_acquire(&thread_from_tid(tid)->lock_exit);	//jinu3 acquire owner here
	lock_acquire(&thread_from_tid(tid)->lock_exec);		
//	lock_acquire_other(&thread_from_tid(tid)->lock_exec, thread_from_tid(tid));		

	if (tid == TID_ERROR)
    palloc_free_page (fn_copy); 
	
	//child process fails to start
	if(thread_from_tid(tid) == NULL || thread_from_tid(tid)->exit_soon==true)
	{
			tid=TID_ERROR;
  }
	
	return tid;
}

/* A thread function that loads a user process and starts it
   running. */

static void
start_process (void *file_name_)
{
  char *file_name = file_name_;
  struct intr_frame if_;
  bool success;
	

  /* Initialize interrupt frame and load executable. */
  memset (&if_, 0, sizeof if_);
  if_.gs = if_.fs = if_.es = if_.ds = if_.ss = SEL_UDSEG;
  if_.cs = SEL_UCSEG;
  if_.eflags = FLAG_IF | FLAG_MBS;
  success = load (file_name, &if_.eip, &if_.esp);
  /* If load failed, quit. */
  palloc_free_page (file_name);
 
	 if (!success) 
	{
		/*by jinu (PROJECT2_2 User program file system call)
			child process fails to start.
			alarm to parent's process_execute and make it return -1
		*/
			thread_current()->exit_status = -1;
			thread_current()->exit_soon=true;
			//sema_up(&thread_current()->sema_exec);
			lock_release(&thread_current()->lock_exec);
			thread_exit ();

  }
	else
		//sema_up(&thread_current()->sema_exec);
		lock_release(&thread_current()->lock_exec);
	/* Start the user process by simulating a return from an
     interrupt, implemented by intr_exit (in
     threads/intr-stubs.S).  Because intr_exit takes all of its
     arguments on the stack in the form of a `struct intr_frame',
     we just point the stack pointer (%esp) to our stack frame
     and jump to it. */
  asm volatile ("movl %0, %%esp; jmp intr_exit" : : "g" (&if_) : "memory");
  NOT_REACHED ();

}

/* Waits for thread TID to die and returns its exit status.  If
   it was terminated by the kernel (i.e. killed due to an
   exception), returns -1.  If TID is invalid or if it was not a
   child of the calling process, or if process_wait() has already
   been successfully called for the given TID, returns -1
   immediately, without waiting.

   This function will be implemented in problem 2-2.  For now, it
   does nothing. */

/* by jinu (PROJECT2_1 User program basic)
*/
int
process_wait (tid_t child_tid UNUSED) 

{
	int exit_status=-1;

	/* by jinu (PROJECT2_1 User program basic)
	 do not wait if
	1. child_tid is TID_ERROR
	2. child_tid is out of range
	3. child_tid already died recently. and didn't recreate yet
	*/	//2017-10-15 by jinu	
	if(child_tid == TID_ERROR
		||(tid_range(false,NULL) < child_tid || 0 > child_tid)
		||thread_from_tid(child_tid) == NULL
		)
		return -1;
		

//if child terminated before parent wait
	/* by jinu (PROJECT2_1 User program basic)
	 synchonize to wait for child procee dies 
	exit_status will be changed from exit() system call and return 
	semaphore is used to wait and there are two semaphore named sema_wait, sema_exit
	sema_wait is for waiting this process until child process send dying signal
*/
//	sema_down(&thread_from_tid(child_tid)->sema_wait);	
//	lock_acquire(&thread_from_tid(child_tid)->lock_exit);	//jinu3 acquire owner here
//	lock_acquire_other(&thread_from_tid(child_tid)->lock_wait, thread_from_tid(child_tid)); // 
	lock_acquire(&thread_from_tid(child_tid)->lock_wait); // 
/* by jinu (PROJECT2_1 User program basic) wait unitl child process dies */
	exit_status = thread_from_tid(child_tid)->exit_status;
/* by jinu (PROJECT2_1 User program basic) wake up the child process then child process will die completely */
//	sema_up(&thread_from_tid(child_tid)->sema_exit);
//	sema_down(&thread_from_tid(child_tid)->sema_wait);
	lock_release(&thread_from_tid(child_tid)->lock_exit);
//	lock_acquire_other(&thread_from_tid(child_tid)->lock_wait, thread_from_tid(child_tid));
	lock_acquire(&thread_from_tid(child_tid)->lock_exit2);

	return exit_status;
}

/* Free the current process's resources. */
void
process_exit (void)
{
  struct thread *cur = thread_current ();
  uint32_t *pd;

  /* Destroy the current process's page directory and switch back
     to the kernel-only page directory. */
  pd = cur->pagedir;
  if (pd != NULL) 
    {
      /* Correct ordering here is crucial.  We must set
         cur->pagedir to NULL before switching page directories,
         so that a timer interrupt can't switch back to the
         process page directory.  We must activate the base page
         directory before destroying the process's page
         directory, or our active page directory will be one
         that's been freed (and cleared). */
      cur->pagedir = NULL;
      pagedir_activate (NULL);
      pagedir_destroy (pd);
    }
}

/* Sets up the CPU for running user code in the current
   thread.
   This function is called on every context switch. */
void
process_activate (void)
{
  struct thread *t = thread_current ();

  /* Activate thread's page tables. */
  pagedir_activate (t->pagedir);

  /* Set thread's kernel stack for use in processing
     interrupts. */
  tss_update ();
}

/* We load ELF binaries.  The following definitions are taken
   from the ELF specification, [ELF1], more-or-less verbatim.  */

/* ELF types.  See [ELF1] 1-2. */
typedef uint32_t Elf32_Word, Elf32_Addr, Elf32_Off;
typedef uint16_t Elf32_Half;

/* For use with ELF types in printf(). */
#define PE32Wx PRIx32   /* Print Elf32_Word in hexadecimal. */
#define PE32Ax PRIx32   /* Print Elf32_Addr in hexadecimal. */
#define PE32Ox PRIx32   /* Print Elf32_Off in hexadecimal. */
#define PE32Hx PRIx16   /* Print Elf32_Half in hexadecimal. */

/* Executable header.  See [ELF1] 1-4 to 1-8.
   This appears at the very beginning of an ELF binary. */
struct Elf32_Ehdr
  {
    unsigned char e_ident[16];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
  };

/* Program header.  See [ELF1] 2-2 to 2-4.
   There are e_phnum of these, starting at file offset e_phoff
   (see [ELF1] 1-6). */
struct Elf32_Phdr
  {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
  };

/* Values for p_type.  See [ELF1] 2-3. */
#define PT_NULL    0            /* Ignore. */
#define PT_LOAD    1            /* Loadable segment. */
#define PT_DYNAMIC 2            /* Dynamic linking info. */
#define PT_INTERP  3            /* Name of dynamic loader. */
#define PT_NOTE    4            /* Auxiliary info. */
#define PT_SHLIB   5            /* Reserved. */
#define PT_PHDR    6            /* Program header table. */
#define PT_STACK   0x6474e551   /* Stack segment. */

/* Flags for p_flags.  See [ELF3] 2-3 and 2-4. */
#define PF_X 1          /* Executable. */
#define PF_W 2          /* Writable. */
#define PF_R 4          /* Readable. */

static bool setup_stack (void **esp);
static bool validate_segment (const struct Elf32_Phdr *, struct file *);
static bool load_segment (struct file *file, off_t ofs, uint8_t *upage,
                          uint32_t read_bytes, uint32_t zero_bytes,
                          bool writable);

/* Loads an ELF executable from FILE_NAME into the current thread.
   Stores the executable's entry point into *EIP
   and its initial stack pointer into *ESP.
   Returns true if successful, false otherwise. */
bool
load (const char *argv_list, void (**eip) (void), void **esp) 
{
  struct thread *t = thread_current ();
  struct Elf32_Ehdr ehdr;
  struct file *file = NULL;
  off_t file_ofset;
  bool success = false;
  int i;
	char *file_name;	
	char *tp_ptr;
  /* Allocate and activate page directory. */
  //printf("load : %d\n",t->tid);
	t->pagedir = pagedir_create (); //kernel mapping만 된 상태의 pd를 리턴
  if (t->pagedir == NULL) 
    goto done;
  process_activate (); //CPU가 현재 실행할 프로세스의 pd를 갖고있음

/* by jinu (PROJECT2_1 User program basic)
	this function is for parsing argv_list 
	the function will extract file name from argv_list to use	
	*/
	//file_name = (char*) malloc (sizeof(char) * (strlen(*argv_list)+1));
	//memcpy(tp_ptr, *argv_list, strlen(*argv_list)+1);
	//*file_name = strtok_r(tp_ptr, " ", &next_ptr);	
	tp_ptr = (char*) malloc (sizeof(char) * (strlen(argv_list)+1));
	parse_filename(&argv_list,&file_name, tp_ptr);
  //free(tp_ptr);
  
	/* Open executable file. */
  file = filesys_open (file_name);
  free(tp_ptr);
	//free(file_name);
	if (file == NULL) 
    {
      printf ("load: %s: open failed\n", file_name);
      goto done; 
    }

  /* Read and verify executable header. */
  if (file_read (file, &ehdr, sizeof ehdr) != sizeof ehdr
      || memcmp (ehdr.e_ident, "\177ELF\1\1\1", 7)
      || ehdr.e_type != 2
      || ehdr.e_machine != 3
      || ehdr.e_version != 1
      || ehdr.e_phentsize != sizeof (struct Elf32_Phdr)
      || ehdr.e_phnum > 1024) 
    {
      printf ("load: %s: error loading executable\n", file_name);
      goto done; 
    }

	//jinu2
//	file_deny_write(file);
	//file_deny_write(file);
//	printf("load() file_name : %s\n",file_name);
 
	 /* Read program headers. */
  file_ofset = ehdr.e_phoff;
  for (i = 0; i < ehdr.e_phnum; i++) 
    {
      struct Elf32_Phdr phdr;	//program header

      if (file_ofset < 0 || file_ofset > file_length (file))
        goto done;
      file_seek (file, file_ofset);

      if (file_read (file, &phdr, sizeof phdr) != sizeof phdr)
        goto done;
      file_ofset += sizeof phdr;
      switch (phdr.p_type) 
        {
        case PT_NULL:
        case PT_NOTE:
        case PT_PHDR:
        case PT_STACK:
        default:
          /* Ignore this segment. */
          break;
        case PT_DYNAMIC:
        case PT_INTERP:
        case PT_SHLIB:
          goto done;
        case PT_LOAD:
          if (validate_segment (&phdr, file)) 
            {
              bool writable = (phdr.p_flags & PF_W) != 0;
              uint32_t file_page = phdr.p_offset & ~PGMASK;
              uint32_t mem_page = phdr.p_vaddr & ~PGMASK;
              uint32_t page_offset = phdr.p_vaddr & PGMASK;
              uint32_t read_bytes, zero_bytes;
		  				if (phdr.p_filesz > 0)
                {
                  /* Normal segment.
                     Read initial part from disk and zero the rest. */
                  read_bytes = page_offset + phdr.p_filesz;
                  zero_bytes = (ROUND_UP (page_offset + phdr.p_memsz, PGSIZE)
                                - read_bytes);
                }
              else 
                {
                  /* Entirely zero.
                     Don't read anything from disk. */
                  read_bytes = 0;
                  zero_bytes = ROUND_UP (page_offset + phdr.p_memsz, PGSIZE);
                }
              if (!load_segment (file, file_page, (void *) mem_page,
                                 read_bytes, zero_bytes, writable))
                goto done;
            }
          else
            goto done;
          break;
        }
    }
	/* Set up stack. */
  if (!setup_stack (esp))
    goto done;

  /* Start address. */
  *eip = (void (*) (void)) ehdr.e_entry;

/* by jinu (PROJECT2_1 User program basic)
	this function is to allocate every arguments in argv_list into the stack 
*/	
	if(construct_ESP(esp, argv_list)==false)	goto done;
  
	success = true;

 done:
  /* We arrive here whether the load is successful or not. */
	//jinu2	
#ifdef USERPROG
	file_deny_write(file);
	t->file_cur = file;
//jinu trun off file_close  
//file_close (file);
#endif
	//printf("fin)load esp: %x, eip : %x\n", *esp, *eip);
  return success;
}

/* load() helpers. */
//jinu4
//static bool install_page (void *upage, void *kpage, bool writable);

/* Checks whether PHDR describes a valid, loadable segment in
   FILE and returns true if so, false otherwise. */
static bool
validate_segment (const struct Elf32_Phdr *phdr, struct file *file) 
{
  /* p_offset and p_vaddr must have the same page offset. */
  if ((phdr->p_offset & PGMASK) != (phdr->p_vaddr & PGMASK)) 
    return false; 

  /* p_offset must point within FILE. */
  if (phdr->p_offset > (Elf32_Off) file_length (file)) 
    return false;

  /* p_memsz must be at least as big as p_filesz. */
  if (phdr->p_memsz < phdr->p_filesz) 
    return false; 

  /* The segment must not be empty. */
  if (phdr->p_memsz == 0)
    return false;
  
  /* The virtual memory region must both start and end within the
     user address space range. */
  if (!is_user_vaddr ((void *) phdr->p_vaddr))
    return false;
  if (!is_user_vaddr ((void *) (phdr->p_vaddr + phdr->p_memsz)))
    return false;

  /* The region cannot "wrap around" across the kernel virtual
     address space. */
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr)
    return false;

  /* Disallow mapping page 0.
     Not only is it a bad idea to map page 0, but if we allowed
     it then user code that passed a null pointer to system calls
     could quite likely panic the kernel by way of null pointer
     assertions in memcpy(), etc. */
  if (phdr->p_vaddr < PGSIZE)
    return false;

  /* It's okay. */
  return true;
}

/* Loads a segment starting at offset OFS in FILE at address
   UPAGE.  In total, READ_BYTES + ZERO_BYTES bytes of virtual
   memory are initialized, as follows:

        - READ_BYTES bytes at UPAGE must be read from FILE
          starting at offset OFS.

        - ZERO_BYTES bytes at UPAGE + READ_BYTES must be zeroed.

   The pages initialized by this function must be writable by the
   user process if WRITABLE is true, read-only otherwise.

   Return true if successful, false if a memory allocation error
   or disk read error occurs. */
static bool
load_segment (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
  ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
  ASSERT (pg_ofs (upage) == 0);
  ASSERT (ofs % PGSIZE == 0);

  file_seek (file, ofs);
  while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      /* Get a page of memory. */
      uint8_t *knpage = palloc_get_page (PAL_USER);
      if (knpage == NULL)
        return false;

      /* Load this page. */
      if (file_read (file, knpage, page_read_bytes) != (int) page_read_bytes)
        {
          palloc_free_page (knpage);
          return false; 
        }
		
		//jinu4 vm
		/* byte_to_sector : convert to sector address
			 fs_device  : file sysem block device 

			*/
//			SPT_insert(upage, fs_device, byte_to_sector(file_get_inode (file), file_get_pos(file)));
//			SPT_insert(upage, fs_device, get_block_addr(file));
			//byte_to_sector(file->inode, file->pos);
			memset (knpage + page_read_bytes, 0, page_zero_bytes);

      /* Add the page to the process's address space. */
      if (!install_page (upage, knpage, writable)) 
        {
          palloc_free_page (knpage);
          return false; 
        }

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      upage += PGSIZE;
    }
  return true;
}

/* Create a minimal stack by mapping a zeroed page at the top of
   user virtual memory. */
static bool
setup_stack (void **esp) 
{
  uint8_t *kpage;
  bool success = false;

  kpage = palloc_get_page (PAL_USER | PAL_ZERO);
  if (kpage != NULL) 
    {
      success = install_page (((uint8_t *) PHYS_BASE) - PGSIZE, kpage, true);
      if (success)
      {
			  *esp = PHYS_BASE;
      //jinu4 vm
	//			SPT_insert(((uint8_t *)PHYS_BASE) - PGSIZE, fs_device, PGSIZE);
				
			}
			else
        palloc_free_page (kpage);
    }
  return success;
}

/* Adds a mapping from user virtual address UPAGE to kernel
   virtual address KPAGE to the page table.
   If WRITABLE is true, the user process may modify the page;
   otherwise, it is read-only.
   UPAGE must not already be mapped.
   KPAGE should probably be a page obtained from the user pool
   with palloc_get_page().
   Returns true on success, false if UPAGE is already mapped or
   if memory allocation fails. */

//static bool	//jinu4 exception.c should know this function
bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *th = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */

	//jinu4 vm
	//현재 엉성하게 짜놨는데, 나중에 정밀하게 전체 수정해주기.
	//allocate frame entry to frame table
/*	if(pagedir_get_page (th->pagedir, upage) != NULL)
	{
;		//kpage가 실제 물리적 메모리 주소라고 판단됨.
	//jinu4 vm
		//frame_insert(kpage);	
	}	
*/
  return (pagedir_get_page (th->pagedir, upage) == NULL
          && pagedir_set_page (th->pagedir, upage, kpage, writable));
}

/* by jinu (PROJECT2_1 User program basic)
	this function will parsing and subtract the file name from argv_list
	argv_list is like this for example,
		"ls -l -a -b"
	then file name would be "ls"
*/
void parse_filename (char **argv_list, char **file_name, char *tp_ptr)
{
	char* next_ptr;
	char* real_file_name;

	memcpy(tp_ptr, *argv_list, strlen(*argv_list)+1);
	*file_name = strtok_r(tp_ptr, " ", &next_ptr);	
}

/* by jinu (PROJECT2_1 User program basic)
 insert argument value into the stack
		you should put the value in stack like this way
							STACK
	  ------	PHY_BASE ------
		|				argv[4]				|
		|				argv[3]				|
		|				argv[2]				|
		|				argv[1]				|
		|				argv[0]				|
		|				  0    				|
		|			 &argv[5](0)	  |
		|			 &argv[4]				|
		|			 &argv[3]				|
		|			 &argv[2]				|
		|			 &argv[1]				|
		|			 &argv[0]				|
		|			 	argc  				|
		|			 return addr 		|
		-----------------------	

	created by jinu
	data : 2017/10/10
*/

bool construct_ESP (void **esp, char *argv_list)
{
	char **_argv_list = NULL;
	char *next_ptr, *tp_ptr;
	uint32_t *argv_addr;
	uint32_t num_argv = 0;
	int i;
	void* buf_;
	char* tp;
	bool success;

	/*parse the argv_list and put each arguments in _argv_list*/
	tp_ptr = strtok_r(argv_list, " ", &next_ptr);
	while(tp_ptr)
	{
		_argv_list = (char**) realloc (_argv_list,  sizeof(char*) * ++num_argv);
		if(_argv_list == NULL)	goto done;
		_argv_list[num_argv-1] = tp_ptr;
		tp_ptr = strtok_r(NULL, " ", &next_ptr);
	}

/*insert argument value into the stack*/
	argv_addr = (uint32_t*) malloc (sizeof(uint32_t) * (1+num_argv));
	if(argv_addr == NULL)	goto done;
	for(i=num_argv-1; i>=0; i--)
	{	
		*esp = *esp - strlen(_argv_list[i]) -1;	//+1 is for '\0'
		memcpy(*esp, _argv_list[i], sizeof(char)*(1+strlen(_argv_list[i])));
		argv_addr[i] = *esp;
	}
	
	*esp = *esp -sizeof(char);
	memset(*esp, 0, sizeof(char));
	
	*esp = *esp -sizeof(char*);
	memset(*esp, 0, sizeof(char*));

	for(i=num_argv-1; i>=0; i--)
	{
		*esp = *esp -sizeof(char*);
		memcpy(*esp, &argv_addr[i], sizeof(char*));
	}
	
	argv_addr[num_argv] = *esp;

	*esp = *esp -sizeof(char*);
	memcpy(*esp, &argv_addr[num_argv], sizeof(char*));

	*esp = *esp - sizeof(uint32_t);
	memcpy(*esp, &num_argv, sizeof(uint32_t));
	
	*esp = *esp - sizeof(uint32_t);
	memset(*esp, 0x00, sizeof(uint32_t));

	success = true;
	done:
	free(argv_addr);
	free(_argv_list);
	return success;
}


