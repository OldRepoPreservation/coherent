/*
 * Screen driver functions connected with getLoc().
 */
#include <scn.h>

#define CTRL(c) (c - '@')
WINDOW *errWindow = NULL;

/*
 * Get a field given it's loc pointer. if (sw) display the
 * default and return (0); Return the last character entered.
 */
static
getLoc(f, sw) register loc *f; int sw;
{
	int	got,	/* number of chars processed */
		c,	/* current char */
		y, x,	/* cursor loc */
		i,	/* pos in Default field */
		state,	/* 1 == ESC sequence in process */
		len;	/* field length */

	move(y = f->row, x = f->col);

	len = f->len ? f->len : f->skipf;
	/* copy Default into field and zero fill field */
	for (got = i = 0; got < len; got++) {
		if (c = f->Default[i]) {
			i++;
			addch(c);
		}
		else
			addch(' ');
		f->field[got] = c;
	}
	f->field[got] = '\0';
	refresh();

	if (sw || !f->len)
		return (0);

	if (NULL != errWindow) {
		wmove(errWindow, 1, 0);
		wclrtoeol(errWindow);
		if (NULL != f->help) {
			wmove(errWindow, 1, 0);
			waddstr(errWindow, f->help);
		}
		wrefresh(errWindow);
	}
	move(y, x);

	for (state = got = 0; got < f->len; ) {
		refresh();
		c = getChr();
		/*
 		 * Up Arrow key is esc [ A
		 * on some systems and
		 * esc A on others.
		 */
		if (state) {	/* got an escape */
			switch (c) {
			case '[':
				continue;
			case 'A':
				c = CTRL('P');
				break;
			case 'B':
				c = CTRL('N');
				break;
			case 'C':
				c = CTRL('F');
				break;
			case 'D':
				c = CTRL('B');
				break;
			}
			state = 0;
		}
		switch (c) {
		case CTRL('['):
			state = 1;
			continue;
		case '\r':	/* close out line */
		case '\n':
			f->field[got] = '\0';
			for (; got < f->len; got++)
				addch(' ');
			continue;
		case '\b':	/* backspace */
			if (!got)
				continue;
			got--;
			move(y, --x);
		case CTRL('D'):
		case 127:	/* delete key */
			if (!f->field[got])
				continue;
			for (i = got; c = f->field[i] = f->field[i + 1]; i++)
				addch(c);
			addch(' ');

			move(y, x);
			continue;
		case CTRL('B'):		/* back one char */
			if (!got)
				continue;
			move(y, --x);
			got--;
			continue;
		case '\t':		/* go to next field with verify */
		case CTRL('N'):		/* go to next field */
		case CTRL('P'):		/* go to previous field */
		case CTRL('Z'):		/* end of screen */
			refresh();
			return (c);
		case CTRL('C'):		/* Give user a chance to kill */
			userCtlc();
			continue;
		case CTRL('A'):		/* beginning of line */
			move(y = f->row, x = f->col);
			got = 0;
			continue;
		case CTRL('E'):		/* end of line */
			while (f->field[got]) {
				got++;
				x++;
			}
			while (got >= f->len) { /* don't trigger end of field */
				got--;
				x--;
			}
			move(y, x);
			continue;
		case CTRL('F'):		/* forward one char */
			if (!(c = f->field[got]))
				continue;
		default:
			addch(c);
			x++;
			f->field[got++] = c;
		}
	}
	f->field[got] = '\0';
	refresh();
	return (c);
}

/*
 * Show all default items.
 */
void
showDefs(data, fields) backGrnd *data; loc *fields;
{
	register loc *f;

	showBak(data);

	for (f = fields; NULL != f->field; f++)
		getLoc(f, 1);
}
/*
 * Print background and get all fields.
 */
void
scnDriv(data, fields) backGrnd *data; loc *fields;
{
	clear();
	showBak(data);
	getAll(fields);
}

/*
 * Get all fields on a given screen.
 * allow emacs style navagation.
 */
void
getAll(fields) loc *fields;
{
	register loc *f;
	char c;

	for (c = 0; c != CTRL('Z'); ) {	
		for (f = fields; (c != CTRL('Z')) && (NULL != f->field); f++) {
			switch (c = getLoc(f, 0)) {
			case CTRL('P'):
				for (;;) {
					if (f == fields) /* wrap */
						while (NULL != f->field)
							f++;
					if ((--f)->len) {
						f--;
						break;
					}
				}
			case CTRL('Z'):
			case CTRL('N'):
				break;
			default:
				if (NULL != f->verify) {
					switch ((*f->verify)(f->field)) {
					case -1:
						if (f->skipf) {
							f += f->skipf;
							break;
						}
					case 0:
						f--;
						break;
					}
				}
			}
		}
	}

	/* verify all fields */
	for (f = fields; NULL != f->field; f++) {
		if (NULL != f->verify) {
			switch ((*f->verify)(f->field)) {
			case -1:
				if (f->skipf) {
					f += f->skipf;
					break;
				}
			case 0:
				getLoc(f, 0);
				f--;
				break;
			}
		}
	}
}

/*
 * Get a field by field location.
 */
getField(table, field) loc  *table; char *field;
{
	register loc *f;

	for (f = table; NULL != f->field; f++)
		if (field == f->field)
			return (getLoc(f, 0));
	fatal("Invalid use of getField");
}

/*
 * Put a field by field location.
 */
void
putField(table, field) loc  *table; char *field;
{
	register loc *f;

	for (f = table; NULL != f->field; f++)
		if (field == f->field) {
			getLoc(f, 1);
			return;
		}
	fatal("Invalid use of putField");
}
