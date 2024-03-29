/*
    SPDX-FileCopyrightText: 2012 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CODEDESCRIPTIONMETATYPES_H
#define KDEVPLATFORM_CODEDESCRIPTIONMETATYPES_H

#include "codedescription.h"

#include <grantlee/metatype.h>

#define GRANTLEE_LOOKUP_PROPERTY(name)          \
    if (property == QLatin1String(# name)) \
        return QVariant::fromValue(object.name);

#define GRANTLEE_LOOKUP_LIST_PROPERTY(name)     \
    if (property == QLatin1String(# name)) \
        return QVariant::fromValue(KDevelop::CodeDescription::toVariantList(object.name));

GRANTLEE_BEGIN_LOOKUP(KDevelop::VariableDescription)
GRANTLEE_LOOKUP_PROPERTY(name)
GRANTLEE_LOOKUP_PROPERTY(type)
GRANTLEE_LOOKUP_PROPERTY(access)
GRANTLEE_LOOKUP_PROPERTY(value)
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::FunctionDescription)
GRANTLEE_LOOKUP_PROPERTY(name)
GRANTLEE_LOOKUP_PROPERTY(access)
GRANTLEE_LOOKUP_LIST_PROPERTY(arguments)
GRANTLEE_LOOKUP_LIST_PROPERTY(returnArguments)
GRANTLEE_LOOKUP_PROPERTY(isConstructor)
GRANTLEE_LOOKUP_PROPERTY(isDestructor)
GRANTLEE_LOOKUP_PROPERTY(isVirtual)
GRANTLEE_LOOKUP_PROPERTY(isAbstract)
GRANTLEE_LOOKUP_PROPERTY(isOverriding)
GRANTLEE_LOOKUP_PROPERTY(isFinal)
GRANTLEE_LOOKUP_PROPERTY(isStatic)
GRANTLEE_LOOKUP_PROPERTY(isConst)
GRANTLEE_LOOKUP_PROPERTY(isSignal)
GRANTLEE_LOOKUP_PROPERTY(isSlot)
if (property == QLatin1String("returnType"))
{
    return object.returnType();
}
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::InheritanceDescription)
GRANTLEE_LOOKUP_PROPERTY(inheritanceMode)
GRANTLEE_LOOKUP_PROPERTY(baseType)
GRANTLEE_END_LOOKUP

GRANTLEE_BEGIN_LOOKUP(KDevelop::ClassDescription)
GRANTLEE_LOOKUP_PROPERTY(name)
GRANTLEE_LOOKUP_LIST_PROPERTY(baseClasses)
GRANTLEE_LOOKUP_LIST_PROPERTY(members)
GRANTLEE_LOOKUP_LIST_PROPERTY(methods)
GRANTLEE_END_LOOKUP

#endif // KDEVPLATFORM_CODEDESCRIPTIONMETATYPES_H
