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

#include <QCoreApplication>
#include <QHash>
#include <QMultiMap>
#include <QTimer>
#include <QReadWriteLock>
#include <qatomic.h>

#include <kglobal.h>

#include <KTextEditor/Document>

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
#include "duchainutils.h"
#include "duchainregister.h"
#include "repositories/itemrepository.h"
#include <util/google/dense_hash_map>
#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <unistd.h>

Q_DECLARE_METATYPE(KDevelop::IndexedString)
Q_DECLARE_METATYPE(KDevelop::IndexedTopDUContext)
Q_DECLARE_METATYPE(KDevelop::ReferencedTopDUContext)

//Additional "soft" cleanup steps that are done before the actual cleanup.
//During "soft" cleanup, the consistency is not guaranteed. The repository is
//marked to be updating during soft cleanup, so if kdevelop crashes, it will be cleared.
//The big advantage of the soft cleanup steps is, that the duchain is always only locked for
//short times, which leads to no lockup in the UI.
const int SOFT_CLEANUP_STEPS = 1;

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
    
    DUChainItemSystem::self().copy(*m_file->d_func(), *(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem)), true);
    Q_ASSERT((*(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem))).m_range == m_file->d_func()->m_range);
    Q_ASSERT((*(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem))).classId == m_file->d_func()->classId);
    Q_ASSERT((*(DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem))).m_dynamic == false);
  }

  bool equals(const EnvironmentInformationItem* item) const {
    return m_index == item->m_topContext;
  }

  const ParsingEnvironmentFile* m_file;
  uint m_index;
};

///A list of environment-informations/top-contexts mapped to a file-name
class EnvironmentInformationListItem {
  public:
  EnvironmentInformationListItem() {
    initializeAppendedLists(true);
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
    AverageSize = 64 //This should be the approximate average size of an Item
  };

  unsigned int hash() const {
    return m_file.hash();
  }

  size_t itemSize() const {
    return m_item->itemSize();
  }

  void createItem(EnvironmentInformationListItem* item) const {
    item->initializeAppendedLists(false);
    item->m_file = m_file;
    item->copyListsFrom(*m_item);
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
        m_stopRunning = true;
        m_wait.wakeAll(); //Wakes the thread up, so it notices it should exit
        wait();
      }
      
