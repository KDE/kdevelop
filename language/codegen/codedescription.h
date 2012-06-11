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

#ifndef KDEVELOP_CODEMODEL_H
#define KDEVELOP_CODEMODEL_H

#include "../languageexport.h"
#include <duchain/duchainpointer.h>

#include <QString>
#include <QList>

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
    };
    
    typedef QList<FunctionDescription> FunctionDescriptionList;
    
    struct ClassDescription
    {
        ClassDescription();
        ClassDescription(const QString& name);
        
        QString name;
        QList<QString> baseClasses;
        VariableDescriptionList members;
        FunctionDescriptionList methods;
    };

}

#endif // KDEVELOP_CODEMODEL_H
