/* This  is part of KDevelop
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

#include "duchain.h"
#include "duchainlock.h"

#include <QCoreApplication>

#include <kglobal.h>

#include "editorintegrator.h"

#include "topducontext.h"
#include "parsingenvironment.h"
#include "declaration.h"
#include "definition.h"
#include "use.h"
#include "abstractfunctiondeclaration.h"

namespace KDevelop
{

class DUChainPrivate
{
public:
  DUChainPrivate()
  {
    qRegisterMetaType<DUChainBasePointer>("KDevelop::DUChainBasePointer");
    qRegisterMetaType<DUContextPointer>("KDevelop::DUContextPointer");
    qRegisterMetaType<TopDUContextPointer>("KDevelop::TopDUContextPointer");
    qRegisterMetaType<DeclarationPointer>("KDevelop::DeclarationPointer");
    qRegisterMetaType<UsePointer>("KDevelop::UsePointer");
    qRegisterMetaType<DefinitionPointer>("KDevelop::DefinitionPointer");
    qRegisterMetaType<FunctionDeclarationPointer>("KDevelop::FunctionDeclarationPointer");
    qRegisterMetaType<DUChainObserver::Modification>("KDevelop::DUChainObserver::Modification");
    qRegisterMetaType<DUChainObserver::Relationship>("KDevelop::DUChainObserver::Relationship");
      
    notifier = new DUChainObserver();
  }

  DUChain instance;
  DUChainLock lock;
  QMap<IdentifiedFile, TopDUContext*> m_chains;
  QMap<int,ParsingEnvironmentManager*> m_managers;
  DUChainObserver* notifier;

  ParsingEnvironmentManager* managerForType(int type)
  {
    QMap<int,ParsingEnvironmentManager*>::const_iterator it = m_managers.find(type);

    if( it != m_managers.end() )
      return *it;
    else
      return 0;
  }
};

K_GLOBAL_STATIC(DUChainPrivate, sdDUChainPrivate)


DUChain::DUChain()
{
}

DUChain::~DUChain()
{
}

DUChain* DUChain::self()
{
  return &sdDUChainPrivate->instance;
}

DUChainLock* DUChain::lock()
{
  return &sdDUChainPrivate->lock;
}

QList<TopDUContext*> DUChain::allChains() const
{
  return sdDUChainPrivate->m_chains.values();
}

void DUChain::updateContextEnvironment( TopDUContext* context, ParsingEnvironmentFile* file ) {
  ENSURE_CHAIN_WRITE_LOCKED

  removeFromEnvironmentManager( context );
  
  if( context->parsingEnvironmentFile() )
    sdDUChainPrivate->m_chains.remove( context->parsingEnvironmentFile()->identity() );
  
  context->setParsingEnvironmentFile( file );
  
  sdDUChainPrivate->m_chains.insert( context->parsingEnvironmentFile()->identity(), context );
  
  addToEnvironmentManager( context );
}


void DUChain::removeDocumentChain( const  IdentifiedFile& document )
{
  ENSURE_CHAIN_WRITE_LOCKED

  kDebug(9505) << "duchain: removing document" << document.toString();
  sdDUChainPrivate->m_chains.remove(document);
}

void DUChain::addDocumentChain( const IdentifiedFile& document, TopDUContext * chain )
{
  ENSURE_CHAIN_WRITE_LOCKED

  kDebug(9505) << "duchain: adding document" << document.toString() << " " << chain;
  Q_ASSERT(chain);

  if(chainForDocument(document)) {
    ///@todo practically this will result in lost memory, we will currently never delete the overwritten chain. Care about such stuff.
    kDebug(9505) << "duchain: error: A document with the same identity is already in the du-chain";
  }
  
  {
    ///Remove obsolete versions of the document
    IdentifiedFile firstDoc( document.url(), 0 );
    QMap<IdentifiedFile, TopDUContext*>::Iterator it = sdDUChainPrivate->m_chains.lowerBound(firstDoc);

    ModificationRevision rev = EditorIntegrator::modificationRevision( document.url() );

    for( ;it != sdDUChainPrivate->m_chains.end() && it.key().url() == document.url(); )
    {
      ModificationRevision thisRev = (*it)->parsingEnvironmentFile()->modificationRevision();
      
      if( (*it)->parsingEnvironmentFile() && !(thisRev == rev)  )
      {
        //Don't remove obsolete chains here, because the caller may not be prepared to it, and also the chains may yet be updated.
/*        kDebug(9505) << "duchain: removing obsolete document " << (*it)->parsingEnvironmentFile()->identity().toString() << " from du-chain. Current rev.: " << rev << " document's rev.: " << (*it)->parsingEnvironmentFile()->modificationRevision();

        if( ParsingEnvironmentManager* manager = sdDUChainPrivate->managerForType((*it)->parsingEnvironmentFile()->type() ) )
          manager->removeFile( (*it)->parsingEnvironmentFile().data() );
        
        delete *it;
        it = sdDUChainPrivate->m_chains.erase(it);
        continue;*/
      } else {
        kDebug(9505) << "duchain: leaving other version of document " << (*it)->parsingEnvironmentFile()->identity().toString() << " in du-chain. Current rev.: " << rev << " document's rev.: " << (*it)->parsingEnvironmentFile()->modificationRevision();
      }
      ++it;
    }
  }

  sdDUChainPrivate->m_chains.insert(document, chain);
  {
    //This is just for debugging, and should be disabled later.
    int realChainCount = 0;
    int proxyChainCount = 0;
    for(QMap<IdentifiedFile, TopDUContext*>::const_iterator it = sdDUChainPrivate->m_chains.begin(); it != sdDUChainPrivate->m_chains.end(); ++it) {
      if((*it)->flags() & TopDUContext::ProxyContextFlag)
        ++proxyChainCount;
      else
        ++realChainCount;
    }
        
    kDebug(9505) << "new count of real chains: " << realChainCount << " proxy-chains: " << proxyChainCount << endl;
  }
  chain->setInDuChain(true);
  addToEnvironmentManager(chain);

  //contextChanged(0L, DUChainObserver::Addition, DUChainObserver::ChildContexts, chain);
  branchAdded(chain);
}

