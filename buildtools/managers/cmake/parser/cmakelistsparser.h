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
#include <QStringList>
#include <QString>

#include <KUrl>

#include "cmListFileLexer.h"
#include "cmakeexport.h"
// #include "cmakemodelitems.h"

class CMakeAst;
struct CMakeFunctionArgument;

struct KDEVCMAKECOMMON_EXPORT CMakeFunctionDesc
{
    QString name;
    QList<CMakeFunctionArgument> arguments;
    QString filePath;
    quint32 line;
/*  int numSpacesBeforeLeftParen;
    int numSpacesAfterLeftParen;
    int numSpacesBeforeRightParen; */

    bool operator==(const CMakeFunctionDesc &other) const;
    void addArguments( const QStringList& );
    QString writeBack() const;
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
 * CMake files function descriptor collector
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */

typedef QList<CMakeFunctionDesc> CMakeFileContent;

class KDEVCMAKECOMMON_EXPORT CMakeListsParser : public QObject
{
public:
    CMakeListsParser(QObject *parent = 0);
    ~CMakeListsParser();
    
    static CMakeFileContent readCMakeFile(const QString& fileName);
    
private:
    static bool readCMakeFunction( cmListFileLexer*,
                                    CMakeFunctionDesc&,
                                    const QString& fileName);
//     static bool parseCMakeFunction( cmListFileLexer*,
//                                     CMakeFunctionDesc&,
//                                     const QString& fileName, CMakeAst* parent);
// // public:
//     static bool parseCMakeFile( CMakeAst* root, const QString& fileName );

};

#endif
