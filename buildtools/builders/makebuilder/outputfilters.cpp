/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "outputfilters.h"
#include "makeitem.h"
#include <kdebug.h>
#include <QRegExp>
#include <QList>
#include <QApplication>
#include <QStandardItem>
#include <QString>
#include <QPalette>
#include <klocale.h>
#include <kcolorscheme.h>
#include <QFont>
////////////////////////////////////////////////////////

class ErrorFormat
{
public:
    ErrorFormat() {}
    ErrorFormat( const char *, int, int, int );
    ErrorFormat( const char *, int, int, int, QString );
    QRegExp expression;
    int fileGroup;
    int lineGroup;
    int textGroup;
    QString compiler;
};

ErrorFormat::ErrorFormat( const char * regExp, int file, int line, int text )
    : expression( regExp )
    , fileGroup( file )
    , lineGroup( line )
    , textGroup( text )
{}

ErrorFormat::ErrorFormat( const char * regExp, int file, int line, int text, QString comp )
    : expression( regExp )
    , fileGroup( file )
    , lineGroup( line )
    , textGroup( text )
    , compiler( comp )
{}

ErrorFilter::ErrorFilter( )
{
    // @todo could get these from emacs compile.el
        // GCC - another case, eg. for #include "pixmap.xpm" which does not exists
    m_errList << ErrorFormat( "([^: \t]+):([0-9]+):(?:[0-9]+):([^0-9]+)", 1, 2, 3 );
        // GCC
    m_errList << ErrorFormat( "([^: \t]+):([0-9]+):([^0-9]+)", 1, 2, 3 );
        // ICC
    m_errList << ErrorFormat( "([^: \\t]+)\\(([0-9]+)\\):([^0-9]+)", 1, 2, 3, "intel" );
        //libtool link
    m_errList << ErrorFormat( "(libtool):( link):( warning): ", 0, 0, 0 );
        // ld
    m_errList << ErrorFormat( "undefined reference", 0, 0, 0 );
    m_errList << ErrorFormat( "undefined symbol", 0, 0, 0 );
    m_errList << ErrorFormat( "ld: cannot find", 0, 0, 0 );
    m_errList << ErrorFormat( "No such file", 0, 0, 0 );
        // make
    m_errList << ErrorFormat( "No rule to make target", 0, 0, 0 );
        // Fortran
    m_errList << ErrorFormat( "\"(.*)\", line ([0-9]+):(.*)", 1, 2, 3 );
        // Jade
    m_errList << ErrorFormat( "[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)", 1, 2, 3 );
        // ifort
    m_errList << ErrorFormat( "fortcom: Error: (.*), line ([0-9]+):(.*)", 1, 2, 3, "intel" );
        // PGI
    m_errList << ErrorFormat( "PGF9(.*)-(.*)-(.*)-(.*) \\((.*): ([0-9]+)\\)", 5, 6, 4, "pgi" );
        // PGI (2)
    m_errList << ErrorFormat( "PGF9(.*)-(.*)-(.*)-Symbol, (.*) \\((.*)\\)", 5, 5, 4, "pgi" );
}

ErrorFilter::~ErrorFilter()
{}

