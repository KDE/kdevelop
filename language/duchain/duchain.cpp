/* This  is part of KDevelop
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

#include "duchain.h"
#include "duchainlock.h"

#include <QtCore/QCoreApplication>
#include <QApplication>
#include <QtCore/QHash>
#include <QtCore/QMultiMap>
#include <QtCore/QTimer>
#include <QtCore/QReadWriteLock>
#include <QtCore/qatomic.h>

#include <kglobal.h>

#include <KDE/KTextEditor/Document>
#include <KDE/KTextEditor/SmartInterface>

#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguage.h>
#include <interfaces/ilanguagecontroller.h>

#include "../editor/editorintegrator.h"
#include "../interfaces/ilanguagesupport.h"
#include "../interfaces/icodehighlighting.h"
#include "../backgroundparser/backgroundparser.h"

#include "topducontext.h"
#include "topducontextdata.h"
#include "topducontextdynamicdata.h"
#include "parsingenvironment.h"
#include "declaration.h"
#include "definitions.h"
#include "duchainutils.h"
#include "use.h"
#include "uses.h"
#include "abstractfunctiondeclaration.h"
#include "smartconverter.h"
#include "duchainregister.h"
#include "persistentsymboltable.h"
#include "repositories/itemrepository.h"
#include <util/google/dense_hash_map>
#include <QtCore/qthread.h>
#include <QtCore/qwaitcondition.h>
#include <QtCore/qmutex.h>
#include <unistd.h>
#include "waitforupdate.h"
#include "referencecounting.h"

Q_DECLARE_METATYPE(KDevelop::IndexedString)
Q_DECLARE_METATYPE(KDevelop::IndexedTopDUContext)
Q_DECLARE_METATYPE(KDevelop::ReferencedTopDUContext)

namespace {
//Additional "soft" cleanup steps that are done before the actual cleanup.
//During "soft" cleanup, the consistency is not guaranteed. The repository is
//marked to be updating during soft cleanup, so if kdevelop crashes, it will be cleared.
//The big advantage of the soft cleanup steps is, that the duchain is always only locked for
//short times, which leads to no lockup in the UI.
const int SOFT_CLEANUP_STEPS = 1;

const uint cleanupEverySeconds = 200;

///Approximate maximum count of top-contexts that are checked during final cleanup
const uint maxFinalCleanupCheckContexts = 2000;
const uint minimumFinalCleanupCheckContextsPercentage = 10; //Check at least n% of all top-contexts during cleanup
//Set to true as soon as the duchain is deleted
bool duChainDeleted = false;
}

namespace KDevelop
{
//This thing is not actually used, but it's needed for compiling
DEFINE_LIST_MEMBER_HASH(EnvironmentInformationListItem, items, uint)

//An entry for the item-repository that holds some meta-data. Behind this entry, the actual ParsingEnvironmentFileData is stored.
class EnvironmentInformationItem {
  public:
  EnvironmentInformationItem(uint topContext, uint size) : m_topContext(topContext), m_size(size) {
  }

  ~EnvironmentInformationItem() {
  }

  unsigned int hash() const {
    return m_topContext;
  }

  unsigned int itemSize() const {
    return sizeof(*this) + m_size;
  }

  uint m_topContext;
  uint m_size;//Size of the data behind, that holds the actual item
};

struct ItemRepositoryIndexHash
{
  size_t
  operator()(unsigned int __x) const
  { return 173*(__x>>2) + 11 * (__x >> 16); }
};


class EnvironmentInformationRequest {
  public:

  ///This constructor should only be used for lookup
  EnvironmentInformationRequest(uint topContextIndex) : m_file(0), m_index(topContextIndex) {
  }

  EnvironmentInformationRequest(const ParsingEnvironmentFile* file) : m_file(file), m_index(file->indexedTopContext().index()) {
  }

  enum {
    AverageSize = 32 //This should be the approximate average size of an Item
  };

  unsigned int hash() const {
    return m_index;
  }

  size_t itemSize() const {
    return sizeof(EnvironmentInformationItem) + DUChainItemSystem::self().dynamicSize(*m_file->d_func());
  }

  void createItem(EnvironmentInformationItem* item) const {
    new (item) EnvironmentInformationItem(m_index, DUChainItemSystem::self().dynamicSize(*m_file->d_func()));
    Q_ASSERT(m_file->d_func()->m_dynamic);
    DUChainItemSystem::self().copy(*m_file->d_func(), *(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem)), true);
    Q_ASSERT((*(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem))).m_range == m_file->d_func()->m_range);
    Q_ASSERT((*(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem))).classId == m_file->d_func()->classId);
    Q_ASSERT((*(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem))).m_dynamic == false);
  }
  
  static void destroy(EnvironmentInformationItem* item, KDevelop::AbstractItemRepository&) {
    item->~EnvironmentInformationItem();
    //We don't need to call the destructor, because that's done in DUChainBase::makeDynamic()
    //We just need to make sure that every environment-file is dynamic when it's deleted
//     DUChainItemSystem::self().callDestructor((DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem)));
  }
  
  static bool persistent(const EnvironmentInformationItem* ) {
    //Cleanup done separately
    return true;
  }

  bool equals(const EnvironmentInformationItem* item) const {
    return m_index == item->m_topContext;
  }

  const ParsingEnvironmentFile* m_file;
  uint m_index;
};

///A list of environment-information/top-contexts mapped to a file-name
class EnvironmentInformationListItem {
  public:
  EnvironmentInformationListItem() {
    initializeAppendedLists(true);
  }
  
  EnvironmentInformationListItem(const EnvironmentInformationListItem& rhs, bool dynamic = true) {
    initializeAppendedLists(dynamic);
    m_file = rhs.m_file;
    copyListsFrom(rhs);
  }
  
  ~EnvironmentInformationListItem() {
    freeAppendedLists();
  }

  unsigned int hash() const {
    //We only compare the declaration. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return m_file.hash();
  }

  unsigned short int itemSize() const {
    return dynamicSize();
  }

  IndexedString m_file;

  uint classSize() const {
    return sizeof(*this);
  }

  START_APPENDED_LISTS(EnvironmentInformationListItem);
  ///Contains the index of each contained environment-item
  APPENDED_LIST_FIRST(EnvironmentInformationListItem, uint, items);
  END_APPENDED_LISTS(EnvironmentInformationListItem, items);
};

class EnvironmentInformationListRequest {
  public:

  ///This constructor should only be used for lookup
  EnvironmentInformationListRequest(const IndexedString& file) : m_file(file), m_item(0) {
  }
  ///This is used to actually construct the information in the repository
  EnvironmentInformationListRequest(const IndexedString& file, const EnvironmentInformationListItem& item) : m_file(file), m_item(&item) {
  }

  enum {
    AverageSize = 160 //This should be the approximate average size of an Item
  };

  unsigned int hash() const {
    return m_file.hash();
  }

  size_t itemSize() const {
    return m_item->itemSize();
  }

  void createItem(EnvironmentInformationListItem* item) const {
    Q_ASSERT(m_item->m_file == m_file);
    new (item) EnvironmentInformationListItem(*m_item, false);
  }
  
  static void destroy(EnvironmentInformationListItem* item, KDevelop::AbstractItemRepository&) {
    item->~EnvironmentInformationListItem();
  }

  static bool persistent(const EnvironmentInformationListItem*) {
    //Cleanup is done separately
    return true;
  }

  bool equals(const EnvironmentInformationListItem* item) const {
    return m_file == item->m_file;
  }

  IndexedString m_file;
  const EnvironmentInformationListItem* m_item;
};

class DUChainPrivate;
static DUChainPrivate* duChainPrivateSelf = 0;
class DUChainPrivate
{
  class CleanupThread : public QThread {
    public:
      CleanupThread(DUChainPrivate* data) : m_stopRunning(false), m_data(data) {
      }

      void stopThread() {
        m_waitMutex.lock();
        m_stopRunning = true;
        m_wait.wakeAll(); //Wakes the thread up, so it notices it should exit
        m_waitMutex.unlock();
        wait();
      }

    private:
      void run() {
        while(1) {
          m_waitMutex.lock();
          if(m_stopRunning)
            break;
          m_wait.wait(&m_waitMutex, 1000 * cleanupEverySeconds);
          m_waitMutex.unlock();
          if(m_stopRunning)
            break;

          //Just to make sure the cache is cleared periodically
          ModificationRevisionSet::clearCache();

          m_data->doMoreCleanup(SOFT_CLEANUP_STEPS);
          if(m_stopRunning)
            break;
        }
      }
      bool m_stopRunning;
      QWaitCondition m_wait;
      QMutex m_waitMutex;
      DUChainPrivate* m_data;
  };
public:
  DUChainPrivate() : m_chainsMutex(QMutex::Recursive), instance(0), m_cleanupDisabled(false), m_destroyed(false), m_environmentListInfo("Environment Lists"), m_environmentInfo("Environment Information")
  {
#if defined(TEST_NO_CLEANUP)
    m_cleanupDisabled = true;
#endif

    m_chainsByIndex.set_empty_key(0);
    m_chainsByIndex.set_deleted_key(0xffffffff);
    duChainPrivateSelf = this;
    qRegisterMetaType<DUChainBasePointer>("KDevelop::DUChainBasePointer");
    qRegisterMetaType<DUContextPointer>("KDevelop::DUContextPointer");
    qRegisterMetaType<TopDUContextPointer>("KDevelop::TopDUContextPointer");
    qRegisterMetaType<DeclarationPointer>("KDevelop::DeclarationPointer");
    qRegisterMetaType<FunctionDeclarationPointer>("KDevelop::FunctionDeclarationPointer");
    qRegisterMetaType<KDevelop::IndexedString>("KDevelop::IndexedString");
    qRegisterMetaType<KDevelop::IndexedTopDUContext>("KDevelop::IndexedTopDUContext");
    qRegisterMetaType<KDevelop::ReferencedTopDUContext>("KDevelop::ReferencedTopDUContext");

    notifier = new DUChainObserver();
    instance = new DUChain();
    m_cleanup = new CleanupThread(this);
    m_cleanup->start();
    
    duChainDeleted = false;
  }
  ~DUChainPrivate() {
    duChainDeleted = true;
    m_cleanup->stopThread();
    delete m_cleanup;
    delete instance;
  }

  void clear() {

    if(!m_cleanupDisabled)
      doMoreCleanup();
    
    QMutexLocker l(&m_chainsMutex);

    DUChainWriteLocker writeLock(DUChain::lock());

    for(google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = m_chainsByIndex.begin(); it != m_chainsByIndex.end(); ++it)
      removeDocumentChainFromMemory((*it).second);

    m_indexEnvironmentInformations.clear();
    m_fileEnvironmentInformations.clear();

    Q_ASSERT(m_fileEnvironmentInformations.isEmpty());
    Q_ASSERT(m_chainsByUrl.isEmpty());
    Q_ASSERT(m_chainsByIndex.empty());
  }
  
  ///DUChain must be write-locked
  ///Also removes from the environment-manager if the top-context is not on disk
  void removeDocumentChainFromMemory(TopDUContext* context) {
    QMutexLocker l(&m_chainsMutex);

    {
      QMutexLocker l(&m_referenceCountsMutex);

      if(m_referenceCounts.contains(context)) {
      //This happens during shutdown, since everything is unloaded
      kDebug() << "removed a top-context that was reference-counted:" << context->url().str() << context->ownIndex();
      m_referenceCounts.remove(context);
      }
    }

    uint index = context->ownIndex();

  //   kDebug(9505) << "duchain: removing document" << context->url().str();
    google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::iterator it = m_chainsByIndex.find(index);
    
    Q_ASSERT(it != m_chainsByIndex.end());
    m_chainsByUrl.remove(context->url(), context);

    if (context->smartRange())
      ICore::self()->languageController()->backgroundParser()->removeManagedTopRange(context->smartRange());

    if(!context->isOnDisk())
      instance->removeFromEnvironmentManager(context);

    l.unlock();
    //DUChain is write-locked, so we can do whatever we want on the top-context, including deleting it
    context->deleteSelf();
    l.relock();
    it = m_chainsByIndex.find(index); //May have been changed due to loading
    Q_ASSERT(it != m_chainsByIndex.end());
    m_chainsByIndex.erase(it);
    
    Q_ASSERT(m_chainsByIndex.find(index) == m_chainsByIndex.end());
  }

  ///Must be locked before accessing content of this class.
  ///Should be released during expensive disk-operations and such.
  QMutex m_chainsMutex;

  CleanupThread* m_cleanup;

  DUChain* instance;
  DUChainLock lock;
  QMultiMap<IndexedString, TopDUContext*> m_chainsByUrl;
  google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash> m_chainsByIndex;

  //Must be locked before accessing m_referenceCounts
  QMutex m_referenceCountsMutex;
  QHash<TopDUContext*, uint> m_referenceCounts;

  DUChainObserver* notifier;
  Definitions m_definitions;
  Uses m_uses;
  QSet<uint> m_loading;
  bool m_cleanupDisabled;

  ///Used to keep alive the top-context that belong to documents loaded in the editor
  QSet<ReferencedTopDUContext> m_openDocumentContexts;

  bool m_destroyed;

  ///The item must not be stored yet
  ///m_chainsMutex should not be locked, since this can trigger I/O
  void addEnvironmentInformation(ParsingEnvironmentFilePointer info) {
    Q_ASSERT(!findInformation(info->indexedTopContext().index()));
    Q_ASSERT(m_environmentInfo.findIndex(info->indexedTopContext().index()) == 0);

    QMutexLocker lock(&m_chainsMutex);
    m_fileEnvironmentInformations.insert(info->url(), info);
    
    m_indexEnvironmentInformations.insert(info->indexedTopContext().index(), info);
    
    Q_ASSERT(info->d_func()->classId);
  }

  ///The item must be managed currently
  ///m_chainsMutex does not need to be locked
  void removeEnvironmentInformation(ParsingEnvironmentFilePointer info) {

    info->makeDynamic(); //By doing this, we make sure the data is actually being destroyed in the destructor

    m_chainsMutex.lock();
    bool removed = m_fileEnvironmentInformations.remove(info->url(), info);
    
    bool removed2 = m_indexEnvironmentInformations.remove(info->indexedTopContext().index());
    
    m_chainsMutex.unlock();
    
    {
      //Remove it from the environment information lists if it was there
      QMutexLocker lock(m_environmentListInfo.mutex());
      uint index = m_environmentListInfo.findIndex(info->url());
      
      if(index) {
        EnvironmentInformationListItem item(*m_environmentListInfo.itemFromIndex(index));
        if(item.itemsList().removeOne(info->indexedTopContext().index())) {
          m_environmentListInfo.deleteItem(index);
          if(!item.itemsList().isEmpty())
            m_environmentListInfo.index(EnvironmentInformationListRequest(info->url(), item));
        }
      }
    }
    
    QMutexLocker lock(m_environmentInfo.mutex());
    uint index = m_environmentInfo.findIndex(info->indexedTopContext().index());
    if(index) {
      m_environmentInfo.deleteItem(index);
    }

kDebug() << index << removed << removed2;
    Q_ASSERT(index || (removed && removed2));
    Q_ASSERT(!findInformation(info->indexedTopContext().index()));
  }

  ///m_chainsMutex should _not_ be locked, because this may trigger I/O
  QList<ParsingEnvironmentFilePointer> getEnvironmentInformation(IndexedString url) {
    QList<ParsingEnvironmentFilePointer> ret;
    uint listIndex = m_environmentListInfo.findIndex(url);

    if(listIndex) {
      KDevVarLengthArray<uint> topContextIndices;
      
      {
        //First store all the possible intices into the KDevVarLengthArray, so we can unlock the mutex before processing them.
        
        QMutexLocker lock(m_environmentListInfo.mutex()); //Lock the mutex to make sure the item isn't changed while it's being iterated
        const EnvironmentInformationListItem* item = m_environmentListInfo.itemFromIndex(listIndex);
        FOREACH_FUNCTION(uint topContextIndex, item->items)
          topContextIndices << topContextIndex;
      }
      
      //Process the indices in a separate step after copying them from the array, so we don't need m_environmentListInfo.mutex locked,
      //and can call loadInformation(..) safely, which else might lead to a deadlock.
      FOREACH_ARRAY(uint topContextIndex, topContextIndices) {
        KSharedPtr< ParsingEnvironmentFile > p = ParsingEnvironmentFilePointer(loadInformation(topContextIndex));
        if(p) {
         ret << p;
        }else{
          kDebug() << "Failed to load enviromment-information for" << TopDUContextDynamicData::loadUrl(topContextIndex).str();
        }
      }
    }

    QMutexLocker l(&m_chainsMutex);
    
    //Add those information that have not been added to the stored lists yet
    foreach(ParsingEnvironmentFilePointer file, m_fileEnvironmentInformations.values(url))
      if(!ret.contains(file))
        ret << file;

    return ret;
  }

  ///Makes sure that the chain with the given index is loaded
  ///@warning m_chainsMutex must NOT be locked when this is called
  void loadChain(uint index, QSet<uint>& loaded) {

    QMutexLocker l(&m_chainsMutex);

    if(m_chainsByIndex.find(index) == m_chainsByIndex.end()) {
      if(m_loading.contains(index)) {
        //It's probably being loaded by another thread. So wait until the load is ready
        while(m_loading.contains(index)) {
          l.unlock();
          kDebug() << "waiting for another thread to load index" << index;
          usleep(50000);
          l.relock();
        }
        loaded.insert(index);
        return;
      }
      m_loading.insert(index);
      loaded.insert(index);
      
      l.unlock();
      kDebug() << "loading top-context" << index;
      TopDUContext* chain = TopDUContextDynamicData::load(index);
      if(chain) {
        chain->setParsingEnvironmentFile(loadInformation(chain->ownIndex()));

        if(!chain->usingImportsCache()) {
          //Eventually also load all the imported chains, so the import-structure is built
          foreach(const DUContext::Import &import, chain->DUContext::importedParentContexts()) {
            if(!loaded.contains(import.topContextIndex())) {
              loadChain(import.topContextIndex(), loaded);
            }
          }
         }
        chain->rebuildDynamicImportStructure();

        chain->setInDuChain(true);
        instance->addDocumentChain(chain);
      }
      
      l.relock();
      m_loading.remove(index);
    }
  }


  ///Stores all environment-information
  ///Also makes sure that all information that stays is referenced, so it stays alive.
  ///@param atomic If this is false, the write-lock will be released time by time
  void storeAllInformation(bool atomic, DUChainWriteLocker& locker) {

    uint cnt = 0;

    QList<IndexedString> urls;
    {
      QMutexLocker lock(&m_chainsMutex);
      urls += m_fileEnvironmentInformations.keys();
    }

    foreach(const IndexedString &url, urls) {
      QList<ParsingEnvironmentFilePointer> check;
      {
        QMutexLocker lock(&m_chainsMutex);
        check = m_fileEnvironmentInformations.values(url);
      }

      foreach(ParsingEnvironmentFilePointer file, check) {

        EnvironmentInformationRequest req(file.data());
        QMutexLocker lock(m_environmentInfo.mutex());
        uint index = m_environmentInfo.findIndex(req);

        if(file->d_func()->isDynamic()) {
          //This item has been changed, or isn't in the repository yet

          //Eventually remove an old entry
          if(index)
            m_environmentInfo.deleteItem(index);

          //Add the new entry to the item repository
          index = m_environmentInfo.index(req);
          Q_ASSERT(index);

          EnvironmentInformationItem* item = const_cast<EnvironmentInformationItem*>(m_environmentInfo.itemFromIndex(index));
          DUChainBaseData* theData = (DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem));
          static DUChainBaseData* dataCopy;
          dataCopy = theData;

          Q_ASSERT(theData->m_range == file->d_func()->m_range);
          Q_ASSERT(theData->m_dynamic == false);
          Q_ASSERT(theData->classId == file->d_func()->classId);

          file->setData( theData );

          ++cnt;
        }else{
          m_environmentInfo.itemFromIndex(index); //Prevent unloading of the data, by accessing the item
        }
      }

      ///We must not release the lock while holding a reference to a ParsingEnvironmentFilePointer, else we may miss the deletion of an
      ///information, and will get crashes.
      if(!atomic && (cnt % 100 == 0)) {
          //Release the lock on a regular basis
          locker.unlock();
          locker.lock();
      }

      storeInformationList(url);

      //Access the data in the repository, so the bucket isn't unloaded
      uint index = m_environmentListInfo.findIndex(EnvironmentInformationListRequest(url));
      if(index) {
        m_environmentListInfo.itemFromIndex(index);
      }else{
        QMutexLocker lock(&m_chainsMutex);
        kDebug(9505) << "Did not find stored item for" << url.str() << "count:" << m_fileEnvironmentInformations.values(url);
      }
      if(!atomic) {
        locker.unlock();
        locker.lock();
      }
    }
  }
  
  QMutex& cleanupMutex() {
    static QMutex mutex(QMutex::Recursive);
    return mutex;
  }

  ///@param retries When this is nonzero, then doMoreCleanup will do the specified amount of cycles
  ///doing the cleanup without permanently locking the du-chain. During these steps the consistency
  ///of the disk-storage is not guaranteed, but only few changes will be done during these steps,
  ///so the final step where the duchain is permanently locked is much faster.
  void doMoreCleanup(int retries = 0, bool needLockRepository = true) {

    if(m_cleanupDisabled)
      return;

    //This mutex makes sure that there's never 2 threads at he same time trying to clean up
    QMutexLocker lockCleanupMutex(&cleanupMutex());

    if(m_destroyed || m_cleanupDisabled)
      return;
    
    Q_ASSERT(!instance->lock()->currentThreadHasReadLock() && !instance->lock()->currentThreadHasWriteLock());
    DUChainWriteLocker writeLock(instance->lock());
    PersistentSymbolTable::self().clearCache();

    //This is used to stop all parsing before starting to do the cleanup. This way less happens during the
    //soft cleanups, and we have a good chance that during the "hard" cleanup only few data has to be written.
    QList<ILanguage*> lockedParseMutexes;

    QList<QReadWriteLock*> locked;

    if(needLockRepository) {

      if (ICore* core = ICore::self())
        if (ILanguageController* lc = core->languageController())
          lockedParseMutexes = lc->loadedLanguages();

      writeLock.unlock();

      //Here we wait for all parsing-threads to stop their processing
      foreach(ILanguage* language, lockedParseMutexes) {
        language->parseLock()->lockForWrite();
        locked << language->parseLock();
      }

      writeLock.lock();

      globalItemRepositoryRegistry().lockForWriting();
      kDebug(9505) << "starting cleanup";
    }

    QTime startTime = QTime::currentTime();

    storeAllInformation(!retries, writeLock); //Puts environment-information into a repository

    //We don't need to increase the reference-count, since the cleanup-mutex is locked
    QSet<TopDUContext*> workOnContexts;

    {
      QMutexLocker l(&m_chainsMutex);
      
      for(google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = m_chainsByIndex.begin(); it != m_chainsByIndex.end(); ++it)
        workOnContexts << (*it).second;
    }

    foreach(TopDUContext* context, workOnContexts) {

      context->m_dynamicData->store();
      
      if(retries) {
        //Eventually give other threads a chance to access the duchain
        writeLock.unlock();
        //Sleep to give the other threads a realistic chance to get a read-lock in between
        usleep(500);
        writeLock.lock();
      }
    }

      //Unload all top-contexts that don't have a reference-count and that are not imported by a referenced one

      QSet<IndexedString> unloadedNames;
      bool unloadedOne = true;

      bool unloadAllUnreferenced = !retries;

      //Now unload contexts, but only ones that are not imported by any other currently loaded context
      //The complication: Since during the lock-break new references may be added, we must never keep
      //the du-chain in an invalid state. Thus we can only unload contexts that are not imported by any
      //currently loaded contexts. In case of loops, we have to unload everything at once.
      while(unloadedOne) {
        unloadedOne = false;
        int hadUnloadable = 0;

        unloadContexts:

        foreach(TopDUContext* unload, workOnContexts) {

          bool hasReference = false;

          {
            QMutexLocker l(&m_referenceCountsMutex);
            //Test if the context is imported by a referenced one
            foreach(TopDUContext* context, m_referenceCounts.keys()) {
              if(context == unload || context->imports(unload, SimpleCursor())) {
                workOnContexts.remove(unload);
                hasReference = true;
              }
            }
          }

          if(!hasReference)
            ++hadUnloadable; //We have found a context that is not referenced
          else
            continue; //This context is referenced

          bool isImportedByLoaded = !unload->loadedImporters().isEmpty();

          //If we unload a context that is imported by other contexts, we create a bad loaded state
          if(isImportedByLoaded && !unloadAllUnreferenced)
            continue;

          unloadedNames.insert(unload->url());
          //Since we've released the write-lock in between, we've got to call store() again to be sure that none of the data is dynamic
          //If nothing has changed, it is only a low-cost call.
          unload->m_dynamicData->store();
          Q_ASSERT(!unload->d_func()->m_dynamic);
          removeDocumentChainFromMemory(unload);
          workOnContexts.remove(unload);
          unloadedOne = true;

          if(!unloadAllUnreferenced) {
            //Eventually give other threads a chance to access the duchain
            writeLock.unlock();
            //Sleep to give the other threads a realistic chance to get a read-lock in between
            usleep(500);
            writeLock.lock();
          }
        }
        if(hadUnloadable && !unloadedOne) {
          Q_ASSERT(!unloadAllUnreferenced);
          //This can happen in case of loops. We have o unload everything at one time.
          kDebug(9505) << "found" << hadUnloadable << "unloadable contexts, but could not unload separately. Unloading atomically.";
          unloadAllUnreferenced = true;
          hadUnloadable = 0; //Reset to 0, so we cannot loop forever
          goto unloadContexts;
        }
      }


      if(retries == 0) {
        QMutexLocker lock(&m_chainsMutex);
        //Do this atomically, since we must be sure that _everything_ is already saved
        for(QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it = m_fileEnvironmentInformations.begin(); it != m_fileEnvironmentInformations.end(); ) {
          ParsingEnvironmentFile* f = (*it).data();
          Q_ASSERT(f->d_func()->classId);
          if(f->ref == 1) {
            Q_ASSERT(!f->d_func()->isDynamic()); //It cannot be dynamic, since we have stored before
            //The ParsingEnvironmentFilePointer is only referenced once. This means that it does not belong to any
            //loaded top-context, so just remove it to save some memory and processing time.
            ///@todo use some kind of timeout before removing
            it = m_fileEnvironmentInformations.erase(it);
          }else{
            ++it;
          }
        }
      }

      if(retries)
        writeLock.unlock();

      //This must be the last step, due to the on-disk reference counting
      globalItemRepositoryRegistry().store(); //Stores all repositories

      if(retries) {
        doMoreCleanup(retries-1, false);
        writeLock.lock();
      }

      if(needLockRepository) {
        globalItemRepositoryRegistry().unlockForWriting();

        int elapsedSeconds = startTime.secsTo(QTime::currentTime());
        kDebug(9505) << "seconds spent doing cleanup: " << elapsedSeconds << "top-contexts still open:" << m_chainsByIndex.size();;
      }
      if(!retries) {
        int elapesedMilliSeconds = startTime.msecsTo(QTime::currentTime());
        kDebug(9505) << "milliseconds spent doing cleanup with locked duchain: " << elapesedMilliSeconds;
      }

      foreach(QReadWriteLock* lock, locked)
        lock->unlock();
  }

  ///Checks whether the information is already loaded.
  ParsingEnvironmentFile* findInformation(uint topContextIndex) {
    QMutexLocker lock(&m_chainsMutex);
    QHash<uint, ParsingEnvironmentFilePointer>::iterator it = m_indexEnvironmentInformations.find(topContextIndex);
    if(it != m_indexEnvironmentInformations.end())
      return (*it).data();
    return 0;
  }

  ///Loads/gets the environment-information for the given top-context index, or returns zero if none exists
  ///@warning m_chainsMutex should NOT be locked when this is called, because it triggers I/O
  ///@warning no other mutexes should be locked, as that may lead to a dedalock
  ParsingEnvironmentFile* loadInformation(uint topContextIndex) {

    ParsingEnvironmentFile* alreadyLoaded = findInformation(topContextIndex);
    if(alreadyLoaded)
      return alreadyLoaded;

    //Step two: Check if it is on disk, and if is, load it
    uint dataIndex = m_environmentInfo.findIndex(EnvironmentInformationRequest(topContextIndex));
    if(!dataIndex) {
      //No environment-information stored for this top-context
      return 0;
    }

    const EnvironmentInformationItem& item(*m_environmentInfo.itemFromIndex(dataIndex));

    alreadyLoaded = findInformation(topContextIndex);
    if(alreadyLoaded)
      return alreadyLoaded;
    
    ParsingEnvironmentFile* ret = dynamic_cast<ParsingEnvironmentFile*>(DUChainItemSystem::self().create( (DUChainBaseData*)(((char*)&item) + sizeof(EnvironmentInformationItem)) ));
    if(ret) {
      QMutexLocker lock(&m_chainsMutex);
      Q_ASSERT(ret->d_func()->classId);
      Q_ASSERT(ret->indexedTopContext().index() == topContextIndex);
      ParsingEnvironmentFilePointer retPtr(ret);
      
      m_fileEnvironmentInformations.insert(ret->url(), retPtr);
      
      Q_ASSERT(!m_indexEnvironmentInformations.contains(ret->indexedTopContext().index()));
      m_indexEnvironmentInformations.insert(ret->indexedTopContext().index(), retPtr);
    }
    return ret;
  }
  
  struct CleanupListVisitor {
    QList<uint> checkContexts;
    bool operator()(const EnvironmentInformationItem* item) {
      checkContexts << item->m_topContext;
      return true;
    }
  };

  ///Will check a selection of all top-contexts for up-to-date ness, and remove them if out of date
  void cleanupTopContexts() {
    DUChainWriteLocker lock( DUChain::lock() );
    kDebug() << "cleaning top-contexts";
    CleanupListVisitor visitor;
    uint startPos = 0;
    m_environmentInfo.visitAllItems(visitor);
    
    int checkContextsCount = maxFinalCleanupCheckContexts;
    int percentageOfContexts = (visitor.checkContexts.size() * 100) / minimumFinalCleanupCheckContextsPercentage;
    
    if(checkContextsCount < percentageOfContexts)
      checkContextsCount = percentageOfContexts;
    
    if(visitor.checkContexts.size() > (int)checkContextsCount)
      startPos = qrand() % (visitor.checkContexts.size() - checkContextsCount);
    
    int endPos = startPos + maxFinalCleanupCheckContexts;
    if(endPos > visitor.checkContexts.size())
      endPos = visitor.checkContexts.size();
    QSet< uint > check;
    for(int a = startPos; a < endPos && check.size() < checkContextsCount; ++a)
      if(check.size() < checkContextsCount)
        addContextsForRemoval(check, IndexedTopDUContext(visitor.checkContexts[a]));

    foreach(uint topIndex, check) {
      IndexedTopDUContext top(topIndex);
      if(top.data()) {
        kDebug() << "removing top-context for" << top.data()->url().str() << "because it is out of date";
        instance->removeDocumentChain(top.data());      
      }
    }
    kDebug() << "check ready";
  }

private:
  
  void addContextsForRemoval(QSet<uint>& topContexts, IndexedTopDUContext top) {
    if(topContexts.contains(top.index()))
      return;
    
    KSharedPtr<ParsingEnvironmentFile> info( instance->environmentFileForDocument(top) );
    ///@todo Also check if the context is "useful"(Not a duplicate context, imported by a useful one, ...)
    if(info && info->needsUpdate()) {
      //This context will be removed
    }else{
      return;
    }
    
    topContexts.insert(top.index());
    
    if(info) {
      //Check whether importers need to be removed as well
      QList< KSharedPtr<ParsingEnvironmentFile> > importers = info->importers();

      QSet< KSharedPtr<ParsingEnvironmentFile> > checkNext;
      
      //Do breadth first search, so less imports/importers have to be loaded, and a lower depth is reached
      
      for(QList< KSharedPtr<ParsingEnvironmentFile> >::iterator it = importers.begin(); it != importers.end(); ++it) {
        IndexedTopDUContext c = (*it)->indexedTopContext();
        if(!topContexts.contains(c.index())) {
          topContexts.insert(c.index()); //Prevent useless recursion
          checkNext.insert(*it);
        }
      }
      
      for(QSet< KSharedPtr<ParsingEnvironmentFile> >::const_iterator it = checkNext.begin(); it != checkNext.end(); ++it) {
        topContexts.remove((*it)->indexedTopContext().index()); //Enable full check again
        addContextsForRemoval(topContexts, (*it)->indexedTopContext());
      }
    }
  }

  template<class Entry>
  bool listContains(const Entry entry, const Entry* list, uint listSize) {
    for(uint a = 0; a < listSize; ++a)
      if(list[a] == entry)
        return true;
    return false;
  }

  ///Stores the environment-information for the given url
  void storeInformationList(IndexedString url) {

    QMutexLocker lock(m_environmentListInfo.mutex());

    EnvironmentInformationListItem newItem;
    newItem.m_file = url;
    
    QSet<uint> newItems;

    {
      QMutexLocker lock(&m_chainsMutex);
      QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator start = m_fileEnvironmentInformations.lowerBound(url);
      QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator end = m_fileEnvironmentInformations.upperBound(url);
      
      for(QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it = start; it != end; ++it) {
        uint topContextIndex = (*it)->indexedTopContext().index();
        newItems.insert(topContextIndex);
        newItem.itemsList().append(topContextIndex);
      }
    }
    
    uint index = m_environmentListInfo.findIndex(url);
    
    if(index) {
      //We only handle adding items here, since we can never be sure whether everything is loaded
      //Removal is handled directly in removeEnvironmentInformation
      
      const EnvironmentInformationListItem* item = m_environmentListInfo.itemFromIndex(index);
      QSet<uint> oldItems;
      FOREACH_FUNCTION(uint topContextIndex, item->items) {
        oldItems.insert(topContextIndex);
        if(!newItems.contains(topContextIndex)) {
          newItems.insert(topContextIndex);
          newItem.itemsList().append(topContextIndex);
        }
      }

      if(oldItems == newItems)
        return;

      ///Update/insert a new list
      m_environmentListInfo.deleteItem(index); //Remove the previous item
    }

    Q_ASSERT(m_environmentListInfo.findIndex(EnvironmentInformationListRequest(url)) == 0);

    //Insert the new item
    m_environmentListInfo.index(EnvironmentInformationListRequest(url, newItem));

    Q_ASSERT(m_environmentListInfo.findIndex(EnvironmentInformationListRequest(url)));
  }

  //Loaded environment-informations. Protected by m_chainsMutex
  QMultiMap<IndexedString, ParsingEnvironmentFilePointer> m_fileEnvironmentInformations;
  QHash<uint, ParsingEnvironmentFilePointer> m_indexEnvironmentInformations;
  
  ///The following repositories are thread-safe, and m_chainsMutex should not be locked when using them, because
  ///they may trigger I/O. Still it may be required to lock their local mutexes.
  ///Maps filenames to a list of top-contexts/environment-information.
  ItemRepository<EnvironmentInformationListItem, EnvironmentInformationListRequest> m_environmentListInfo;
  ///Maps top-context-indices to environment-information item.
  ItemRepository<EnvironmentInformationItem, EnvironmentInformationRequest> m_environmentInfo;
};

K_GLOBAL_STATIC(DUChainPrivate, sdDUChainPrivate)


DUChain::DUChain()
{
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));

  connect(EditorIntegrator::notifier(), SIGNAL(documentAboutToBeDeleted(KTextEditor::Document*)), SLOT(documentAboutToBeDeleted(KTextEditor::Document*)));
  connect(EditorIntegrator::notifier(), SIGNAL(documentAboutToBeDeletedFinal(KTextEditor::Document*)), SLOT(documentAboutToBeDeletedFinal(KTextEditor::Document*)));
  if(ICore::self()) {
    Q_ASSERT(ICore::self()->documentController());
    connect(ICore::self()->documentController(), SIGNAL(documentLoadedPrepare(KDevelop::IDocument*)), this, SLOT(documentLoadedPrepare(KDevelop::IDocument*)));
    connect(ICore::self()->documentController(), SIGNAL(documentUrlChanged(KDevelop::IDocument*)), this, SLOT(documentRenamed(KDevelop::IDocument*)));
    connect(ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), this, SLOT(documentActivated(KDevelop::IDocument*)));
  }
}

DUChain::~DUChain()
{
  duChainDeleted = true;
}

DUChain* DUChain::self()
{
  return sdDUChainPrivate->instance;
}

DUChainLock* DUChain::lock()
{
  return &sdDUChainPrivate->lock;
}

QList<TopDUContext*> DUChain::allChains() const
{
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
  QList<TopDUContext*> ret;
  for(google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = sdDUChainPrivate->m_chainsByIndex.begin(); it != sdDUChainPrivate->m_chainsByIndex.end(); ++it)
    ret << (*it).second;

  return ret;
}

void DUChain::updateContextEnvironment( TopDUContext* context, ParsingEnvironmentFile* file ) {

  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  removeFromEnvironmentManager( context );

  context->setParsingEnvironmentFile( file );

  addToEnvironmentManager( context );

  branchModified(context);
}

void DUChain::removeDocumentChain( TopDUContext* context )
{
  ENSURE_CHAIN_WRITE_LOCKED;
  IndexedTopDUContext indexed(context->indexed());
  Q_ASSERT(indexed.data() == context); ///This assertion fails if you call removeDocumentChain(..) on a document that has not been added to the du-chain
  branchRemoved(context);
  context->m_dynamicData->deleteOnDisk();
  Q_ASSERT(indexed.data() == context);
  sdDUChainPrivate->removeDocumentChainFromMemory(context);
  Q_ASSERT(!indexed.data());
  Q_ASSERT(!environmentFileForDocument(indexed));
}

void DUChain::addDocumentChain( TopDUContext * chain )
{
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

//   kDebug(9505) << "duchain: adding document" << chain->url().str() << " " << chain;
  Q_ASSERT(chain);
  if (chain->smartRange()) {
    Q_ASSERT(!chain->smartRange()->parentRange());
    ICore::self()->languageController()->backgroundParser()->addManagedTopRange(KUrl(chain->url().str()), chain->smartRange());
  }

  Q_ASSERT(sdDUChainPrivate->m_chainsByIndex.find(chain->ownIndex()) == sdDUChainPrivate->m_chainsByIndex.end());

  sdDUChainPrivate->m_chainsByIndex.insert(std::make_pair(chain->ownIndex(), chain));
  sdDUChainPrivate->m_chainsByUrl.insert(chain->url(), chain);

/*  {
    //This is just for debugging, and should be disabled later.
    int realChainCount = 0;
    int proxyChainCount = 0;
    for(QMap<IdentifiedFile, TopDUContext*>::const_iterator it = sdDUChainPrivate->m_chains.begin(); it != sdDUChainPrivate->m_chains.end(); ++it) {
      if((*it)->flags() & TopDUContext::ProxyContextFlag)
        ++proxyChainCount;
      else
        ++realChainCount;
    }

    kDebug(9505) << "new count of real chains: " << realChainCount << " proxy-chains: " << proxyChainCount;
  }*/
  chain->setInDuChain(true);
  
  l.unlock();
  
  addToEnvironmentManager(chain);

  //contextChanged(0L, DUChainObserver::Addition, DUChainObserver::ChildContexts, chain);

  KTextEditor::Document* doc = EditorIntegrator::documentForUrl(chain->url());
  if(doc) {
    //Make sure the context stays alive at least as long as the context is open
    ReferencedTopDUContext ctx(chain);
    sdDUChainPrivate->m_openDocumentContexts.insert(ctx);
  }

  branchAdded(chain);
}

