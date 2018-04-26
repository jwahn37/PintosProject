#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "devices/input.h"
#include "lib/kernel/console.h"
#include "filesys/directory.h"
#include "filesys/off_t.h"

static void syscall_handler (struct intr_frame *);
void check_address(uint32_t *address);
void get_argv (uint32_t *syscall_num, struct intr_frame *intr, uint32_t **argv);
void terminate_process(void);
uint32_t fdtable_size(int fd);
uint32_t fdtable_idx(int fd);
void sys_halt (uint32_t *syscall_num, struct intr_frame *intr); 
void sys_exit (uint32_t *syscall_num, struct intr_frame *intr);
void sys_exec (uint32_t *syscall_num, struct intr_frame *intr);
void sys_wait (uint32_t *syscall_num, struct intr_frame *intr);



void sys_read (uint32_t *syscall_num, struct intr_frame *intr);
void sys_write (uint32_t *syscall_num, struct intr_frame *intr);


void sys_create (uint32_t *syscall_num, struct intr_frame *intr);
void sys_remove (uint32_t *syscall_num, struct intr_frame *intr);
void sys_open (uint32_t *syscall_num, struct intr_frame *intr);
void sys_filesize (uint32_t *syscall_num, struct intr_frame *intr);

void sys_seek (uint32_t *syscall_num, struct intr_frame *intr);
void sys_tell (uint32_t *syscall_num, struct intr_frame *intr);
void sys_close (uint32_t *syscall_num, struct intr_frame *intr);
void sys_mmap (uint32_t *syscall_num, struct intr_frame *intr);
void sys_mummap (uint32_t *syscall_num, struct intr_frame *intr);
void sys_chdir (uint32_t *syscall_num, struct intr_frame *intr);
void sys_mkdir (uint32_t *syscall_num, struct intr_frame *intr);
void sys_readdir (uint32_t *syscall_num, struct intr_frame *intr);
void sys_isdir (uint32_t *syscall_num, struct intr_frame *intr);
void sys_inumber (uint32_t *syscall_num, struct intr_frame *intr);
void sys_sum_of_four_integers (uint32_t *syscall_num, struct intr_frame *intr);
void sys_fibonacci (uint32_t *syscall_um, struct intr_frame *intr);
//array of system call handler
void (*sys_handler_[]) (uint32_t *, struct intr_frame *)={
  sys_halt,
  sys_exit,
  sys_exec,
  sys_wait,
  sys_create,
  sys_remove,
  sys_open,
  sys_filesize,

  sys_read,
  sys_write,
  sys_seek,
  sys_tell,
  sys_close,
  sys_mmap,
  sys_mummap,
  sys_chdir,
  sys_mkdir,
  sys_readdir,
  sys_isdir,
  sys_inumber,
	
	sys_fibonacci,
	sys_sum_of_four_integers
};

//the number of system call's parameter
uint32_t sys_argv_num[] = {
	0, 1, 1, 1, 2, 1, 1, 1, 3, 3,
	2, 1, 1, 2, 1, 1, 1, 2, 1, 1,
	1, 4 };

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

}

/* by jinu (PROJECT2_1 User program basic)*/
/* handle the system call according to system call number*/
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	uint32_t *syscall_num;

	syscall_num = (f->esp);
	/* check user space address here*/
	check_address(syscall_num);		
	sys_handler_[*syscall_num](syscall_num, f);
}

void sys_halt (uint32_t *syscall_num, struct intr_frame *intr)
{
	shutdown_power_off();
}
void sys_exit (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	uint32_t status;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	status =(uint32_t*) argv[0];
	thread_current()->exit_status = status;
	printf("%s: exit(%d)\n", thread_current()->name, status);
	thread_exit();
	
	free(argv);
}

