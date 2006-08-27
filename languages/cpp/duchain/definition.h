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

#include "identifier.h"
#include "kdevdocumentrangeobject.h"
#include "cppnamespace.h"
#include "typesystem.h"

class AbstractType;
class DUContext;
class DefinitionUse;

/**
 * Represents a single variable definition in a definition-use chain.
 */
class Definition : public KDevDocumentRangeObject
{
public:
  enum Scope {
    GlobalScope,
    NamespaceScope,
    ClassScope,
    FunctionScope,
    LocalScope
  };

  Definition(KTextEditor::Range* range, Scope scope);
  virtual ~Definition();

  DUContext* context() const;
  void setContext(DUContext* context);

  Scope scope() const;

  template <class T>
  KSharedPtr<T> type() const { return KSharedPtr<T>::dynamicCast(abstractType()); }

  template <class T>
  void setType(KSharedPtr<T> type) { setAbstractType(AbstractType::Ptr::staticCast(type)); }

  AbstractType::Ptr abstractType() const;
  void setAbstractType(AbstractType::Ptr type);

  void setIdentifier(const Identifier& identifier);
  const Identifier& identifier() const;

  QualifiedIdentifier qualifiedIdentifier() const;

  const QList<DefinitionUse*>& uses() const;
  void addUse(DefinitionUse* range);
  void removeUse(DefinitionUse* range);

  bool operator==(const Definition& other) const;

  virtual QString toString() const;

private:
  DUContext* m_context;
  Scope m_scope;
  AbstractType::Ptr m_type;
  Identifier m_identifier;

  QList<DefinitionUse*> m_uses;
};

#endif // DEFINITION_H

// kate: indent-width 2;
