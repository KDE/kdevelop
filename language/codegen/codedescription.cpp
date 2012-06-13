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

#include "codedescription.h"
#include <duchain/duchainlock.h>
#include <duchain/duchain.h>
#include <duchain/declaration.h>
#include <duchain/types/functiontype.h>
#include <KLocalizedString>

#define GRANTLEE_LOOKUP_PROPERTY(name)      \
if (property == #name) return QVariant::fromValue(object.name);

using namespace KDevelop;

VariableDescription::VariableDescription()
{

}

VariableDescription::VariableDescription(const QString& type, const QString& name)
: name(name)
, type(type)
{
    
}

VariableDescription::VariableDescription(const DeclarationPointer& declaration)
{
    DUChainReadLocker locker(DUChain::lock());
    
    if (declaration)
    {
        name = declaration->identifier().toString();
        type = declaration->abstractType()->toString();
    }
}

FunctionDescription::FunctionDescription()
{

}

FunctionDescription::FunctionDescription(const QString& name, const VariableDescriptionList& arguments, const VariableDescriptionList& returnArguments)
: name(name)
, arguments(arguments)
, returnArguments(returnArguments)
{

}

FunctionDescription::FunctionDescription(const DeclarationPointer& declaration)
{
    DUChainReadLocker locker(DUChain::lock());
    
    if (declaration)
    {
        name = declaration->identifier().toString();
        
        foreach (Declaration* arg, declaration->internalContext()->localDeclarations())
        {
            arguments << VariableDescription(DeclarationPointer(arg));
        }
        
        FunctionType::Ptr functionType = declaration->abstractType().cast<FunctionType>();
        Q_ASSERT(functionType);
        
        if (functionType && functionType->returnType() && functionType->returnType())
        {
            returnArguments << VariableDescription(name, functionType->returnType()->toString());
        }
    }
}

ClassDescription::ClassDescription()
{

}

ClassDescription::ClassDescription(const QString& name)
: name(name)
{

}

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

class KDevelop::ClassDescriptionModelPrivate
{
public:
    ClassDescription description;
};

ClassDescriptionModel::ClassDescriptionModel(const ClassDescription& description, QObject* parent)
: QAbstractItemModel(parent)
, d(new ClassDescriptionModelPrivate)
{
    setDescription(description);
}

ClassDescriptionModel::ClassDescriptionModel(QObject* parent)
: QAbstractItemModel(parent)
, d(new ClassDescriptionModelPrivate)
{

}


ClassDescriptionModel::~ClassDescriptionModel()
{

}

void ClassDescriptionModel::setDescription(const ClassDescription& description)
{
    beginResetModel();
    d->description = description;
    endResetModel();
}

ClassDescription ClassDescriptionModel::description() const
{
    return d->description;
}

QModelIndex ClassDescriptionModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return createIndex(row, column, TopLevelRowCount);
    }
    else
    {
        return createIndex(row, column, parent.row());
    }
}

QModelIndex ClassDescriptionModel::parent(const QModelIndex& child) const
{
    if (child.internalId() == TopLevelRowCount)
    {
        return QModelIndex();
    }
    
    return index(child.internalId(), 0);
}

bool ClassDescriptionModel::hasChildren(const QModelIndex& parent) const
{
    return !parent.isValid() || !parent.parent().isValid();
}

int ClassDescriptionModel::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return TopLevelRowCount;
    }
    
    if (parent.parent().isValid())
    {
        return 0;
    }
    
    switch (parent.row())
    {
        case ClassNameRow:
            return 0;
            
        case InheritanceRow:
            return d->description.baseClasses.size();
            
        case MembersRow:
            return d->description.members.size();
            
        case FunctionsRow:
            return d->description.methods.size();
    }
    
    return 0;
}

int ClassDescriptionModel::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return 1;
    }
    
    if (parent.parent().isValid())
    {
        return 0;
    }
    
    switch (parent.row())
    {
        case ClassNameRow:
            return 0;
            
        case InheritanceRow:
            return 2; // inheritance type + base class name
            
        case MembersRow:
            return 2; // type + name`
            
        case FunctionsRow:
            return 3; // type, name, arguments
    }
    
    return 0;
}

