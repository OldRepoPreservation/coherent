#*********************************************************************
#                         COPYRIGHT NOTICE                           *
#*********************************************************************
#        This software is copyright (C) 1982 by Pavel Curtis         *
#                                                                    *
#        Permission is granted to reproduce and distribute           *
#        this file by any means so long as no fee is charged         *
#        above a nominal handling fee and so long as this            *
#        notice is always included in the copies.                    *
#                                                                    *
#        Other rights are reserved except as explicitly granted      *
#        by written permission of the author.                        *
#                Pavel Curtis                                        *
#                Computer Science Dept.                              *
#                405 Upson Hall                                      *
#                Cornell University                                  *
#                Ithaca, NY 14853                                    *
#                                                                    *
#                Ph- (607) 256-4934                                  *
#                                                                    *
#                Pavel.Cornell@Udel-Relay   (ARPAnet)                *
#                decvax!cornell!pavel       (UUCPnet)                *
#********************************************************************/

#
# $Header:   RCS/MKterm.h.v  Revision 2.1  82/10/25  14:45:11  pavel  Exp$
#

BEGIN		{
		    print "/*"
		    print "**	term.h -- Definition of struct term"
		    print "*/"
		    print ""
		    print "#ifndef SGTTY"
		    print "#    include \"curses.h\""
		    print "#endif"
		    print ""
		    print "#ifdef SINGLE"
		    print "#	define CUR _first_term."
		    print "#else"
		    print "#	define CUR cur_term->"
		    print "#endif"
		    print ""
		    print ""
		}


$3 == "bool"	{
		    printf "#define %-30s CUR Booleans[%d]\n", $1, BoolCount++
		}

$3 == "number"	{
		    printf "#define %-30s CUR Numbers[%d]\n", $1, NumberCount++
		}

$3 == "str"	{
		    printf "#define %-30s CUR Strings[%d]\n", $1, StringCount++
		}


END		{
			print  ""
			print  ""
			print  "struct term"
			print  "{"
			print  "   char	 *term_names;	/* offset in str_table of terminal names */"
			print  "   char	 *str_table;	/* pointer to string table */"
			print  "   short Filedes;	/* file description being written to */"
			print  "   SGTTY Ottyb,		/* original state of the terminal */"
			print  "	 Nttyb;		/* current state of the terminal */"
			print  ""
			printf "   char		 Booleans[%d];\n", BoolCount
			printf "   short	 Numbers[%d];\n", NumberCount
			printf "   char		 *Strings[%d];\n", StringCount
			print  "};"
			print  ""
			print  "struct term	_first_term;"
			print  "struct term	*cur_term;"
			print  ""
			printf "#define BOOLCOUNT %d\n", BoolCount
			printf "#define NUMCOUNT  %d\n", NumberCount
			printf "#define STRCOUNT  %d\n", StringCount
		}
