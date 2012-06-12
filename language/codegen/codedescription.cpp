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

#define GRANTLEE_LOOKUP_PROPERTY(name)      \
if (property == #name) return QVariant::fromValue(object.name);

using namespace KDevelop;

VariableDescription::VariableDescription()
{

}

VariableDescription::VariableDescription(const QString& name, const QString& type)
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
