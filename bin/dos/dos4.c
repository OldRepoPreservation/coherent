/* dos4.c */

#include "dos0.h"

/*
 * Compare two MDIR names.
 * Directories precede ordinary files.
 * Called by qsort().
 */
int
namecmp(mdpp1, mdpp2) MDIR **mdpp1, **mdpp2;
{
	register MDIR *mdp1, *mdp2;
	unsigned long t1, t2;

	mdp1 = *mdpp1;
	mdp2 = *mdpp2;
	if (isdir(mdp1) != isdir(mdp2))
		return (isdir(mdp1)) ? -1 : 1;
	if (nflag) {
		/*
		 * Compare by time with unsigned long compare.
		 * m_junk[10] is really the first byte of the time.
		 * Kludgy but it works.
		 */
		t1 = *(unsigned long *)&mdp1->m_junk[10];
		t2 = *(unsigned long *)&mdp2->m_junk[10];
		if (t1 != t2)
			return (t1 < t2) ? -1 : 1;
	}
	return strncmp(mdp1->m_name, mdp2->m_name, 11);
}

/*
 * Allocate a new DIR and link it into the DIR tree.
 * Initialize it with the supplied information.
 * Return a pointer to it.
 * The MDIR array is not allocated and read until required.
 */
DIR *
newdir(parent, mdp, files)
DIR *parent;
register MDIR *mdp;
int files;
{
	register DIR *dp;

	/* Allocate a new DIR. */
	dbprintf(("newdir(parent=\"%s\" mdp=%x files=%u): ", (parent==NULL)?"<root>":parent->d_dname, mdp, files));
	if ((dp = (DIR *)malloc(sizeof(DIR))) == NULL)
		fatal("directory allocation failed");

	/* Initialize the DIR. */
	dp->d_parent = parent;
	dp->d_child = NULL;
	if (parent == NULL) {
		/* Root. */
		dp->d_sibling = NULL;
		dp->d_cluster = 0;
		strcpy(dp->d_dname, "<root>     ");
	} else {
		/* Subdirectory. */
		dp->d_sibling = parent->d_child;
		parent->d_child = dp;
		dp->d_cluster = mdp->m_cluster;
		strncpy(dp->d_dname, mdp->m_name, 11);
		dp->d_dname[11] = '\0';
	}
	dp->d_files = files;
	dp->d_dirblocks = (files * sizeof(MDIR) + ssize - 1) / ssize;
	dp->d_dir = dp->d_edp = NULL;
	dp->d_dirflag = 0;
	dbprintf(("\"%s\" cl=%x blocks=%u dp=%x\n", dp->d_dname, dp->d_cluster, dp->d_dirblocks, dp));
	return dp;
}

/*
 * Print a nonfatal error message.
 * Set the return status to 1.
 * Uses the nonportable "%r" format.
 */
void
nonfatal(x) char *x;
{
	fprintf(stderr, "dos: %r\n", &x);
	estat = 1;
}

/*
 * Allocate space for an MS-DOS directory.
 * Read in the directory unless dp is not root and its d_cluster is 0.
 */
void
readmdir(dp) register DIR *dp;
{
	register MDIR *mdp;
	register int n;

	dbprintf(("readmdir(%s) blocks=%u cl=%x\n", dp->d_dname, dp->d_dirblocks, dp->d_cluster));
	if ((mdp = (MDIR *)calloc(dp->d_dirblocks, ssize)) == NULL)
		fatal("directory allocation failed");
	dp->d_dir = mdp;
	dp->d_edp = mdp + dp->d_files;

	if (dp->d_cluster == 0) {
		if (dp->d_parent != NULL)
			return;			/* new subdirectory */
		diskread(mdp, dirbase, dp->d_dirblocks, "directory");
	} else
		for (n = dp->d_cluster ; n <= CLMAX; n = getcluster(n)) {
			diskread(mdp, cltosec(n), clsize, "subdirectory");
			mdp += mdirsize;
		}

	/* Scan through the directory for volume label and subdirectories. */
	for (mdp = dp->d_dir; mdp < dp->d_edp; mdp++) {
		if ((n = mdp->m_name[0]) == MFREE || n == MEMPTY || n == MMDIR)
			continue;
		if (mdp->m_attr & MVOLUME)
			volume = mdp;
		if isdir(mdp)
			newdir(dp, mdp,	dirclusters(mdp) * mdirsize);
	}
}

/*
 * Replace one or more files on the MS-DOS file system.
 */
