/*
 * File:	ssqueue.c
 *
 * Purpose:
 *	Queueing routines for Seagate SCSI driver.
 *	Should be generalizable for other hard drives.
 *
 * $Log:	/usr/src/sys/i8086/drv/RCS/ssqueue.c,v $
 * Revision 1.1	91/03/22  17:39:06	root
 * Initial code - not tested yet
 * 
 */

/*
 * Includes.
 */
#include <coherent.h>
#include <sys/buf.h>
#include <scsiwork.h>

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
static scsi_work_t	* ssq_head;	/* point to first node */
static scsi_work_t	* ssq_tail;	/* point to last node */
static int		ssq_count;	/* number of nodes in the queue */

/*
 * Functions.
 *	Import Functions.
 *	Export Functions.
 *	Local Functions.
 */
void ssq_wr_tail();
scsi_work_t * ssq_rd_head();
scsi_work_t * ssq_rm_head();

/*
 * ssq_wr_tail()
 *
 * Append a scsi_work_t object to the doubly-linked queue.
 * Object to be inserted has been allocated by the caller.
 */
void ssq_wr_tail(sw)
scsi_work_t * sw;
{
	if (ssq_count == 0) {
		ssq_head = ssq_tail = sw;
		sw->sw_actf = sw->sw_actl = NULL;
	} else {
		ssq_tail->sw_actf = sw;
		sw->sw_actf = NULL;
		sw->sw_actl = ssq_tail;
		ssq_tail = sw;
	}
	ssq_count++;
}

/*
 * ssq_rd_head()
 *
 * Nondestructively fetch the head entry in the queue - i.e., this routine
 * does not remove an entry from the queue (see ss_rm_head() for that).
 * Return NULL if queue is empty, else return pointer to head item.
 */
scsi_work_t * ssq_rd_head()
{
	return ssq_head;
}

/*
 * ssq_rm_head()
 *
 * Delete head item from the queue.  Return a pointer to the node deleted,
 * or NULL if the queue was already empty.
 *
 * This routine does NOT deallocate the node.  That must be done by the
 * calling function after this routine runs.
 */
scsi_work_t * ssq_rm_head()
{
	scsi_work_t * ret;

	if (ssq_count > 0) {
		ret = ssq_head;
		if (ssq_count == 1) {
			ssq_head = ssq_tail = NULL;
		} else {
			ssq_head = ssq_head->sw_actf;
			ssq_head->sw_actl = NULL;
		}
		ssq_count--;
	} else
		ret = NULL;

	return ret;
}
