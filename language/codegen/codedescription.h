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

namespace KDevelop {

    struct VariableDescription
    {
        VariableDescription();
        VariableDescription(const QString& name, const QString& type);
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
        Q_PROPERTY(ClassDescription description READ description WRITE setDescription)
        
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
        virtual ~ClassDescriptionModel();
        
        virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
        virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
        
        ClassDescription description() const;
        void setDescription(const ClassDescription& description);
        
        virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
        
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
    
namespace Grantlee
{
    template<> QVariant TypeAccessor<KDevelop::VariableDescription&>::lookUp(const KDevelop::VariableDescription& object, const QString& property);
    template<> QVariant TypeAccessor<KDevelop::FunctionDescription&>::lookUp(const KDevelop::FunctionDescription& object, const QString& property);
    template<> QVariant TypeAccessor<KDevelop::InheritanceDescription&>::lookUp(const KDevelop::InheritanceDescription& object, const QString& property);
    template<> QVariant TypeAccessor<KDevelop::ClassDescription&>::lookUp(const KDevelop::ClassDescription& object, const QString& property);
}

#endif // KDEVELOP_CODEDESCRIPTION_H
