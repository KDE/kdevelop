/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2012 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_OUTPUTFORMATS_H
#define KDEVPLATFORM_OUTPUTFORMATS_H

#include <QString>
#include <QRegularExpression>

namespace KDevelop
{

struct ActionFormat
{
    ActionFormat() = default;
    ActionFormat( const QString& _tool, const QString& regExp, int file );
    ActionFormat( int file, const QString& regExp );
    QRegularExpression expression;
    QString tool;
    int fileGroup;
};

struct ErrorFormat
{
    ErrorFormat() = default;
    ErrorFormat( const QString& regExp, int file, int line, int text, int column=-1 );
    ErrorFormat( const QString& regExp, int file, int line, int text, const QString& comp, int column=-1 );
    QRegularExpression expression;
    int fileGroup;
    int lineGroup, columnGroup;
    int textGroup;
    QString compiler;

    // Returns the column number starting with 0 as the first column
    // If no match was found for columns or if index was not valid
    // (i.e. less than zero) - 0 is returned.
    int columnNumber(const QRegularExpressionMatch& match) const;
};

}
#endif