void sys_exec (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	tid_t proc_tid;
	char *file_args;
	struct inode * inode = NULL;;
	char * next_ptr, *tp_ptr;
	char *file_name;

	/* get the arguments from stack */
	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	check_address((uint32_t *)argv[0]);	

	file_args = (char*) argv[0];
  
  file_name  = (char*) malloc (sizeof(char) * (strlen(file_args)+1));
  memcpy(file_name, file_args, strlen(file_args)+1);	
	file_name = strtok_r(file_name, " ", &next_ptr);

	/*check whther the file exists or not */	
	if(!dir_lookup_file (file_name))
		intr->eax = TID_ERROR;
	else
	{
		proc_tid = process_execute(file_args);
		intr->eax = proc_tid;
	}
	free(file_name);
	free(argv);
}

void sys_wait (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	tid_t proc_tid;
	int exit_status;
	int i;
	
	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	proc_tid = (tid_t)argv[0];

	exit_status = process_wait(proc_tid);
	intr->eax = exit_status;
	free(argv);
}

void sys_create (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	char *file_name;
	unsigned initial_size;
	bool success;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	file_name = (char*) argv[0];
	initial_size = (unsigned) argv[1];

	check_address((uint32_t *)argv[0]);	

	if(file_name == NULL)
		terminate_process();

	success = filesys_create(file_name, initial_size);

	intr->eax = success;
	free(argv);
}

void sys_remove (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	char* file_name;
	bool success = false;
	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	check_address(argv[0]);
	file_name = (char*) argv[0];

	if(file_name){
		success=filesys_remove(file_name);
		intr->eax = success;
	}
	else
		intr->eax = success;
	free(argv);
}
void sys_open (uint32_t *syscall_num, struct intr_frame *intr)
{
	char *file_name;
	uint32_t *argv;
	struct file **files_table, *file;
	int fd;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	
	check_address(argv[0]);	
	file_name = (char*) argv[0];
	if(file_name == NULL)
		terminate_process();

	file = filesys_open(file_name);
	
	if(file){
		//increase inode count to protect delete

		fd = ++(thread_current()->fd);
		
		if((fd-2) % 20 == 0)//나중에 건철이가 알려준 간지나는 리알록으로	
			thread_current()->files_table = (struct file**) realloc (thread_current()->files_table, sizeof(struct file*)*(fdtable_size(fd)+20));

		thread_current()->files_table[fdtable_idx(fd)] = file;
		intr->eax = fd;
	}
	else	//file open error
		intr->eax = -1;

	free(argv);
}
void sys_filesize (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	int fd;
	off_t bytes;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	
	fd = (int) argv[0];
	
	if(fd>1 && fd<=thread_current()->fd)
	{
		bytes=file_length(thread_current()->files_table[fd-2]);
	}
	else
		bytes=-1;
	intr->eax =bytes;
	
	free(argv);
}
void sys_read (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	int fd;
	char *buf;
	unsigned size;
	unsigned i;	
	char *buf_ptr;
	off_t bytes;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);

	fd = (int)argv[0];
	buf = (char*) argv[1];
	check_address(buf);
	size = (unsigned) argv[2];
	check_address(buf+sizeof(char)*size);	
	buf_ptr = buf;

	if(fd==0)
	{
		for(i=0; i<size; i++)
		{
			*buf_ptr = input_getc();
			buf_ptr++;		
			check_address(buf_ptr);	
		}
		bytes = i;
	}
	else if(fd <= thread_current()->fd && fd>1)
	{
		bytes = file_read(thread_current()->files_table[fd-2], buf, size);
		
	}
	else
	{
		bytes=-1;
	}

	intr->eax = bytes; //temp you have to change it!

	free(argv);
}
void sys_write (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	int fd;
	char *buf;
	unsigned size;
	off_t bytes;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	
	fd = (int)argv[0];
	check_address(buf = (char*) argv[1]);
	size = (unsigned) argv[2];
	check_address(buf+sizeof(char)*size);	
	if(fd==1)
	{
		putbuf(buf, size); 

		bytes = size;
	}

	else if(fd <= thread_current()->fd && fd>1)
	{
		bytes = file_write(thread_current()->files_table[fd-2], buf, size);
	}
	else
	{
		bytes = -1;
	}
	intr->eax = bytes;	//temp you have to changed it!
	free(argv);
}
void sys_seek (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	int fd;
	unsigned position;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	
	fd = (int) argv[0];
	position = (unsigned) argv[1];

	if(fd <= thread_current()->fd && fd>1)
	{
		file_seek(thread_current()->files_table[fd-2], position);
	}
	free(argv);
}