void DUChain::addToEnvironmentManager( TopDUContext * chain ) {

  ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
  if( !file )
    return; //We don't need to manage

  Q_ASSERT(file->indexedTopContext().index() == chain->ownIndex());

  if(ParsingEnvironmentFile* alreadyHave = sdDUChainPrivate->findInformation(file->indexedTopContext().index()))
  {
    ///If this triggers, there has already been another environment-information registered for this top-context.
    ///removeFromEnvironmentManager should have been called before to remove the old environment-information.
    Q_ASSERT(alreadyHave == file.data());
    return;
  }

  sdDUChainPrivate->addEnvironmentInformation(file);
}

void DUChain::removeFromEnvironmentManager( TopDUContext * chain ) {

  ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
  if( !file )
    return; //We don't need to manage

  sdDUChainPrivate->removeEnvironmentInformation(file);
}

TopDUContext* DUChain::chainForDocument(const KUrl& document) const {
  return chainForDocument(IndexedString(document.pathOrUrl()));
}

bool DUChain::isInMemory(uint topContextIndex) const {
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = sdDUChainPrivate->m_chainsByIndex.find(topContextIndex);
  return it != sdDUChainPrivate->m_chainsByIndex.end();
}

IndexedString DUChain::urlForIndex(uint index) const {
  {
    QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
    google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = sdDUChainPrivate->m_chainsByIndex.find(index);
    if(it != sdDUChainPrivate->m_chainsByIndex.end())
      return (*it).second->url();
  }
  
  return TopDUContextDynamicData::loadUrl(index);
}


