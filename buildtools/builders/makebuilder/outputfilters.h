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
#ifndef OUTPUTFILTERS_H
#define OUTPUTFILTERS_H

#include <QString>
#include <QRegExp>

class ActionFormat
{
    public:
        static QList<ActionFormat> actionFormats;
        ActionFormat( const QString&, const QString&, const QString& regExp, int file);
        ActionFormat( const QString&, int tool, int file, const QString& regExp);
        QString action;
        QRegExp expression;
        QString tool;
        int toolGroup;
        int fileGroup;
};

class ErrorFormat
{
    public:
        static QList<ErrorFormat> errorFormats;
        ErrorFormat( const QString&, int, int, int );
        ErrorFormat( const QString&, int, int, int, const QString& );
        QRegExp expression;
        int fileGroup;
        int lineGroup;
        int textGroup;
        QString compiler;
};

#endif
