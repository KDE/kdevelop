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

#include "makeitemfactory.h"
#include "makeitem.h"
#include <QRegExp>
#include <QApplication>
#include <QPalette>

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

//////////////////////////////////////////////////////////////////////////

MakeItemFactory::MakeItemFactory( const MakeBuilder *builder )
    : IOutputViewItemFactory()
    , m_builder(builder)
{}

MakeItemFactory::~MakeItemFactory()
{}

// ported from KDev3.4's outputview
IOutputViewItem* MakeItemFactory::createItem( const QString& line )
{
    QList<ErrorFormat> list;
    // @todo could get these from emacs compile.el
        // GCC - another case, eg. for #include "pixmap.xpm" which does not exists
    list << ErrorFormat( "([^: \t]+):([0-9]+):(?:[0-9]+):([^0-9]+)", 1, 2, 3 );
        // GCC
    list << ErrorFormat( "([^: \t]+):([0-9]+):([^0-9]+)", 1, 2, 3 );
        // ICC
    list << ErrorFormat( "([^: \\t]+)\\(([0-9]+)\\):([^0-9]+)", 1, 2, 3, "intel" );
        //libtool link
    list << ErrorFormat( "(libtool):( link):( warning): ", 0, 0, 0 );
        // ld
    list << ErrorFormat( "undefined reference", 0, 0, 0 );
    list << ErrorFormat( "undefined symbol", 0, 0, 0 );
    list << ErrorFormat( "ld: cannot find", 0, 0, 0 );
    list << ErrorFormat( "No such file", 0, 0, 0 );
        // make
    list << ErrorFormat( "No rule to make target", 0, 0, 0 );
        // Fortran
    list << ErrorFormat( "\"(.*)\", line ([0-9]+):(.*)", 1, 2, 3 );
        // Jade
    list << ErrorFormat( "[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:(.*)", 1, 2, 3 );
        // ifort
    list << ErrorFormat( "fortcom: Error: (.*), line ([0-9]+):(.*)", 1, 2, 3, "intel" );
        // PGI
    list << ErrorFormat( "PGF9(.*)-(.*)-(.*)-(.*) \\((.*): ([0-9]+)\\)", 5, 6, 4, "pgi" );
        // PGI (2)
    list << ErrorFormat( "PGF9(.*)-(.*)-(.*)-Symbol, (.*) \\((.*)\\)", 5, 5, 4, "pgi" );

    bool hasmatch = false;
    QString file;
    int lineNum = 0;
    QString text;
    QString compiler;
    bool isWarning = false;
    bool isInstantiationInfo = false;

    Q_FOREACH( ErrorFormat format, list )
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
            ret = new MakeWarningItem( line, m_builder );
            ret->setForeground(QApplication::palette().highlight());
        }
        else // case of real error
        {
            ret = new MakeErrorItem( line, m_builder );
            ret->setForeground(QApplication::palette().linkVisited());
        }
        ret->file = file;
        ret->lineNo = lineNum;
        ret->errorText = text;
        return ret;
    }
    else
    {
        IOutputViewItem *ret =  new IOutputViewItem( line );
        return ret;
    }
}


