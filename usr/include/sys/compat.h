#ifndef	__SYS_COMPAT_H__
#define	__SYS_COMPAT_H__

/*
 * This file provides glue to export the system-private compatibility
 * definitions from <sys/ccompat.h> with names that it is valid for user-level
 * code to use.
 */

#include <sys/ccompat.h>

#ifdef	__USE_PROTO__
# define	USE_PROTO	__USE_PROTO__
#endif

# define	PROTO(p)	__PROTO (p)
# define	CONST		__CONST__
# define	VOLATILE	__VOLATILE__
# define	VOID		__VOID__
# define	NOTUSED(name)	__NOTUSED (name)
# define	REGISTER	__REGISTER__
# define	EXTERN_C	__EXTERN_C__
# define	EXTERN_C_BEGIN	__EXTERN_C_BEGIN__
# define	EXTERN_C_END	__EXTERN_C_END__
# define	NON_ISO(k)	__NON_ISO(k)


#ifdef	__NO_INLINE__
# define	NO_INLINE
#endif

# ifdef	__NO_INLINEL__
#  define	NO_INLINEL
#endif

# define	INLINE		__INLINE__
# define	INLINEL		__INLINEL__

# define	ANY_ARGS	__ANY_ARGS__

# define	ARGS(x)		__ARGS(x)

# define	LOCAL		static

#endif	/* ! defined (__SYS_COMPAT_H__) */
