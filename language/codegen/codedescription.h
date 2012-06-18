/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVELOP_CODEDESCRIPTION_H
#define KDEVELOP_CODEDESCRIPTION_H

#include "../languageexport.h"
#include <language/duchain/duchainpointer.h>

#include <grantlee/metatype.h>

#include <QString>
#include <QList>
#include <QAbstractItemModel>

#define GRANTLEE_LOOKUP_PROPERTY(name)      \
if (property == #name) return QVariant::fromValue(object.name);

namespace KDevelop {

struct VariableDescription
{
    VariableDescription();
    VariableDescription(const QString& type, const QString& name);
    VariableDescription(const DeclarationPointer& declaration);

    QString name;
    QString type;
};

typedef QList<VariableDescription> VariableDescriptionList;

struct FunctionDescription
{
    FunctionDescription();
    FunctionDescription(const QString& name, const VariableDescriptionList& arguments, const VariableDescriptionList& returnArguments);
    FunctionDescription(const DeclarationPointer& declaration);

    QString name;
    QList<VariableDescription> arguments;
    QList<VariableDescription> returnArguments;

    bool isConstructor;
    bool isDestructor;
    bool isVirtual;
    bool isStatic;
};

typedef QList<FunctionDescription> FunctionDescriptionList;

struct InheritanceDescription
{
    QString inheritanceMode;
    QString baseType;
};

typedef QList<InheritanceDescription> InheritanceDescriptionList;

struct ClassDescription
{
    ClassDescription();
    ClassDescription(const QString& name);

    QString name;
    InheritanceDescriptionList baseClasses;
    VariableDescriptionList members;
    FunctionDescriptionList methods;
};

class ClassDescriptionModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(KDevelop::ClassDescription description READ description WRITE setDescription)

public:
    enum TopLevelRow
    {
        ClassNameRow = 0,
        InheritanceRow,
        MembersRow,
        FunctionsRow,
        TopLevelRowCount
    };

    explicit ClassDescriptionModel(const ClassDescription& description, QObject* parent = 0);
    explicit ClassDescriptionModel(QObject* parent = 0);
    virtual ~ClassDescriptionModel();

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;

    ClassDescription description() const;
    void setDescription(const ClassDescription& description);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    void moveRow(int source, int destination, const QModelIndex& parent);

private:
    class ClassDescriptionModelPrivate* const d;
};

}

Q_DECLARE_METATYPE(KDevelop::VariableDescription)
Q_DECLARE_METATYPE(KDevelop::VariableDescriptionList)
Q_DECLARE_METATYPE(KDevelop::FunctionDescription)
Q_DECLARE_METATYPE(KDevelop::FunctionDescriptionList)
Q_DECLARE_METATYPE(KDevelop::InheritanceDescription)
Q_DECLARE_METATYPE(KDevelop::InheritanceDescriptionList)
Q_DECLARE_METATYPE(KDevelop::ClassDescription)

GRANTLEE_BEGIN_LOOKUP(KDevelop::VariableDescription)
    GRANTLEE_LOOKUP_PROPERTY(name)
    GRANTLEE_LOOKUP_PROPERTY(type)
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::FunctionDescription)
    GRANTLEE_LOOKUP_PROPERTY(name)
    GRANTLEE_LOOKUP_PROPERTY(arguments)
    GRANTLEE_LOOKUP_PROPERTY(returnArguments)
    GRANTLEE_LOOKUP_PROPERTY(isConstructor)
    GRANTLEE_LOOKUP_PROPERTY(isDestructor)
    GRANTLEE_LOOKUP_PROPERTY(isVirtual)
    GRANTLEE_LOOKUP_PROPERTY(isStatic)
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::InheritanceDescription)
    GRANTLEE_LOOKUP_PROPERTY(inheritanceMode)
    GRANTLEE_LOOKUP_PROPERTY(baseType)
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::ClassDescription)
    GRANTLEE_LOOKUP_PROPERTY(name)
    GRANTLEE_LOOKUP_PROPERTY(baseClasses)
    GRANTLEE_LOOKUP_PROPERTY(members)
    GRANTLEE_LOOKUP_PROPERTY(methods)
GRANTLEE_END_LOOKUP

#endif // KDEVELOP_CODEDESCRIPTION_H
