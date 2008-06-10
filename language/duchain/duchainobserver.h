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

#include "language/duchain/duchainpointer.h"
#include "language/interfaces/iproblem.h"

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
class KDEVPLATFORMLANGUAGE_EXPORT DUChainObserver : public QObject
{
  Q_OBJECT
  friend class DUChain;

public:
  virtual ~DUChainObserver();

  enum Modification {
    Addition /**< Something was added to the duchain */,
    Removal  /**< Something was removed to the duchain */,
    Change   /**< Something was changed in the duchain */,
    Deletion /**< Something was deleted in the duchain */
  };

  enum Relationship {
    // Context
    ParentContext           /**< related object is the parent context */,
    ChildContexts           /**< related object is a child context  */,
    ImportedParentContexts  /**< related object is an imported parent context  */,
    ImportedChildContexts   /**< related object is an imported child context  */,
    LocalDeclarations       /**< related object is a local declaration */,
    LocalDefinitions        /**< related object is a local definition */,
    UsingNamespaces         /**< related object uses namespaces */,
    ContextType             /**< related object is a context type */,

    // Context + Declaration
    Uses                    /**< related object is a use */,
    Identifier              /**< related object is an identifier */,

    // Declaration
    DataType                /**< the related object is a datatype */,
    ForwardDeclarations     /**< the related object is a forward declaration */,

    // Declaration + Definition + Use
    Context                 /**< the related object is context */,

    // Declaration + Definition
    DefinitionRelationship  /**< the related object definition */,

    // Use
    DeclarationRelationship /**< the related object is a declaration */,

    // All
    NotApplicable           /**< the related object is no applicable */
  };

Q_SIGNALS:
  /*void contextChanged(KDevelop::DUContextPointer context, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);

  void declarationChanged(KDevelop::DeclarationPointer declaration, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);

  void definitionChanged(KDevelop::DefinitionPointer definition, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);

  void useChanged(KDevelop::UsePointer use, KDevelop::DUChainObserver::Modification change, KDevelop::DUChainObserver::Relationship relationship, KDevelop::DUChainBasePointer relatedObject);*/

  void branchAdded(KDevelop::DUContextPointer context);
  void branchModified(KDevelop::DUContextPointer context);
  void branchRemoved(KDevelop::DUContextPointer context);
};

}

#endif // DUCHAINOBSERVER_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
