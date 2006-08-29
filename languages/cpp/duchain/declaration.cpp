/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "declaration.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "ducontext.h"
#include "use.h"

using namespace KTextEditor;

Declaration::Declaration(KTextEditor::Range* range, Scope scope )
  : KDevDocumentRangeObject(range)
  , m_context(0)
  , m_scope(scope)
{
}

Declaration::~Declaration()
{
  qDeleteAll(m_uses);
}

void Declaration::removeUse( Use* range )
{
  range->setDeclaration(0L);
  m_uses.removeAll(range);
}

void Declaration::addUse( Use* range )
{
  range->setDeclaration(this);
  m_uses.append(range);
}

const QList< Use* > & Declaration::uses( ) const
{
  return m_uses;
}

const Identifier& Declaration::identifier( ) const
{
  return m_identifier;
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  m_identifier = identifier;
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  return m_type;
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  m_type = type;
}

Declaration::Scope Declaration::scope( ) const
{
  return m_scope;
}

QualifiedIdentifier Declaration::qualifiedIdentifier() const
{
  QualifiedIdentifier ret = context()->scopeIdentifier();
  ret.push(identifier());
  return ret;
}

DUContext * Declaration::context() const
{
  return m_context;
}

void Declaration::setContext(DUContext* context)
{
  m_context = context;
}

bool Declaration::operator ==(const Declaration & other) const
{
  return this == &other;
}

QString Declaration::toString() const
{
  return QString("Declaration: %3 %4").arg(abstractType() ? abstractType()->toString() : QString("<notype>")).arg(identifier().toString());
}

// kate: indent-width 2;
