/*
 * Rec'd from Lauren Weinstein, 7-16-84.
 * Substitute user-id temporarily
 * or become super user (as you wish).
 *
 * Compile -s -n -i
 */

#include <stdio.h>
#include <pwd.h>

#define ACCNAME "remacc"	/* Remote access password dummy username */
#define PASSLEN 13		/* Encrypted password length */

char	*getpass();
char	*getenv();
short	gid;
char	*password;		/* Set by getuname */
char	salt[3];
char	shell[] = "/bin/sh";
char	*shargs[] = {
	"su",
	NULL
};

char	prs[50] = "PS1=";
char	*prompt = prs;

main(argc, argv)
char *argv[];
{
	register int uid;
	register int count = 0;	
	char *command, *prompta, *promptb, *passp;
	char **args;

	if ((uid = getuname(argc>1 ? argv[1] : "0")) < 0) /* check username */
	   bye();  /* yep */
	if (password[0] != '\0' && getuid())   /* check password if not su */
	{  passp = getpass("Password: ");  /* get input password choice */
	   while (count++ < 2)
	   {  if (count > 0)
		 if (getuname("0") < 0)  /* check root password too */ 
		    bye();  /* failure */
	      if ((strlen(password) != PASSLEN) ||
	            (strcmp(crypt(passp, salt), password))) 
	      {  if (count > 0)  /* if we've tried both passwords */
	            bye();  /* failure */
	      }	
	      else break;  /* password ok */
	   }
	}

	if (argc > 2) {
		command = argv[2];
		args = &argv[2];
	} else {
		command = shell;
		args = shargs;
	}
	setgid(gid);
	setuid(uid);
	prompta = getenv("PSN");  /* check for normal prompt */
	promptb = getenv("PSS");  /* check for desired su prompt */
	addenviron(uid == 0 ? (promptb ? promptb : "# ") :
	   (prompta ? prompta : "$ "));  /* change prompt as appropriate */
	execvp(command, args);
	printf("%s: not found\n", command);
}

/*
 * Get a user-name from a string.
 * If the string starts with a numeric use 
 * directly as a number.
 * The string `password' is set with
 * the user's password for checking later.
 * Returns uid.
 */
getuname(s)
register char *s;
{
	register struct passwd *pwp;
	register short uid;

	if (*s>='0' && *s<='9') {
		uid = atoi(s);
		if ((pwp = getpwuid(uid)) == NULL) {
			fprintf(stderr, "%d: bad user number\n", uid);
			exit(1);
		}
	} else if ((pwp = getpwnam(s)) == NULL) {
		fprintf(stderr, "%s: not a user name\n", s);
		exit(1);
	}
	if (!strcmp(pwp->pw_name, ACCNAME))  /* dummy access username? */
	   return(-1);  /* yes */
	password = pwp->pw_passwd;
	salt[0] = pwp->pw_passwd[0];
	salt[1] = pwp->pw_passwd[1];
	salt[2] = '\0';
	gid = pwp->pw_gid;
	return (pwp->pw_uid);
}

/*
 * Add string `s' to the environment as "PS1".
 */
addenviron(s)
char *s;
{
	extern char **environ;
	register char **epp1, **epp2;
	register char **newenv;
	int n;
	char *malloc();

	for (epp1 = environ; *epp1!=NULL; epp1++)
		;
	n = (epp1-environ+1) * sizeof (char *);
	if ((newenv = (char **)malloc(n)) == NULL) {
		fprintf(stderr, "Out of memory for environments\n");
		exit(1);
	}
	strcat(prompt, s);
	for (epp1=environ, epp2=newenv; *epp1 != NULL; epp1++)
		if (strncmp(*epp1, "PS1=", 4) != 0)
			*epp2++ = *epp1;
		else {
			*epp2++ = prompt;
			prompt = NULL;
		}
	*epp2++ = prompt;
	*epp2 = NULL;
	environ = newenv;
}

bye()
{	fprintf(stderr, "Sorry\n");
	exit(1);
}