QStandardItem* ErrorFilter::processAndCreate( const QString& line )
{
    bool hasmatch = false;
    QString file;
    int lineNum = 0;
    QString text;
    QString compiler;
    bool isWarning = false;
    bool isInstantiationInfo = false;

    Q_FOREACH( ErrorFormat format, m_errList )
    {
        QRegExp& regExp = format.expression;

        if ( regExp.indexIn( line ) != -1 )
        {
            hasmatch = true;
            file    = regExp.cap( format.fileGroup );
            lineNum = regExp.cap( format.lineGroup ).toInt() - 1;
            text    = regExp.cap( format.textGroup );
            compiler = format.compiler;
            QString cap = regExp.cap(3);
            if (cap.contains("warning:", Qt::CaseInsensitive) ||
                cap.contains("Warnung:", Qt::CaseInsensitive))
            {
                isWarning = true;
            }
            if (regExp.cap(3).contains("instantiated from", Qt::CaseInsensitive))
            {
                isInstantiationInfo = true;
            }
            break;
        }
    }

    if( hasmatch )
    {
        // Add hacks for error strings you want excluded here
        if( text.indexOf( QString("(Each undeclared identifier is reported only once") ) >= 0
            || text.indexOf( QString("for each function it appears in.)") ) >= 0 )
            hasmatch = false;
    }

    if( hasmatch )
    {
        MakeWarningItem *ret = 0;
        if( isWarning )
        {
            //TODO: Maybe using KColorUtils::mix() is better here, the default NeutralText is a blue-greenish color and doesn't really fit, IMHO
            ret = new MakeWarningItem( line );
            ret->setForeground(KColorScheme().foreground(KColorScheme::NeutralText));
        }
        else // case of real error
        {
            ret = new MakeErrorItem( line );
            ret->setForeground(KColorScheme().foreground(KColorScheme::NegativeText));
        }

        ret->errorText = text;
        ret->lineNo = lineNum;
        ret->file = file;
        return ret;
    }
    else
    {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////////

class ActionFormat
{
public:
    ActionFormat():m_toolGroup(-1) {}
    ActionFormat( const QString&, const QString&, const char * regExp, int file);
    ActionFormat( const QString&, int tool, int file, const char * regExp);
    QString tool();
    QString file();
    const QString& action() const;
    bool matches(const QString& line);
private:
    QString m_action;
    QRegExp m_expression;
    QString m_tool;
    int m_toolGroup;
    int m_fileGroup;
};

ActionFormat::ActionFormat( const QString& _action, const QString& _tool, const char * regExp, int file )
    : m_action( _action )
    , m_expression( regExp )
    , m_tool( _tool )
    , m_toolGroup(-1)
    , m_fileGroup( file )
{
}

ActionFormat::ActionFormat( const QString& _action, int tool, int file, const char * regExp)
    : m_action( _action )
    , m_expression( regExp )
    , m_tool()
    , m_toolGroup(tool)
    , m_fileGroup( file )
{
}

const QString& ActionFormat::action() const
{
    return m_action;
}

QString ActionFormat::tool()
{
    if (m_toolGroup==-1)
        return m_tool;
    return m_expression.cap(m_toolGroup);
}

QString ActionFormat::file()
{
    if( m_fileGroup == -1 )
        return QString();
    return m_expression.cap(m_fileGroup);
}

bool ActionFormat::matches(const QString& line)
{
    return ( m_expression.indexIn( line ) != -1 );
}

MakeActionFilter::MakeActionFilter()
{
//     m_actlist << ActionFormat( i18n("compiling"), 1, 2, "(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+)\\S* (?:\\S* )*-c (?:\\S* )*`[^`]*`(?:[^/\\s;]*/)*([^/\\s;]+)");
//     m_actlist << ActionFormat( i18n("compiling"), 1, 2, "(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+)\\S* (?:\\S* )*-c (?:\\S* )*-o (?:\\S* )(?:[^/;]*/)*([^/\\s;]+)");
//     m_actlist << ActionFormat( i18n("compiling"), 1, 2, "(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+)\\S* (?:\\S* )*-c (?:\\S* )*(?:[^/]*/)*([^/\\s;]*)");
    m_actlist << ActionFormat( i18n("compiling"), 1, 2, "(?:^|[^=])\\b(gcc|CC|cc|distcc|c\\+\\+|g\\+\\+)\\s+.*-c.*[/'\\\\]+(\\w+\\.(?:cpp|CPP|c|C|cxx|CXX|cs|java|hpf|f|F|f90|F90|f95|F95))");
    m_actlist << ActionFormat( i18n("compiling"), 1, 1, "^compiling (.*)" ); //unsermake
    m_actlist << ActionFormat( i18n("compiling"), -1, 1, "\\[.+%\\] Building .* object (.*)" ); //cmake

    m_actlist << ActionFormat( i18n("built"), -1, 1, "\\[.+%\\] Built target (.*)" ); //cmake
    m_actlist << ActionFormat( i18n("generating"), -1, 1, "\\[.+%\\] Generating (.*)" ); //cmake

    //moc and uic
    m_actlist << ActionFormat( i18n("generating"), 1, 2, "/(moc|uic)\\b.*\\s-o\\s([^\\s;]+)");
    m_actlist << ActionFormat( i18n("generating"), 1, 2, "^generating (.*)" ); //unsermake

    m_actlist << ActionFormat( i18n("linking"), "libtool", "/bin/sh\\s.*libtool.*--mode=link\\s.*\\s-o\\s([^\\s;]+)", 1 );
    //can distcc link too ?
    m_actlist << ActionFormat( i18n("linking"), 1, 2, "(gcc|cc|c\\+\\+|g\\+\\+)\\S* (?:\\S* )*-o ([^\\s;]+)");
    m_actlist << ActionFormat( i18n("linking"), 1, 2, "^linking (.*)" ); //unsermaker
    m_actlist << ActionFormat( i18n("linking"), -1, 1, "^Linking .* module (.*)" ); //cmake
    m_actlist << ActionFormat( i18n("linking"), -1, 1, "^Linking (.*)" ); //cmake

    m_actlist << ActionFormat( i18n("creating"), "", "/(?:bin/sh\\s.*mkinstalldirs).*\\s([^\\s;]+)", 1 );
    m_actlist << ActionFormat( i18n("installing"), "", "/(?:usr/bin/install|bin/sh\\s.*mkinstalldirs|bin/sh\\s.*libtool.*--mode=install).*\\s([^\\s;]+)", 1 );
    m_actlist << ActionFormat( i18n("generating"), "dcopidl", "dcopidl .* > ([^\\s;]+)", 1 );
    m_actlist << ActionFormat( i18n("compiling"), "dcopidl2cpp", "dcopidl2cpp (?:\\S* )*([^\\s;]+)", 1 );
    m_actlist << ActionFormat( i18n("installing"), -1, 1, "-- Installing (.*)" ); //cmake
}

MakeActionFilter::~MakeActionFilter()
{}

QStandardItem* MakeActionFilter::processAndCreate( const QString& line )
{
// #ifdef DEBUG
//     QTime t;
//     t.start();
// #endif
    /// \FIXME This is very slow, possibly due to the regexr matching. It can take
    //900-1000ms to execute on an Athlon XP 2000+, while the UI is completely blocked.
    Q_FOREACH( ActionFormat format, m_actlist )
    {
        if( format.matches( line ) )
        {
            QString txt;
            if( format.tool().isEmpty() || format.file().isEmpty() )
            {
                txt = line;
            }
            else
            {
                QString itemString = QString(format.action()).append(" ").append(format.file());
                itemString.append(" (").append(format.tool()).append(")");

                txt = itemString;
            }

            QStandardItem *actionItem = new QStandardItem( txt );
            QFont newfont( actionItem->font() );
            newfont.setBold( true );
            actionItem->setFont( newfont );
            kDebug( 9038 ) << "Found: " << format.action() << " " << format.file() << "(" << format.tool() << ")" << endl;
            return actionItem;
        }
// #ifdef DEBUG
//         if ( t.elapsed() > 100 )
//             kDebug(9038) << "MakeActionFilter::processLine: SLOW regexp matching: " << t.elapsed() << " ms \n";
// #endif
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////

CustomFilter::CustomFilter()
{
}

CustomFilter::~CustomFilter()
{}

QStandardItem* CustomFilter::processAndCreate( const QString& /*line*/ )
{
    return 0;
}