    private:
      void run() {
        while(1) {
          m_waitMutex.lock();
          m_wait.wait(&m_waitMutex, 1000 * 90); //Wait 90s by default
          m_waitMutex.unlock();
          if(m_stopRunning)
            break;
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
  DUChainPrivate() : m_chainsMutex(QMutex::Recursive), instance(0), m_destroyed(false), m_environmentListInfo("Environment Lists"), m_environmentInfo("Environment Information"), m_cleanupDisabled(false)
  {
    m_chainsByIndex.set_empty_key(0);
    m_chainsByIndex.set_deleted_key(0xffffffff);
    duChainPrivateSelf = this;
    qRegisterMetaType<DUChainBasePointer>("KDevelop::DUChainBasePointer");
    qRegisterMetaType<DUContextPointer>("KDevelop::DUContextPointer");
    qRegisterMetaType<TopDUContextPointer>("KDevelop::TopDUContextPointer");
    qRegisterMetaType<DeclarationPointer>("KDevelop::DeclarationPointer");
    qRegisterMetaType<FunctionDeclarationPointer>("KDevelop::FunctionDeclarationPointer");
    qRegisterMetaType<Problem>("KDevelop::Problem");
    qRegisterMetaType<KDevelop::IndexedString>("KDevelop::IndexedString");
    qRegisterMetaType<KDevelop::IndexedTopDUContext>("KDevelop::IndexedTopDUContext");
    qRegisterMetaType<KDevelop::ReferencedTopDUContext>("KDevelop::ReferencedTopDUContext");
    
    notifier = new DUChainObserver();
    instance = new DUChain();
    m_cleanup = new CleanupThread(this);
    m_cleanup->start();
  }
  ~DUChainPrivate() {
    m_cleanup->stopThread();
    delete m_cleanup;
    delete instance;
  }

  void clear() {
    QMutexLocker l(&m_chainsMutex);

    if(!m_cleanupDisabled)
      doMoreCleanup();
    
    DUChainWriteLocker writeLock(DUChain::lock());

    for(google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = m_chainsByIndex.begin(); it != m_chainsByIndex.end(); ++it)
      instance->removeDocumentChain((*it).second);

    m_fileEnvironmentInformations.clear();

    Q_ASSERT(m_fileEnvironmentInformations.isEmpty());
    Q_ASSERT(m_chainsByUrl.isEmpty());
    Q_ASSERT(m_chainsByIndex.empty());
  }

  ///Must be locked before accessing content of this class
  QMutex m_chainsMutex;

  CleanupThread* m_cleanup;
  
  DUChain* instance;
  DUChainLock lock;
  QMultiMap<IndexedString, TopDUContext*> m_chainsByUrl;
  google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash> m_chainsByIndex;
  QHash<TopDUContext*, uint> m_referenceCounts;
  DUChainObserver* notifier;
  Definitions m_definitions;
  Uses m_uses;
  QSet<uint> m_loading;
  bool m_cleanupDisabled;

  ///Used to keep alive the top-context that belong to documents loaded in the editor
  QSet<ReferencedTopDUContext> m_openDocumentContexts;

  bool m_destroyed;
  
  ParsingEnvironmentFile* findEnvironmentFor(IndexedTopDUContext top) {
    IndexedString url = top.url();
    
    QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator start = m_fileEnvironmentInformations.lowerBound(url);
    QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator end = m_fileEnvironmentInformations.upperBound(url);
    
    while(start != end) {
      if((*start)->indexedTopContext().index() == top.index())
        return start->data();
      ++start;
    }
    
    return 0;
  }
  
  ///The item must not be stored yet
  void addEnvironmentInformation(ParsingEnvironmentFilePointer info) {
    Q_ASSERT(!findEnvironmentFor(info->indexedTopContext()));
    Q_ASSERT(m_environmentInfo.findIndex(info->indexedTopContext().index()) == 0);
    
    m_fileEnvironmentInformations.insert(info->url(), info);
    Q_ASSERT(info->d_func()->classId);
  }

  ///The item must be managed currently
  void removeEnvironmentInformation(ParsingEnvironmentFilePointer info) {
    
    info->makeDynamic(); //By doing this, we make sure the data is actually being destroyed in the destructor
    
    bool removed = (bool)m_fileEnvironmentInformations.remove(info->url(), info);
    uint index = m_environmentInfo.findIndex(info->indexedTopContext().index());
    if(index) {
      m_environmentInfo.deleteItem(index);
    }
    
    Q_ASSERT(index || removed);
  }

  QList<ParsingEnvironmentFilePointer> getEnvironmentInformation(IndexedString url) {
    QList<ParsingEnvironmentFilePointer> ret;
    
    uint listIndex = m_environmentListInfo.findIndex(url);
    
    if(listIndex) {
      const EnvironmentInformationListItem* item = m_environmentListInfo.itemFromIndex(listIndex);
    
      FOREACH_FUNCTION(uint topContextIndex, item->items)
        ret << ParsingEnvironmentFilePointer(loadInformation(url, topContextIndex));
    }
    //Add those informations that have not been added to the stored lists yet
    ret += m_fileEnvironmentInformations.values(url);
    
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
      TopDUContext* chain = TopDUContextDynamicData::load(index);
      if(chain) {
//         kDebug() << "url" << chain->url().str();
        chain->setParsingEnvironmentFile(loadInformation(chain->url(), chain->ownIndex()));

        l.unlock();
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
        l.relock();
        instance->addDocumentChain(chain);
      }
      m_loading.remove(index);
    }
  }


  ///Stores all environment-information
  ///Also makes sure that all information that stays is referenced, so it stays alive.
  ///@param atomic If this is false, the write-lock will be released time by time
  void storeAllInformation(bool atomic, DUChainWriteLocker& locker) {
//     QMutexLocker l(&m_chainsMutex);

    uint cnt = 0;
    
    QList<IndexedString> urls = m_fileEnvironmentInformations.keys();
    
    foreach(const IndexedString &url, urls) {
      
      QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator start = m_fileEnvironmentInformations.lowerBound(url);
      QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator end = m_fileEnvironmentInformations.upperBound(url);
      
      for(QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it = start; it  != end; ++it) {
        
        EnvironmentInformationRequest req(it->data());
        uint index = m_environmentInfo.findIndex(req);
        
        Q_ASSERT((*it)->d_func()->classId);
        (*it)->aboutToSave();
        Q_ASSERT((*it)->d_func()->classId);
        
        if((*it)->d_func()->isDynamic()) {
          //This item has been changed, or isn't in the repositor yet
          
          //Eventually remove an old entry
          if(index) {
//           EnvironmentInformationItem* item = const_cast<EnvironmentInformationItem*>(m_environmentInfo.itemFromIndex(index));
//           DUChainBaseData* theData = (DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem));
//             
//             foreach(ParsingEnvironmentFilePointer p, m_fileEnvironmentInformations.values())
//               if(p != *it)
//                 Q_ASSERT(p->d_func() != theData); //Make sure the data we're deleting isn't used by any other existing environment-info
//             
            m_environmentInfo.deleteItem(index);
          }
          
          Q_ASSERT((*it)->d_func()->classId);
          
          //Add the new entry to the item repository
          index = m_environmentInfo.index(req);
          Q_ASSERT(index);
          
          EnvironmentInformationItem* item = const_cast<EnvironmentInformationItem*>(m_environmentInfo.itemFromIndex(index));
          DUChainBaseData* theData = (DUChainBaseData*)(((char*)item) + sizeof(EnvironmentInformationItem));
          static DUChainBaseData* dataCopy;
          dataCopy = theData;
          
          Q_ASSERT(theData->m_range == (*it)->d_func()->m_range);
          Q_ASSERT(theData->m_dynamic == false);
          Q_ASSERT(theData->classId == (*it)->d_func()->classId);
          
          (*it)->setData( theData );
          Q_ASSERT((*it)->d_func()->classId);
          
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
        kDebug(9505) << "Did not find stored item for" << url.str() << "count:" << m_fileEnvironmentInformations.values(url);
      }
      if(!atomic) {
        locker.unlock();
        locker.lock();
      }
    }
  }

  ///@param retries When this is nonzero, then doMoreCleanup will do the specified amount of cycles
  ///doing the cleanup without permanently locking the du-chain. During these steps the consistency
  ///of the disk-storage is not guaranteed, but only few changes will be done during these steps,
  ///so the final step where the duchain is permanetly locked is much faster.
  void doMoreCleanup(int retries = 0, bool needLockRepository = true) {
    
    if(m_cleanupDisabled)
      return;
    
    //This mutex makes sure that there's never 2 threads at he same time trying to clean up
    static QMutex cleanupMutex(QMutex::Recursive);
    QMutexLocker lockCleanupMutex(&cleanupMutex);
    
    Q_ASSERT(!instance->lock()->currentThreadHasReadLock() && !instance->lock()->currentThreadHasWriteLock());
    DUChainWriteLocker writeLock(instance->lock());

    //This is used to stop all parsing before starting to do the cleanup. This way less happens during the
    //soft cleanups, and we have a good chance that during the "hard" cleanup only few data has to be wriitten.
    QList<ILanguage*> lockedParseMutexes;
    
    QList<QReadWriteLock*> locked;
    
    if(needLockRepository) {
      
      lockedParseMutexes = ICore::self()->languageController()->loadedLanguages();
      
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
      
    for(google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = m_chainsByIndex.begin(); it != m_chainsByIndex.end(); ++it)
      workOnContexts << (*it).second;
    
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
          
          //Test if the context is imported by a referenced one
          foreach(TopDUContext* context, m_referenceCounts.keys())
            if(context == unload || context->imports(unload, SimpleCursor())) {
              workOnContexts.remove(unload);
              hasReference = true;
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
          instance->removeDocumentChain(unload);
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
          goto unloadContexts;
        }
      }
      
      
      if(retries == 0) {
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
      
      globalItemRepositoryRegistry().store(); //Stores all repositories
      
      if(retries) {
        doMoreCleanup(retries-1, false);
        writeLock.lock();
      }

      if(needLockRepository) {
        globalItemRepositoryRegistry().unlockForWriting();
      
        int elapsedSeconds = startTime.secsTo(QTime::currentTime());
        kDebug(9505) << "seconds spent doing cleanup: " << elapsedSeconds;
      }
      if(!retries) {
        int elapesedMilliSeconds = startTime.msecsTo(QTime::currentTime());
        kDebug(9505) << "milliseconds spent doing cleanup with locked duchain: " << elapesedMilliSeconds;
      }
      
      foreach(QReadWriteLock* lock, locked)
        lock->unlock();
  }
  
  ///Loads/gets the environment-information for the given top-context index, or returns zero if none exists
  ParsingEnvironmentFile* loadInformation(IndexedString url, uint topContextIndex) {
    
    {
      //Step one: Check if the info is already loaded on m_fileEnvironmentInformations
      QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator start = m_fileEnvironmentInformations.lowerBound(url);
      QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator end = m_fileEnvironmentInformations.upperBound(url);
      
      while(start != end) {
        if((*start)->indexedTopContext().index() == topContextIndex)
          return start->data();
        ++start;
      }
    }
    
    //Step two: Check if it is on disk, and if is, load it
    uint dataIndex = m_environmentInfo.findIndex(EnvironmentInformationRequest(topContextIndex));
    if(!dataIndex) {
      //No environment-information stored for this top-context
      return 0;
    }

    const EnvironmentInformationItem& item(*m_environmentInfo.itemFromIndex(dataIndex));
    
    ParsingEnvironmentFile* ret = dynamic_cast<ParsingEnvironmentFile*>(DUChainItemSystem::self().create( (DUChainBaseData*)(((char*)&item) + sizeof(EnvironmentInformationItem)) ));
    if(ret) {
      Q_ASSERT(ret->d_func()->classId);
      
      m_fileEnvironmentInformations.insert(url, ParsingEnvironmentFilePointer(ret));
      Q_ASSERT(ret->url() == url);
    }
    return ret;
  }
  
private:

  template<class Entry>
  bool listContains(const Entry entry, const Entry* list, uint listSize) {
    for(uint a = 0; a < listSize; ++a)
      if(list[a] == entry)
        return true;
    return false;
  }
  
  ///Stores the environment-information for the given url
  void storeInformationList(IndexedString url) {

    QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator start = m_fileEnvironmentInformations.lowerBound(url);
    QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator end = m_fileEnvironmentInformations.upperBound(url);

    if(start == end)
        return;

    uint index = m_environmentListInfo.findIndex(url);
    bool listChanged = true;
    if(index) {
      listChanged = false;
      const EnvironmentInformationListItem* item = m_environmentListInfo.itemFromIndex(index);
      ///@todo Make the list sorted, so the "contains" function is efficient
      for(QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it = start; it != end; ++it) {
        if(!listContains((*it)->indexedTopContext().index(), item->items(), item->itemsSize())) {
          listChanged = true;
          break;
        }
      }
    }
    
    if(listChanged) {
      ///Update/insert a new list
      uint index = m_environmentListInfo.findIndex(url);
      EnvironmentInformationListItem item;
      
      if(index) {
        item.copyListsFrom(*m_environmentListInfo.itemFromIndex(index));
        m_environmentListInfo.deleteItem(index); //Remove the previous item
      }

      Q_ASSERT(m_environmentListInfo.findIndex(EnvironmentInformationListRequest(url)) == 0);
      
      for(QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it = start; it != end; ++it)
        if(!listContains((*it)->indexedTopContext().index(), item.items(), item.itemsSize()))
          item.itemsList().append( (*it)->indexedTopContext().index() );
      
      //Insert the new item
      m_environmentListInfo.index(EnvironmentInformationListRequest(url, item));
    }

    Q_ASSERT(m_environmentListInfo.findIndex(EnvironmentInformationListRequest(url)));
  }

  QMultiMap<IndexedString, ParsingEnvironmentFilePointer> m_fileEnvironmentInformations;
  ///Maps filenames to a list of top-contexts/environment-informations
  ItemRepository<EnvironmentInformationListItem, EnvironmentInformationListRequest> m_environmentListInfo;
  ///Maps top-context-indices to environment-information item
  ItemRepository<EnvironmentInformationItem, EnvironmentInformationRequest> m_environmentInfo;
};

K_GLOBAL_STATIC(DUChainPrivate, sdDUChainPrivate)


DUChain::DUChain()
{
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));

  connect(EditorIntegrator::notifier(), SIGNAL(documentAboutToBeDeleted(KTextEditor::Document*)), SLOT(documentAboutToBeDeleted(KTextEditor::Document*)));
  if(ICore::self()) {
    Q_ASSERT(ICore::self()->documentController());
    connect(ICore::self()->documentController(), SIGNAL(documentLoadedPrepare(KDevelop::IDocument*)), this, SLOT(documentLoadedPrepare(KDevelop::IDocument*)));
    connect(ICore::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), this, SLOT(documentActivated(KDevelop::IDocument*)));
  }
}

