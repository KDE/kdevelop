/*
   Copyright 2009 Aleix Pol Gonzalez <aleixpol@kde.org>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CMAKETYPES_H
#define CMAKETYPES_H

#include "cmakelistsparser.h"
#include "variablemap.h"

#include <language/duchain/indexeddeclaration.h>

struct Macro
{
    QString name;
    QStringList knownArgs;
    CMakeFileContent code;
    bool isFunction;
};

struct CacheEntry
{
    CacheEntry(const QString& value=QString(), const QString &doc=QString()) : value(value), doc(doc) {}
    QString value;
    QString doc;
};

struct Target
{
    typedef QMap<QString, QString> Properties;
    enum Type { Library, Executable, Custom };
    KDevelop::IndexedDeclaration declaration;
    QStringList files;
    Type type;
    CMakeFunctionDesc desc;
    QString name;
    QStringList includes;
};
typedef Target CMakeTarget; //Workaround for namespacing. fix me!

struct Subdirectory
{
    QString name;
    CMakeFunctionDesc desc;
    QString build_dir;
};

struct Test
{
    Test() : isTarget(false) {}
    QString name;
    QString executable;
    QStringList arguments;
    QStringList files;
    bool isTarget;
    QMap<QString, QString> properties;
};

enum PropertyType { GlobalProperty, DirectoryProperty, TargetProperty, SourceProperty, TestProperty, VariableProperty };
typedef QMap<QString, QMap<QString, QStringList> > CategoryType;
typedef QMap<PropertyType, CategoryType > CMakeProperties;

typedef QHash<QString, Macro> MacroMap;
typedef QHash<QString, QString> CMakeDefinitions;
typedef QHash<QString, CacheEntry> CacheValues;

Q_DECLARE_METATYPE(QList<Test>)

#endif