QVariant ClassDescriptionModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    
    if (!index.parent().isValid())
    {
        switch (index.row())
        {
            case ClassNameRow:
                return d->description.name;
                
            case InheritanceRow:
                return i18n("Base Classes");
                
            case MembersRow:
                return i18n("Data Members");
                
            case FunctionsRow:
                return i18n("Member Functions");
        }
    }
    
    const int c = index.column();
    const int r = index.row();
    const ClassDescription& cd = d->description;
    switch (index.parent().row())
    {
        case InheritanceRow:
            return (c == 0) ? cd.baseClasses[r].inheritanceMode : cd.baseClasses[r].baseType;
            
        case MembersRow:
            return (c == 0) ? cd.members[r].type : cd.members[r].name;
            
        case FunctionsRow:
        {
            FunctionDescription f = cd.methods[r];
            switch (c)
            {
                case 0:
                {
                    if (f.returnArguments.isEmpty())
                    {
                        return QString("void");
                    }
                    else
                    {
                        return f.returnArguments.first().type;
                    }
                }
                case 1:
                    return f.name;
                    
                case 2:
                {
                    QStringList args;
                    foreach (const VariableDescription& var, f.arguments)
                    {
                        args << QString("%1 %2").arg(var.type).arg(var.name);
                    }
                    return QString('(' + args.join(", ") + ')');
                }
            }
        }
    }
    
    return QVariant();
}

Qt::ItemFlags ClassDescriptionModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);    
    if (index.parent().isValid())
    {
        flags |= Qt::ItemIsEditable;
    }
    
    return flags;
}

bool ClassDescriptionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
    {
        return false;
    }
    
    if (!index.parent().isValid())
    {
        switch (index.row())
        {
            case ClassNameRow:
                d->description.name = value.toString();
                break;
                
            default:
                return false;
        }
    }
    
    const int c = index.column();
    const int r = index.row();
    ClassDescription& cd = d->description;
    switch (index.parent().row())
    {
        case InheritanceRow:
            if (c == 0)
            {
                cd.baseClasses[r].inheritanceMode = value.toString();
            }
            else
            {
                cd.baseClasses[r].baseType = value.toString();
            }
            break;
            
        case MembersRow:
            if (c == 0)
            {
                cd.members[r].type = value.toString();
            }
            else
            {
                cd.members[r].name = value.toString();
            }
            break;
            
        case FunctionsRow:
        {
            FunctionDescription f = cd.methods[r];
            switch (c)
            {
                case 0:
                {
                    if (value.toString().isEmpty())
                    {
                        f.returnArguments.clear();
                    }
                    else
                    {
                        if (f.returnArguments.isEmpty())
                        {
                            f.returnArguments.append(VariableDescription());
                        }
                        
                        f.returnArguments[0].type = value.toString();
                    }
                }
                case 1:
                    f.name = value.toString();
                    
                case 2:
                {
                    // TODO: Editing function arguments some other way
                    
                    return false;
                }
            }
        }
        break;
    }
    
    return true;
}

void ClassDescriptionModel::moveRow(int source, int destination, const QModelIndex& parent)
{
    if (parent.row() == InheritanceRow)
    {
        beginRemoveRows(parent, source, source);
        InheritanceDescription desc = d->description.baseClasses.takeAt(source);
        endRemoveRows();
        
        beginInsertRows(parent, destination, destination);
        d->description.baseClasses.insert(destination, desc);
        endInsertRows();
    }
    
    else if (parent.row() == MembersRow)
    {
        beginRemoveRows(parent, source, source);
        VariableDescription desc = d->description.members.takeAt(source);
        endRemoveRows();
        
        beginInsertRows(parent, destination, destination);
        d->description.members.insert(destination, desc);
        endInsertRows();
    }
    
    else if (parent.row() == FunctionsRow)
    {
        beginRemoveRows(parent, source, source);
        FunctionDescription desc = d->description.methods.takeAt(source);
        endRemoveRows();
        
        beginInsertRows(parent, destination, destination);
        d->description.methods.insert(destination, desc);
        endInsertRows();
    }
}

bool ClassDescriptionModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (!parent.isValid() || parent.parent().isValid())
    {
        // Only first-level indexes can have parents
        return false;
    }
    
    beginInsertRows(parent, row, row+count-1);
    for (int i = row; i < row+count; ++i)
    switch (parent.row())
    {
        case MembersRow:
            d->description.members.insert(i, VariableDescription());
            break;
            
        case FunctionsRow:
            d->description.methods.insert(i, FunctionDescription());
            break;
            
        case InheritanceRow:
            d->description.baseClasses.insert(i, InheritanceDescription());
            break;
    }
    
    endInsertRows();
    
    return true;
}

bool ClassDescriptionModel::removeRows(int row, int count, const QModelIndex& parent)
{

    if (!parent.isValid() || parent.parent().isValid())
    {
        // Only first-level indexes can have parents
        return false;
    }
    
    beginRemoveRows(parent, row, row+count-1);
    for (int i = row; i < row+count; ++i)
    switch (parent.row())
    {
        case MembersRow:
            d->description.members.takeAt(i);
            break;
            
        case FunctionsRow:
            d->description.methods.takeAt(i);
            break;
            
        case InheritanceRow:
            d->description.baseClasses.takeAt(i);
            break;
    }
    
    endRemoveRows();
    
    return true;}

