void DUChain::addToEnvironmentManager( TopDUContext * chain ) {
  ENSURE_CHAIN_WRITE_LOCKED

  ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
  if( !file )
    return; //We don't need to manage

  QMap<int,ParsingEnvironmentManager*>::const_iterator it = sdDUChainPrivate->m_managers.find(file->type());

  if( it != sdDUChainPrivate->m_managers.end() ) {
    (*it)->addFile( file.data() );
  } else {
    //No manager available for the type
  }
}

void DUChain::removeFromEnvironmentManager( TopDUContext * chain ) {
  ENSURE_CHAIN_WRITE_LOCKED

  ParsingEnvironmentFilePointer file = chain->parsingEnvironmentFile();
  if( !file )
    return; //We don't need to manage

  QMap<int,ParsingEnvironmentManager*>::const_iterator it = sdDUChainPrivate->m_managers.find(file->type());

  if( it != sdDUChainPrivate->m_managers.end() ) {
    (*it)->removeFile( file.data() );
  } else {
    //No manager available for the type
  }
}

TopDUContext* DUChain::chainForDocument(const KUrl& document) const {
  return chainForDocument( IdentifiedFile(document) );
}

TopDUContext * DUChain::chainForDocument( const IdentifiedFile & document ) const
{
  if (!document.identity()) {
    {
      int count = 0;
      QMap<IdentifiedFile, TopDUContext*>::Iterator it = sdDUChainPrivate->m_chains.lowerBound(document);
      for( ; it != sdDUChainPrivate->m_chains.end() && it.key().url() == document.url(); ++it )
        ++count;
      if( count > 1 )
        kDebug(9505) << "found " << count << " chains for " << document.url();

    }
    // Match any parsed version of this document
    QMap<IdentifiedFile, TopDUContext*>::Iterator it = sdDUChainPrivate->m_chains.lowerBound(document);
    if (it != sdDUChainPrivate->m_chains.constEnd())
      if (it.key().url() == document.url())
        return it.value();

  } else if (sdDUChainPrivate->m_chains.contains(document))
    return sdDUChainPrivate->m_chains[document];

  kDebug(9505) << "No chain found for document " << document.toString() << endl;
    
  return 0;
}

QList<TopDUContext*> DUChain::chainsForDocument(const KUrl& document) const
{
  QList<TopDUContext*> chains;

  // Match all parsed versions of this document
  for (QMap<IdentifiedFile, TopDUContext*>::Iterator it = sdDUChainPrivate->m_chains.lowerBound(IdentifiedFile(document)); it != sdDUChainPrivate->m_chains.constEnd(); ++it) {
    if (it.key().url() == document.url())
      chains << it.value();
    else
      break;
  }

  return chains;
}

