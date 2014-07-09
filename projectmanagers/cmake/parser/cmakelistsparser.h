/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2007 Aleix Pol Gonzalez <aleixpol@gmail.com>
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

// #include "cmakemodelitems.h"
#include "cmListFileLexer.h"
#include <cmakeexport.h>
#include <language/editor/rangeinrevision.h>

struct CMakeFunctionArgument
{
    CMakeFunctionArgument(): value(), quoted(false), line(0), column(0) {}
    CMakeFunctionArgument(const QString& v);
    CMakeFunctionArgument(const QString& v, bool q, quint32 l = 0, quint32 c=0);
    inline bool operator == (const CMakeFunctionArgument& r) const
    {
        return (this->value == r.value) && (this->quoted == r.quoted);
    }

    inline bool operator != (const CMakeFunctionArgument& r) const
    {
        return (this->value != r.value) || (this->quoted != r.quoted);
    }

    bool operator==( const QString& rhs )
    {
        return ( this->value == rhs );
    }

    bool isCorrect() const { return column>0; }

    static QString unescapeValue(const QString& value);
    
    KDevelop::RangeInRevision range() const
    { return KDevelop::RangeInRevision(line-1, column-1, line-1, column+value.length()-1); }

    bool isValid() const { return quoted || !value.isEmpty(); }
    
    QString value;
    bool quoted;
    quint32 line;
    quint32 column;
    static const QMap<QChar, QChar> scapings;
};
Q_DECLARE_METATYPE( CMakeFunctionArgument )


class KDEVCMAKECOMMON_EXPORT CMakeFunctionDesc
{
public:
    CMakeFunctionDesc();
    
    ///useful when writing unit tests mostly
    CMakeFunctionDesc(const QString& name, const QStringList& args);

    QString name;
    QList<CMakeFunctionArgument> arguments;
    QString filePath;
    quint32 line;
    quint32 column;
    quint32 endLine;
    quint32 endColumn;
/*  int numSpacesBeforeLeftParen;
    int numSpacesAfterLeftParen;
    int numSpacesBeforeRightParen; */

    KDevelop::RangeInRevision nameRange() const
    { return KDevelop::RangeInRevision(line-1, column-1, line-1, column-1+name.length()); }

    KDevelop::RangeInRevision range() const
    { return KDevelop::RangeInRevision(line-1, column-1, endLine-1, endColumn); }
    
    KDevelop::RangeInRevision argRange() const
    { 
        if( !arguments.isEmpty() ) 
        { 
            return KDevelop::RangeInRevision(arguments.first().range().start,
                                   arguments.last().range().end); 
        } else 
        { 
            return KDevelop::RangeInRevision( line-1, column-1, endLine-1, endColumn); 
        }
    }

    bool operator==(const CMakeFunctionDesc &other) const;
    void addArguments( const QStringList&, bool addEvenIfEmpty=true );
    QString writeBack() const;
};
Q_DECLARE_METATYPE( CMakeFunctionDesc )

/**
 * CMake files function descriptor collector
 * @author Matt Rogers <mattr@kde.org>
 * @author Aleix Pol <aleixpol@gmail.com>
 */

typedef QList<CMakeFunctionDesc> CMakeFileContent;

class KDEVCMAKECOMMON_EXPORT CMakeListsParser : public QObject
{
public:
    CMakeListsParser(QObject *parent = 0) : QObject(parent) {}
    ~CMakeListsParser() {}
    
    static CMakeFileContent readCMakeFile(const QString& fileName);
    
private:
    static bool readCMakeFunction( cmListFileLexer* lexer, CMakeFunctionDesc& func);

};

#endif
