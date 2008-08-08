/*
 * KDevelop xUnit testing support
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
#include "methodskeleton.h"
#include <language/duchain/types/functiontype.h>

using Veritas::MethodSkeleton;
using KDevelop::FunctionType;

MethodSkeleton::MethodSkeleton()
  : m_empty(true), m_arguments("()")
{}

MethodSkeleton::~MethodSkeleton()
{}

void MethodSkeleton::setFunctionType(FunctionType::Ptr fn)
{
    m_arguments = fn->partToString(FunctionType::SignatureArguments);
    m_returnType = fn->partToString(FunctionType::SignatureReturn);
}

bool MethodSkeleton::isEmpty() const
{
    return m_empty;
}

void MethodSkeleton::setName(const QString& name)
{
    m_empty = false;
    m_name = name;
}

void MethodSkeleton::setBody(const QString& body)
{
    m_body = body;
}

void MethodSkeleton::setReturnType(const QString& rt)
{
    m_returnType = rt;
}

void MethodSkeleton::setArguments(const QString& sig)
{
    m_arguments = sig;
}

QString MethodSkeleton::name() const
{
    return m_name;
}

QString MethodSkeleton::body() const
{
    return m_body;
}

QString MethodSkeleton::returnType() const
{
    return m_returnType;
}

QString MethodSkeleton::arguments() const
{
    return m_arguments;
}
