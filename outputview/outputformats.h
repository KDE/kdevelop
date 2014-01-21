/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>               *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KDEVPLATFORM_OUTPUTFORMATS_H
#define KDEVPLATFORM_OUTPUTFORMATS_H

#include <QString>
#include <QRegExp>

namespace KDevelop
{

struct ActionFormat
{
    ActionFormat() = default;
    ActionFormat( const char* context, const char* _action, const QString& _tool, const QString& regExp, int file );
    ActionFormat( const char* context, const char* _action, int tool, int file, const QString& regExp );
    const char* context; // translation context for action
    const char* action;  // translated string
    QRegExp expression;
    QString tool;
    int toolGroup;
    int fileGroup;
};

struct ErrorFormat
{
    ErrorFormat() = default;
    ErrorFormat( const QString& regExp, int file, int line, int text, int column=-1 );
    ErrorFormat( const QString& regExp, int file, int line, int text, const QString& comp, int column=-1 );
    QRegExp expression;
    int fileGroup;
    int lineGroup, columnGroup;
    int textGroup;
    QString compiler;
};

}
#endif



