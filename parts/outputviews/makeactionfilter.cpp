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

//#define DEBUG

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

//NOTE: whenever you discover a case that is not correctly recognized by the filter, please add it as a test
//item and be sure that your modifications don't break the already existing test cases.

// returns an array of ActionFormat
MakeActionFilter::ActionFormat* MakeActionFilter::actionFormats()
{
	static ActionFormat formats[] = {
		ActionFormat( i18n("compiling"), "g++", "g\\+\\+\\S* (?:\\S* )*-c (?:\\S* )*`[^`]*`(?:[^/\\s;]*/)*([^/\\s;]+)", 1 ),
		ActionFormat( i18n("compiling"), "g++", "g\\+\\+\\S* (?:\\S* )*-c (?:\\S* )*-o (?:\\S* )(?:[^/;]*/)*([^/\\s;]+)", 1 ),
		ActionFormat( i18n("compiling"), "gcc", "g\\c\\c\\S* (?:\\S* )*-c (?:\\S* )*`[^`]*`(?:[^/\\s;]*/)*([^/\\s;]+)", 1 ),
		ActionFormat( i18n("compiling"), "gcc", "g\\c\\c\\S* (?:\\S* )*-c (?:\\S* )*(?:[^/]*/)*([^/\\s;]*)", 1 ),
		ActionFormat( i18n("compiling"), "distcc", "distcc (?:\\S* )*-c (?:\\S* )*`[^`]*`(?:[^/\\s;]*/)*([^/\\s;]+)", 1 ),
		ActionFormat( i18n("compiling"), "distcc", "distcc (?:\\S* )*-c (?:\\S* )*(?:[^/]*/)*([^/\\s;]*)", 1 ),
		ActionFormat( i18n("compiling"), "unknown", "^compiling (.*)", 1 ),
		ActionFormat( i18n("generating"), "moc", "/moc\\b.*\\s-o\\s([^\\s;]+)", 1 ),
		ActionFormat( i18n("generating"), "uic", "/uic\\b.*\\s-o\\s([^\\s;]+)", 1 ),
		ActionFormat( i18n("linking"), "libtool", "/bin/sh\\s.*libtool.*--mode=link\\s.*\\s-o\\s([^\\s;]+)", 1 ),
		ActionFormat( i18n("linking"), "g++", "g\\+\\+\\S* (?:\\S* )*-o ([^\\s;]+)", 1 ),
		ActionFormat( i18n("linking"), "gcc", "g\\c\\c\\S* (?:\\S* )*-o ([^\\s;]+)", 1 ),
		ActionFormat( i18n("creating"), "", "/(?:bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)", 1 ),
		ActionFormat( i18n("installing"), "", "/(?:usr/bin/install|bin/sh\\s.*mkinstalldirs|bin/sh\\s.*libtool.*--mode=install).*\\s([^\\s;]+)", 1 ),
		ActionFormat( i18n("generating"), "dcopidl", "dcopidl .* > ([^\\s;]+)", 1 ),
		ActionFormat( i18n("compiling"), "dcopidl2cpp", "dcopidl2cpp (?:\\S* )*([^\\s;]+)", 1 ),

		ActionFormat( QString::null, QString::null, 0, 0 )
	};

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
#ifdef DEBUG
	QTime t;
	t.start();
#endif
	/// \FIXME This is very slow, possibly due to the regexr matching. It can take
	//900-1000ms to execute on an Athlon XP 2000+, while the UI is completely blocked.
	int i = 0;
	ActionFormat* aFormats = actionFormats();
	ActionFormat* format = &aFormats[i];

	while ( !format->action.isNull() )
	{
//		kdDebug(9004) << "Testing filter: " << format->action << ": " << format->tool << endl;
		QRegExp& regExp = format->expression;
		if ( regExp.search( line ) != -1 )
		{
                   ActionItem *actionItem = new ActionItem( format->action, regExp.cap( format->fileGroup ), format->tool, line );
	     	   kdDebug( 9004 ) << "Found: " << actionItem->m_action << " " << actionItem->m_file << "(" << actionItem->m_tool << ")" << endl;
		   return actionItem;
		}
#ifdef DEBUG
		if ( t.elapsed() > 100 )
			kdDebug(9004) << "MakeActionFilter::processLine: SLOW regexp matching: " << t.elapsed() << " ms \n";
#endif
		format = &aFormats[++i];
	}
	return 0;
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
	<< TestItem( // automake 1.7, srcdir != builddir
		"if g++ -DHAVE_CONFIG_H -I. -I/home/john/src/kde/kdevelop/src -I.. -I/home/john/src/kde/kdevelop/lib/interfaces "
		"-I/home/john/src/kde/kdevelop/lib/sourceinfo -I/home/john/src/kde/kdevelop/lib/util -I/home/john/src/kde/kdevelop/lib "
		"-I/home/john/src/kde/kdevelop/lib/qextmdi/include -I/home/john/src/kde/kdevelop/lib/structure -I/usr/local/kde/include "
		"-I/home/john/src/kde/qt-copy/include -I/usr/X11R6/include -DQT_THREAD_SUPPORT -D_REENTRANT -Wnon-virtual-dtor "
		"-Wno-long-long -Wundef -Wall -pedantic -W -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -ansi "
		"-D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wcast-align -Wconversion -Wchar-subscripts -fno-builtin -g3 -Wformat-security "
		"-Wmissing-format-attribute -fno-exceptions -fno-check-new -fno-common -MT mainwindowideal.o -MD -MP -MF "
		"\".deps/mainwindowideal.Tpo\" -c -o mainwindowideal.o `test -f '/home/john/src/kde/kdevelop/src/mainwindowideal.cpp' "
		"|| echo '/home/john/src/kde/kdevelop/src/'`/home/john/src/kde/kdevelop/src/mainwindowideal.cpp; then mv "
		"\".deps/mainwindowideal.Tpo\" \".deps/mainwindowideal.Po\"; else rm -f \".deps/mainwindowideal.Tpo\"; exit 1; fi",
		"compiling", "g++", "mainwindowideal.cpp" )
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
	<< TestItem( //libtool, linking 2
		"/bin/sh ../libtool --silent --mode=link --tag=CXX g++  -Wnon-virtual-dtor -Wno-long-long -Wundef -Wall -pedantic "
		"-W -Wpointer-arith -Wwrite-strings -ansi -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wcast-align -Wconversion -Wchar-subscripts "
		"-fno-builtin -g3 -fno-exceptions -fno-check-new -fno-common    -o libkfilereplacepart.la.closure libkfilereplacepart_la_closure.lo "
		"-module -no-undefined  -L/usr/X11R6/lib -L/usr/lib/qt3/lib -L/opt/kde3/lib  -version-info 1:0:0 kfilereplacepart.lo kfilereplacedoc.lo "
		"kfilereplaceview.lo kaboutkfilereplace.lo kaddstringdlg.lo kconfirmdlg.lo kernel.lo kexpression.lo kfilereplacepref.lo "
		"klistviewstring.lo knewprojectdlg.lo koptionsdlg.lo kresultview.lo filelib.lo knewprojectdlgs.lo -lkio -lkparts -lkhtml",
		"linking", "libtool", "libkfilereplacepart.la.closure")
	<< TestItem( //libtool, linking 3
		"/bin/sh ../libtool --silent --mode=link --tag=CXX g++  -Wnon-virtual-dtor -Wno-long-long -Wundef -Wall -pedantic "
		"-W -Wpointer-arith -Wwrite-strings -ansi -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -Wcast-align -Wconversion -Wchar-subscripts "
		"-fno-builtin -g3 -fno-exceptions -fno-check-new -fno-common    -o libkfilereplacepart.la -rpath /opt/kde3/lib/kde3 "
		"-module -no-undefined  -L/usr/X11R6/lib -L/usr/lib/qt3/lib -L/opt/kde3/lib  -version-info 1:0:0 kfilereplacepart.lo "
		"kfilereplacedoc.lo kfilereplaceview.lo kaboutkfilereplace.lo kaddstringdlg.lo kconfirmdlg.lo kernel.lo kexpression.lo "
		"kfilereplacepref.lo klistviewstring.lo knewprojectdlg.lo koptionsdlg.lo kresultview.lo filelib.lo knewprojectdlgs.lo -lkio -lkparts -lkhtml",
		"linking", "libtool", "libkfilereplacepart.la")
	<< TestItem( //automake, builddir!=srcdir, libtool=no, compiling
		" g++ -DHAVE_CONFIG_H -I. -I/home/andris/cvs-developement/head/quanta/quanta/project "
		"-I../.. -I/home/andris/cvs-developement/head/quanta/quanta/dialogs -I/opt/kde3/include -I/usr/lib/qt3/include -I/usr/X11R6/include  "
		"-I../../quanta/dialogs  -DQT_THREAD_SUPPORT  -D_REENTRANT -DKOMMANDER -DDESIGNER -DQT_NO_SQL -DHAVE_KDE  -Wnon-virtual-dtor "
		"-Wno-long-long -Wundef -Wall -pedantic -W -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -ansi -D_XOPEN_SOURCE=500 -D_BSD_SOURCE "
		"-Wcast-align -Wconversion -Wchar-subscripts -fno-builtin -g3 -DKDE_NO_COMPAT -fno-exceptions -fno-check-new -fno-common  -c -o project.o "
		"`test -f '/home/andris/cvs-developement/head/quanta/quanta/project/project.cpp' || "
		"echo '/home/andris/cvs-developement/head/quanta/quanta/project/'`/home/andris/cvs-developement/head/quanta/quanta/project/project.cpp	",
		"compiling", "g++", "project.cpp")
	<< TestItem(
		"/usr/local/kde/bin/dcopidl /home/john/src/kde/kdevelop/lib/interfaces/KDevAppFrontendIface.h > KDevAppFrontendIface.kidl "
		"|| ( rm -f KDevAppFrontendIface.kidl ; /bin/false )",
		"generating", "dcopidl", "KDevAppFrontendIface.kidl" )
	<< TestItem(
		"/usr/local/kde/bin/dcopidl2cpp --c++-suffix cpp --no-signals --no-stub KDevAppFrontendIface.kidl",
		"compiling", "dcopidl2cpp", "KDevAppFrontendIface.kidl" )
	<< TestItem( //install
		"/usr/bin/install -c -p -m 644 /home/andris/development/quanta/quanta/kommander/editor/kmdr-editor.desktop "
		"/opt/kde3/share/applnk/Editors/kmdr-editor.desktop", "installing", "", "/opt/kde3/share/applnk/Editors/kmdr-editor.desktop")
	<< TestItem( //libtool install
		"/bin/sh ../../libtool --silent --mode=install /usr/bin/install -c -p libkommanderwidgets.la "
		"/opt/kde3/lib/libkommanderwidgets.la", "installing", "", "/opt/kde3/lib/libkommanderwidgets.la")
	<< TestItem( //libtool, automake 1.8
	"if g++ -DHAVE_CONFIG_H -I. -I/home/andris/  "
	"-DBUILD_KAFKAPART  -MT quanta_init.o -MD -MP -MF \".deps/quanta_init.Tpo\" -c -o quanta_init.o "
	"quanta_init.cpp; "
	"then mv -f \".deps/quanta_init.Tpo\" \".deps/quanta_init.Po\"; else rm -f \".deps/quanta_init.Tpo\"; "
	"exit 1; fi",
	"compiling", "g++", "quanta_init.cpp")
	<< TestItem( //libtool, automake 1.8, file with full path
	"if g++ -DHAVE_CONFIG_H -I. -I/home/andris/  "
	"-DBUILD_KAFKAPART  -MT quanta_init.o -MD -MP -MF \".deps/quanta_init.Tpo\" -c -o quanta_init.o "
	"/home/andris/quanta_init.cpp; "
	"then mv -f \".deps/quanta_init.Tpo\" \".deps/quanta_init.Po\"; else rm -f \".deps/quanta_init.Tpo\"; "
	"exit 1; fi",
	"compiling", "g++", "quanta_init.cpp")
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
		} else
		kdDebug( 9004 ) << "Test passed, " << actionItem->m_action << " " << actionItem->m_file << " (" << actionItem->m_tool << ") found." << endl;
		if ( actionItem != NULL )
			delete actionItem;
	}

}

#endif