TopDUContext* DUChain::chainForIndex(uint index) const {

  DUChainPrivate* p = (sdDUChainPrivate.operator->());
  if(p->m_destroyed)
    return 0;

  //QMutexLocker is not used for performance reasons: This function is called extremely often
  p->m_chainsMutex.lock();

  google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = p->m_chainsByIndex.find(index);
  if(it != p->m_chainsByIndex.end()) {
    TopDUContext* ret = (*it).second;
    p->m_chainsMutex.unlock();
    return ret;
  } else {
    p->m_chainsMutex.unlock();
    QSet<uint> loaded;
    p->loadChain(index, loaded);
    p->m_chainsMutex.lock();

    it = p->m_chainsByIndex.find(index);
    if(it != p->m_chainsByIndex.end()) {
      TopDUContext* ret = (*it).second;
      p->m_chainsMutex.unlock();
      return ret;
    } else {
      p->m_chainsMutex.unlock();
      return 0;
    }
  }
}

TopDUContext* DUChain::chainForDocument(const IndexedString& document) const
{
  if(sdDUChainPrivate->m_destroyed)
    return 0;

  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
/*    {
      int count = 0;
      QMap<IdentifiedFile, TopDUContext*>::Iterator it = sdDUChainPrivate->m_chains.lowerBound(document);
      for( ; it != sdDUChainPrivate->m_chains.end() && it.key().url() == document.url(); ++it )
        ++count;
      if( count > 1 )
        kDebug(9505) << "found " << count << " chains for " << document.url().str();

    }*/

  // Match any parsed version of this document
  if(sdDUChainPrivate->m_chainsByUrl.contains(document))
    return *sdDUChainPrivate->m_chainsByUrl.find(document);

  //Eventually load an existing chain from disk
  l.unlock();
  
  QList<ParsingEnvironmentFilePointer> list = sdDUChainPrivate->getEnvironmentInformation(document);
  
  foreach(const ParsingEnvironmentFilePointer &file, list) {
    if(isInMemory(file->indexedTopContext().index()))
      return file->indexedTopContext().data();
  }
  if(!list.isEmpty())
    return list[0]->topContext(); //Load a top-context if there is none in memory

  return 0;
}

