/*	SCCS Id: @(#)tradstdc.h 3.4	1993/05/30	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRADSTDC_H
#define TRADSTDC_H

#define NHSTDC

#ifdef NEED_VARARGS		/* only define these if necessary */
#include <stdarg.h>
# define VA_DECL(typ1,var1)	(typ1 var1, ...) { va_list the_args;
# define VA_DECL2(typ1,var1,typ2,var2)	\
	(typ1 var1, typ2 var2, ...) { va_list the_args;
# define VA_INIT(var1,typ1)
# define VA_NEXT(var1,typ1)	var1 = va_arg(the_args, typ1)
# define VA_ARGS		the_args
# define VA_START(x)		va_start(the_args, x)
# define VA_END()		va_end(the_args)
#endif /* NEED_VARARGS */

/*
 * Used for robust ANSI parameter forward declarations:
 * int VDECL(sprintf, (char *, const char *, ...));
 *
 * NDECL() is used for functions with zero arguments;
 * FDECL() is used for functions with a fixed number of arguments;
 * VDECL() is used for functions with a variable number of arguments.
 * Separate macros are needed because ANSI will mix old-style declarations
 * with prototypes, except in the case of varargs, and the OVERLAY-specific
 * trampoli.* mechanism conflicts with the ANSI <<f(void)>> syntax.
 */

# define NDECL(f)	f(void)	/* overridden later if USE_TRAMPOLI set */

# define FDECL(f,p)	f p

# define VDECL(f,p)	f p

/* generic pointer, always a macro; genericptr_t is usually a typedef */
# define genericptr	void *

#ifndef genericptr_t
typedef genericptr genericptr_t;	/* (void *) or (char *) */
#endif

/*
 * Allow gcc2 to check parameters of printf-like calls with -Wformat;
 * append this to a prototype declaration (see pline() in extern.h).
 */
#ifdef __GNUC__
# if __GNUC__ >= 2
#define PRINTF_F(f,v) __attribute__ ((format (printf, f, v)))
# endif
#endif
#ifndef PRINTF_F
#define PRINTF_F(f,v)
#endif

#endif /* TRADSTDC_H */
