/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2007 Aleix Pol Gonzalez <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CMAKELISTSPARSER_H
#define CMAKELISTSPARSER_H

#include <QMetaType>
#include <QString>
#include <QVector>

#include "cmListFileLexer.h"
#include <cmakecommonexport.h>
#include <language/editor/rangeinrevision.h>

#include <QStringList>

struct CMakeFunctionArgument
{
    CMakeFunctionArgument() {}
    explicit CMakeFunctionArgument(const QString& v);
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
    bool quoted = false;
    quint32 line = 0;
    quint32 column = 0;
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
    QVector<CMakeFunctionArgument> arguments;
    QString filePath;
    quint32 line = 0;
    quint32 column = 0;
    quint32 endLine = 0;
    quint32 endColumn = 0;

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

using CMakeFileContent = QVector<CMakeFunctionDesc>;

namespace CMakeListsParser
{
    KDEVCMAKECOMMON_EXPORT CMakeFileContent readCMakeFile(const QString& fileName);
}

#endif
