/*
 * fnkey.c
 * 3/4/87
 * Usage: fnkey [ -n ] [ keyname newvalue ] ...
 * Reassign special function keys under MS-DOS.
 * Assumes device ANSI.SYS installed.
 * References: DOS Technical Reference, pp. 2-11, 2-12; DOS BASIC, p. G-6.
 * This could easily recognize e.g. "\n" and "^Z" but currently does not.
 * It could also remap other keys but currently does not.
 */

#include <stdio.h>
#define	ESCAPE	0x1B			/* ASCII ESCape */

/* Key names and representations as ASCII digit sequences. */
struct key {
	char	*key_name;
	char	*key_value;
} keys[] = {
	{ "F1",		"59"	},
	{ "F2",		"60"	},
	{ "F3",		"61"	},
	{ "F4",		"62"	},
	{ "F5",		"63"	},
	{ "F6",		"64"	},
	{ "F7",		"65"	},
	{ "F8",		"66"	},
	{ "F9",		"67"	},
	{ "F10",	"68"	},
	{ "HOME",	"71"	},
	{ "UP",		"72"	},
	{ "PGUP",	"73"	},
	{ "LEFT",	"75"	},
	{ "RIGHT",	"77"	},
	{ "END",	"79"	},
	{ "DOWN",	"80"	},
	{ "PGDN",	"81"	},
	{ "DEL",	"83"	},
	{ "CPRTSC",	"114"	},
	{ "CLEFT",	"115"	},
	{ "CRIGHT",	"116"	},
	{ "CEND",	"117"	},
	{ "CHOME",	"119"	}
};
#define	NKEYS	(sizeof(keys) / sizeof(struct key))

main(argc, argv) int argc; char *argv[];
{
	register struct key *kp;
	register char *name, *value;
	char *nflag;

	if (argc > 1 && strcmp(*++argv, "-n") == 0) {
		nflag = "";
		++argv;
		--argc;
	}
	else
		nflag = ";13";
	if (argc % 2 == 0) {
		fprintf(stderr, "Usage: fnkey [ -v ] [ keyname value ] ...\n");
		exit(1);
	}
	while ((name = *argv++) != NULL && (value = *argv++) != NULL) {
		for (kp = &keys[0]; kp < &keys[NKEYS]; kp++)
			if (strcmp(name, kp->key_name) == 0) {
				printf("%c[0;%s;\"%s\"%sp",
					ESCAPE, kp->key_value, value, nflag);
				break;
			}
		if (kp == &keys[NKEYS]) {
			fprintf(stderr, "fnkey: unrecognized key name \"%s\"\n", name);
			exit(1);
		}
	}
	exit(0);
}

/* end of fnkey.c */