void sys_tell (uint32_t *syscall_num, struct intr_frame *intr) 
{
	uint32_t *argv;
	int fd;
	unsigned pos;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	
	fd = (int) argv[0];

	if(fd <= thread_current()->fd && fd>1)
	{
		pos=file_tell(thread_current()->files_table[fd-2]);
	}	
	else
		pos=-1;

	intr->eax = pos;
	free(argv);
}
void sys_close (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t *argv;
	int fd;
	int i;
	struct file** files_table;
	
	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	fd = (int) argv[0];
	if(fd <= thread_current()->fd && fd>1)// && thread_current()->files_table[fdtable_idx(fd)])
	{
		if(thread_current()->files_table[fdtable_idx(fd)]){
			file_close(thread_current()->files_table[fdtable_idx(fd)]);
			thread_current()->files_table[fdtable_idx(fd)] = NULL;
		}
	}	
	free(argv);
}
void sys_mmap (uint32_t *syscall_num, struct intr_frame *intr) {}
void sys_mummap (uint32_t *syscall_num, struct intr_frame *intr) {}
void sys_chdir (uint32_t *syscall_num, struct intr_frame *intr) {}
void sys_mkdir (uint32_t *syscall_num, struct intr_frame *intr) {}
void sys_readdir (uint32_t *syscall_num, struct intr_frame *intr) {}
void sys_isdir (uint32_t *syscall_num, struct intr_frame *intr) {}
void sys_inumber (uint32_t *syscall_num, struct intr_frame *intr) {}

void sys_fibonacci (uint32_t *syscall_num, struct intr_frame *intr)
{
	uint32_t* argv;
	int n;
	int a1=0, a2=1;
	int i, tp;

	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	n = (int) argv[0];

	if(n==0)	intr->eax = a1;
	else if(n==1)	intr->eax = a2;
	else{
		for(i=2; i<=n; i++)
		{
			a1 = a1 + a2; 

			if(i==n)	break;	
			tp = a1;
			a1 = a2;
			a2 = tp;			
		}	
		intr->eax = a1;
	}
	free(argv);
}
void sys_sum_of_four_integers (uint32_t *syscall_num, struct intr_frame *intr) 
{
	uint32_t *argv;
	int a, b, c, d;
	
	argv = (uint32_t *) malloc (sizeof(uint32_t) * sys_argv_num[*syscall_num]);
	get_argv(syscall_num, intr, &argv);
	a = (int) argv[0];
	b = (int) argv[1];
	c = (int) argv[2];
	d = (int) argv[3];

	intr->eax = a+b+c+d;
	free(argv);
}

/* by jinu (PROJECT2_1 User program basic)
	this function return argument value array
*/

void get_argv (uint32_t *syscall_num, struct intr_frame *intr, uint32_t **argv)
{
	uint32_t *cur_argv;
	int i;
	
	for (i=0; i<sys_argv_num[*syscall_num]; i++)
	{
		cur_argv = ((uint32_t *)intr->esp + (1+i));
		check_address(cur_argv); 	
		(*argv)[i] = *cur_argv;
	}
}

/* by jinu (PROJECT2_1 User program basic)*/
/*jinu 2017/10/12
	check the address range and if there's problem, terminate the process with unlock/free 
*/
void check_address(uint32_t* address)
{
	if(is_kernel_vaddr(address)
		||!pagedir_is_mapped(thread_current()->pagedir, address)
		||address==NULL
		)
	{
		terminate_process();
	}

}

/* by jinu (PROJECT2_1 User program basic)
 terminate process */
void terminate_process()
{
	int status=-1;
	/* free and unlock */
	thread_current()->exit_status = status;
	printf("%s: exit(%d)\n", thread_current()->name, status);
	thread_exit ();
}

uint32_t fdtable_idx(int fd)
{
	return fd-2;
}
uint32_t fdtable_size(int fd)
{
	return fd-1;
}
