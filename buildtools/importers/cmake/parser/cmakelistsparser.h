/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CMAKELISTSPARSER_H
#define CMAKELISTSPARSER_H

#include <QList>
#include <QObject>
#include <QMetaType>
#include "cmListFileLexer.h"
#include <kdevexport.h>

class CMakeAst;
struct CMakeFunctionArgument;

struct CMakeFunctionDesc
{
    QString name;
    QList<CMakeFunctionArgument> arguments;
    QString filePath;
    quint32 line;
/*  int numSpacesBeforeLeftParen;
    int numSpacesAfterLeftParen;
    int numSpacesBeforeRightParen; */

    void addArguments( const QStringList& );
};
Q_DECLARE_METATYPE( CMakeFunctionDesc )

struct CMakeFunctionArgument
{
    CMakeFunctionArgument(): value(), quoted(false), filePath(0), line(0) {}
    CMakeFunctionArgument(const CMakeFunctionArgument& r):
        value(r.value), quoted(r.quoted), filePath(r.filePath), line(r.line) {}
    CMakeFunctionArgument(const QString& v, bool q = false,
                          const QString& file = QString(), quint32 l = 0)
        : value(v), quoted(q), filePath(file), line(l) {}
    bool operator == (const CMakeFunctionArgument& r) const
    {
        return (this->value == r.value) && (this->quoted == r.quoted);
    }

    bool operator==( const QString& rhs )
    {
        return ( this->value == rhs );
    }

    bool operator != (const CMakeFunctionArgument& r) const
    {
        return !(*this == r);
    }

    QString value;
    bool quoted;
    QString filePath;
    quint32 line;
};
Q_DECLARE_METATYPE( CMakeFunctionArgument )

/**
 * Recursive descent parser for CMakeLists.txt files
 * @author Matt Rogers <mattr@kde.org>
 */
class KDEVCMAKECOMMON_EXPORT CMakeListsParser : public QObject
{
public:
    CMakeListsParser(QObject *parent = 0);
    ~CMakeListsParser();

    static bool parseCMakeFile( CMakeAst* root, const QString& fileName );

private:
    static bool parseCMakeFunction( cmListFileLexer*,
                                    CMakeFunctionDesc&,
                                    const QString fileName );
};

#endif
