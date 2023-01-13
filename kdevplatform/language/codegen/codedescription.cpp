/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "codedescription.h"
#include <debug.h>
#include <language/duchain/duchainutils.h>
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
    if (member) {
        switch (member->accessPolicy())
        {
        case Declaration::Private:
            return QStringLiteral("private");
        case Declaration::Protected:
            return QStringLiteral("protected");
        case Declaration::Public:
            return QStringLiteral("public");
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

    if (declaration) {
        name = declaration->identifier().toString();
        if (auto abstractType = declaration->abstractType()) {
            type = abstractType->toString();
        }
    }

    access = accessPolicyName(declaration);
}

FunctionDescription::FunctionDescription()
    : FunctionDescription::FunctionDescription({}, {}, {})
{
}

FunctionDescription::FunctionDescription(const QString& name, const VariableDescriptionList& arguments,
                                         const VariableDescriptionList& returnArguments)
    : name(name)
    , arguments(arguments)
    , returnArguments(returnArguments)
    , isConstructor(false)
    , isDestructor(false)
    , isVirtual(false)
    , isStatic(false)
    , isSlot(false)
    , isSignal(false)
    , isConst(false)
{
}

FunctionDescription::FunctionDescription(const DeclarationPointer& declaration)
    : FunctionDescription::FunctionDescription({}, {}, {})
{
    DUChainReadLocker lock;

    if (declaration) {
        name = declaration->identifier().toString();
        DUContext* context = declaration->internalContext();

        DUChainPointer<FunctionDeclaration> function = declaration.dynamicCast<FunctionDeclaration>();
        if (function) {
            context = DUChainUtils::argumentContext(declaration.data());
        }

        DUChainPointer<ClassFunctionDeclaration> method = declaration.dynamicCast<ClassFunctionDeclaration>();

        if (method) {
            isConstructor = method->isConstructor();
            isDestructor = method->isDestructor();
            isVirtual = method->isVirtual();
            isAbstract = method->isAbstract();
            isFinal = method->isFinal();
            isOverriding = (DUChainUtils::overridden(method.data()) != nullptr);
            isStatic = method->isStatic();
            isSlot = method->isSlot();
            isSignal = method->isSignal();
        }

        int i = 0;
        const auto localDeclarations = context->localDeclarations();
        arguments.reserve(localDeclarations.size());
        for (Declaration* arg : localDeclarations) {
            VariableDescription var = VariableDescription(DeclarationPointer(arg));
            if (function) {
                var.value = function->defaultParameterForArgument(i).str();
                qCDebug(LANGUAGE) << var.name << var.value;
            }
            arguments << var;
            ++i;
        }

        auto functionType = declaration->abstractType().dynamicCast<FunctionType>();

        if (functionType) {
            isConst = (functionType->modifiers() & AbstractType::ConstModifier);
        }

        if (functionType && functionType->returnType()) {
            returnArguments << VariableDescription(functionType->returnType()->toString(), QString());
        }

        access = accessPolicyName(declaration);
    }
}

QString FunctionDescription::returnType() const
{
    if (returnArguments.isEmpty()) {
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
