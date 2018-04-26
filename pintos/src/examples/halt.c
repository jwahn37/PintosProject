/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

	static int i=0;
#include <syscall.h>
#include <stdlib.h>
int
main (int argc, char *argv[])
{
//	static int i=0;
//	for(i=0;i<3;i++)
//	{
		if(i<=30){
		wait(exec("halt"));
		printf("%d\n", i);
		i=i+1;
		}

//	}
 // int *PHYS_BASE = (int *)0xC0000000;

//	int hi;
 // *(int *) NULL = 42;
/*
  hi = *(int *) NULL;

   *PHYS_BASE;
*/
//   *PHYS_BASE = 42;

//   open ((char *)PHYS_BASE);
//   exit (-1);

//  msg ("create(NULL): %d", create (NULL, 0));

//  *(int *)NULL = 42;
//  int handle;
//  char buffer[16];
 /* 
  CHECK ((handle = open ("rox-simple")) > 1, "open \"rox-simple\"");
  CHECK (read (handle, buffer, sizeof buffer) == (int) sizeof buffer,
         "read \"rox-simple\"");
  CHECK (write (handle, buffer, sizeof buffer) == 0,
         "try to write \"rox-simple\"");
*/
/*	create("hello.txt", 0);
//  exec ((char *) 0x20101234);

	int fd;
	char buf_r[20];
	int i;	

	for(i=0;i<128;i++)
	{
		fd = open("hello.txt");
		printf("fd is %d\n",fd);
	}
*/
/*
  int handle[5];;
	int i;
	create("sample.txt", 10);
  for(i=0;i<5;i++){
	handle[i]=open ("sample.txt");
  printf("sample txt opend : %d\n",handle[i]);
*/

	//msg ("close \"sample.txt\"");
 // close (handle);
//	printf("closed\n");
/*
}
	for(i=0;i<5;i++)
{
		close(handle[i]);
		printf("closed : %d\n",handle[i]);
}
*/
/*	int fd;
	char buf_r[100];
	
	create("jinu.txt", 100);
	printf("jinu.txt created\n");

	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
	fd=open("jinu.txt");
	printf("opened : %d\n",fd);
*/
/*
	printf("halt opend fd : %d\n",fd);
	write(fd, "HELLO JINU\n", 10);
	printf("halt wrote\n");
	seek(fd, 0);
	printf("halt seek\n");
	read(fd, buf_r, 10);
	printf("halt read buffer : %s\n",buf_r);
*/
//	create(NULL,0);
  //msg ("create(0x20101234): %d", create ((char *) 0x20101234, 0));

//  exec ((char *) 0x20101234);


/*
	int sum;
	int pibo=0;
	int a,b,c,d;

	a = atoi(argv[0]);
	b = atoi(argv[1]);
	c = atoi(argv[2]);
	d = atoi(argv[3]);

	pibo = pibonacci(a);
	sum = sum_of_four_integers(a, b, c, d);

	printf("%d %d\n", pibo, sum);
	*/
	return 0;
}
