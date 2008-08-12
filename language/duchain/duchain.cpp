/* This  is part of KDevelop
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

#include "duchain.h"
#include "duchainlock.h"

#include <QCoreApplication>
#include <QHash>
#include <QMultiMap>
#include <QTimer>
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
#include "topducontextdynamicdata.h"
#include "parsingenvironment.h"
#include "declaration.h"
#include "definitions.h"
#include "duchainutils.h"
#include "use.h"
#include "uses.h"
#include "symboltable.h"
#include "abstractfunctiondeclaration.h"
#include "smartconverter.h"
#include "duchainutils.h"
#include "duchainregister.h"
#include "repositories/itemrepository.h"

namespace KDevelop
{
//This thing is not actually used, but it's needed for compiling
DEFINE_LIST_MEMBER_HASH(EnvironmentInformationItem, sizes, uint)

///Represtens the environment-information for exactly one file in the repository, holding all known instances
class EnvironmentInformationItem {
  public:
  EnvironmentInformationItem() {
    initializeAppendedLists(true);
  }

  ~EnvironmentInformationItem() {
    freeAppendedLists();
  }

  unsigned int hash() const {
    //We only compare the declaration. This allows us implementing a map, although the item-repository
    //originally represents a set.
    return m_file.hash();
  }

  unsigned short int itemSize() const {
    uint dataSize = 0;
    FOREACH_FUNCTION(uint size, sizes)
      dataSize += size;
    return dynamicSize() + dataSize;
  }

  IndexedString m_file;

  uint classSize() const {
    return sizeof(*this);
  }

  START_APPENDED_LISTS(EnvironmentInformationItem);
  ///Contains a size for each contained data item. The items are stored behind the end of this list.
  APPENDED_LIST_FIRST(EnvironmentInformationItem, uint, sizes);
  END_APPENDED_LISTS(EnvironmentInformationItem, sizes);
};

class EnvironmentInformationRequest {
  public:

  ///This constructor should only be used for lookup
  EnvironmentInformationRequest(const IndexedString& file) : m_file(file) {
  }
  ///This is used to actually construct the information in the repository
  EnvironmentInformationRequest(const IndexedString& file, QList<ParsingEnvironmentFilePointer> informations) : m_file(file), m_informations(informations) {
  }

  enum {
    AverageSize = 45 //This should be the approximate average size of an Item
  };

  unsigned int hash() const {
    return m_file.hash();
  }

  size_t itemSize() const {
    uint dataSize = 0;

    foreach(ParsingEnvironmentFilePointer info, m_informations)
      dataSize += DUChainItemSystem::self().dynamicSize( *info->d_func() );

    return sizeof(EnvironmentInformationItem) + sizeof(uint) * m_informations.size() + dataSize;
  }

  void createItem(EnvironmentInformationItem* item) const {
    item->m_file = m_file;
    item->initializeAppendedLists(false);
    item->sizesData = m_informations.size();
    char* pos = ((char*)item) + sizeof(EnvironmentInformationItem);

    //Store the sizes
    foreach(ParsingEnvironmentFilePointer info, m_informations) {
      *(uint*)pos = DUChainItemSystem::self().dynamicSize( *info->d_func() );
      pos += sizeof(uint);
    }
    //Store the data
    foreach(ParsingEnvironmentFilePointer info, m_informations) {
      DUChainItemSystem::self().copy(*info->d_func(), *(DUChainBaseData*)pos, true);
      pos += DUChainItemSystem::self().dynamicSize( *info->d_func() );;
    }
  }

  bool equals(const EnvironmentInformationItem* item) const {
    return m_file == item->m_file;
  }

  IndexedString m_file;
  QList<ParsingEnvironmentFilePointer> m_informations;
};

class DUChainPrivate;
static DUChainPrivate* duChainPrivateSelf = 0;
class DUChainPrivate
{
public:
  DUChainPrivate() : m_chainsMutex(QMutex::Recursive), instance(0), m_destroyed(false), m_environmentInfo("Environment Information")
  {
    m_unloadingEnabled = true;
    duChainPrivateSelf = this;
    qRegisterMetaType<DUChainBasePointer>("KDevelop::DUChainBasePointer");
    qRegisterMetaType<DUContextPointer>("KDevelop::DUContextPointer");
    qRegisterMetaType<TopDUContextPointer>("KDevelop::TopDUContextPointer");
    qRegisterMetaType<DeclarationPointer>("KDevelop::DeclarationPointer");
    qRegisterMetaType<FunctionDeclarationPointer>("KDevelop::FunctionDeclarationPointer");
    //qRegisterMetaType<DUChainObserver::Modification>("KDevelop::DUChainObserver::Modification");
    //qRegisterMetaType<DUChainObserver::Relationship>("KDevelop::DUChainObserver::Relationship");
    qRegisterMetaType<Problem>("KDevelop::Problem");

    notifier = new DUChainObserver();
    instance = new DUChain();

    m_cleanupTimer = new QTimer(instance);
    m_cleanupTimer->setInterval(60000);
    QObject::connect(m_cleanupTimer, SIGNAL(timeout()), instance, SLOT(cleanup()));
  }
  ~DUChainPrivate() {
    delete instance;
  }

  void clear() {
    QMutexLocker l(&m_chainsMutex);

    //Store all top-contexts to disk
    foreach(TopDUContext* context, m_chainsByIndex.values())
      context->m_dynamicData->store();

    foreach (TopDUContext* context, m_chainsByIndex.values())
      instance->removeDocumentChain(context);

    //Store all the environment-information to disk
    while(!m_fileEnvironmentInformations.isEmpty())
      unloadInformation(m_fileEnvironmentInformations.begin().key());

    Q_ASSERT(m_fileEnvironmentInformations.isEmpty());
    Q_ASSERT(m_chainsByUrl.isEmpty());
    Q_ASSERT(m_chainsByIndex.isEmpty());
  }

  ///Must be locked before accessing content of this class
  QMutex m_chainsMutex;

  DUChain* instance;
  QTimer* m_cleanupTimer;
  DUChainLock lock;
  QMultiMap<IndexedString, TopDUContext*> m_chainsByUrl;
  QHash<uint, TopDUContext*> m_chainsByIndex;
  QHash<TopDUContext*, uint> m_referenceCounts;
  DUChainObserver* notifier;
  Definitions m_definitions;
  Uses m_uses;
  
  ///Used to keep alive the top-context that belong to documents loaded in the editor
  QSet<ReferencedTopDUContext> m_openDocumentContexts;

  bool m_destroyed;
  bool m_unloadingEnabled; //When this is true, unreferenced top-contexts are unloaded

  void addEnvironmentInformation(IndexedString url, ParsingEnvironmentFilePointer info) {
    loadInformation(url);
    m_fileEnvironmentInformations.insert(url, info);
  }

  void removeEnvironmentInformation(IndexedString url, ParsingEnvironmentFilePointer info) {
    loadInformation(url);
    m_fileEnvironmentInformations.remove(url, info);
  }

  QList<ParsingEnvironmentFilePointer> getEnvironmentInformation(IndexedString url) {
    loadInformation(url);
    return m_fileEnvironmentInformations.values(url);
  }

  ///Makes sure that the chain with the given index is loaded
  ///@warning m_chainsMutex must NOT be locked when this is called
  void loadChain(uint index, QSet<uint>& loading) {

    QMutexLocker l(&m_chainsMutex);

    if(!m_chainsByIndex.contains(index)) {
      loading.insert(index);
      TopDUContext* chain = TopDUContextDynamicData::load(index);
      if(chain) {
//         kDebug() << "url" << chain->url().str();
        if(EditorIntegrator::documentForUrl(chain->url())) {
          l.unlock(); //Must be unlocked to prevent deadlocks
          {
            EditorIntegrator editor;
            SmartConverter sc(&editor);
            sc.convertDUChain(chain);
          }
          l.relock();
        }

        loadInformation(chain->url());
        for(QMultiMap<IndexedString, ParsingEnvironmentFilePointer>::iterator it = m_fileEnvironmentInformations.lowerBound(chain->url()); it != m_fileEnvironmentInformations.end() && it.key() == chain->url(); ++it) {
          if((*it)->indexedTopContext() == IndexedTopDUContext(index))
            chain->setParsingEnvironmentFile((*it).data());
        }

        l.unlock();
        //Also load all the imported chains, so they are in the symbol table and the import-structure is built
        foreach(DUContext::Import import, chain->DUContext::importedParentContexts()) {
          if(!loading.contains(import.topContextIndex())) {
            loadChain(import.topContextIndex(), loading);
          }
        }
        chain->rebuildDynamicImportStructure();

        chain->setInDuChain(true);
        l.relock();
        instance->addDocumentChain(chain);
      }
      loading.remove(index);
    }
  }
  
  
  ///Stores all environment-information
  ///Also makes sure that all information that stays is referenced, so it stays alive.
  void storeAllInformation() {
    QMutexLocker l(&m_chainsMutex);
    QList<IndexedString> urls = m_fileEnvironmentInformations.keys();
    foreach(IndexedString url, urls) {
      storeInformation(url);
      
      //Access the data in the repository, so the bucket isn't unloaded
      uint index = m_environmentInfo.findIndex(EnvironmentInformationRequest(url));
      Q_ASSERT(index);
      m_environmentInfo.itemFromIndex(index);
    }
  }
  
  void doCleanup() {
    m_fileEnvironmentInformations.size();
    doMoreCleanup();
  }
  
  ///@param force If this is true, no lock timeout is used
  void doMoreCleanup(bool force = false) {
    DUChainWriteLocker writeLock(instance->lock(), force ? 0 : 300);
    if(writeLock.locked()) {
      
      globalItemRepositoryRegistry().lockForWriting();
      
      storeAllInformation(); //Puts environment-information into a repository
      
      QList<TopDUContext*> allLoadedContexts = m_chainsByIndex.values();
      foreach(TopDUContext* context, allLoadedContexts)
        context->m_dynamicData->store();

      //Unload all top-contexts that don't have a reference-count and that are not imported by a referenced one
      QSet<TopDUContext*> referenced;
      foreach(TopDUContext* context, m_referenceCounts.keys())
        addRecursiveImports(referenced, context);
      
      QSet<IndexedString> unloadedNames;
      
      foreach(TopDUContext* context, allLoadedContexts) {
        if(!referenced.contains(context)) {
          //Unload the context, it's not referenced or imported by a referenced context
          kDebug() << "unloading a top-context for" << context->url().str();
          unloadedNames.insert(context->url());
          
          instance->removeDocumentChain(context);
        }
      }
      
      if(m_unloadingEnabled) {
      foreach(IndexedString unloaded, unloadedNames)
        if(!m_chainsByUrl.contains(unloaded))
          //No more top-contexts with the url are loaded, so also unload the environment-info
          unloadInformation(unloaded);
      }

      globalItemRepositoryRegistry().store(); //Stores all repositories

      globalItemRepositoryRegistry().unlockForWriting();
    }
  }
  
private:
  void addRecursiveImports(QSet<TopDUContext*>& contexts, TopDUContext* current);
  
  void loadInformation(IndexedString url) {
    if(m_fileEnvironmentInformations.find(url) != m_fileEnvironmentInformations.end())
      return;

    uint index = m_environmentInfo.findIndex(EnvironmentInformationRequest(url));
    if(!index) {
      //No information there yet for this file
      return;
    }

    const EnvironmentInformationItem& item(*m_environmentInfo.itemFromIndex(index));
    char* pos = (char*)&item;
    pos += item.dynamicSize();
    FOREACH_FUNCTION(uint size, item.sizes) {
      DUChainBase* data = DUChainItemSystem::self().create((DUChainBaseData*)pos);
      Q_ASSERT(dynamic_cast<ParsingEnvironmentFile*>(data));
      m_fileEnvironmentInformations.insert(url, ParsingEnvironmentFilePointer(static_cast<ParsingEnvironmentFile*>(data)));
      pos += size;
    }
  }

  ///Stores the environment-information for the given url
  void storeInformation(IndexedString url) {
    
    QList<ParsingEnvironmentFilePointer> informations = m_fileEnvironmentInformations.values(url);

    //Here we check whether any of the stored items have been changed. If none have been changed(all data is still constant),
    //then we don't need to update.
    bool allInfosConstant = true;
    foreach(ParsingEnvironmentFilePointer info, informations) {
      info->aboutToSave();
      if(info->d_func()->isDynamic())
        allInfosConstant = false;
    }

    if(allInfosConstant) {
      //None of the informations have data that is marked "dynamic". This means it hasn't been changed, and thus we don't
      //need to save anything.
    }else{
      foreach(ParsingEnvironmentFilePointer info, informations) {
        ///@todo Don't do this, instead directly copy data from repository to repository
        info->makeDynamic(); //We have to make everything dynamic, so it survives when we remove the index
      }

      uint index = m_environmentInfo.findIndex(EnvironmentInformationRequest(url));
      if(index)
        m_environmentInfo.deleteItem(index); //Remove the previous item

      Q_ASSERT(m_environmentInfo.findIndex(EnvironmentInformationRequest(url)) == 0);

      //Insert the new item
      m_environmentInfo.index(EnvironmentInformationRequest(url, informations));
      Q_ASSERT(m_environmentInfo.findIndex(EnvironmentInformationRequest(url)));
    }
  }

  //Stores the information into the repository, and removes it from m_fileEnvironmentInformations
  void unloadInformation(IndexedString url) {
    storeInformation(url);

    m_fileEnvironmentInformations.remove(url);
  }

  QMultiMap<IndexedString, ParsingEnvironmentFilePointer> m_fileEnvironmentInformations;
  //Persistent version of m_fileEnvironmentInformations
  ItemRepository<EnvironmentInformationItem, EnvironmentInformationRequest> m_environmentInfo;
};

void DUChainPrivate::addRecursiveImports(QSet<TopDUContext*>& contexts, TopDUContext* current) {
  if(contexts.contains(current))
    return;
  contexts.insert(current);
  for(RecursiveImports::const_iterator it = current->recursiveImports().constBegin(); it != current->recursiveImports().constEnd(); ++it)
    addRecursiveImports(contexts, const_cast<TopDUContext*>(it.key()));
}

K_GLOBAL_STATIC(DUChainPrivate, sdDUChainPrivate)


DUChain::DUChain()
{
  SymbolTable::self(); //Initialize singleton

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
  return sdDUChainPrivate->m_chainsByIndex.values();
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

  uint index = context->ownIndex();

//   kDebug(9505) << "duchain: removing document" << context->url().str();
  if (sdDUChainPrivate->m_chainsByIndex.contains(index)) {
    sdDUChainPrivate->m_chainsByUrl.remove(context->url(), context);

    if (context->smartRange())
      ICore::self()->languageController()->backgroundParser()->removeManagedTopRange(context->smartRange());

    branchRemoved(context);

    if(!context->isOnDisk())
      removeFromEnvironmentManager(context);

    context->deleteSelf();

    sdDUChainPrivate->m_chainsByIndex.remove(index);
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

  Q_ASSERT(!sdDUChainPrivate->m_chainsByIndex.contains(chain->ownIndex()));

  sdDUChainPrivate->m_chainsByIndex.insert(chain->ownIndex(), chain);
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

  sdDUChainPrivate->addEnvironmentInformation(chain->url(), file);
}

void DUChain::removeFromEnvironmentManager( TopDUContext * chain ) {
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
  if( !file )
    return; //We don't need to manage

  sdDUChainPrivate->removeEnvironmentInformation(chain->url(), file);
}

TopDUContext* DUChain::chainForDocument(const KUrl& document) const {
  return chainForDocument(IndexedString(document.pathOrUrl()));
}

bool DUChain::isInMemory(uint topContextIndex) const {
  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  QHash<uint, TopDUContext*>::const_iterator it = sdDUChainPrivate->m_chainsByIndex.find(topContextIndex);
  return it != sdDUChainPrivate->m_chainsByIndex.end();
}

TopDUContext* DUChain::chainForIndex(uint index) const {

  if(sdDUChainPrivate->m_destroyed)
    return 0;

  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  QHash<uint, TopDUContext*>::const_iterator it = sdDUChainPrivate->m_chainsByIndex.find(index);
  if(it != sdDUChainPrivate->m_chainsByIndex.end())
    return *it;
  else {

    l.unlock();
    QSet<uint> loading;
    sdDUChainPrivate->loadChain(index, loading);
    l.relock();

    it = sdDUChainPrivate->m_chainsByIndex.find(index);
    if(it != sdDUChainPrivate->m_chainsByIndex.end())
      return *it;
    else
      return 0;
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
  foreach(ParsingEnvironmentFilePointer file, list) {
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
  return chainsForDocument(IndexedString(document.pathOrUrl()));
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

TopDUContext* DUChain::chainForDocument( const KUrl& document, const ParsingEnvironment* environment, TopDUContext::Flags flags ) const {
  return chainForDocument( IndexedString(document.pathOrUrl()), environment, flags );
}
TopDUContext* DUChain::chainForDocument( const IndexedString& document, const ParsingEnvironment* environment, TopDUContext::Flags flags ) const {

  if(sdDUChainPrivate->m_destroyed)
    return 0;

  QMutexLocker l(&sdDUChainPrivate->m_chainsMutex);

  QList< ParsingEnvironmentFilePointer> list = sdDUChainPrivate->getEnvironmentInformation(document);
  QList< ParsingEnvironmentFilePointer>::const_iterator it = list.constBegin();
  while(it != list.constEnd()) {
    if(*it && (*it)->matchEnvironment(environment)) {
      TopDUContext* ctx = (*it)->topContext();
      if(ctx && (flags == TopDUContext::AnyFlag || ctx->flags() == flags))
        return ctx;
      else if(!ctx) {
        kDebug() << "could not get context from environment-information for" << document.str();
      }
    }
    ++it;
  }

  return 0;
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
  foreach (const TopDUContext* ctx, sdDUChainPrivate->m_chainsByIndex.values())
    ret << KUrl(ctx->url().str());

  return ret;
}

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

  foreach(ReferencedTopDUContext top, sdDUChainPrivate->m_openDocumentContexts) {
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
  SmartConverter sc(&editor);

  QList<TopDUContext*> chains = chainsForDocument(doc->url());

  foreach (TopDUContext* chain, chains) {
    sc.convertDUChain(chain);
    if(chain->smartRange())
      ICore::self()->languageController()->backgroundParser()->addManagedTopRange(doc->url(), chain->smartRange());
  }

  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(doc->url());

  TopDUContext* standardContext = DUChainUtils::standardContextForUrl(doc->url());
  if(standardContext) {
    sdDUChainPrivate->m_openDocumentContexts.insert(standardContext);
    if(!standardContext->smartRange()) {
      //May happen during loading
      sc.convertDUChain(standardContext);
      if(standardContext->smartRange())
        ICore::self()->languageController()->backgroundParser()->addManagedTopRange(doc->url(), standardContext->smartRange());
    }
    foreach( KDevelop::ILanguage* language, languages)
      if(language->languageSupport() && language->languageSupport()->codeHighlighting())
        language->languageSupport()->codeHighlighting()->highlightDUChain(standardContext);

    if(!standardContext->smartRange()) {
      kDebug() << "Could not create smart-range for document during startup";
    }

    if(!standardContext->smartRange() || (standardContext->parsingEnvironmentFile() && standardContext->parsingEnvironmentFile()->needsUpdate()))
      ICore::self()->languageController()->backgroundParser()->addDocument(doc->url());
  }else{
    ICore::self()->languageController()->backgroundParser()->addDocument(doc->url());
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
  DUChainWriteLocker writeLock(lock());
  sdDUChainPrivate->doCleanup();
  sdDUChainPrivate->clear();
  sdDUChainPrivate->m_destroyed = true;
}


uint DUChain::newTopContextIndex() {
  static QAtomicInt& currentId( globalItemRepositoryRegistry().getCustomCounter("Top-Context Counter", 1) );
  return currentId.fetchAndAddRelaxed(1);
}

void DUChain::refCountUp(TopDUContext* top) {
  if(!sdDUChainPrivate->m_referenceCounts.contains(top))
    sdDUChainPrivate->m_referenceCounts.insert(top, 1);
  else
    ++sdDUChainPrivate->m_referenceCounts[top];
}

void DUChain::refCountDown(TopDUContext* top) {
  if(!sdDUChainPrivate->m_referenceCounts.contains(top)) {
    kWarning() << "tried to decrease reference-count for" << top->url().str() << "but this top-context is not referenced";
    return;
  }
  --sdDUChainPrivate->m_referenceCounts[top];
  if(!sdDUChainPrivate->m_referenceCounts[top])
    sdDUChainPrivate->m_referenceCounts.remove(top);

  if(!sdDUChainPrivate->m_cleanupTimer->isActive())
    sdDUChainPrivate->m_cleanupTimer->start();
}

void DUChain::cleanup() {
  ///@todo Move this into a background thread
  sdDUChainPrivate->doCleanup();
  sdDUChainPrivate->m_cleanupTimer->stop();
}

void DUChain::emitDeclarationSelected(DeclarationPointer decl) {
  emit declarationSelected(decl);
}

}

#include "duchain.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