TopDUContext* DUChain::chainForDocument( const KUrl& document, const ParsingEnvironment* environment, TopDUContext::Flags flags ) const {

  //Use this struct to search for context that match the specified flags
  struct FlagFileAcceptor : public ParsingEnvironmentFileAcceptor {
    TopDUContext::Flags searchFlags;
    FlagFileAcceptor(TopDUContext::Flags f) : searchFlags(f) {
    }
    virtual bool accept(const ParsingEnvironmentFile& file) {
      if(searchFlags == TopDUContext::AnyFlag)
        return true;
      else {
        TopDUContext* ctx = DUChain::self()->chainForDocument(file.identity());
        if(ctx && ctx->flags() == searchFlags)
          return true;
        else
          return false;
      }
    }
  };

  FlagFileAcceptor acceptor(flags);
  
  QMap<int,ParsingEnvironmentManager*>::const_iterator it = sdDUChainPrivate->m_managers.find(environment->type());
  if( it != sdDUChainPrivate->m_managers.end() ) {
    ParsingEnvironmentFilePointer file( (*it)->find(document, environment, &acceptor) );
    if( !file )
      return 0;

      return chainForDocument( file->identity() );
  } else {
    //No manager available for the type
    return chainForDocument( document );
  }
}

void DUChain::clear()
{
  DUChainWriteLocker writeLock(lock());
  foreach (TopDUContext* context, sdDUChainPrivate->m_chains) {
    KDevelop::EditorIntegrator::releaseTopRange(context->textRangePtr());
    delete context;
  }

  foreach( ParsingEnvironmentManager* manager, sdDUChainPrivate->m_managers )
    manager->clear();

  sdDUChainPrivate->m_chains.clear();
}

DUChainObserver* DUChain::notifier()
{
  return sdDUChainPrivate->notifier;
}

/*void DUChain::contextChanged(DUContext* context, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  emit sdDUChainPrivate->notifier->contextChanged(DUContextPointer(context), change, relationship, DUChainBasePointer(relatedObject));
}

void DUChain::declarationChanged(Declaration* declaration, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  emit sdDUChainPrivate->notifier->declarationChanged(DeclarationPointer(declaration), change, relationship, DUChainBasePointer(relatedObject));
}

void DUChain::definitionChanged(Definition* definition, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  emit sdDUChainPrivate->notifier->definitionChanged(DefinitionPointer(definition), change, relationship, DUChainBasePointer(relatedObject));
}

void DUChain::useChanged(Use* use, DUChainObserver::Modification change, DUChainObserver::Relationship relationship, DUChainBase* relatedObject)
{
  emit sdDUChainPrivate->notifier->useChanged(UsePointer(use), change, relationship, DUChainBasePointer(relatedObject));
}*/

void DUChain::branchAdded(DUContext* context)
{
  emit sdDUChainPrivate->notifier->branchAdded(DUContextPointer(context));
}

void DUChain::deleteDUChainObject(DUChainBase* object)
{
  delete object;
}

void DUChain::addParsingEnvironmentManager( ParsingEnvironmentManager* manager ) {
  ENSURE_CHAIN_WRITE_LOCKED
  Q_ASSERT( sdDUChainPrivate->m_managers.find(manager->type()) == sdDUChainPrivate->m_managers.end() ); //If this fails, there may be multiple managers with the same type, which is wrong

  sdDUChainPrivate->m_managers[manager->type()] = manager;
  }

void DUChain::removeParsingEnvironmentManager( ParsingEnvironmentManager* manager ) {
  ENSURE_CHAIN_WRITE_LOCKED
  QMap<int,ParsingEnvironmentManager*>::iterator it = sdDUChainPrivate->m_managers.find(manager->type());

  if( it != sdDUChainPrivate->m_managers.end() ) {
    Q_ASSERT( *it == manager ); //If this fails, there may be multiple managers with the same type, which is wrong
    sdDUChainPrivate->m_managers.erase(it);
  }
}

QList<KUrl> DUChain::documents() const
{
  QList<KUrl> ret;
  foreach (const IdentifiedFile& file, sdDUChainPrivate->m_chains.keys()) {
    ret << file.url();
  }
  return ret;
}

}

#include "duchain.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
