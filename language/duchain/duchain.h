/* This file is part of KDevelop
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>

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

#include <QtCore/QObject>

#include "duchainobserver.h"
#include "topducontext.h"

class KUrl;

namespace KDevelop
{

class IDocument;
class TopDUContext;
class DUChainLock;

class IdentifiedFile;
class ParsingEnvironmentManager;
class ParsingEnvironment;
class ParsingEnvironmentFile;
class Definitions;
class Uses;

/**
 * \short Holds references to all top level source file contexts.
 *
 * The DUChain is a global static class which manages the definition-use
 * chains.  It performs the following functions:
 * \li registers chains with addDocumentChain() and deregisters with removeDocumentChain()
 * \li allows querying for existing chains
 * \li watches text editors, registering and deregistering them with the BackgroundParser when files
 *     are opened and closed.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChain : public QObject
{
  Q_OBJECT

public:
  /**
   * Return a list of all chains available
   */
  QList<TopDUContext*> allChains() const;

  /**
   * Return any chain for the given document
   * If available, the version accepting IndexedString should be used instead of this, for performance reasons.
   * When no fitting chain is in memory, one may be loaded from disk.
   * */
  TopDUContext* chainForDocument(const KUrl& document) const;
  TopDUContext* chainForDocument(const IndexedString& document) const;

  /**
   * Return all chains for the given document that are currently in memory.
   * This does not load any chains from disk.
   * */
  QList<TopDUContext*> chainsForDocument(const KUrl& document) const;

  /**
   * Return all chains for the given document that are currently in memory.
   * This does not load any chains from disk.
   * Should be preferred over the KUrl version.
   * */
  QList<TopDUContext*> chainsForDocument(const IndexedString& document) const;

  /**
   * Find a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   * @param flags If this is TopDUContext::AnyFlag, context-flags will be ignored while searching.
   *              Else a context will be searched that exactly matches the given flags.
   * */
  TopDUContext* chainForDocument(const KUrl& document, const ParsingEnvironment* environment, TopDUContext::Flags flags = TopDUContext::AnyFlag) const;

  /**
   * Find a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   * @param flags If this is TopDUContext::AnyFlag, context-flags will be ignored while searching.
   *              Else a context will be searched that exactly matches the given flags.
   *
   * Prefer this over the KUrl version.
   * */
  TopDUContext* chainForDocument(const IndexedString& document, const ParsingEnvironment* environment, TopDUContext::Flags flags = TopDUContext::AnyFlag) const;

  ///Returns the top-context that has the given index assigned, or zero if it doesn't exist. @see TopDUContext::ownIndex
  TopDUContext* chainForIndex(uint index) const;

  /// Only used for debugging at the moment
  QList<KUrl> documents() const;

  /**
   * Registers a new definition-use \a chain for the given \a document.
   */
  void addDocumentChain(TopDUContext* chain);

  /// Returns the global static instance.
  static DUChain* self();

  /// Returns the structure that manages mapping between definitions and declarations
  static Definitions* definitions();

  /// Returns the structure that manages mapping between declarations, and which top level contexts contain uses of them.
  static Uses* uses();

  /**
   * Retrieve the read write lock for the entire definition-use chain.
   * To call non-const methods, you must be holding a write lock.
   *
   * Evaluations made prior to holding a lock (including which objects
   * exist) must be verified once the lock is held, as they may have changed
   * or been deleted.
   *
   * \threadsafe
   */
  static DUChainLock* lock();

  /// Returns the object which emits signals regarding duchain changes
  static DUChainObserver* notifier();

  // Distribute the notifications
  /*static void contextChanged(DUContext* context, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void declarationChanged(Declaration* declaration, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void definitionChanged(Definition* definition, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void useChanged(Use* use, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);*/

  /// Notify that a branch was added to \a context
  static void branchAdded(DUContext* context);
  /// Notify that a branch was modified within \a context
  static void branchModified(DUContext* context);
  /// Notify that a branch was removed from \a context
  static void branchRemoved(DUContext* context);

  /// Returns whether the top-context with the given index is currently loaded in memory
  bool isInMemory(uint topContextIndex) const;
  
  /**
   * Changes the environment attached to the given top-level context, and updates the management-structures to reflect that
   * */
  void updateContextEnvironment( TopDUContext* context, ParsingEnvironmentFile* file );

  ///Allocates a new identity for a new top-context, no lock needed. The returned value is never zero
  static uint newTopContextIndex();

public Q_SLOTS:
  ///Removes the given top-context from the duchain, and deletes it.
  void removeDocumentChain(TopDUContext* document);

private Q_SLOTS:
  void documentActivated(KDevelop::IDocument* doc);
  void documentAboutToBeDeleted(KTextEditor::Document* doc);
  void documentLoadedPrepare(KDevelop::IDocument* document);
  void aboutToQuit();
private:
  void addToEnvironmentManager( TopDUContext * chain );
  void removeFromEnvironmentManager( TopDUContext * chain );
  DUChain();
  ~DUChain();

  friend class DUChainPrivate;
};

}

#endif // DUCHAIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