QList<TopDUContext*> DUChain::chainsForDocument(const KUrl& document) const
{
  return chainsForDocument(IndexedString(document));
}

QList<TopDUContext*> DUChain::chainsForDocument(const IndexedString& document) const
{
  QList<TopDUContext*> chains;

  if(sdDUChainPrivate->m_destroyed)
    return chains;

  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  // Match all parsed versions of this document
  for (QMultiMap<IndexedString, TopDUContext*>::Iterator it = sdDUChainPrivate->m_chainsByUrl.lowerBound(document); it != sdDUChainPrivate->m_chainsByUrl.end(); ++it) {
    if (it.key() == document)
      chains << it.value();
    else
      break;
  }

  return chains;
}

TopDUContext* DUChain::chainForDocument( const KUrl& document, const ParsingEnvironment* environment, bool onlyProxyContexts, bool noProxyContexts ) const {
  return chainForDocument( IndexedString(document), environment, onlyProxyContexts, noProxyContexts );
}

ParsingEnvironmentFilePointer DUChain::environmentFileForDocument( const IndexedString& document, const ParsingEnvironment* environment, bool onlyProxyContexts, bool noProxyContexts ) const {

  if(sdDUChainPrivate->m_destroyed)
    return ParsingEnvironmentFilePointer();
  QList< ParsingEnvironmentFilePointer> list = sdDUChainPrivate->getEnvironmentInformation(document);

//    kDebug() << document.str() << ": matching" << list.size() << (onlyProxyContexts ? "proxy-contexts" : (noProxyContexts ? "content-contexts" : "contexts"));

  QList< ParsingEnvironmentFilePointer>::const_iterator it = list.constBegin();
  while(it != list.constEnd()) {
    if(*it && (*it)->matchEnvironment(environment) && (!onlyProxyContexts || (*it)->isProxyContext()) && (!noProxyContexts || !(*it)->isProxyContext())) {
      return *it;
    }
    ++it;
  }
  return ParsingEnvironmentFilePointer();
}

