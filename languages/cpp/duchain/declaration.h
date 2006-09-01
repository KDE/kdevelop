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

#ifndef DECLARATION_H
#define DECLARATION_H

#include <QList>

#include "identifier.h"
#include "kdevdocumentrangeobject.h"
#include "cppnamespace.h"
#include "typesystem.h"

class AbstractType;
class DUContext;
class Use;
class Definition;

/**
 * Represents a single declaration in a definition-use chain.
 */
class Declaration : public KDevDocumentRangeObject
{
  friend class DUContext;

public:
  enum Scope {
    GlobalScope,
    NamespaceScope,
    ClassScope,
    FunctionScope,
    LocalScope
  };

  Declaration(KTextEditor::Range* range, Scope scope);
  virtual ~Declaration();

  bool isDefinition() const;
  void setDeclarationIsDefinition(bool dd);

  Definition* definition() const;
  void setDefinition(Definition* definition);

  DUContext* context() const;

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

  /**
   * Provides a mangled version of this definition's identifier, for use in a symbol table.
   */
  QString mangledIdentifier() const;

  bool inSymbolTable() const;
  void setInSymbolTable(bool inSymbolTable);

  const QList<Use*>& uses() const;
  void addUse(Use* range);
  void removeUse(Use* range);

  bool operator==(const Declaration& other) const;

  virtual QString toString() const;

private:
  void setContext(DUContext* context);

  DUContext* m_context;
  Scope m_scope;
  AbstractType::Ptr m_type;
  Identifier m_identifier;

  Definition* m_definition;
  QList<Use*> m_uses;

  bool m_isDefinition  : 1;
  bool m_inSymbolTable : 1;
};

#endif // DECLARATION_H

// kate: indent-width 2;