DUChain::~DUChain()
{
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
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  if(sdDUChainPrivate->m_referenceCounts.contains(context)) {
  //This happens during shutdown, since everything is unloaded
  kDebug() << "removed a top-context that was reference-counted:" << context->url().str() << context->ownIndex();
  sdDUChainPrivate->m_referenceCounts.remove(context);
  }
  
  uint index = context->ownIndex();

//   kDebug(9505) << "duchain: removing document" << context->url().str();
  google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::iterator it = sdDUChainPrivate->m_chainsByIndex.find(index);
  
  if (it != sdDUChainPrivate->m_chainsByIndex.end()) {
    sdDUChainPrivate->m_chainsByUrl.remove(context->url(), context);

    if (context->smartRange())
      ICore::self()->languageController()->backgroundParser()->removeManagedTopRange(context->smartRange());

    branchRemoved(context);

    if(!context->isOnDisk())
      removeFromEnvironmentManager(context);

    context->deleteSelf();

    sdDUChainPrivate->m_chainsByIndex.erase(it);
  }
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
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
  if( !file )
    return; //We don't need to manage
  
  if(ParsingEnvironmentFile* alreadyHave = sdDUChainPrivate->findEnvironmentFor(file->indexedTopContext())) 
  {
    ///If this triggers, there has already been another environment-information registered for this top-context.
    ///removeFromEnvironmentManager should have been called before to remove the old environment-information.
    Q_ASSERT(alreadyHave == file.data());
    return;
  }

  sdDUChainPrivate->addEnvironmentInformation(file);
}

