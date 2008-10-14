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
typedef KSharedPtr<ParsingEnvironmentFile> ParsingEnvironmentFilePointer;
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
  Q_SCRIPTABLE QList<TopDUContext*> allChains() const;

  /**
   * Makes sure the given top-context is up-to-date, and has at least the given features.
   * This may trigger a parsing in background, so a QObject can be given that will be notified
   * asyonchronously once the update is ready.
   *
   * @param topContext The context to update
   * @param features The requested features
   * @param notifyReady An optional pointer to a QObject that should contain a slot
   *                    "void topContextUpdated(KDevelop::ReferencedTopDUContext topContext)" that will
   *                    be invoked once the top-context is updated, using a qeuued connection.
   * @return Whether an update was scheduled. If false is returned, the top-context is already up-to-date, and has the needed features.
   */
//   Q_SCRIPTABLE bool updateContextForUrl(const IndexedString& document, TopDUContext::Features minFeatures, QObject* notifyReady = 0) const;
  
  /**
   * Return any chain for the given document
   * If available, the version accepting IndexedString should be used instead of this, for performance reasons.
   * When no fitting chain is in memory, one may be loaded from disk.
   * */
  Q_SCRIPTABLE TopDUContext* chainForDocument(const KUrl& document) const;
  Q_SCRIPTABLE TopDUContext* chainForDocument(const IndexedString& document) const;

  /**
   * Return all chains for the given document that are currently in memory.
   * This does not load any chains from disk.
   * */
  Q_SCRIPTABLE QList<TopDUContext*> chainsForDocument(const KUrl& document) const;

  /**
   * Return all chains for the given document that are currently in memory.
   * This does not load any chains from disk.
   * Should be preferred over the KUrl version.
   * */
  Q_SCRIPTABLE QList<TopDUContext*> chainsForDocument(const IndexedString& document) const;

  /**
   * Find a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   * @param onlyProxyContexts If this is true, only contexts are found that have an ParsingEnvironmentFile that has the proxy-flag set.
   * */
  Q_SCRIPTABLE TopDUContext* chainForDocument(const KUrl& document, const ParsingEnvironment* environment, bool onlyProxyContexts = false, bool noProxyContexts = false) const;

  /**
   * Find a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   * @param onlyProxyContexts If this is true, only contexts are found that have an ParsingEnvironmentFile that has the proxy-flag set.
   *
   * Prefer this over the KUrl version.
   * */
  Q_SCRIPTABLE TopDUContext* chainForDocument(const IndexedString& document, const ParsingEnvironment* environment, bool onlyProxyContexts = false, bool noProxyContexts = false) const;

  /**
   * Find the environment-file of a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   *
   * This should be preferred over chainForDocument when only the environment-info is needed, because the TopDUContext is not loaded in this function.
   * 
   ** @param onlyProxyContexts If this is true, only contexts are found that have an ParsingEnvironmentFile that has the proxy-flag set.
   *
   * Prefer this over the KUrl version.
   * */
  Q_SCRIPTABLE ParsingEnvironmentFilePointer environmentFileForDocument(const IndexedString& document, const ParsingEnvironment* environment, bool onlyProxyContexts = false, bool noProxyContexts = false) const;  
  
  ///Returns the top-context that has the given index assigned, or zero if it doesn't exist. @see TopDUContext::ownIndex
  Q_SCRIPTABLE TopDUContext* chainForIndex(uint index) const;

  ///Returns the url for the given top-context index if available. This does have some cost, so avoid it when possible.
  Q_SCRIPTABLE IndexedString urlForIndex(uint index) const;
  
  /// Only used for debugging at the moment
  Q_SCRIPTABLE QList<KUrl> documents() const;

  /**
   * Registers a new definition-use \a chain for the given \a document.
   */
  Q_SCRIPTABLE void addDocumentChain(TopDUContext* chain);

  /// Returns the global static instance.
  Q_SCRIPTABLE static DUChain* self();

  /// Returns the structure that manages mapping between definitions and declarations
  Q_SCRIPTABLE static Definitions* definitions();

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
  Q_SCRIPTABLE static DUChainLock* lock();

  /// Returns the object which emits signals regarding duchain changes
  Q_SCRIPTABLE static DUChainObserver* notifier();

  // Distribute the notifications
  /*static void contextChanged(DUContext* context, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void declarationChanged(Declaration* declaration, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void definitionChanged(Definition* definition, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);
  static void useChanged(Use* use, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject = 0);*/

  /// Notify that a branch was added to \a context
  Q_SCRIPTABLE static void branchAdded(DUContext* context);
  /// Notify that a branch was modified within \a context
  Q_SCRIPTABLE static void branchModified(DUContext* context);
  /// Notify that a branch was removed from \a context
  Q_SCRIPTABLE static void branchRemoved(DUContext* context);

  /// Returns whether the top-context with the given index is currently loaded in memory
  Q_SCRIPTABLE bool isInMemory(uint topContextIndex) const;
  
  /**
   * Changes the environment attached to the given top-level context, and updates the management-structures to reflect that
   * */
  Q_SCRIPTABLE void updateContextEnvironment( TopDUContext* context, ParsingEnvironmentFile* file );

  ///Allocates a new identity for a new top-context, no lock needed. The returned value is never zero
  static uint newTopContextIndex();
  
Q_SIGNALS:
  ///Is emitted when the declaration has been selected somewhere in the user-interface, for example in the completion-list
  void declarationSelected(DeclarationPointer decl);
public Q_SLOTS:
  ///Removes the given top-context from the duchain, and deletes it.
  void removeDocumentChain(TopDUContext* document);
  ///Emits the declarationSelected signal, so other parties can notice it.
  void emitDeclarationSelected(DeclarationPointer decl);

private Q_SLOTS:
  void documentActivated(KDevelop::IDocument* doc);
  void documentAboutToBeDeleted(KTextEditor::Document* doc);
  void documentLoadedPrepare(KDevelop::IDocument* document);
  void aboutToQuit();
private:
  /// Increases the reference-count for the given top-context. The result: It will not be unloaded.
  /// Do this to prevent KDevelop from unloading a top-context that you plan to use. Don't forget calling unReferenceToContext again,
  /// else the top-context will stay in memory for ever.
  void refCountUp(TopDUContext* top);

  /// Decreases the reference-count for the given top-context. When it reaches zero, KDevelop is free to unload it at any time,
  /// also invalidating all the contained declarations and contexts.
  void refCountDown(TopDUContext* top);
  
  void addToEnvironmentManager( TopDUContext * chain );
  void removeFromEnvironmentManager( TopDUContext * chain );
  DUChain();
  ~DUChain();

  friend class DUChainPrivate;
  friend class ReferencedTopDUContext;
};

}

#endif // DUCHAIN_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