void
replace(nargs, args) int nargs; char *args[];
{
	register char **ap;
	struct stat s;

	if ((clbuf = malloc(clsize * ssize)) == NULL)
		fatal("cluster buffer allocation failed");
	if (nargs == 0)
		replacedir(NULL);
	else if (pflag) {
		if (nargs!=1)
			fatal("replace: exactly one file required with 'p' option");
		replacefile(args[0]);
	} else for (ap=args; *ap != NULL; ap++) {
		if (stat(*ap, &s) == -1)
			fatal("replace: \"%s\" not found", *ap);
		else if ((s.st_mode & S_IFDIR) == S_IFDIR)
			replacedir(*ap);
		else if ((s.st_mode & S_IFREG) == S_IFREG)
			replacefile(*ap);
		else
			nonfatal("replace: \"%s\" not ordinary file: suppressed",
				*ap);
	}
	free(clbuf);
}

/*
 * Replace a directory; NULL means ".".
 * Replace subdirectories recursively unless sflag.
 */
void
replacedir(name) char *name;
{
	register struct direct *dirp;
	register char *cp;
	int fd;
	struct stat s;
	char dirbuf[sizeof(struct direct) + 1];
	char namebuf[NAMEMAX];

	dbprintf(("replacedir(%s)\n", name));
	if (name == NULL || strcmp(name, ".") == 0) {
		name = ".";
		cp = namebuf;
	} else {
		strcpy(namebuf, name);
		strcat(namebuf, "/");
		cp = &namebuf[strlen(namebuf)];
	}
	dirp = dirbuf;
	dirbuf[sizeof(struct direct)] = '\0';		/* NUL-terminate d_name */
	if ((fd = open(name, 0)) == -1)
		fatal("cannot search directory \"%s\"", name);
	while (read(fd, dirbuf, sizeof(struct direct)) == sizeof(struct direct)) {
		if (dirp->d_ino == 0
		 || strcmp(dirp->d_name, ".") == 0
		 || strcmp(dirp->d_name, "..") == 0) 
			continue;
		strncpy(cp, dirp->d_name, DIRSIZ);
		if (stat(namebuf, &s) == -1)
			fatal("replacedir botch");
		if (s.st_mode & S_IFREG)
			replacefile(namebuf);
		else if (!sflag && (s.st_mode & S_IFDIR))
			replacedir(namebuf);
	}
	close(fd);
}

/*
 * Replace a file.
 */
void
replacefile(file) char *file;
{
	register MDIR *mdp;
	register int nread;
	DIR *dp;
	FILE *ifp;
	char *cp, *filename;
	int writesize;
	unsigned int next, prev;

	/* Create the file in the appropriate directory. */
	dbprintf(("replacefile(%s)\n", file));
	if ((cp = rindex(file, '/')) == NULL) {
		filename = file;
		dp = root;
	} else {
		filename = cp + 1;
		*cp = '\0';			/* NUL-terminate dirname */
		dp = creatdir(file);		/* find the directory */
		*cp = '/';			/* restore the / */
	}
	mdp = creatfile(filename, dp);		/* create the file */

	/* Read from the COHERENT file and write the MS-DOS file. */
	if (vflag)
		fprintf(stderr, "r %s\n", file);
	if (pflag)
		ifp = stdin;
	else if ((ifp = fopen(file, "r")) == NULL)
		fatal("replace: cannot open \"%s\"", file);
	writesize = clsize * ssize;
	for (prev = 0; ; prev = next) {
		if ((nread = finput(ifp, clbuf, writesize)) == 0)
			break;
		mdp->m_size += nread;
		next = freecluster();
		if (prev == 0)
			mdp->m_cluster = next;
		else
			putcluster(prev, next);
		diskwrite(clbuf, cltosec(next), clsize, file);
		if (nread < writesize)
			break;
	}
	if (nread < 0)
		fatal("replace: read error on file \"%s\"", file);
	if (!pflag)
		fclose(ifp);
	dostime(mdp, file);
}

/*
 * Produce a listing of the MS-DOS file system.
 */
