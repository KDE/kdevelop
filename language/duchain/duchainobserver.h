/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef DUCHAINOBSERVER_H
#define DUCHAINOBSERVER_H

#include <QtCore/QObject>

#include <KUrl>

#include <KTextEditor/Range>

#include "duchainpointer.h"

namespace KDevelop
{

class DUChainBase;
class DUContext;
class Declaration;
class Definition;
class Use;

/**
 * Abstract class for observers of the definition-use chain to receive
 * feedback on changes.
 *
 * \todo change name to DUChainNotifier ?
 */
class DUChainObserver : public QObject
{
  Q_OBJECT
  friend class DUChain;

public:
  virtual ~DUChainObserver();

  enum Modification {
    Addition,
    Removal,
    Change,
    Deletion
  };

  enum Relationship {
    // Context
    ParentContext,
    ChildContexts,
    ImportedParentContexts,
    ImportedChildContexts,
    LocalDeclarations,
    LocalDefinitions,
    UsingNamespaces,
    ContextType,

    // Context + Declaration
    Uses,
    Identifier,

    // Declaration
    DataType,
    ForwardDeclarations,

    // Declaration + Definition + Use
    Context,

    // Declaration + Definition
    DefinitionRelationship,

    // Use
    DeclarationRelationship,

    // All
    NotApplicable
  };

Q_SIGNALS:
  /*void contextChanged(KDevelop::DUContextPointer context, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);

  void declarationChanged(KDevelop::DeclarationPointer declaration, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);

  void definitionChanged(KDevelop::DefinitionPointer definition, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);

  void useChanged(KDevelop::UsePointer use, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);*/

  void branchAdded(KDevelop::DUContextPointer context);

  void problemEncountered(const KUrl& url, const KTextEditor::Range& range, const QString& problem);
};

}

#endif // DUCHAINOBSERVER_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
