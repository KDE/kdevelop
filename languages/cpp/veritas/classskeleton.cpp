/*
 * KDevelop xUnit test support
 *
 * Copyright 2008 Manuel Breugelmans
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "classskeleton.h"

using Veritas::ClassSkeleton;
using Veritas::ConstructorSkeleton;
using Veritas::MethodSkeleton;

ClassSkeleton::ClassSkeleton()
  : m_empty(true)
{}

ClassSkeleton::~ClassSkeleton()
{}

bool ClassSkeleton::isEmpty() const
{
    return m_empty;
}

QString ClassSkeleton::name() const
{
    return m_name;
}

void ClassSkeleton::setName(const QString& name)
{
    m_empty = false;
    m_name = name;
}

void ClassSkeleton::addMethod(const MethodSkeleton& mtd)
{
    m_methods << mtd;
}

ConstructorSkeleton ClassSkeleton::constructor() const
{
    return m_constructor;
}

MethodSkeleton ClassSkeleton::destructor() const
{
    return m_destructor;
}

QList<MethodSkeleton> ClassSkeleton::methods() const
{
    return m_methods;
}

void ClassSkeleton::setConstructor(const ConstructorSkeleton& ctr)
{
    m_constructor = ctr;
}

void ClassSkeleton::setDestructor(const MethodSkeleton& dtr)
{
    m_destructor = dtr;
}

int ClassSkeleton::memberCount() const
{
    return m_members.count();
}

QString ClassSkeleton::member(int i) const
{
    return m_members.value(i);
}

void ClassSkeleton::addMember(const QString& member)
{
    m_members << member;
}

bool ClassSkeleton::hasMethods() const
{
    return m_methods.count()!=0;
}

bool ClassSkeleton::hasMembers() const
{
    return memberCount()!=0;
}

bool ClassSkeleton::hasConstructor() const
{
    return !constructor().isEmpty();
}

bool ClassSkeleton::hasDestructor() const
{
    return !destructor().isEmpty();
}

bool ClassSkeleton::hasSomething() const
{
    return hasMembers() || hasMethods() || hasConstructor() || hasDestructor();
}

void ClassSkeleton::addSuper(const QString& super)
{
    m_super = super;
}

QString ClassSkeleton::super() const
{
    return m_super;
}

