/*-------------------------------------------------------------------------
 *
 * pg_putenv_proxy.c
 *	  pg_putenv_proxy for putenv
 *
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/port/pg_putenv_proxy.c
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"

int
pg_putenv_proxy(char *envval)
{
#ifdef WIN32
	return pgwin32_putenv(envval);
#else
	return putenv(envval);
#endif
}