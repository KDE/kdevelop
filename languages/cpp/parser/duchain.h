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

#ifndef DUCHAIN_H
#define DUCHAIN_H

#include <QList>
#include <QPair>

#include <ktexteditor/cursor.h>

namespace KTextEditor { class SmartRange; }

class AbstractType;

/**
 * Represents a single variable definition in a definition-use chain.
 */
class Definition
{
public:
  enum Scope {
    GlobalScope,
    NamespaceScope,
    ClassScope,
    FunctionScope,
    LocalScope
  };

  Definition(AbstractType* type, const QString& identifier, Scope scope);

  Scope scope() const;

  AbstractType* type() const;
  const QString& identifier() const;

  const QList<KTextEditor::SmartRange*>& uses() const;
  void addUse(KTextEditor::SmartRange* range);
  void removeUse(KTextEditor::SmartRange* range);

private:
  Scope m_scope;
  AbstractType* m_type;
  QString m_identifier;

  QList<KTextEditor::SmartRange*> m_uses;
};

/**
 * A single context in source code, represented as a node in a
 * directed acyclic graph.
 *
 * \todo change child relationships to a linked list within the context?
 */
class DUContext
{
public:
  DUContext(DUContext* parent = 0);
  inline virtual ~DUContext() {};

  /**
   * Returns the text range covered by this DUContext.  This usually includes the actual
   * definition.
   */
  KTextEditor::SmartRange* textRange() const;

  const QList<DUContext*>& parentDUContexts() const;
  const QList<DUContext*>& childDUContexts() const;

  /**
   * Returns the context in which \a identifier was defined, or
   * null if one is not found.
   */
  DUContext* definitionDUContext(const QString& identifier);

  /**
   * Returns the type of any existing valid \a identifier in this context, or
   * null if one is not found.
   *
   * \overload
   */
  Definition* definitionForIdentifier(const QString& identifier);

  /**
   * Returns the type of any \a identifier defined in this context, or
   * null if one is not found.
   */
  Definition* definitionForLocalIdentifier(const QString& identifier);

  /**
   * Adds a new definition to this context. Passes back that definition for convenience.
   */
  Definition* addDefinition(Definition* definition);

  // TODO perhaps make this take the abstract type? dunno.
  void removeDefinition(const QString& name);

private:
  QList<DUContext*> m_parentDUContexts;
  QList<DUContext*> m_childDUContexts;

  // TODO: consider splitting context into two separate classes with a common base class,
  // to prevent having a list for every context where only one use is defined.
  QList< QPair<QString, AbstractType*> > m_locallyDefinedIdentifiers;
};

/**
 * Holds references to all scopes in one parsed source file.
 * Uses a directed acyclic graph design.
 */
class DUChain : public DUContext
{
public:
  DUContext* contextAt(const KTextEditor::Cursor& pos) const;

  QList< QPair<QString, AbstractType*> > definitionsAt(const KTextEditor::Cursor& position) const;
  AbstractType* findDefinition(const QString& identifier, const KTextEditor::Cursor& position) const;

private:
  QList<DUContext*> m_contexts;
};

#endif // DUCHAIN_H

// kate: indent-width 2;
