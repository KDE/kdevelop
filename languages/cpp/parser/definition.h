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

#ifndef DEFINITION_H
#define DEFINITION_H

#include <QList>
#include <QPair>

#include "textlocation.h"

class AbstractType;

/**
 * Represents a single variable definition in a definition-use chain.
 */
class Definition : public TextRange
{
public:
  enum Scope {
    GlobalScope,
    NamespaceScope,
    ClassScope,
    FunctionScope,
    LocalScope
  };

  Definition(const AbstractType* type, const QString& identifier, Scope scope);

  Scope scope() const;

  const AbstractType* type() const;
  const QString& identifier() const;

  const QList<KTextEditor::SmartRange*>& uses() const;
  void addUse(KTextEditor::SmartRange* range);
  void removeUse(KTextEditor::SmartRange* range);

private:
  Scope m_scope;
  const AbstractType* m_type;
  QString m_identifier;

  QList<KTextEditor::SmartRange*> m_uses;
};

#endif // DEFINITION_H

// kate: indent-width 2;