QList<ParsingEnvironmentFilePointer> DUChain::allEnvironmentFiles(const IndexedString& document) {
  return sdDUChainPrivate->getEnvironmentInformation(document);
}

ParsingEnvironmentFilePointer DUChain::environmentFileForDocument(IndexedTopDUContext topContext) const {
   if(topContext.index() == 0)
     return ParsingEnvironmentFilePointer();

   return ParsingEnvironmentFilePointer(sdDUChainPrivate->loadInformation(topContext.index()));
}

TopDUContext* DUChain::chainForDocument( const IndexedString& document, const ParsingEnvironment* environment, bool onlyProxyContexts, bool noProxyContexts ) const {

  if(sdDUChainPrivate->m_destroyed)
    return 0;
  ParsingEnvironmentFilePointer envFile = environmentFileForDocument(document, environment, onlyProxyContexts, noProxyContexts);
  if(envFile) {
    return envFile->topContext();
  }else{
    return 0;
  }
}

DUChainObserver* DUChain::notifier()
{
  return sdDUChainPrivate->notifier;
}

void DUChain::branchAdded(DUContext* context)
{
  emit sdDUChainPrivate->notifier->branchAdded(DUContextPointer(context));
}

void DUChain::branchModified(DUContext* context)
{
  emit sdDUChainPrivate->notifier->branchModified(DUContextPointer(context));
}

