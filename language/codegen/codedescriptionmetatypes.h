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

#ifndef KDEVPLATFORM_CODEDESCRIPTIONMETATYPES_H
#define KDEVPLATFORM_CODEDESCRIPTIONMETATYPES_H

#include "codedescription.h"

#include <grantlee/metatype.h>

#define GRANTLEE_LOOKUP_PROPERTY(name)          \
if (property == #name) return QVariant::fromValue(object.name);

#define GRANTLEE_LOOKUP_LIST_PROPERTY(name)     \
if (property == #name) return QVariant::fromValue(KDevelop::CodeDescription::toVariantList(object.name));

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
    GRANTLEE_LOOKUP_PROPERTY(isStatic)
    GRANTLEE_LOOKUP_PROPERTY(isConst)
    GRANTLEE_LOOKUP_PROPERTY(isSignal)
    GRANTLEE_LOOKUP_PROPERTY(isSlot)
    if (property == "returnType")
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
