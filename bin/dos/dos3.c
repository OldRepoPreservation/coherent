/* dos3.c */

#include "dos0.h"

MDIR *smdp;
DIR *sdp;
char *scp, *sname;

/*
 * Find the given filename relative to the given directory.
 * Return a pointer to the MDIR identifying the file, or NULL if not found.
 * Store a DIR pointer through the supplied dpp.
 * The stored DIR pointer gives the containing DIR for non-directory files
 * and the subdirectory DIR for directories.
 */
MDIR *
find(name, dp, dpp) char *name; register DIR *dp; DIR **dpp;
{
	register MDIR *mdp;
	register char *s;
	register char *cp;

dbprintf(("find(%s)\n", name));
	if ((s = strchr(name, '/')) == NULL) 
	{
		/* No pathname, look for name in the directory. */
		cp = dosname(name);
		for (mdp = dp->d_dir; mdp < dp->d_edp; mdp++) 
		{
			s = mdp->m_name;
			if (*s == MEMPTY || *s == MFREE)
				continue;
 			dbprintf(("lmatch(%s,%s) = %d\n", name, cohn(s), lmatch(name, cohn(s))));
			if (lmatch(name, cohn(s)) == 1) 
			{
				if((strncmp (s, cp, 11) ==0) && isdir(mdp))
				{
					/* Subdirectory, find its DIR. */
					for (dp = dp->d_child; dp != NULL; 
							   dp = dp->d_sibling)
						if (strncmp(dp->d_dname, cp, 
								    11) == 0)
							break;
					if (dp == NULL)
						fatal("find subdirectory botch");
					/* Read in the MDIR if necessary. */
					if (dp->d_dir == NULL)
						readmdir(dp);
				}
				if (dpp != NULL)
					*dpp = dp;
				smdp = mdp;
				scp = cp;
				sdp = dp;
				sname = name;
dbprintf(("before findnext(%s)\t\mdp = %d\tdp->d_edp=%d\n", cp, mdp, dp->d_edp));
				return mdp; 
			}
		}
		return NULL;
	} 
	else if (s == name)
		return find(++s, dp, dpp);   /* "/foo" means look for "foo" */
	/* Explicit pathname, find the directory and recur. */
	*s = '\0';			/* NUL-terminate dirname */
	cp = dosname(name);
	*s++ = '/';			/* Restore the '/' */
	for (dp = dp->d_child; dp != NULL; dp = dp->d_sibling)
		if (strncmp(dp->d_dname, cp, 11) == 0)
			break;
	if (dp == NULL)
		return NULL;		/* Not found */
	if (dp->d_dir == NULL)
		readmdir(dp);
	return find(s, dp, dpp);
}

MDIR *
findnext(dpp) DIR **dpp;
{
	register MDIR *mdp = ++smdp;
	register char *s;
	register char *cp = scp;
	register DIR *dp = sdp;

dbprintf(("during findnext(%s)\t\mdp = %d\tdp->d_edp=%d\n", cp, mdp, dp->d_edp));
	for (; mdp < dp->d_edp; mdp++) 
	{
		s = mdp->m_name;
		if (*s == MEMPTY || *s == MFREE)
			continue;
		dbprintf(("lmatch(%s,%s) = %d\n", sname, cohn(s), lmatch(sname, cohn(s))));
 		if (lmatch(sname, cohn(s)) == 1) 
		{
			if ((strncmp (s, cp, 11) ==0) && isdir(mdp)) 
			{
				/* Subdirectory, find its DIR. */
				for (dp = dp->d_child; dp != NULL; 
							dp = dp->d_sibling)
				if (strncmp(dp->d_dname, cp, 11) == 0)
					break;
				if (dp == NULL)
					fatal("find subdirectory botch");
				/* Read in the MDIR if necessary. */
				if (dp->d_dir == NULL)
					readmdir(dp);
			}
			if (dpp != NULL)
				*dpp = dp;
			smdp = mdp;
			return mdp;
		}
	}
	return NULL;
}


/*
 * See if a pattern matches a string.
 * '\' escapes the next character.
 */
lmatch(pp, sp)
register char *pp;
register char *sp;
{
	int c2;
	register int c1;

 	
 	if (*sp == '.')
 		return 0;
	if ((strcspn(pp, " ") == 0) && (strcspn(sp, " ") == 0))
		return 1;
	while ((c1=*pp++)) {
		switch (c1) {
		case '?':
			if (*sp++)
				continue;
			return (0);
		case '*':
			do {
				if (lmatch(pp, sp))
					return (1);
			} while (*sp++);
			return (0);
		case '[':
			if ((c2=*sp++) == '\0')
				return (0);
			for (;;) {
				if ((c1=*pp++) == '\0' || c1 == ']')
					return (0);
				if (c1 == '\\' && (c1=*pp++) == '\0')
					return (0);
				if (c1 == c2)
					break;
				if (*pp == '-') {
					pp += 1;
					if (c2 < c1)
						continue;
					if ((c1=*pp++) == '\0')
						return (0);
					if (c1 == '\\' && (c1=*pp++) == '\0')
						return (0);
					if (c2 <= c1)
						break;
				}
			}
			while ((c1 = *pp++) != ']') {
				if (c1 == '\0')
					return (0);
				if (c1 == '\\' && *pp++ == '\0')
					return (0);
			}
			continue;
		case '\\':
			if ((c1=*pp++) == '\0')
				return (0);
			/* fall through */
		default:
			if (c1 == *sp++)
				continue;
			return (0);
		}
	}
	return (*sp=='\0');
}