void DUChain::branchRemoved(DUContext* context)
{
  emit sdDUChainPrivate->notifier->branchRemoved(DUContextPointer(context));
}

QList<KUrl> DUChain::documents() const
{
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
  
  QList<KUrl> ret;
  for(google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = sdDUChainPrivate->m_chainsByIndex.begin(); it != sdDUChainPrivate->m_chainsByIndex.end(); ++it) {
    ret << KUrl((*it).second->url().str());
  }

  return ret;
}

/*Q_SCRIPTABLE bool DUChain::updateContext(TopDUContext* topContext, TopDUContext::Features minFeatures, QObject* notifyReady) const
{
  if( (topContext->features() & minFeatures) != minFeatures || (topContext->parsingEnvironmentFile() && topContext->parsingEnvironmentFile()->needsUpdate()) ) {
    ICore::self()->languageController()->backgroundParser()->addUpdateJob(topContext, minFeatures, notifyReady);
    return true;
  }else{
    //No update needed, or we don't know since there's no ParsingEnvironmentFile attached
    return false;
  }
}*/

void DUChain::documentActivated(KDevelop::IDocument* doc)
{
  if(sdDUChainPrivate->m_destroyed)
    return;
  //Check whether the document has an attached environment-manager, and whether that one thinks the document needs to be updated.
  //If yes, update it.
  DUChainReadLocker lock( DUChain::lock() );
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
  TopDUContext* ctx = DUChainUtils::standardContextForUrl(doc->url());
  if(ctx && ctx->parsingEnvironmentFile())
    if(ctx->parsingEnvironmentFile()->needsUpdate())
      ICore::self()->languageController()->backgroundParser()->addDocument(doc->url());
}

