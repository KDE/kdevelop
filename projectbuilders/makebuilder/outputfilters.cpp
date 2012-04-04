/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>               *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "outputfilters.h"

#include <klocale.h>

// @todo could get these from emacs compile.el
QList<ErrorFormat> ErrorFormat::errorFormats = QList<ErrorFormat>()
    // GCC - another case, eg. for #include "pixmap.xpm" which does not exists
    << ErrorFormat( "^([^:\t]+):([0-9]+):([0-9]+):([^0-9]+)", 1, 2, 4, 3 )
    // GCC
    << ErrorFormat( "^([^:\t]+):([0-9]+):([^0-9]+)", 1, 2, 3 )
    // ICC
    << ErrorFormat( "^([^: \\t]+)\\(([0-9]+)\\):([^0-9]+)", 1, 2, 3, "intel" )
    //libtool link
    << ErrorFormat( "^(libtool):( link):( warning): ", 0, 0, 0 )
    // ld
    << ErrorFormat( "undefined reference", 0, 0, 0 )
    << ErrorFormat( "undefined symbol", 0, 0, 0 )
    << ErrorFormat( "ld: cannot find", 0, 0, 0 )
    << ErrorFormat( "No such file", 0, 0, 0 )
    // make
    << ErrorFormat( "No rule to make target", 0, 0, 0 )
    // Fortran
    << ErrorFormat( "\"(.*)\", line ([0-9]+):(.*)", 1, 2, 3 )
    // Jade
    << ErrorFormat( "^[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)", 1, 2, 3 )
    // ifort
    << ErrorFormat( "^fortcom: Error: (.*), line ([0-9]+):(.*)", 1, 2, 3, "intel" )
    // PGI
    << ErrorFormat( "PGF9(.*)-(.*)-(.*)-(.*) \\((.*): ([0-9]+)\\)", 5, 6, 4, "pgi" )
    // PGI (2)
    << ErrorFormat( "PGF9(.*)-(.*)-(.*)-Symbol, (.*) \\((.*)\\)", 5, 5, 4, "pgi" );

QList<ActionFormat> ActionFormat::actionFormats = QList<ActionFormat>()
    //     << ActionFormat( i18n("compiling"), 1, 2, "(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+)\\S* (?:\\S* )*-c (?:\\S* )*`[^`]*`(?:[^/\\s;]*/)*([^/\\s;]+)");
    //     << ActionFormat( i18n("compiling"), 1, 2, "(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+)\\S* (?:\\S* )*-c (?:\\S* )*-o (?:\\S* )(?:[^/;]*/)*([^/\\s;]+)");
    //     << ActionFormat( i18n("compiling"), 1, 2, "(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+)\\S* (?:\\S* )*-c (?:\\S* )*(?:[^/]*/)*([^/\\s;]*)");
    << ActionFormat( i18n("compiling"), 1, 2, "(?:^|[^=])\\b(gcc|CC|cc|distcc|c\\+\\+|"
                     "g\\+\\+|icc|icpc)\\s+.*-c.*[/ '\\\\]+(\\w+\\.(?:cpp|CPP|c|C|cxx|CXX|cs|"
                     "java|hpf|f|F|f90|F90|f95|F95))")
    //moc and uic
    << ActionFormat( i18n("generating"), 1, 2, "/(moc|uic)\\b.*\\s-o\\s([^\\s;]+)")
    //libtool linking
    << ActionFormat( i18nc("Linking object files into a library or executable", "linking"),
                     "libtool", "/bin/sh\\s.*libtool.*--mode=link\\s.*\\s-o\\s([^\\s;]+)", 1 )
    //unsermake
    << ActionFormat( i18n("compiling"), 1, 1, "^compiling (.*)" )
    << ActionFormat( i18n("generating"), 1, 2, "^generating (.*)" )
    << ActionFormat( i18nc("Linking object files into a library or executable",
                     "linking"), 1, 2, "(gcc|cc|c\\+\\+|g\\+\\+|icc|icpc)\\S* (?:\\S* )*-o ([^\\s;]+)")
    << ActionFormat( i18nc("Linking object files into a library or executable",
                     "linking"), 1, 2, "^linking (.*)" )
    //cmake
    << ActionFormat( i18n("built"), -1, 1, "\\[.+%\\] Built target (.*)" )
    << ActionFormat( i18n("compiling"), "cmake", "\\[.+%\\] Building .* object (.*)/CMakeFiles/", 1 )
    << ActionFormat( i18n("generating"), -1, 1, "\\[.+%\\] Generating (.*)" )
    << ActionFormat( i18nc("Linking object files into a library or executable",
                     "linking"), -1, 1, "^Linking (.*)" )
    << ActionFormat( i18n("installing"), -1, 1, "-- Installing (.*)" )
    //libtool install
    << ActionFormat( i18n("creating"), "", "/(?:bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)",
 1 )
    << ActionFormat( i18n("installing"), "", "/(?:usr/bin/install|bin/sh\\s.*mkinstalldirs"
                     "|bin/sh\\s.*libtool.*--mode=install).*\\s([^\\s;]+)", 1 )
    //dcop
    << ActionFormat( i18n("generating"), "dcopidl", "dcopidl .* > ([^\\s;]+)", 1 )
    << ActionFormat( i18n("compiling"), "dcopidl2cpp", "dcopidl2cpp (?:\\S* )*([^\\s;]+)",
 1 )
    // match against Entering directory to update current build dir
    << ActionFormat( "cd", "", "make[^:]*: Entering directory `([^']+)'", 1);

ErrorFormat::ErrorFormat( const QString& regExp, int file, int line, int text, int column )
    : expression( regExp )
    , fileGroup( file )
    , lineGroup( line )
    , columnGroup( column )
    , textGroup( text )
{}

ErrorFormat::ErrorFormat( const QString& regExp, int file, int line, int text, const QString& comp, int column )
    : expression( regExp )
    , fileGroup( file )
    , lineGroup( line )
    , columnGroup( column )
    , textGroup( text )
    , compiler( comp )
{}

ActionFormat::ActionFormat( const QString& _action, const QString& _tool, const QString& regExp, int file )
    : action( _action )
    , expression( regExp )
    , tool( _tool )
    , toolGroup(-1)
    , fileGroup( file )
{
}

ActionFormat::ActionFormat( const QString& _action, int tool, int file, const QString& regExp)
    : action( _action )
    , expression( regExp )
    , toolGroup( tool )
    , fileGroup( file )
{
}

