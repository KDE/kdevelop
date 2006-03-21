/***************************************************************************
 *   Copyright (C) 2004-2005 by Sascha Cunz                                *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevlicense.h"

#include <QFile>
#include <QDateTime>
#include <QRegExp>
#include <QTextStream>

KDevLicense::KDevLicense( const QString& name, const QString& fileName )
    : m_name( name ), m_KAboutDataEnum( "Custom" )
{
    readFile( fileName );
}

void KDevLicense::readFile( const QString& fileName )
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly))
        return;
    QTextStream stream(&f);
    QString str;
    enum { readingText, readingKAboutDataEnum, readingFiles } mode = readingText;
    for(;;)
    {
        str = stream.readLine();
        if( str.isNull() )
            break;
        QString strlow = str.lower();
        if( strlow == "[files]" )
            mode = readingFiles;
        else if( strlow == "[kaboutdatalicense]" )
            mode = readingKAboutDataEnum;
        else if( strlow == "[prefix]" )
            mode = readingText;
        else if( mode == readingFiles )
        {
            if( !str.isEmpty() )
            {
                m_copyFiles.append( str );
            }
        } else if( mode == readingKAboutDataEnum )
        {
            m_KAboutDataEnum = str;
        } else
        {
            m_rawLines.append( str );
        }
    }
}

QString KDevLicense::assemble( KDevFile::CommentingStyle commentingStyle, const QString& /*author*/, const QString& /*email*/, int leadingSpaces )
{
    // first, build a CPP Style license

    QString strFill;
    strFill.fill( ' ', leadingSpaces );

    QString str =
        strFill + "/***************************************************************************\n" +
        strFill + " *   Copyright (C) $YEAR$ by $AUTHOR$   *\n" +
        strFill + " *   $EMAIL$   *\n" +
        strFill + " *                                                                         *\n";

//  str = str.arg(QDate::currentDate().year()).arg(author.left(45),-45).arg(email.left(67),-67);

    foreach( QString current, m_rawLines )
        str += QString( "%1 *   %2 *\n").arg( strFill ).arg( current, -69 );

    str += strFill + " ***************************************************************************/\n";

    switch( commentingStyle )
    {
        case KDevFile::CPPStyle:
            return str;

        case KDevFile::PascalStyle:
            str.replace(QRegExp("/\\**\n \\*"), "{\n  ");
            str.replace(QRegExp("\\*\n \\*"), " \n  ");
            str.replace(QRegExp(" *\\**/\n"), "}\n");
            return str;

        case KDevFile::AdaStyle:
            str.replace(QRegExp("/\\*"), "--");
            str.replace(QRegExp(" \\*"), "--");
            str.replace(QRegExp("\\*/"), "*");
            return str;

        case KDevFile::BashStyle:
            str.replace(QRegExp("\\*|/"), "#");
            str.replace(QRegExp("\n ##"), "\n##");
            str.replace(QRegExp("\n #"), "\n# ");
            return str;

        default:
            break;
    }

    return "currently unknown/unsupported commenting style";
}

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