char * cohn(name) char * name;
{
	static char lname[14];
	char *tmp = lname;
	int i = 0;

	do {
		if ((i == 8) && (*name != ' '))
			*(tmp++) = '.';
		else if (*name == ' ')
			*name++;
		else {
			*(tmp++) = tolower(*name);
			name++;
		}
		i++;
	} while ( *(name - 1) != '\0');
	return lname;
}


/*
 * Input up to nb characters into bp from COHERENT file fp,
 * taking into account the ASCII flag.
 * Pad to nb characters with NUL or CTRLZ.
 * Return the number of characters actually read.
 */
int
finput(fp, bp, nb) FILE *fp; register char *bp; unsigned nb;
{
	register int c;
	register unsigned n;
	register char *ep;
	static char needlf = 0;

	if (!aflag) {
		n = read(fileno(fp), bp, nb);
		for (ep = bp+nb, bp += n; bp < ep; )
			*bp++ = '\0';
		return n;
	}
	for (n = 0, ep = bp+nb; bp < ep; ) {
		if (needlf) {
			c = '\n';
			needlf = 0;
		} else {
			if ((c = getc(fp)) == '\r')
				continue;
			else if (c == '\n') {
				c = '\r';
				needlf++;
			} else if (c == EOF)
				c = CTRLZ;
		}
		if (c == CTRLZ)
			break;
		n++;
		*bp++ = c;
	}
	while (bp < ep)
		*bp++ = CTRLZ;				/* pad with EOFs */
	return n;
}

/*
 * Format an MS-DOS filesystem.
 * The optional argument is a boot block.
 */
void
format(nargs, args) int nargs; char *args[];
{
	register char *cp;
	int i, c;
	char *bp;
	unsigned int nsize;
	struct stat sbuf;

	/* Prompt to make sure. */
	printf("Are you sure you want to build an MS-DOS filesystem on device %s? ", device);
	fflush(stdout);
	c = getchar();
	while ((i = getchar()) != '\n' && i != EOF)
		;
	if (c != 'y' && c != 'Y')
		exit(1);
	if (nargs > 1)
		fatal("format: only one bootstrap file allowed");

	/* Get BPB for the specified format. */
	if (fstat(fsfd, &sbuf) < 0)
		fatal("format: cannot stat");
	i = sbuf.st_rdev;
	if (major(i) != FL_MAJOR)
		fatal("format: illegal device major number");
	switch (minor(i) & 0x0F) {
	case  0:	bpb = &s8floppy;	break;
	case  3:	bpb = &s9floppy;	break;
	case  9:	bpb = &d8floppy;	break;
	case 12:	bpb = &d9floppy;	break;
	case 13:	bpb = &q9floppy;	break;
	case 14:	bpb = &d15floppy;	break;
	case 15:	bpb = &d18floppy;	break;
	default:
		fatal("format: unsupported diskette type %d", minor(i) & 0x0F);
	}
	setglobals();

	/* Read boot block or build one. */
	if (nargs > 0) {
		if ((i=open(args[0], 0)) < 0)
			fatal("format: cannot open boot block file \"%s\"", args[0]);
		if (read(i, bootb, BBSIZE) != BBSIZE)
			fatal("format: boot block read error");
		close(i);
	} else {
		/* Patch in "jmp short ." with required flag byte. */
		bootb[0] = 0xEB;		/* JMP short */
		bootb[1] = 0xFE;		/* . (PC-relative) */
		bootb[2] = 0x90;		/* required flag */
		strncpy(&bootb[3], "COHERENT", 8);
		for (cp=&bootb[BPBOFF], bp=bpb, i=0; i < sizeof(BPB); i++)
			*cp++ = *bp++;		/* copy the BPB */
	}
	if (write(fsfd, bootb, BBSIZE) != BBSIZE)
		fatal("format: boot block write error");

	/* Write the FATs (media id and zero bytes). */
	if (fatbytes != 1)
		fatal("format: fatbytes=%d", fatbytes);
	nsize = (maxcluster + 1) * sizeof(int);
	if (nsize < fatsize * ssize)
		nsize = fatsize * ssize;
	if ((fatcache = calloc(nsize, 1)) == NULL)
		fatal("format: FAT allocation failed");
	fatcache[0] = 0xFF00 | bpb->b_media;
	fatcache[1] = CLEOF;
	fatcfirst = 0;
	fatccount = fatsize;
	fatcflag = 1;
	writefat();
	free(fatcache);

	/* Write the directory (all zero bytes). */
	if ((cp = calloc(dirsize, ssize)) == NULL)
		fatal("format: directory allocation failed");
	diskwrite(cp, dirbase, dirsize, "directory");
	free(cp);
}

