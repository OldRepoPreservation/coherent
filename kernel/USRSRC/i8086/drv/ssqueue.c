/*
 * File:	ssqueue.c
 *
 * Purpose:
 *	Queueing routines for Seagate SCSI driver.
 *	Should be generalizable for other hard drives.
 *
 * $Log:	/usr/src/sys/i8086/drv/RCS/ssqueue.c,v $
 * Revision 1.3.1.1	91/04/11  15:59:53	root
 * debug printing added
 * 
 * Revision 1.3	91/03/25  19:05:25	root
 * run at high priority
 * 
 * Revision 1.2	91/03/25  13:04:04	root
 * Minor code fixes.  Now passes unit test.
 * 
 * Revision 1.1	91/03/22  17:39:06	root
 * Initial code - not tested yet
 * 
 */

/*
 * Includes.
 */
#include <coherent.h>
#include <sys/buf.h>

/*
 * Definitions.
 *	Constants.
 *	Macros with argument lists.
 *	Typedefs.
 *	Enums.
 */

/*
 * Global Data.
 *	Import Variables.
 *	Export Variables.
 *	Local Variables.
 */
static BUF	* ssq_head;	/* point to first node */
static BUF	* ssq_tail;	/* point to last node */
static int		ssq_count;	/* number of nodes in the queue */

/*
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
void ssq_wr_tail();
BUF * ssq_rd_head();
BUF * ssq_rm_head();

/*
 * Debug macros.
 */
#if 1
#define QSIZE	printf("Q%d:", ssq_count)
#else
#define QSIZE
#endif

/*
 * ssq_wr_tail()
 *
 * Append a BUF object to the doubly-linked queue.
 * Object to be inserted has been allocated by the caller.
 * Run at high priority.
 */
void ssq_wr_tail(bp)
BUF * bp;
{
	int s;

	s = sphi();
	if (ssq_count == 0) {
		ssq_head = ssq_tail = bp;
		bp->b_actf = bp->b_actl = NULL;
	} else {
		ssq_tail->b_actf = bp;
		bp->b_actf = NULL;
		bp->b_actl = ssq_tail;
		ssq_tail = bp;
	}
	ssq_count++;
QSIZE;
	spl(s);
}

/*
 * ssq_rd_head()
 *
 * Nondestructively fetch the head entry in the queue - i.e., this routine
 * does not remove an entry from the queue (see ss_rm_head() for that).
 * Return NULL if queue is empty, else return pointer to head item.
 */
BUF * ssq_rd_head()
{
	return ssq_head;
}

/*
 * ssq_rm_head()
 *
 * Delete head item from the queue.  Return a pointer to the node deleted,
 * or NULL if the queue was already empty.
 * Run at high priority.
 *
 * This routine does NOT deallocate the node.  That must be done by the
 * calling function after this routine runs.
 */
BUF * ssq_rm_head()
{
	BUF * ret;
	int s;

	s = sphi();
	if (ssq_count > 0) {
		ret = ssq_head;
		if (ssq_count == 1) {
			ssq_head = ssq_tail = NULL;
		} else {
			ssq_head = ssq_head->b_actf;
			ssq_head->b_actl = NULL;
		}
		ssq_count--;
QSIZE;
	} else
		ret = NULL;
	spl(s);

	return ret;
}
