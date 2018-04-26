/* halt.c

   Simple program to test whether running a user program works.
 	
   Just invokes a system call that shuts down the OS. */

#include <syscall.h>
#include <stdlib.h>
int
main (int argc, char *argv[])
{
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
	create("hello.txt", 0);
//  exec ((char *) 0x20101234);

	int fd;
	char buf_r[20];
	int i;	

	for(i=0;i<128;i++)
	{
		fd = open("hello.txt");
		printf("fd is %d\n",fd);
	}
//	create("jinu.txt", 100);
//	printf("jinu.txt created\n");
/*
	fd=open("halt");
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
}