/*
 * Output nb characters from bp to COHERENT file fp,
 * taking into account the ASCII flag.
 * Return the number of characters actually written.
 */
int
foutput(fn, fp, bp, nb) char *fn; FILE *fp; register char *bp; unsigned nb;
{
	register unsigned n;
	register char *ep;

	if (!aflag) {
		if (write(fileno(fp), bp, nb) != nb)
			fatal("extract: write error on file \"%s\"", fn);
		return nb;
	}
	for (n = 0, ep = bp+nb; bp < ep; bp++) {
		if (*bp == '\r')
			continue;
		else if (*bp == CTRLZ)
			break;
		n++;
		putc(*bp, fp);
	}
	if (ferror(fp))
		fatal("extract: write error on file \"%s\"", fn);
	return n;
}

/*
 * Return the next free cluster on the diskette.
 * Stick an EOF marker in the cluster.
 * Failure is fatal.
 */
unsigned int
freecluster()
{
	register unsigned int n;

	for (n = 2; n <= maxcluster; n++)
		if (getcluster(n) == CLFREE) {
			putcluster(n, CLEOF);
			return n;
		}
	fatal("out of space on MS-DOS disk");
}

/*
 * Read the key for the command.
 * Set globals accordingly.
 * Return the required device mode for the command.
 */
int
key(s) register char *s;
{
	register int c, nfun;

	nfun = 0;
	if (*s == '-')
		++s;			/* ignore optional '-' */
	while ((c = *s++) != '\0')
		switch (c) {

		/* Functions. */
		case 'd':	fun = delete;	++nfun;	break;
		case 'F':	fun = format;	++nfun;	break;
		case 'l':	fun = label;	++nfun;	break;
		case 'r':	fun = replace;	++nfun;	break;
		case 't':	fun = table;	++nfun;	break;
		case 'x':	fun = extract;	++nfun;	break;

		/* Flags. */
		case 'a':	aflag++;		break;
		case 'c':	cflag++;		break;
		case 'k':	kflag++;		break;
		case 'n':	nflag++;		break;
		case 'p':	pflag++;		break;
		case 's':	sflag++;		break;
		case 'v':	vflag++;		break;
		case 'V':
			fprintf(stderr, "dos: V%s\n", VERSION);
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			xpart = c - '0';
			break;

		default:
			usage();
		}

	if (nfun == 0)
		fun = table;			/* default */
	else if (nfun > 1)
		fatal("must specify one function [dFlrtx]");
	return (cflag || fun==table || fun==extract) ? 0 : 2;
}

/*
 * Label a disk with a volume label.
 */
void
label(nargs, args) int nargs; char *args[];
{
	if (nargs != 1)
		fatal("label: single argument required");
	if (find(args[0], root, NULL) != NULL)
		fatal("label: file \"%s\" already exists", args[0]);
	if (strchr(args[0], '/') != NULL)
		fatal("label: label cannot use character '/'");
	if (volume != NULL)
		deletefile(volume, root);
	volume = creatfile(args[0], root);
	volume->m_attr = MVOLUME;
}

/*
 * Initialize an MDIR with the current time and date,
 * the given name, attribute and cluster, and size 0L.
 * Set the flag requiring the DIR to be written.
 */
void
mdirinit(mdp, dp, name, attr, cluster)
register MDIR *mdp;
DIR *dp;
char *name;
unsigned int attr, cluster;
{
	register int i;

	dbprintf(("mdirinit(mdp=%x dp=\"%s\" name=\"%s\" attr=%x cl=%x)\n", mdp, dp->d_dname, name, attr, cluster));
	strncpy(mdp->m_name, name, 11);
	mdp->m_attr = attr;
	for (i=0; i<10; i++)
		mdp->m_junk[i] = 0;
	dostime(mdp, NULL);
	mdp->m_cluster = cluster;
	mdp->m_size = 0;
	dp->d_dirflag = 1;
}

/* end of dos3.c */

#if 0
MDIR *
findnext(name, dp, dpp) char *name; register DIR *dp; DIR **dpp;
{
	register MDIR *mdp;
	register char *s;
	register char *cp;

printf("findnext(%s)\n", sname);
	cp = scp;
	for (mdp = ++smdp; mdp < dp->d_edp; mdp++) 
	{
		s = mdp->m_name;
		if (*s == MEMPTY || *s == MFREE)
			continue;
		printf("fn - lmatch(%s,%s) = %d\n", sname, cohn(s), lmatch(name, cohn(s)));
		if (lmatch(sname, cohn(s)) == 1) 
		{
			if((strncmp (s, cp, 11) == 0) && isdir(mdp)) 
			{
				/* Subdirectory, find its DIR. */
				for (dp = dp->d_child; dp != NULL; 
						   dp = dp->d_sibling)
					if (strncmp(dp->d_dname, cp, 11) == 0)
						break;
				if (dp == NULL)
					fatal("find subdirectory botch");
				/* Read in the MDIR if necessary. */
				if (dp->d_dir == NULL)
					readmdir(dp);
			}
			if (dpp != NULL)
				*dpp = dp;
			smdp = mdp;
			return mdp; 
		}
		return NULL;
	}
}

#endif
