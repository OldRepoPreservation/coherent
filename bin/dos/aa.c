#include <stdio.h>
#include <signal.h>

short die_signal();
main()
{
	int i;

	for (i=17; i < 32; i++)
		;

	signal(18, die_signal);

	system("mkdir bob");
}

short die_signal(s) short s;
{
	printf("received signal %d, quitting.\n", s);
	exit(1);
}
