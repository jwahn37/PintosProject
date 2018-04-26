#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
int main(int argc, char *argv[])
{
	int sum;
	int fibo=0;
	int a,b,c,d;

	a = atoi(argv[1]);
	b = atoi(argv[2]);
	c = atoi(argv[3]);
	d = atoi(argv[4]);
	
	fibo = fibonacci(a);
	sum = sum_of_four_integers(a, b, c, d);

	printf("%d %d\n", fibo, sum);
}
