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
#  $Header:   RCS/MKnames.v  Revision 2.1  82/10/25  14:45:06  pavel  Exp$
#

BEGIN		{
			print  "/*" > "boolnames"
			print  " *	dump_names.c - Arrays of capability names for use by the" > "boolnames"
			print  " *			dump program." > "boolnames"
			print  " *" > "boolnames"
			print  " */" > "boolnames"
			print  "" > "boolnames"
			print  "" > "boolnames"
			print  "char	*BoolNames[] = {" > "boolnames"
			print  "char	*NumNames[] = {" > "numnames"
			print  "char	*StrNames[] = {" > "strnames"
		}

$3 == "bool"	{
			printf "\t\t%s,\n", $2 > "boolnames"
		}

$3 == "number"	{
			printf "\t\t%s,\n", $2 > "numnames"
		}

$3 == "str"	{
			printf "\t\t%s,\n", $2 > "strnames"
		}

END		{
			print  "};" > "boolnames"
			print  "" > "boolnames"
			print  "};" > "numnames"
			print  "" > "numnames"
			print  "};" > "strnames"
		}
