/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdatetime.h>
#include <kdebug.h>

#include "makeactionfilter.h"
#include "makeactionfilter.moc"
#include "makeitem.h"

#include <klocale.h>
#include <kdebug.h>

MakeActionFilter::ActionFormat::ActionFormat( const QString& _action, const QString& _tool, const char * regExp, int file )
	: action( _action )
	, tool( _tool )
	, expression( regExp )
	, fileGroup( file )
{
}

MakeActionFilter::MakeActionFilter( OutputFilter& next )
	: OutputFilter( next )
{
#ifdef DEBUG
	test();
#endif
}

QValueList<MakeActionFilter::ActionFormat>& MakeActionFilter::actionFormats()
{
	static QValueList<ActionFormat> formats
		= QValueList<ActionFormat>()

	<< ActionFormat( i18n("compiling"), "g++", "/bin/sh\\s.*libtool.*--mode=compile.*`.*`(?:[^/\\s;]*/)*([^/\\s;]+)", 1 )
	<< ActionFormat( i18n("compiling"), "g++", "g\\+\\+ (?:\\S* )*-c (?:\\S* )*([^\\s;]*)", 1 )
	<< ActionFormat( i18n("generating"), "moc", ".*/moc\\b.*\\s-o\\s([^\\s;]+)", 1 )
	<< ActionFormat( i18n("generating"), "uic", ".*/uic\\b.*\\s-o\\s([^\\s;]+)", 1 )
	<< ActionFormat( i18n("linking"), "libtool", "/bin/sh\\s.*libtool.*--mode=link .* -o ([^\\s;]+)", 1 )
	<< ActionFormat( i18n("linking"), "g++", "g\\+\\+ (?:\\S* )*-o ([^\\s;]+)", 1 )
	<< ActionFormat( i18n("installing"), "", "^\\s*(?:/usr/bin/install|/bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)", 1 );

	return formats;
}

void MakeActionFilter::processLine( const QString& line )
{
	ActionItem* actionItem = matchLine( line );
	if ( actionItem != NULL )
	{
		emit item( actionItem );
	}
	else
	{
		OutputFilter::processLine( line );
	}
}

ActionItem* MakeActionFilter::matchLine( const QString& line )
{
	QTime t;
	t.start();
	//FIXME: This is very slow, possibly due to the regexr matching. It can take
	//900-1000ms to execute on an Athlon XP 2000+, while the UI is completely blocked.
	QValueList<ActionFormat>::iterator it = actionFormats().begin();
	for( ; it != actionFormats().end(); ++it )
	{
		QRegExp& regExp = (*it).expression;
		if ( regExp.search( line ) == -1 )
			continue;
		if ( t.elapsed() > 100 )
			kdDebug(9004) << "MakeActionFilter::processLine: SLOW regexp matching: " << t.elapsed() << " ms \n";
		return new ActionItem( (*it).action, regExp.cap( (*it).fileGroup ), (*it).tool, line );
	}
	return NULL;
}

struct TestItem
{
	TestItem() {}
	TestItem( const QString& _line, const QString& _action, const QString& _tool, const QString& _file )
		: line( _line )
		, action( _action )
		, tool( _tool )
		, file( _file )
	{}
	QString line;
	QString action;
	QString tool;
	QString file;
};

#ifdef DEBUG