void
table(nargs, args) int nargs; char *args[];
{
	register int n, i;
	register char **ap;
	register MDIR *mdp;
	DIR *dp;
	int free, bad, reserved;
	char buf[12];

	if (vflag) {
		/* Print media information. */
		free = bad = reserved = 0;
		for (n = 2; n <= maxcluster; n++)
			if ((i = getcluster(n)) == 0)
				++free;
			else if (i == CLBAD)
				++bad;
			else if (i > CLMAX && i < CLBAD)
				++reserved;
		if (volume == NULL)
			printf("Unlabelled disk:\n");
		else {
			strncpy(buf, volume->m_name, 11);
			buf[11] = '\0';
			printf("Disk labelled %s:\n", buf);
		}
		printf("\t%ld bytes free\n",
			(long)free*clsize*ssize);
		if (bad)
			printf("\t%ld bytes in bad sectors\n",
				(long)bad*clsize*ssize);
		if (reserved)
			printf("\t%ld bytes in reserved sectors\n",
				(long)reserved*clsize*ssize);
		putchar('\n');
	}
	if (nargs == 0)
		tabledir(root, NULL);
	else for (ap = args; *ap != NULL; ap++) {
		if ((mdp = find(*ap, root, &dp)) == NULL)
			nonfatal("%s not found", *ap);
		else if isdir(mdp)
			tabledir(dp, *ap);
		else
			tablefile(mdp, *ap);
	}
}

/*
 * Print a sorted directory.
 * Suppress '.', '..' and volume label.
 */
void
tabledir(dp, name) register DIR *dp; char *name;
{
	register MDIR *mdp;
	register MDIR **pp, **beg, **end;
	unsigned char c;

	if (name != NULL)
		printf("Directory %s:\n", name);
	if ((beg = pp = (MDIR **)malloc(dp->d_files * sizeof(MDIR *))) == NULL)
		fatal("cannot allocate directory pointer table");
	for (mdp = dp->d_dir; mdp < dp->d_edp; mdp++)
		if ((c = mdp->m_name[0]) != MFREE && c != MEMPTY && c != MMDIR
		 && (mdp->m_attr & MVOLUME) == 0)
			*pp++ = mdp;
	end = pp;
	qsort(beg, end - beg, sizeof(MDIR *), namecmp);
	for (pp = beg; pp < end; pp++)
		tablefile(*pp, NULL);
	free(beg);
	putchar('\n');
}

/*
 * Print a line describing a file.
 */
void
tablefile(mdp, name) register MDIR *mdp; char *name;
{
	register int attr;

	if (vflag) {
		attr = mdp->m_attr;
		putchar((attr & MRONLY ) ? 'r' : '-');
		putchar((attr & MHIDDEN) ? 'h' : '-');
		putchar((attr & MSYSTEM) ? 's' : '-');
		putchar((attr & MVOLUME) ? 'v' : '-');
		putchar((attr & MSUBDIR) ? 'd' : '-');
		putchar((attr & MARCHIV) ? 'a' : '-');
		printf("  %02d/%02d/%02d %02d:%02d %6ld  ",
			mdp->m_mon, mdp->m_day, mdp->m_year+80, mdp->m_hour, mdp->m_min,
			(isdir(mdp)) ? (long)dirclusters(mdp)*clsize*ssize : mdp->m_size);
	}
	if (name == NULL) {
		cohname(mdp->m_name, root);
		name = cohfile;
	}
	printf("%s\n", name);
}

/*
 * Convert NUL-terminated name to UPPERCASE in place.
 */
char *
uppercase(name) unsigned char *name;
{
	register unsigned char *s;
	register int c;

	for (s = name; (c = *s) != '\0'; s++)
		if (islower(c))
			*s = toupper(c);
	return name;
}

/*
 * Write the changed directories of the MS-DOS file system.
 */
void
writedir(dp) register DIR *dp;
{
	register MDIR *mdp;
	register unsigned int n;

	if (dp->d_child != NULL)
		writedir(dp->d_child);
	if (dp->d_sibling != NULL)
		writedir(dp->d_sibling);
	mdp = dp->d_dir;
	dbprintf(("writedir(%s) flag=%u cl=%x blocks=%u mdp=%x\n", dp->d_dname, dp->d_dirflag, dp->d_cluster, dp->d_dirblocks, mdp));
	if (dp->d_dirflag == 0)
		return;				/* DIR was unchanged */
	if (dp->d_cluster == 0)			/* Write the root */
		diskwrite(dp->d_dir, dirbase, dp->d_dirblocks, "directory");
	else					/* Write a subdirectory */
		for (mdp = dp->d_dir, n = dp->d_cluster; n <= CLMAX; n = getcluster(n)) {
			diskwrite(mdp, cltosec(n), clsize, "subdirectory");
			mdp += mdirsize;
		}
}

/* end of dos4.c */
