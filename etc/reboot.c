/*
 * Reboot the processor by transferring to the reset vector of the 8088.
 *
 * $Log: $
 * 86/12/19	Allan Cornish		/usr/src/cmd/etc/reboot.c
 * reboot.s converted into reboot.c and rebootas.s to provide time for
 * disk drives to turn off before initiating reboot.
 */

#include <signal.h>

sigquiet( sig )
int sig;
{
	signal( sig, sigquiet );
}

main( argc, argv )
register char ** argv;
{
	/*
	 * Trap alarm signals.
	 */
	signal( SIGALRM, sigquiet );

	/*
	 * Wait at least 4 seconds for drives to turn off, etc.
	 */
	alarm( 5 );
	pause();

	/*
	 * Reboot the processor.
	 */
	reboot();

	/*
	 * Should never reach here.
	 */
	exit(1);
}