static void deconvertDUChainInternal(DUContext* context)
{
  foreach (Declaration* dec, context->localDeclarations())
    dec->clearSmartRange();

  context->clearUseSmartRanges();

  foreach (DUContext* child, context->childContexts())
    deconvertDUChainInternal(child);

  context->clearSmartRange();
}

void DUChain::documentAboutToBeDeletedFinal(KTextEditor::Document* doc)
{
  if(sdDUChainPrivate->m_destroyed)
    return;
  
  QList<TopDUContext*> chains = chainsForDocument(doc->url());

  KTextEditor::SmartInterface* smart = dynamic_cast<KTextEditor::SmartInterface*>(doc);
  if(!smart)
    return;
  
  foreach (TopDUContext* top, chains) {
    
    QMutexLocker lockSmart(smart->smartMutex());
    
    deconvertDUChainInternal(top);
  }
}

void DUChain::documentAboutToBeDeleted(KTextEditor::Document* doc)
{
  if(sdDUChainPrivate->m_destroyed)
    return;

  foreach(const ReferencedTopDUContext &top, sdDUChainPrivate->m_openDocumentContexts) {
    if(top->url().str() == doc->url().pathOrUrl())
      sdDUChainPrivate->m_openDocumentContexts.remove(top);
  }
}

void DUChain::documentLoadedPrepare(KDevelop::IDocument* doc)
{
  if(sdDUChainPrivate->m_destroyed)
    return;
  DUChainWriteLocker lock( DUChain::lock() );
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  // Convert any duchains to the smart equivalent first
  EditorIntegrator editor;
  if(doc->textDocument())
    editor.insertLoadedDocument(doc->textDocument()); //Make sure the editor-integrator knows the document

  TopDUContext* standardContext = DUChainUtils::standardContextForUrl(doc->url());
  QList<TopDUContext*> chains = chainsForDocument(doc->url());

  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(doc->url());

  if(standardContext) {
    Q_ASSERT(chains.contains(standardContext)); //We have just loaded it

    {
      ///Make the standard-context editor-smart
      SmartConverter sc(&editor);
      
      if(standardContext->smartRange()) {
        Q_ASSERT(standardContext->smartRange()->document() == doc->textDocument());
        kWarning() << "Strange: context already has smart-range! Probably another document is already open for it. Deconverting";
        sc.deconvertDUChain(standardContext);
      }
      sc.convertDUChain(standardContext);
      Q_ASSERT(standardContext->smartRange());
      ICore::self()->languageController()->backgroundParser()->addManagedTopRange(doc->url(), standardContext->smartRange());
    }

    sdDUChainPrivate->m_openDocumentContexts.insert(standardContext);

    bool needsUpdate = standardContext->parsingEnvironmentFile() && standardContext->parsingEnvironmentFile()->needsUpdate();
    if(!needsUpdate) {

        //Only apply the highlighting if we don't need to update, else we might highlight total crap
        //Do instant highlighting only if all imports are loaded, to make sure that we don't block the user-interface too long
        //Else the highlighting will be done in the background-thread
        //This is not exactly right, as the direct imports don't necessarily equal the real imports used by uses
        //but it approximates the correct behavior.
        bool allImportsLoaded = true;
        foreach(DUContext::Import import, standardContext->importedParentContexts())
          if(!import.indexedContext().indexedTopContext().isLoaded())
            allImportsLoaded = false;

        if(allImportsLoaded) {
          foreach( KDevelop::ILanguage* language, languages)
            if(language->languageSupport() && language->languageSupport()->codeHighlighting())
              language->languageSupport()->codeHighlighting()->highlightDUChain(standardContext);
          kDebug() << "highlighted" << doc->url() << "in foreground";
          return;
        }
    }else{
      kDebug() << "not highlighting the duchain because the documents needs an update";
    }
    
    if(needsUpdate || !(standardContext->features() & TopDUContext::AllDeclarationsContextsAndUses)) {
      ICore::self()->languageController()->backgroundParser()->addDocument(doc->url(), (TopDUContext::Features)(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdate));
      return;
    }
  }
    
  //Add for highlighting etc.
  ICore::self()->languageController()->backgroundParser()->addDocument(doc->url(), TopDUContext::AllDeclarationsContextsAndUses);
}

