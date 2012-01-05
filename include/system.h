/*	SCCS Id: @(#)system.h	3.4	2001/12/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

/*
 * When using GNU autoconf to configure, the switches are output to
 * autoconf.h which should override the defaults set earlier in
 * config.h and *conf.h
 */
#include "autoconf.h"
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_DECL_STRNICMP && !HAVE_DECL_STRNCMPI
#define strncmpi(a,b,c) strnicmp(a,b,c)
#endif

#include <sys/types.h>

/* You may want to change this to fit your system, as this is almost
 * impossible to get right automatically.
 * This is the type of signal handling functions.
 */
#include <signal.h>
#define SIG_RET_TYPE RETSIGTYPE (*)()

#include <string.h>

#endif /* SYSTEM_H */
