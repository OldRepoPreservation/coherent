/* (-lgl
 * 	COHERENT Version 4.0
 * 	Copyright (c) 1982, 1992 by Mark Williams Company.
 * 	All rights reserved. May not be copied without permission.
 -lgl) */
#ifndef	CON_H
#define	CON_H
/*
 * Device driver configuration.
 */

#include <sys/types.h>

/*
 * Device driver table.
 */
typedef struct drv {
	struct	 con *d_conp;		/* Pointer to configuration */
	struct	 seg *d_segp;		/* Segmentation containing driver */
	dmap_t	 d_map;			/* Segmentation map */
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
#define	DFBLK	0000001			/* Block device */
#define	DFCHR	0000002			/* Character device */
#define DFTAP	0000004			/* Tape */
#define	DFPOL	0000010			/* Pollable device */
#define	DFERR	0100000			/* Error */

#ifdef KERNEL
/*
 * Functions.
 */
extern	CON	*drvmap();		/* bio.c */

/*
 * Global variables.
 */
extern	int	drvn;			/* Number of entries in table */
extern	DRV	drvl[];			/* Driver table */

#endif

#endif
