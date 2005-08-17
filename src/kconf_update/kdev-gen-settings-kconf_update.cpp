/*
    kconf_update app for migrating kdevelop's ui settings to the new
    code that will be in 3.3.

    Copyright (c) 2005 by Matt Rogers <mattr@kde.org>
    Based on code Copyright (c) 2003 by Martijn Klingens <klingens@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qmap.h>
#include <qtextstream.h>
#include <qregexp.h>

static QTextStream qcin ( stdin,  QIODevice::ReadOnly );
static QTextStream qcout( stdout, QIODevice::WriteOnly );
static QTextStream qcerr( stderr, QIODevice::WriteOnly );

// Group cache. Yes, I know global vars are ugly :)
bool needFlush = false;
QString newKeyValue;
int newDataValue;

void parseKey( const QString &group, const QString &key,
               const QString &value, const QString &rawLine )
{

    //qcerr << "*** group='" << group << "'" << endl;
    if ( group == "General Options" && key == "Embed KDevDesigner")
    {
        newKeyValue = "Designer App";
        if ( value.lower() == "true" )
            newDataValue = 0;
        else
            newDataValue = 2;
        qcout << newKeyValue << "=" << newDataValue << endl;
        qcout << "# DELETE [" << key << "]" << endl;

    }
    else if ( group == "General Options" && key == "Application Font" )
    {
        newKeyValue = "OutputViewFont";
        qcout << newKeyValue << "=" << value << endl;
        qcout << "# DELETE [" << key << "]" << endl;
    }
    else if ( group == "MakeOutputView" && key == "Messages Font" )
    {
        qcout << "# DELETE [" << key << "]" << endl;
    }
    else if ( group == "TerminalEmulator" && key == "UseKDESetting" )
    {
            newKeyValue = "UseKDESetting";
            if ( value.lower() == "true" )
                newDataValue = 0;
            else
                newDataValue = 1;
            qcout << newKeyValue << "=" << newDataValue << endl;
    }
    else
    {
        // keys we don't convert. output the raw line instead.
        qcout << rawLine << endl;
    }
}

int main()
{
    qcin.setEncoding( QTextStream::UnicodeUTF8 );
    qcout.setEncoding( QTextStream::UnicodeUTF8 );

    QString curGroup;

    QRegExp groupRegExp( "^\\[(.*)\\]" );
    QRegExp keyRegExp( "^([a-zA-Z0-9:, _-]*)\\s*=\\s*(.*)\\s*" );
    QRegExp commentRegExp( "^(#.*)?$" );

    while ( !qcin.atEnd() )
    {
        QString line = qcin.readLine();

        if ( commentRegExp.exactMatch( line ) )
        {
            // We found a comment, leave unchanged
            qcout << line << endl;
        }
        else if ( groupRegExp.exactMatch( line ) )
        {
            curGroup = groupRegExp.capturedTexts()[ 1 ];
            qcout << line << endl;
        }
        else if ( keyRegExp.exactMatch( line ) )
        {
            // We found the a key line
            parseKey( curGroup, keyRegExp.capturedTexts()[ 1 ], keyRegExp.capturedTexts()[ 2 ], line );
        }
        else
        {
            qcout << line << endl;
        }
    }

    return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

