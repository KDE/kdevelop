/*
 * file     : dbg.h
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@gmx.net
 * license  : gpl version >= 2
 *
 */

/**
 * just defining some macros for generating normal and debug output
 */
#ifndef _DBG_H
#define _DBG_H

#include <iostream.h>
using namespace std;

#ifdef ENABLEDEBUGOUTPUT
#define errmsg( x ) cerr << x
#define errln( x ) cerr << x << endl
#else
#define errmsg( x )
#define errln( x )
#endif

#ifdef ENABLEMESSAGEOUTPUT
#define outmsg( x ) cout << x
#define outln( x ) cout << x << endl
#else
#define outmsg( x )
#define outln( x )
#endif

#endif