void DUChain::removeFromEnvironmentManager( TopDUContext * chain ) {
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

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
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
  google::dense_hash_map<uint, TopDUContext*, ItemRepositoryIndexHash>::const_iterator it = sdDUChainPrivate->m_chainsByIndex.find(index);
  if(it != sdDUChainPrivate->m_chainsByIndex.end())
    return (*it).second->url();
  return TopDUContextDynamicData::loadUrl(index);
}


TopDUContext* DUChain::chainForIndex(uint index) const {

  DUChainPrivate* p = (sdDUChainPrivate.operator->());
  if(p->m_destroyed)
    return 0;

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
  QList<ParsingEnvironmentFilePointer> list = sdDUChainPrivate->getEnvironmentInformation(document);
  foreach(const ParsingEnvironmentFilePointer &file, list) {
    if(isInMemory(file->indexedTopContext().index()))
      return file->indexedTopContext().data();
  }
  if(!list.isEmpty())
    return list[0]->topContext();

//   kDebug(9505) << "No chain found for document " << document.toString();

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
  for (QMultiMap<IndexedString, TopDUContext*>::Iterator it = sdDUChainPrivate->m_chainsByUrl.lowerBound(document); it != sdDUChainPrivate->m_chainsByUrl.constEnd(); ++it) {
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

  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
  QList< ParsingEnvironmentFilePointer> list = sdDUChainPrivate->getEnvironmentInformation(document);
  l.unlock();
  
//   kDebug() << document.str() << ": matching" << list.size() << (onlyProxyContexts ? "proxy-contexts" : (noProxyContexts ? "content-contexts" : "contexts"));
  
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
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
  return sdDUChainPrivate->getEnvironmentInformation(document);
}

ParsingEnvironmentFilePointer DUChain::environmentFileForDocument(IndexedTopDUContext topContext) const {
   if(topContext.index() == 0)
     return ParsingEnvironmentFilePointer();

   QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
   
   return ParsingEnvironmentFilePointer(sdDUChainPrivate->loadInformation(topContext.url(), topContext.index()));
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
  DUChainWriteLocker lock( DUChain::lock() );
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);
  TopDUContext* ctx = DUChainUtils::standardContextForUrl(doc->url());
  if(ctx && ctx->parsingEnvironmentFile())
    if(ctx->parsingEnvironmentFile()->needsUpdate())
      ICore::self()->languageController()->backgroundParser()->addDocument(doc->url());
}

