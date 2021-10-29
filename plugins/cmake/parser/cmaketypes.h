/*
    SPDX-FileCopyrightText: 2009 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef CMAKETYPES_H
#define CMAKETYPES_H

#include "cmakelistsparser.h"
#include <util/path.h>

#include <language/duchain/indexeddeclaration.h>

#include <QHash>

struct Macro
{
    QString name;
    QStringList knownArgs;
    CMakeFileContent code;
    bool isFunction;
};

struct CacheEntry
{
    explicit CacheEntry(const QString& value=QString(), const QString &doc=QString()) : value(value), doc(doc) {}
    QString value;
    QString doc;
};

struct Target
{
    using Properties = QMap<QString, QString>;
    enum Type { Library, Executable, Custom };
    KDevelop::IndexedDeclaration declaration;
    QStringList files;
    Type type;
    CMakeFunctionDesc desc;
    QString name;
};

struct Subdirectory
{
    QString name;
    CMakeFunctionDesc desc;
    QString build_dir;
};

struct Test
{
    Test() {}
    QString name;
    KDevelop::Path executable;
    QStringList arguments;
    QHash<QString, QString> properties;
};

Q_DECLARE_TYPEINFO(Test, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Subdirectory, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Target, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(CacheEntry, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(Macro, Q_MOVABLE_TYPE);

enum PropertyType { GlobalProperty, DirectoryProperty, TargetProperty, SourceProperty, TestProperty, CacheProperty, VariableProperty };
using CategoryType = QHash<QString, QMap<QString, QStringList>>;
using CMakeProperties = QMap<PropertyType, CategoryType>;

using MacroMap = QHash<QString, Macro>;
using CMakeDefinitions = QHash<QString, QString>;
using CacheValues = QHash<QString, CacheEntry>;

Q_DECLARE_METATYPE(QList<Test>)
Q_DECLARE_METATYPE(PropertyType)

#endif
