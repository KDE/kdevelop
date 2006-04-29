/* -*- C++ -*-

   This file declares debugging aids for multithreaded applications.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, 2005 Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Trolltech, Norway. $

   $Id: DebuggingAids.h 30 2005-08-16 16:16:04Z mirko $
*/
#ifndef DEBUGGINGAIDS_H
#define DEBUGGINGAIDS_H

extern "C"
{
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
}

#include <QMutex>
#include <QString>

namespace ThreadWeaver {

    extern bool Debug;
    extern int DebugLevel;
    extern QMutex GlobalMutex;

    /** Set the debug log level.
	@see debug
    */
    extern inline void setDebugLevel (bool debug, int level);

    /** This method prints a text message on the screen, if debugging is
	enabled. Otherwise, it does nothing. The message is thread safe,
	therefore providing that the messages appear in the order they where
	issued by the different threads.
	All messages are suppressed when Debug is false. All messages with a
	lower importance (higher number) than DebugLevel will be suppressed,
	too. Debug level 0 messages will always be printed as long as
	Debug is true.
        We use our own debugging method, since debugging threads is a more
        complicated experience than debugging single threaded
        contexts. This might change in future in the way that debug
        prints it's messages to another logging facility provided by
        the platform.
	Use setDebugLevel () to integrate adapt debug () to your platform.
    */
    inline void debug(int severity, const char * cformat, ...)
#ifdef __GNUC__
	__attribute__ ( (format (printf, 2, 3 ) ) )
#endif
;

    /** Prints the message to the console if condition is true. */
    inline void debug(bool condition, int severity, const char * cformat, ...)
#ifdef __GNUC__
	__attribute__ ( (format (printf, 3, 4 ) ) )
#endif
;


/** PROTECT executes x with GlobalMutex locked.
    Mostly used for debugging, as in P_ASSERT. */
#ifdef PROTECT
#undef PROTECT
#endif
#define PROTECT(x) do { QMutexLocker l(&ThreadWeaver::GlobalMutex); (x); } while (0)

/** P_ASSERT ensures that error messages occur in the correct order. */
#ifdef P_ASSERT
#undef P_ASSERT
#endif

#define P_ASSERT(x) do { QMutexLocker l(&ThreadWeaver::GlobalMutex); Q_ASSERT(x); } while (0)

    inline void setDebugLevel (bool debug, int level)
    {
	Debug = debug;
	DebugLevel = level;
    }

    inline void debug(int severity, const char * cformat, ...)
    {
	if ( Debug == true && ( severity<=DebugLevel || severity == 0) )
	{
	    QString text;

	    va_list ap;
	    va_start( ap, cformat );
	    PROTECT (vprintf (cformat, ap));
	    va_end (ap);
	}
    }

    inline void debug(bool condition, int severity, const char *cformat, ...)
    {
	if (condition && Debug == true && ( severity<=DebugLevel || severity == 0) )
	{
	    QString text;

	    va_list ap;
	    va_start( ap, cformat );
	    PROTECT (vprintf (cformat, ap));
	    va_end (ap);
	}
    }

}

#endif // DEBUGGINGAIDS_H
