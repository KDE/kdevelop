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
#include "util/debug.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/declaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <language/duchain/functiondeclaration.h>

#include <KLocalizedString>

using namespace KDevelop;

/**
 * The access policy as a string, or an empty string
 * if the policy is set to default
 *
 * The DUChain must be locked when calling this function
 **/
QString accessPolicyName(const DeclarationPointer& declaration)
{
    DUChainPointer<ClassMemberDeclaration> member = declaration.dynamicCast<ClassMemberDeclaration>();
    if (member)
    {
        switch (member->accessPolicy())
        {
            case Declaration::Private:
                return "private";
            case Declaration::Protected:
                return "protected";
            case Declaration::Public:
                return "public";
            default:
                break;
        }
    }
    return QString();
}

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
    DUChainReadLocker lock;

    if (declaration)
    {
        name = declaration->identifier().toString();
        type = declaration->abstractType()->toString();
    }

    access = accessPolicyName(declaration);
}

FunctionDescription::FunctionDescription()
{

}

FunctionDescription::FunctionDescription(const QString& name, const VariableDescriptionList& arguments,
                                         const VariableDescriptionList& returnArguments)
: name(name)
, arguments(arguments)
, returnArguments(returnArguments)
{

}

FunctionDescription::FunctionDescription(const DeclarationPointer& declaration)
{
    DUChainReadLocker lock;

    if (declaration)
    {
        name = declaration->identifier().toString();
        DUContext* context = declaration->internalContext();

        DUChainPointer<FunctionDeclaration> function = declaration.dynamicCast<FunctionDeclaration>();
        if (function && function->internalFunctionContext())
        {
            context = function->internalFunctionContext();
        }

        DUChainPointer<ClassFunctionDeclaration> method = declaration.dynamicCast<ClassFunctionDeclaration>();

        if (method)
        {
            isConstructor = method->isConstructor();
            isDestructor = method->isDestructor();
            isVirtual = method->isVirtual();
            isStatic = method->isStatic();
            isSlot = method->isSlot();
            isSignal = method->isSignal();
        }

        int i = 0;
        foreach (Declaration* arg, context->localDeclarations())
        {
            VariableDescription var = VariableDescription(DeclarationPointer(arg));
            if (function)
            {
                var.value = function->defaultParameterForArgument(i).str();
                qCDebug(LANGUAGE) << var.name << var.value;
            }
            arguments << var;
            ++i;
        }

        FunctionType::Ptr functionType = declaration->abstractType().cast<FunctionType>();

        if (functionType)
        {
            isConst = (functionType->modifiers() & AbstractType::ConstModifier);
        }

        if (functionType && functionType->returnType())
        {
            returnArguments << VariableDescription(functionType->returnType()->toString(), QString());
        }

        access = accessPolicyName(declaration);
    }
}

QString FunctionDescription::returnType() const
{
    if (returnArguments.isEmpty())
    {
        return QString();
    }
    return returnArguments.first().type;
}

ClassDescription::ClassDescription()
{

}

ClassDescription::ClassDescription(const QString& name)
: name(name)
{

}
