set -e
ENTRY=`basename $1`
(							\
	echo ".globl ${ENTRY}_";			\
	echo "${ENTRY}_: mov ax,\$K${ENTRY}_";		\
	echo .byte 0x9A;				\
	echo .word xcalled;				\
	echo .word 0x0060;				\
	echo ret;					\
) > /tmp/$$.s
as -gxo /tmp/$$.o /tmp/$$.s
mv /tmp/$$.o $1.o
rm -f /tmp/$$.s