void MakeActionFilter::test()
{
	static QValueList<TestItem> testItems = QValueList<TestItem>()

	<< TestItem( // simple qmake compile
		"g++ -c -pipe -Wall -W -O2 -DQT_NO_DEBUG -I/home/john/src/kde/qt-copy/mkspecs/default -I. "
		"-I/home/john/src/kde/qt-copy/include -o test.o test.cpp",
		"compiling", "g++", "test.cpp" )
	<< TestItem( // simple qmake link
		"g++ -o ../bin/test test.o -Wl,-rpath,/home/john/src/kde/qt-copy/lib -L/home/john/src/kde/qt-copy/lib "
		"-L/usr/X11R6/lib -lqt-mt -lXext -lX11 -lm",
		"linking", "g++", "../bin/test" )
	<< TestItem( // automake 1.7, srcdir != builddir
		"if /bin/sh ../../libtool --silent --mode=compile --tag=CXX g++ -DHAVE_CONFIG_H -I. "
		"-I/home/john/src/kde/kdevelop/lib/interfaces -I../.. -I/usr/local/kde/include -I/home/john/src/kde/qt-copy/include "
		"-I/usr/X11R6/include -DQT_THREAD_SUPPORT -D_REENTRANT -Wnon-virtual-dtor -Wno-long-long -Wundef -Wall -pedantic -W "
		"-Wpointer-arith -Wmissing-prototypes -Wwrite-strings -ansi -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wcast-align -Wconversion "
		"-Wchar-subscripts -fno-builtin -g3 -Wformat-security -Wmissing-format-attribute -fno-exceptions -fno-check-new "
		"-fno-common -MT kdevcore.lo -MD -MP -MF \".deps/kdevcore.Tpo\" -c -o kdevcore.lo `test -f "
		"'/home/john/src/kde/kdevelop/lib/interfaces/kdevcore.cpp' || echo "
		"'/home/john/src/kde/kdevelop/lib/interfaces/'`/home/john/src/kde/kdevelop/lib/interfaces/kdevcore.cpp; then mv "
		"\".deps/kdevcore.Tpo\" \".deps/kdevcore.Plo\"; else rm -f \".deps/kdevcore.Tpo\"; exit 1; fi",
		"compiling", "g++", "kdevcore.cpp" )
	<< TestItem(
		"source='makewidget.cpp' object='makewidget.lo' libtool=yes depfile='.deps/makewidget.Plo' "
		"tmpdepfile='.deps/makewidget.TPlo' depmode=gcc3 /bin/sh ../../admin/depcomp /bin/sh ../../libtool --mode=compile "
		"--tag=CXX g++ -DHAVE_CONFIG_H -I. -I. -I../.. -I../../lib/interfaces -I../../lib/util -I/opt/kde3/include "
		"-I/usr/lib/qt3/include -I/usr/X11R6/include -DQT_THREAD_SUPPORT -D_REENTRANT -Wnon-virtual-dtor -Wno-long-long -Wundef "
		"-Wall -pedantic -W -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -ansi -D_XOPEN_SOURCE=500 -D_BSD_SOURCE "
		"-Wcast-align -Wconversion -fno-builtin -g -O2 -g3 -O0 -fno-exceptions -fno-check-new -c -o makewidget.lo `test -f "
		"'makewidget.cpp' || echo './'`makewidget.cpp",
		"compiling", "g++", "makewidget.cpp" )
	<< TestItem( // automake 1.7, srcdir != builddir
		"/bin/sh ../../libtool --silent --mode=link --tag=CXX g++ -Wnon-virtual-dtor -Wno-long-long -Wundef -Wall -pedantic "
		"-W -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -ansi -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wcast-align "
		"-Wconversion -Wchar-subscripts -fno-builtin -g3 -Wformat-security -Wmissing-format-attribute -fno-exceptions "
		"-fno-check-new -fno-common -o libkdevoutputviews.la.closure libkdevoutputviews_la_closure.lo -L/usr/X11R6/lib "
		"-L/home/john/src/kde/qt-copy/lib -L/usr/local/kde/lib -avoid-version -module -no-undefined -R /usr/local/kde/lib "
		"-R /home/john/src/kde/qt-copy/lib -R /usr/X11R6/lib outputviewsfactory.lo makeviewpart.lo makewidget.lo "
		"appoutputviewpart.lo appoutputwidget.lo directorystatusmessagefilter.lo outputfilter.lo compileerrorfilter.lo "
		"commandcontinuationfilter.lo makeitem.lo makeactionfilter.lo otherfilter.lo ../../lib/libkdevelop.la",
		"linking", "libtool", "libkdevoutputviews.la.closure" )
	;

	QValueList<TestItem>::const_iterator it = testItems.begin();
	for( ; it != testItems.end(); ++it )
	{
		ActionItem* actionItem = matchLine( (*it).line );
		if ( actionItem == NULL )
		{
			kdError( 9004 ) << "MakeActionFilter::test(): match failed (no match):" << endl;
			kdError( 9004 ) << (*it).line << endl;
		}
		else if ( actionItem->m_action != (*it).action )
		{
			kdError( 9004 ) << "MakeActionFilter::test(): match failed (expected action "
			                << (*it).action << ", got " << actionItem->m_action << endl;
			kdError( 9004 ) << (*it).line << endl;
		}
		else if ( actionItem->m_tool != (*it).tool )
		{
			kdError( 9004 ) << "MakeActionFilter::test(): match failed (expected tool "
			                << (*it).tool << ", got " << actionItem->m_tool << endl;
			kdError( 9004 ) << (*it).line << endl;
		}
		else if ( actionItem->m_file != (*it).file )
		{
			kdError( 9004 ) << "MakeActionFilter::test(): match failed (expected file "
			                << (*it).file << ", got " << actionItem->m_file << endl;
			kdError( 9004 ) << (*it).line << endl;
		}
		if ( actionItem != NULL )
			delete actionItem;
	}

}

#endif
