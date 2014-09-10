/* This file is part of KDevelop
    Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_DUCHAIN_H
#define KDEVPLATFORM_DUCHAIN_H

#include <QtCore/QObject>

#include "topducontext.h"
#include "parsingenvironment.h"

class QUrl;

namespace KDevelop
{

class IDocument;
class TopDUContext;
class DUChainLock;

class ParsingEnvironmentManager;
class ParsingEnvironment;
class ParsingEnvironmentFile;
typedef QExplicitlySharedDataPointer<ParsingEnvironmentFile> ParsingEnvironmentFilePointer;
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
   * Initializes common static item repositories.
   * Must be called once for multi threaded applications to work reliably.
   */
  static void initialize();

  /**
   * Return a list of all chains available
   */
  Q_SCRIPTABLE QList<TopDUContext*> allChains() const;

  /**
   * Makes sure the standard-context for the given url is up-to-date.
   * This may trigger a parsing in background, so a QObject can be given that will be notified
   * asyonchronously once the update is ready.
   * If the context is already up to date, the given QObject is notified directly.
   *
   * @param document Document to update
   * @param features The requested features. If you want to force a full update of the context, give TopDUContext::ForceUpdate.
   *                 If you want to force an update including all imports, use TopDUContext::ForceUpdateRecursive.
   * @param notifyReady An optional pointer to a QObject that should contain a slot
   *                    "void updateReady(KDevelop::IndexedString url, KDevelop::ReferencedTopDUContext topContext)".
   *                    The notification is guaranteed to be called once for each call to updateContextForUrl. The given top-context
   *                    may be invalid if the update failed. A queued connection is used if a re-parse has to be done. The duchain
   *                    will _not_ be locked when updateReady is called.
   * @param priority An optional priority for the job. The lower the value, the higher it's priority.
   * @note The duchain must _not_ be locked when this is called!
   */
   Q_SCRIPTABLE void updateContextForUrl(const IndexedString& document, TopDUContext::Features minFeatures, QObject* notifyReady = 0, int priority = 1) const;
  
   /**
    * Convenience-function similar to updateContextForUrl that blocks this thread until the update of the given document is ready,
    * and returns the top-context.
    * @param document The document to update
    * @param features The requested features. If you want to force a full update of the context, give TopDUContext::ForceUpdate.
    *                 If you want to force an update including all imports, use TopDUContext::ForceUpdateRecursive.
    * @return The up-to-date top-context, or zero if the update failed
    *
    * @note The duchain must _not_ be locked when this is called!
    *
    */
   KDevelop::ReferencedTopDUContext waitForUpdate(const KDevelop::IndexedString& document, KDevelop::TopDUContext::Features minFeatures, bool proxyContext = false);
   
  /**
   * Return any chain for the given document
   * If available, the version accepting IndexedString should be used instead of this, for performance reasons.
   * When no fitting chain is in memory, one may be loaded from disk.
   *
   * @note The duchain must be at least read-locked locked when this is called!
   * */
  Q_SCRIPTABLE TopDUContext* chainForDocument(const QUrl& document, bool proxyContext = false) const;
  Q_SCRIPTABLE TopDUContext* chainForDocument(const IndexedString& document, bool proxyContext = false) const;

  /**
   * Return all chains for the given document that are currently in memory.
   * This does not load any chains from disk.
   * */
  Q_SCRIPTABLE QList<TopDUContext*> chainsForDocument(const QUrl& document) const;

  /**
   * Return all chains for the given document that are currently in memory.
   * This does not load any chains from disk.
   * Should be preferred over the QUrl version.
   * */
  Q_SCRIPTABLE QList<TopDUContext*> chainsForDocument(const IndexedString& document) const;

  /**
   * Find a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   * @param proxyContext If this is true, only contexts are found that have an ParsingEnvironmentFile that has the proxy-flag set. Else, only content-contexts will be returned.
   *
   * @note The duchain must be at least read-locked locked when this is called!
   * */
  Q_SCRIPTABLE TopDUContext* chainForDocument(const QUrl& document, const ParsingEnvironment* environment, bool proxyContext = false) const;

  /**
   * Find a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   * @param proxyContext If this is true, only contexts are found that have an ParsingEnvironmentFile that has the proxy-flag set. Else, only content-contexts will be returned.
   *
   * Prefer this over the QUrl version.
   *
   * @note The duchain must be at least read-locked locked when this is called!
   * */
  Q_SCRIPTABLE TopDUContext* chainForDocument(const IndexedString& document, const ParsingEnvironment* environment, bool proxyContext = false) const;

  /**
   * Find the environment-file of a chain that fits into the given environment. If no fitting chain is found, 0 is returned.
   * When no fitting chain is in memory, one may be loaded from disk.
   *
   * This should be preferred over chainForDocument when only the environment-info is needed, because the TopDUContext is not loaded in this function.
   * 
   ** @param proxyContext If this is true, only contexts are found that have an ParsingEnvironmentFile that has the proxy-flag set. Else, only content-contexts will be returned.
   *
   * Prefer this over the QUrl version.
   *
   * @note The duchain must be at least read-locked locked when this is called!
   * */
  Q_SCRIPTABLE ParsingEnvironmentFilePointer environmentFileForDocument(const IndexedString& document, const ParsingEnvironment* environment, bool proxyContext = false) const;  

  Q_SCRIPTABLE ParsingEnvironmentFilePointer environmentFileForDocument(IndexedTopDUContext topContext) const;  
  
  /**
   * Returns the list of the environment-infos of all versions of the given document.
   */
  Q_SCRIPTABLE QList<ParsingEnvironmentFilePointer> allEnvironmentFiles(const IndexedString& document);
  
  ///Returns the top-context that has the given index assigned, or zero if it doesn't exist. @see TopDUContext::ownIndex
  ///The duchain must be read-locked when this is called
  ///This function is inlined because it is called in a very high frequency
  Q_SCRIPTABLE inline TopDUContext* chainForIndex(uint index)
  {
    if(m_deleted)
      return 0;

    {
      SpinLock<> lock(chainsByIndexLock);

      if(chainsByIndex.size() > index)
      {
        TopDUContext* top = chainsByIndex[index];
        if(top)
          return top;
      }
    }
    
    //Load the top-context
    return loadChain(index);
  }

  ///Returns the url for the given top-context index if available. This does have some cost, so avoid it when possible.
  Q_SCRIPTABLE IndexedString urlForIndex(uint index) const;
  
  /// Only used for debugging at the moment
  Q_SCRIPTABLE QList<QUrl> documents() const;

  /// Only used for debugging at the moment
  /// Prefer that over the QUrl version for performance reasons
  Q_SCRIPTABLE QList<IndexedString> indexedDocuments() const;

  /**
   * Registers a new definition-use \a chain for the given \a document.
   */
  Q_SCRIPTABLE void addDocumentChain(TopDUContext* chain);

  /// Returns true if the global duchain instance has already been deleted
  Q_SCRIPTABLE static bool deleted();
  
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

  /// Returns whether the top-context with the given index is currently loaded in memory
  Q_SCRIPTABLE bool isInMemory(uint topContextIndex) const;
  
  /**
   * Changes the environment attached to the given top-level context, and updates the management-structures to reflect that
   * */
  Q_SCRIPTABLE void updateContextEnvironment( TopDUContext* context, ParsingEnvironmentFile* file );

  ///Allocates a new identity for a new top-context, no lock needed. The returned value is never zero
  static uint newTopContextIndex();
  
  ///If you call this, the persistent disk-storage structure will stay unaffected, and no duchain cleanup will be done.
  ///Call this from within tests.
  void disablePersistentStorage(bool disable = true);
  
  ///Stores the whole duchain and all its repositories in the current state to disk
  ///The duchain must not be locked in any way
  void storeToDisk();
  
  ///Compares the whole duchain and all its repositories in the current state to disk
  ///When the comparison fails, debug-output will show why
  ///The duchain must not be locked when calling this
  ///@return true If the current memory state equals the disk state, else false
  bool compareToDisk();

Q_SIGNALS:
  ///Is emitted when the declaration has been selected somewhere in the user-interface, for example in the completion-list
  void declarationSelected(const KDevelop::DeclarationPointer& decl);

public Q_SLOTS:
  ///Removes the given top-context from the duchain, and deletes it.
  void removeDocumentChain(KDevelop::TopDUContext* document);
  ///Emits the declarationSelected signal, so other parties can notice it.
  void emitDeclarationSelected(const KDevelop::DeclarationPointer& decl);

  /**
   * Shutdown and cleanup the DUChain.
   */
  void shutdown();

private Q_SLOTS:
  void documentActivated(KDevelop::IDocument* doc);
  void documentLoadedPrepare(KDevelop::IDocument* document);
  void documentRenamed(KDevelop::IDocument* document);
  void documentClosed(KDevelop::IDocument*);

private:
  TopDUContext* loadChain(uint index);
  //These two are exported here so that the extremely frequently called chainForIndex(..) can be inlined
  static bool m_deleted;
  static std::vector<TopDUContext*> chainsByIndex;
  static SpinLockData chainsByIndexLock;
  
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

#endif // KDEVPLATFORM_DUCHAIN_H
