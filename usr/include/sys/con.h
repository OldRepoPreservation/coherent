/* (-lgl
 * 	COHERENT Version 3.2.1
 * 	Copyright (c) 1982, 1992 by Mark Williams Company.
 * 	All rights reserved. May not be copied without permission.
 -lgl) */
/*
 * /usr/include/sys/con.h
 *
 * CON struct for device drivers.
 *
 * Revised: Tue May 25 13:36:42 1993 CDT
 */
#ifndef	__SYS_CON_H__
#define	__SYS_CON_H__

#include <common/feature.h>
#include <sys/types.h>
#include <sys/ksynch.h>

#if	! __KERNEL__
# error	You must be compiling the kernel to use this header
#endif

/*
 * Device driver table.
 */
typedef struct drv {
	struct	 con *d_conp;		/* Pointer to configuration */
#if	_I386
	int	foo [2];		/* not used */
#else
	struct	 seg *d_segp;		/* Segmentation containing driver */
	dmap_t	 d_map;			/* Segmentation map */
#endif
	int	 d_time;		/* Timeout is active */
	GATE	 d_gate;		/* Gate for loading */
} DRV;

/*
 * Driver interface entry.
 */
typedef struct con {
	int	c_flag;			/* Flags */
	int	c_mind;			/* Major index */
	int	(*c_open)();		/* Open */
	int	(*c_close)();		/* Close */
	int	(*c_block)();		/* Block */
	int	(*c_read)();		/* Read */
	int	(*c_write)();		/* Write */
	int	(*c_ioctl)();		/* Ioctl */
	int	(*c_power)();		/* Powerfail */
	int	(*c_timer)();		/* Timeout */
	int	(*c_load)();		/* Load */
	int	(*c_uload)();		/* Unload */
	int	(*c_poll)();		/* Poll */
} CON;

/*
 * Flags.
 */
#define	DFBLK	0x01			/* Block device */
#define	DFCHR	0x02			/* Character device */
#define DFTAP	0x04			/* Tape */
#define	DFPOL	0x08			/* Pollable device */

/*
 * Functions.
 */
extern	CON	*drvmap();		/* bio.c */

/*
 * Global variables.
 */
extern	int	drvn;			/* Number of entries in table */
extern	DRV	drvl[];			/* Driver table */

#if	_DDI_DKI || _ENABLE_STREAMS

/*
 * NIGEL: This seems like the easiest place to define the hooks into STREAMS
 * that I have inserted calls to in various key places, including "bio.c" and
 * "clock.c".
 */

__EXTERN_C_BEGIN__

void		STREAMS_TIMEOUT		__PROTO ((void));
void		STREAMS_SCHEDULER	__PROTO ((void));
void		STREAMS_INIT		__PROTO ((void));
void		STREAMS_EXIT		__PROTO ((void));
CON	      *	STREAMS_GETCON		__PROTO ((o_dev_t _dev));

__EXTERN_C_END__

#else	/* ! (_DDI_DKI || _ENABLE_STREAMS) */

#define		STREAMS_TIMEOUT()
#define		STREAMS_SCHEDULER()
#define		STREAMS_INIT()
#define		STREAMS_EXIT()
#define		STREAMS_GETCON(dev)	NULL

#endif	/* ! (_DDI_DKI || _ENABLE_STREAMS) */

#endif	/* ! defined (__SYS_CON_H__) */