void DUChain::documentAboutToBeDeleted(KTextEditor::Document* doc)
{
  if(sdDUChainPrivate->m_destroyed)
    return;
  QList<TopDUContext*> chains = chainsForDocument(doc->url());

  EditorIntegrator editor;
  SmartConverter sc(&editor);

  foreach (TopDUContext* top, chains) {
    DUChainWriteLocker lock( DUChain::lock() );
    sc.deconvertDUChain( top );
  }

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
      if(!standardContext->smartRange()) {
        sc.convertDUChain(standardContext);
        Q_ASSERT(standardContext->smartRange());
      }else{
        kWarning() << "Strange: context already has smart-range";
      }
      ICore::self()->languageController()->backgroundParser()->addManagedTopRange(doc->url(), standardContext->smartRange());
    }
    
    sdDUChainPrivate->m_openDocumentContexts.insert(standardContext);
    
    foreach( KDevelop::ILanguage* language, languages)
      if(language->languageSupport() && language->languageSupport()->codeHighlighting())
        language->languageSupport()->codeHighlighting()->highlightDUChain(standardContext);

    if(!standardContext->smartRange() || (standardContext->parsingEnvironmentFile() && standardContext->parsingEnvironmentFile()->needsUpdate()) || !(standardContext->features() & TopDUContext::AllDeclarationsContextsAndUses))
      ICore::self()->languageController()->backgroundParser()->addDocument(doc->url(), TopDUContext::AllDeclarationsContextsAndUses);
  }else{
    ICore::self()->languageController()->backgroundParser()->addDocument(doc->url(), TopDUContext::AllDeclarationsContextsAndUses);
  }
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
  sdDUChainPrivate->doMoreCleanup();;
  sdDUChainPrivate->m_openDocumentContexts.clear();
  sdDUChainPrivate->m_destroyed = true;
  sdDUChainPrivate->clear();
}

uint DUChain::newTopContextIndex() {
  static QAtomicInt& currentId( globalItemRepositoryRegistry().getCustomCounter("Top-Context Counter", 1) );
  return currentId.fetchAndAddRelaxed(1);
}

void DUChain::refCountUp(TopDUContext* top) {
  DUChainReadLocker l(lock());
  if(!sdDUChainPrivate->m_referenceCounts.contains(top))
    sdDUChainPrivate->m_referenceCounts.insert(top, 1);
  else
    ++sdDUChainPrivate->m_referenceCounts[top];
}

void DUChain::refCountDown(TopDUContext* top) {
  DUChainReadLocker l(lock());
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


}

#include "duchain.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
