/* This file is part of KDevelop
    Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#include "abstractfunctiondeclaration.h"
#include <QStringList>

namespace KDevelop {
class AbstractFunctionDeclarationPrivate
{
public:
  AbstractFunctionDeclarationPrivate() : m_isVirtual(false), m_isInline(false), m_isExplicit(false) {
  }
  bool m_isVirtual: 1;
  bool m_isInline: 1;
  bool m_isExplicit: 1;
  QStringList m_defaultParameters;
  QStringList m_parameterNames;
};

AbstractFunctionDeclaration::AbstractFunctionDeclaration() : d(new AbstractFunctionDeclarationPrivate) {
}

AbstractFunctionDeclaration::AbstractFunctionDeclaration(const AbstractFunctionDeclaration& rhs) : d(new AbstractFunctionDeclarationPrivate(*rhs.d)) {
}

AbstractFunctionDeclaration::~AbstractFunctionDeclaration() {
  delete d;
}

const QList<QString>& AbstractFunctionDeclaration::defaultParameters() const {
  return d->m_defaultParameters;
}

void AbstractFunctionDeclaration::addDefaultParameter(const QString& str) {
  d->m_defaultParameters << str;
}

const QList<QString>& AbstractFunctionDeclaration::parameterNames() const {
  return d->m_parameterNames;
}

void AbstractFunctionDeclaration::addParameterName(const QString& str) {
  d->m_parameterNames << str;
}

bool AbstractFunctionDeclaration::isVirtual() const
{
  return d->m_isVirtual;
}

void AbstractFunctionDeclaration::setVirtual(bool isVirtual)
{
  d->m_isVirtual = isVirtual;
}

bool AbstractFunctionDeclaration::isInline() const
{
  return d->m_isInline;
}

void AbstractFunctionDeclaration::setInline(bool isInline)
{
  d->m_isInline = isInline;
}

bool AbstractFunctionDeclaration::isExplicit() const
{
  return d->m_isExplicit;
}

void AbstractFunctionDeclaration::setExplicit(bool isExplicit)
{
  d->m_isExplicit = isExplicit;
}

void AbstractFunctionDeclaration::setFunctionSpecifiers(FunctionSpecifiers specifiers)
{
  d->m_isInline = specifiers & InlineSpecifier;
  d->m_isExplicit = specifiers & ExplicitSpecifier;
  d->m_isVirtual = specifiers & VirtualSpecifier;
}

}


// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
