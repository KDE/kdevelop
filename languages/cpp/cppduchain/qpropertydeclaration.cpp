/*
   Copyright (c) 2010 KDAB

   Author: Tobias Koenig <tokoe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "qpropertydeclaration.h"
#include <language/duchain/duchainregister.h>

using namespace KDevelop;

namespace Cpp {

REGISTER_DUCHAIN_ITEM(QPropertyDeclaration);

QPropertyDeclaration::QPropertyDeclaration(const QPropertyDeclaration& rhs)
  : KDevelop::ClassMemberDeclaration(*new QPropertyDeclarationData(*rhs.d_func()))
{
  d_func_dynamic()->setClassId(this);
}

QPropertyDeclaration::QPropertyDeclaration(QPropertyDeclarationData& data)
  : KDevelop::ClassMemberDeclaration(data)
{
}

QPropertyDeclaration::QPropertyDeclaration(const KDevelop::SimpleRange& range, KDevelop::DUContext* context)
  : KDevelop::ClassMemberDeclaration(*new QPropertyDeclarationData, range)
{
  d_func_dynamic()->setClassId(this);
  if (context)
    setContext(context);
}

void QPropertyDeclaration::setReadMethod(const IndexedDeclaration &declaration)
{
  d_func_dynamic()->m_readMethod = declaration;
}

IndexedDeclaration QPropertyDeclaration::readMethod() const
{
  return d_func()->m_readMethod;
}

void QPropertyDeclaration::setWriteMethod(const IndexedDeclaration &declaration)
{
  d_func_dynamic()->m_writeMethod = declaration;
}

IndexedDeclaration QPropertyDeclaration::writeMethod() const
{
  return d_func()->m_writeMethod;
}

void QPropertyDeclaration::setResetMethod(const IndexedDeclaration &declaration)
{
  d_func_dynamic()->m_resetMethod = declaration;
}

IndexedDeclaration QPropertyDeclaration::resetMethod() const
{
  return d_func()->m_resetMethod;
}

void QPropertyDeclaration::setNotifyMethod(const IndexedDeclaration &declaration)
{
  d_func_dynamic()->m_notifyMethod = declaration;
}

IndexedDeclaration QPropertyDeclaration::notifyMethod() const
{
  return d_func()->m_notifyMethod;
}

void QPropertyDeclaration::setDesignableMethod(const IndexedDeclaration &declaration)
{
  d_func_dynamic()->m_designableMethod = declaration;
}

IndexedDeclaration QPropertyDeclaration::designableMethod() const
{
  return d_func()->m_designableMethod;
}

void QPropertyDeclaration::setScriptableMethod(const IndexedDeclaration &declaration)
{
  d_func_dynamic()->m_scriptableMethod = declaration;
}

IndexedDeclaration QPropertyDeclaration::scriptableMethod() const
{
  return d_func()->m_scriptableMethod;
}

void QPropertyDeclaration::setIsDesignable(bool designable)
{
  d_func_dynamic()->m_isDesignable = designable;
}

bool QPropertyDeclaration::isDesignable() const
{
  return d_func()->m_isDesignable;
}

void QPropertyDeclaration::setIsScriptable(bool scriptable)
{
  d_func_dynamic()->m_isScriptable = scriptable;
}

bool QPropertyDeclaration::isScriptable() const
{
  return d_func()->m_isScriptable;
}

void QPropertyDeclaration::setIsStored(bool stored)
{
  d_func_dynamic()->m_isStored = stored;
}

bool QPropertyDeclaration::isStored() const
{
  return d_func()->m_isStored;
}

void QPropertyDeclaration::setIsUser(bool user)
{
  d_func_dynamic()->m_isUser = user;
}

bool QPropertyDeclaration::isUser() const
{
  return d_func()->m_isUser;
}

void QPropertyDeclaration::setIsConstant(bool constant)
{
  d_func_dynamic()->m_isConstant = constant;
}

bool QPropertyDeclaration::isConstant() const
{
  return d_func()->m_isConstant;
}

void QPropertyDeclaration::setIsFinal(bool final)
{
  d_func_dynamic()->m_isFinal = final;
}

bool QPropertyDeclaration::isFinal() const
{
  return d_func()->m_isFinal;
}

KDevelop::Declaration* QPropertyDeclaration::clonePrivate() const
{
  return new QPropertyDeclaration(*this);
}

}