void DUChain::documentRenamed(KDevelop::IDocument* doc)
{
  if(sdDUChainPrivate->m_destroyed)
    return;

  ICore::self()->languageController()->backgroundParser()->addDocument(doc->url(), (TopDUContext::Features)(TopDUContext::AllDeclarationsContextsAndUses | TopDUContext::ForceUpdate));
}

Uses* DUChain::uses()
{
  return &sdDUChainPrivate->m_uses;
}

Definitions* DUChain::definitions()
{
  return &sdDUChainPrivate->m_definitions;
}

void DUChain::aboutToQuit()
{
  QMutexLocker lock(&sdDUChainPrivate->cleanupMutex());

  {
    //Acquire write-lock of the repository, so when kdevelop crashes in that process, the repository is discarded
    //Crashes here may happen in an inconsistent state, thus this makes sense, to protect the user from more crashes
    globalItemRepositoryRegistry().lockForWriting();
    sdDUChainPrivate->cleanupTopContexts();
    globalItemRepositoryRegistry().unlockForWriting();
  }

  sdDUChainPrivate->doMoreCleanup(); //Must be done _before_ finalCleanup, else we may be deleting yet needed data

  {
    //Acquire write-lock of the repository, so when kdevelop crashes in that process, the repository is discarded
    //Crashes here may happen in an inconsistent state, thus this makes sense, to protect the user from more crashes
    globalItemRepositoryRegistry().lockForWriting();
    finalCleanup();
    globalItemRepositoryRegistry().unlockForWriting();
  }
  
  sdDUChainPrivate->doMoreCleanup();
  sdDUChainPrivate->m_openDocumentContexts.clear();
  sdDUChainPrivate->m_destroyed = true;
  sdDUChainPrivate->clear();
}

uint DUChain::newTopContextIndex() {
  static QAtomicInt& currentId( globalItemRepositoryRegistry().getCustomCounter("Top-Context Counter", 1) );
  return currentId.fetchAndAddRelaxed(1);
}

void DUChain::refCountUp(TopDUContext* top) {
  QMutexLocker l(&sdDUChainPrivate->m_referenceCountsMutex);
  if(!sdDUChainPrivate->m_referenceCounts.contains(top))
    sdDUChainPrivate->m_referenceCounts.insert(top, 1);
  else
    ++sdDUChainPrivate->m_referenceCounts[top];
}

bool DUChain::deleted() {
  return duChainDeleted;
}

void DUChain::refCountDown(TopDUContext* top) {
  QMutexLocker l(&sdDUChainPrivate->m_referenceCountsMutex);
  if(!sdDUChainPrivate->m_referenceCounts.contains(top)) {
    //kWarning() << "tried to decrease reference-count for" << top->url().str() << "but this top-context is not referenced";
    return;
  }
  --sdDUChainPrivate->m_referenceCounts[top];
  if(!sdDUChainPrivate->m_referenceCounts[top])
    sdDUChainPrivate->m_referenceCounts.remove(top);
}

void DUChain::emitDeclarationSelected(DeclarationPointer decl) {
  emit declarationSelected(decl);
}

KDevelop::ReferencedTopDUContext DUChain::waitForUpdate(const KDevelop::IndexedString& document, KDevelop::TopDUContext::Features minFeatures, bool wantProxyContext) {
  Q_ASSERT(!lock()->currentThreadHasReadLock() && !lock()->currentThreadHasWriteLock());

  WaitForUpdate waiter;
  
  waiter.m_dataMutex.lock();
  
  {
    DUChainReadLocker readLock(DUChain::lock());
  
    updateContextForUrl(document, minFeatures, &waiter);
  }
  
//   waiter.m_waitMutex.lock();
//   waiter.m_dataMutex.unlock();
  while(!waiter.m_ready) {
    ///@todo When we don't do this, the backgroundparser doesn't process anything.
    ///      The background-parser should be moved into an own thread, so we wouldn't need to do this.
    QApplication::processEvents();
    usleep(10000);
//     waiter.m_wait.wait(&waiter.m_waitMutex, 10);
  }

  if(!wantProxyContext) {
    DUChainReadLocker readLock(DUChain::lock());
    if(waiter.m_topContext && waiter.m_topContext->parsingEnvironmentFile() && waiter.m_topContext->parsingEnvironmentFile()->isProxyContext() && !waiter.m_topContext->importedParentContexts().isEmpty())
      return waiter.m_topContext->importedParentContexts()[0].context(0)->topContext();
  }

  return waiter.m_topContext;
}

void DUChain::updateContextForUrl(const IndexedString& document, TopDUContext::Features minFeatures, QObject* notifyReady) const {
  TopDUContext* standardContext = DUChainUtils::standardContextForUrl(document.toUrl());
  if(standardContext && standardContext->parsingEnvironmentFile() && !standardContext->parsingEnvironmentFile()->needsUpdate() && standardContext->parsingEnvironmentFile()->featuresSatisfied(minFeatures)) {
    if(notifyReady)
    QMetaObject::invokeMethod(notifyReady, "updateReady", Qt::DirectConnection, Q_ARG(KDevelop::IndexedString, document), Q_ARG(KDevelop::ReferencedTopDUContext, ReferencedTopDUContext(standardContext)));
  }else{
    ///Start a parse-job for the given document
    ICore::self()->languageController()->backgroundParser()->addDocument(document.toUrl(), minFeatures, 1, notifyReady);
  }
}

void DUChain::disablePersistentStorage() {
  sdDUChainPrivate->m_cleanupDisabled = true;
}

void DUChain::storeToDisk() {
  bool wasDisabled = sdDUChainPrivate->m_cleanupDisabled;
  sdDUChainPrivate->m_cleanupDisabled = false;
  
  sdDUChainPrivate->doMoreCleanup();
  
  sdDUChainPrivate->m_cleanupDisabled = wasDisabled;
}

void DUChain::finalCleanup() {
  DUChainWriteLocker writeLock(DUChain::lock());
  kDebug() << "doing final cleanup";
  
  int cleaned = 0;
  while((cleaned = globalItemRepositoryRegistry().finalCleanup())) {
    kDebug() << "cleaned" << cleaned << "B";
    if(cleaned < 1000) {
      kDebug() << "cleaned enough";
      break;
    }
  }
  kDebug() << "final cleanup ready";
}

bool DUChain::compareToDisk() {
  
  DUChainWriteLocker writeLock(DUChain::lock());
  
  ///Step 1: Compare the repositories
  return true;
}

}

#include "duchain.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
