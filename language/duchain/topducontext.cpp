/* This  is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#include "topducontext.h"

#include <QThread>
#include "symboltable.h"
#include "declaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "parsingenvironment.h"
#include "duchainpointer.h"
#include "declarationid.h"
#include "namespacealiasdeclaration.h"
#include "abstractfunctiondeclaration.h"
#include <hashedstring.h>
#include <iproblem.h>
#include <limits>
#include "uses.h"
#include <ext/hash_map>

#include "ducontext_p.h"

//#define DEBUG_SEARCH

using namespace KTextEditor;

//Do visibility-caching when more then X items are found.
const int visibilityCachingMargin = 10;

namespace std {
  using namespace __gnu_cxx;
};

namespace KDevelop
{

class TopDUContext::CacheData {
  public:
    CacheData(TopDUContextPointer _context) : context(_context) {
    }
    typedef std::hash_map<uint, QVector<DeclarationPointer> > HashType;
    HashType visibleDeclarations; //Contains cached visible declarations. Visible means that they are imported, and does not respect include-positions or similar
    TopDUContextPointer context;
};

struct TopDUContext::AliasChainElement {
  AliasChainElement() { //Creates invalid entries, but we need it fast because it's used to intialize all items in QVarLengthArray
  }
  //id should never be zero for a valid element
  AliasChainElement(const AliasChainElement* _prev, const Identifier* id) : previous(_prev), ownsPrevious(false), identifier(id), hash(0), length(0) {
    if(previous) {
      length = previous->length + 1;
      hash = QualifiedIdentifier::combineHash(previous->hash, previous->length, *identifier);
    } else{
      length = 1;
      hash = QualifiedIdentifier::combineHash(0, 0, *identifier);
    }
  }
  
  //Computes the identifier represented by this chain element(generally the the identifiers across the "previous" chain reversed
  QualifiedIdentifier qualifiedIdentifier() const {
    QualifiedIdentifier ret;
    if(previous)
      ret = previous->qualifiedIdentifier();
    ret.push(*identifier);
#ifdef DEBUG_SEARCH
    if(hash != ret.hash()) {
      kDebug() << "different hash:" << hash << ret.hash();
      Q_ASSERT(0);
    }
#endif
    return ret;
  }
  
  const AliasChainElement* previous;
  bool ownsPrevious;
  const Identifier* identifier;
  uint hash;
  uint length;
};

struct TopDUContext::ContextChecker {

  ContextChecker(const TopDUContext* _top, const SimpleCursor& _position, ContextType _contextType, bool _dontCheckImport) : top(_top), position(_position), contextType(_contextType), dontCheckImport(_dontCheckImport) {
  }
  
  bool operator()(DUContext* context) const {
    const TopDUContext* otherTop = context->topContext();

    if (otherTop != top) {
      if (context->type() != contextType)
        return false;

      // Make sure that this declaration is accessible
      if (!dontCheckImport && !top->importsPrivate(otherTop, position))
        return false;

    } else {
      if (context->type() != contextType)
        return false;

      if (context->range().start > position)
        if(!context->parentContext() || context->parentContext()->type() != Class)
            return false;
    }
    //Success
    return true;
  }
  
  const TopDUContext* top;
  const SimpleCursor& position;
  ContextType contextType;
  bool dontCheckImport;
};


QMutex importStructureMutex(QMutex::Recursive);

class TopDUContextPrivate : public DUContextPrivate
{
public:
  TopDUContextPrivate( TopDUContext* ctxt)
    : DUContextPrivate(ctxt), m_inDuChain(false), m_haveImportStructure(false), m_flags(TopDUContext::NoFlags), m_ctxt(ctxt), m_currentUsedDeclarationIndex(0)
  {
  }

  bool m_hasUses  : 1;
  bool m_deleting : 1;
  bool m_inDuChain : 1;
  mutable bool m_haveImportStructure : 1;
  TopDUContext::Flags m_flags;
  TopDUContext* m_ctxt;
  ParsingEnvironmentFilePointer m_file;
  QList<ProblemPointer> m_problems;

  //Adds the context to this and all contexts that import this, and manages m_recursiveImports
  void addImportedContextRecursively(const TopDUContext* context, bool temporary) {
    QMutexLocker lock(&importStructureMutex);
    
    if(!m_haveImportStructure)
      return;

    context->d_func()->needImportStructure();

    addImportedContextRecursion(context, context, 1, temporary);
    
    QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->d_func()->m_recursiveImports;
    for(RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it)
      addImportedContextRecursion(context, it.key(), (*it).first+1, temporary); //Add contexts that were imported earlier into the given one
  }

  //Removes the context from this and all contexts that import this, and manages m_recursiveImports
  void removeImportedContextRecursively(const TopDUContext* context) {
    QMutexLocker lock(&importStructureMutex);
    
    if(!m_haveImportStructure)
      return;
    
    QSet<QPair<TopDUContext*, const TopDUContext*> > rebuild;
    removeImportedContextRecursion(context, context, 1, rebuild);

    QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->d_func()->m_recursiveImports;
    for(RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
      if(m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == context)
        removeImportedContextRecursion(context, it.key(), it->first+1, rebuild); //Remove all contexts that are imported through the context
    }
    
    rebuildImportStructureRecursion(rebuild);
  }

  void removeImportedContextsRecursively(const QList<TopDUContext*>& contexts) {
    QMutexLocker lock(&importStructureMutex);
    
    if(!m_haveImportStructure)
      return;
    
    QSet<QPair<TopDUContext*, const TopDUContext*> > rebuild;
    foreach(TopDUContext* context, contexts) {
      removeImportedContextRecursion(context, context, 1, rebuild);
  
      QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->d_func()->m_recursiveImports;
      for(RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
        if(m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == context)
          removeImportedContextRecursion(context, it.key(), it->first+1, rebuild); //Remove all contexts that are imported through the context
      }
    }
    
    rebuildImportStructureRecursion(rebuild);
  }
  
  void needImportStructure() const {
    if(m_haveImportStructure)
      return;
    
    for(QVector<DUContextPointer>::const_iterator parentIt = m_importedParentContexts.constBegin(); parentIt != m_importedParentContexts.constEnd(); ++parentIt) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(parentIt->data())); //To avoid detaching, use const iterator
      if(top) {
        RecursiveImports::iterator it = m_recursiveImports.find(top);
        if(it == m_recursiveImports.end() || it->first != 1) {
          
          if(it == m_recursiveImports.end())
            m_recursiveImports.insert(top, qMakePair(1, const_cast<const TopDUContext*>(top)));
          else
            *it = qMakePair(1, const_cast<const TopDUContext*>(top));
          
          top->d_func()->needImportStructure();
          
          for(RecursiveImports::const_iterator importIt = top->d_func()->m_recursiveImports.constBegin(); importIt != top->d_func()->m_recursiveImports.constEnd(); ++importIt) {
            it = m_recursiveImports.find(importIt.key());
            if(it == m_recursiveImports.end())
              m_recursiveImports.insert(importIt.key(), qMakePair(importIt->first+1, const_cast<const TopDUContext*>(top)));
            else if(it->first > importIt->first+1)
              *it = qMakePair(importIt->first+1, const_cast<const TopDUContext*>(top)); //Found a shorter path
          }
        }
      }
    }
    m_haveImportStructure = true;
  }
  
  //Has an entry for every single recursively imported file, that contains the shortest path, and the next context on that path to the imported context.
  //This does not need to be stored to disk, because it is defined implicitly.
  //What makes this most complicated is the fact that loops are allowed in the import structure.
  typedef QHash<const TopDUContext*, QPair<int, const TopDUContext*> > RecursiveImports;
  mutable RecursiveImports m_recursiveImports;

  ///Is used to count up the used declarations while building uses
  uint m_currentUsedDeclarationIndex;

  ///Maps a declarationIndex to a DeclarationId, which is used when the entry in m_usedDeclaration is zero.
  QVector<DeclarationId> m_usedDeclarationIds;
  ///Maps a declarationIndex to an actual used Declaration
  QVector<DeclarationPointer> m_usedDeclarations;

  /**Maps a declarationIndex to local declarations. Generally, negative indices are considered
   * to be indices within m_usedLocalDeclarations, and positive indices within m_usedDeclarationIds
   * Any declarations that are within the same top-context are considered local.
   * */
  QVector<DeclarationPointer> m_usedLocalDeclarations;
  
  mutable QHash<Qt::HANDLE,TopDUContext::CacheData*> m_threadCaches;
  
  TopDUContext::CacheData* currentCache() const {
    QHash<Qt::HANDLE,TopDUContext::CacheData*>::iterator it = m_threadCaches.find(QThread::currentThreadId());
    if(it != m_threadCaches.end())
      return *it;
    else
      return 0;
  }
  private:

    void childClosure(QSet<TopDUContext*>& children) {
      if(children.contains(m_ctxt))
        return;
      children.insert(m_ctxt);
      for(QVector<DUContext*>::const_iterator it = m_importedChildContexts.constBegin(); it != m_importedChildContexts.constEnd(); ++it) {
        TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(*it)); //We need to do const cast, to avoid senseless detaching
        if(top)
          top->d_func()->childClosure(children);
      }
    }
    
  void addImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, int depth, bool temporary = false) {

    if(!m_haveImportStructure)
      return;
    
    if(imported == m_ctxt)
      return;

    const bool computeShortestPaths = false;  ///@todo We do not compute the shortest path. Think what's right.

    traceNext->d_func()->needImportStructure();
    imported->d_func()->needImportStructure();

    RecursiveImports::iterator it = m_recursiveImports.find(imported);
    if(it == m_recursiveImports.end()) {
      //Insert new path to "imported"
      m_recursiveImports[imported] = qMakePair(depth, traceNext);
    }else{
      if(!computeShortestPaths)
        return;

      if(temporary) //For temporary imports, we don't record the best path.
        return;
      //It would be better if we would use the following code, but it creates too much cost in updateImportedContextRecursion when imports are removed again.
      
      //Check whether the new way to "imported" is shorter than the stored one
      if((*it).first > depth) {
        //Add a shorter path
        (*it).first = depth;
        Q_ASSERT(traceNext);
        (*it).second = traceNext;
        Q_ASSERT(traceNext == imported || (traceNext->d_func()->m_recursiveImports.contains(imported) && traceNext->d_func()->m_recursiveImports[imported].first < (*it).first));
      }else{
        //The imported context is already imported through a same/better path, so we can just stop processing. This saves us from endless recursion.
        return;
      }
    }

    if(temporary)
      return;

    for(QVector<DUContext*>::const_iterator it = m_importedChildContexts.constBegin(); it != m_importedChildContexts.constEnd(); ++it) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(*it)); //Avoid detaching, so use const_cast
      if(top)
        top->d_func()->addImportedContextRecursion(m_ctxt, imported, depth+1);
    }
  }

  void removeImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, int distance, QSet<QPair<TopDUContext*, const TopDUContext*> >& rebuild) {

    if(imported == m_ctxt)
      return;
    
    if(!m_haveImportStructure)
      return;
    
    RecursiveImports::iterator it = m_recursiveImports.find(imported);
    if(it == m_recursiveImports.end()) {
      //We don't import. Just return, this saves us from endless recursion.
      return;
    }else{
      //Check whether we have imported "imported" through "traceNext". If not, return. Else find a new trace.
      if((*it).second == traceNext && (*it).first == distance) {
        //We need to remove the import through traceNext. Check whether there is another imported context that imports it.

        m_recursiveImports.erase(it); //In order to prevent problems, we completely remove everything, and re-add it.
                                      //Just updating these complex structures is very hard.
        
        rebuild.insert(qMakePair(m_ctxt, imported));
        //We MUST do this before finding another trace, because else we would create loops
        for(QVector<DUContext*>::const_iterator childIt = m_importedChildContexts.constBegin(); childIt != m_importedChildContexts.constEnd(); ++childIt) {
          TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(*childIt)); //Avoid detaching, so use const iterator
          if(top)
            top->d_func()->removeImportedContextRecursion(m_ctxt, imported, distance+1, rebuild); //Don't use 'it' from here on, it may be invalid
        }
      }
    }
  }

  //Updates the trace to 'imported'
  void rebuildStructure(const TopDUContext* imported) {
    if(m_ctxt == imported)
      return;
    
    for(QVector<DUContextPointer>::const_iterator parentIt = m_importedParentContexts.constBegin(); parentIt != m_importedParentContexts.constEnd(); ++parentIt) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(parentIt->data())); //To avoid detaching, use const iterator
      if(top) {
        top->d_func()->needImportStructure();
        if(top == imported) {
          addImportedContextRecursion(top, imported, 1);
        }else{
          RecursiveImports::const_iterator it2 = top->d_func()->m_recursiveImports.find(imported);
          if(it2 != top->d_func()->m_recursiveImports.end()) {

            addImportedContextRecursion(top, imported, (*it2).first + 1);
          }
        }
      }
    }
  }

  void rebuildImportStructureRecursion(const QSet<QPair<TopDUContext*, const TopDUContext*> >& rebuild) {
    for(QSet<QPair<TopDUContext*, const TopDUContext*> >::const_iterator it = rebuild.begin(); it != rebuild.end(); ++it) {
    //for(int a = rebuild.size()-1; a >= 0; --a) {
      //Find the best imported parent
      it->first->d_func()->rebuildStructure(it->second);
    }
  }
};

///Takes a set of conditions in the constructors, and checks with each call to operator() whether these conditions are fulfilled on the given declaration.
///The import-structure needs to be constructed and locked when this is used
struct TopDUContext::DeclarationChecker {
  DeclarationChecker(const TopDUContext* _top, const SimpleCursor& _position, const AbstractType::Ptr& _dataType, DUContext::SearchFlags _flags, QVector<DeclarationPointer>* _createVisibleCache = 0) : createVisibleCache(_createVisibleCache), top(_top), topDFunc(_top->d_func()), position(_position), dataType(_dataType), flags(_flags) {
  }
  
  bool operator()(Declaration* dec) const {
    const TopDUContext* otherTop = dec->topContext();
    
    if((flags & DUContext::OnlyFunctions) && !dynamic_cast<AbstractFunctionDeclaration*>(dec))
      return false;
    
    if (otherTop != top) {
      bool visible = topDFunc->m_recursiveImports.contains(static_cast<const TopDUContext*>(otherTop));
      if(createVisibleCache && visible)
        createVisibleCache->append(DeclarationPointer(dec));
      
      if (dataType && dec->abstractType() != dataType)
        // The declaration doesn't match the type filter we are applying
        return false;

      // Make sure that this declaration is accessible
      if (!(flags & DUContext::NoImportsCheck) && !visible)
        return false;
    } else {
      if (dataType && dec->abstractType() != dataType)
        // The declaration doesn't match the type filter we are applying
        return false;

      if (dec->range().start >= position)
        if(!dec->context() || dec->context()->type() != DUContext::Class)
            return false; // The declaration is behind the position we're searching from, therefore not accessible
    }
    // Success, this declaration is accessible
    return true;
  }
  
  mutable QVector<DeclarationPointer>* createVisibleCache;
  const TopDUContext* top; 
  const TopDUContextPrivate* topDFunc;
  const SimpleCursor& position;
  const AbstractType::Ptr& dataType;
  DUContext::SearchFlags flags;
};

ImportTrace TopDUContext::importTrace(const TopDUContext* target) const
{
  ImportTrace ret;
  importTrace(target, ret);
  return ret;
}

void TopDUContext::importTrace(const TopDUContext* target, ImportTrace& store) const
{
    QMutexLocker lock(&importStructureMutex);

    const TopDUContext* current = this;
    while(current != target) {
      current->d_func()->needImportStructure();
    
      TopDUContextPrivate::RecursiveImports::const_iterator it = current->d_func()->m_recursiveImports.find(target);
    
      if(it == current->d_func()->m_recursiveImports.end())
        return;
      
      const TopDUContext* nextContext = (*it).second;

      if(nextContext) {
        store.append(ImportTraceItem(current, current->importPosition(nextContext)));

        current = nextContext;
      }else{
        kWarning() << "inconsistent import-structure";
        return;
      }
  }
}

TopDUContext::TopDUContext(const HashedString& url, const SimpleRange& range, ParsingEnvironmentFile* file)
  : DUContext(*new TopDUContextPrivate(this), url, range)
{
  Q_D(TopDUContext);
  d->m_hasUses = false;
  d->m_deleting = false;
  d->m_file = ParsingEnvironmentFilePointer(file);
  setInSymbolTable(true);
}

IdentifiedFile TopDUContext::identity() const {
  Q_D(const TopDUContext);
  if( d->m_file )
    return d->m_file->identity();
  else
    return IdentifiedFile(url());
}

KSharedPtr<ParsingEnvironmentFile> TopDUContext::parsingEnvironmentFile() const {
  return d_func()->m_file;
}

TopDUContext::~TopDUContext( )
{
  d_func()->m_deleting = true;
  clearDeclarationIndices();
}

void TopDUContext::setHasUses(bool hasUses)
{
  d_func()->m_hasUses = hasUses;
}

bool TopDUContext::hasUses() const
{
  return d_func()->m_hasUses;
}

void TopDUContext::setParsingEnvironmentFile(ParsingEnvironmentFile* file) {
  d_func()->m_file = KSharedPtr<ParsingEnvironmentFile>(file);
}

///Decides whether the cache contains a valid list of visible declarations for the given hash.
///@param hash The hash-value, @param data The cache @param items Will be filled with the cached declarations. Will be left alone if none were found.
void eventuallyUseCache(uint hash, TopDUContext::CacheData* cache, QVarLengthArray<Declaration*>& items) {
  //Check whether we have all visible global items cached
  TopDUContext::CacheData::HashType::iterator it = cache->visibleDeclarations.find( hash );
  if( it != cache->visibleDeclarations.end() ) {
    //This is a little expensive, we need to convert DeclarationPointer items to Declaration*, but there's no other way.
    const DeclarationPointer* decls((*it).second.constData());
    int size = (*it).second.size();
    
    bool hadBad = false;
    
    for(int a = 0; a < size; ++a) {
      if(!decls[a]) { //If a declaration has been deleted, clear the cache
        hadBad = true;
        break;
      }
      items.append( decls[a].data() );
    }
    if(hadBad) {
      cache->visibleDeclarations.erase( it );
      items.clear();
    }
  }
}

struct TopDUContext::FindDeclarationsAcceptor {
  FindDeclarationsAcceptor(const TopDUContext* _top, DeclarationList& _target, const DeclarationChecker& _check) : top(_top), target(_target), check(_check) {
    cache = _top->d_func()->currentCache();
  }
  
  void operator() (const AliasChainElement& element) {
    QVarLengthArray<Declaration*> decls;
#ifdef DEBUG_SEARCH
    kDebug() << "accepting" << element.qualifiedIdentifier().toString();
#endif

    if(cache)
      eventuallyUseCache(element.hash, cache, decls);

    if(decls.isEmpty()) {
      SymbolTable::self()->findDeclarationsByHash(element.hash, decls);
      
      if(decls.size() > visibilityCachingMargin && cache)
        check.createVisibleCache = &(*cache->visibleDeclarations.insert(std::make_pair( element.hash, QVector<DeclarationPointer>())).first).second;
    }
    
    FOREACH_ARRAY(Declaration* decl, decls) {
      if(!check(decl))
        continue;
      if(decl->identifier() != *element.identifier) ///@todo eventually do more extensive checking
        continue;
      
      target.append(decl);
    }
    
    check.createVisibleCache = 0;
  }
  
  const TopDUContext* top;
  CacheData* cache;
  DeclarationList& target;
  const DeclarationChecker& check;
};

bool TopDUContext::findDeclarationsInternal(const SearchItem::PtrList& identifiers, const SimpleCursor& position, const AbstractType::Ptr& dataType, DeclarationList& ret, const TopDUContext* /*source*/, SearchFlags flags) const
{
  ENSURE_CAN_READ
      
#ifdef DEBUG_SEARCH
  FOREACH_ARRAY(SearchItem::Ptr idTree, identifiers)
      foreach(QualifiedIdentifier id, idTree->toList())
        kDebug() << "findDeclarationsInternal" << id.toString();
#endif

  DeclarationChecker check(this, position, dataType, flags);
  FindDeclarationsAcceptor storer(this, ret, check);
  
  ///The actual scopes are found within applyAliases, and each complete qualified identifier is given to FindDeclarationsAcceptor.
  ///That stores the found declaration to the output.
  
  QMutexLocker lock(&importStructureMutex);
  d_func()->needImportStructure();

  applyAliases(identifiers, storer, position, false);
  
  return true;
}

///@todo Implement a cache so at least the global import checks don't need to be done repeatedly. The cache should be thread-local, using DUChainPointer for the hashed items, and when an item was deleted, it should be discarded
template<class Acceptor>
void TopDUContext::applyAliases( const AliasChainElement* backPointer, const SearchItem::Ptr& identifier, Acceptor& accept, const SimpleCursor& position, bool canBeNamespace ) const
{
  ///@todo explicitlyGlobal if the first identifier los global
  bool foundAlias = false;

  AliasChainElement newElement(backPointer, &identifier->identifier); //Back-pointer for following elements. Also contains current hash and length.
  
#ifdef DEBUG_SEARCH
  kDebug() << "checking" << newElement.qualifiedIdentifier().toString();
#endif
  
  if( !identifier->next.isEmpty() || canBeNamespace ) { //If it cannot be a namespace, the last part of the scope will be ignored
    
    //Find namespace  aliases
    QVarLengthArray<Declaration*> aliases;
    QVector<DeclarationPointer>* createVisibleCache = 0;
    
    ///Eventually take a reduced list of declarations from the cache, instead of asking the symbol-store.
    if(accept.cache)
      eventuallyUseCache(newElement.hash, accept.cache, aliases);
    
    if(aliases.isEmpty()) {
      SymbolTable::self()->findDeclarationsByHash( newElement.hash, aliases );
      
      if(aliases.size() > visibilityCachingMargin && accept.cache)
        createVisibleCache = &(*accept.cache->visibleDeclarations.insert(std::make_pair( newElement.hash, QVector<DeclarationPointer>())).first).second;
    }
    
    if(!aliases.isEmpty()) {
#ifdef DEBUG_SEARCH
      kDebug() << "found" << aliases.count() << "aliases";
#endif
      DeclarationChecker check(this, position, AbstractType::Ptr(), NoSearchFlags);
      
      //The first part of the identifier has been found as a namespace-alias.
      //In c++, we only need the first alias. However, just to be correct, follow them all for now.
      FOREACH_ARRAY( Declaration* aliasDecl, aliases )
      {
        if(aliasDecl->kind() != Declaration::NamespaceAlias)
          continue;

        if(!check(aliasDecl))
          continue;
        
        if(aliasDecl->identifier() != *newElement.identifier)  //Since we have retrieved the aliases by hash only, we still need to compare the name
          continue;
        
        Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(aliasDecl));
        
        NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(aliasDecl);
  
        foundAlias = true;
        
        QualifiedIdentifier importIdentifier = alias->importIdentifier();
        
        if(importIdentifier.isEmpty()) {
          kDebug() << "found empty import";
          continue;
        }
        
        //Create a chain of AliasChainElements that represent the identifier
        QVarLengthArray<AliasChainElement, 5> newChain;
        newChain.resize(importIdentifier.count());
        for(int a = 0; a < importIdentifier.count(); ++a)
          newChain[a] = AliasChainElement(a == 0 ? 0 : &newChain[a-1], &importIdentifier.at(a));
      
        AliasChainElement* newAliasedElement = &newChain[importIdentifier.count()-1];
        
        if(identifier->next.isEmpty()) {
          //Just insert the aliased namespace identifier
          accept(*newAliasedElement);
        }else{
          //Create an identifiers where namespace-alias part is replaced with the alias target
          FOREACH_ARRAY(SearchItem::Ptr item, identifier->next)
            applyAliases(newAliasedElement, item, accept, position, canBeNamespace);
        }
        break; //We only need one namespace-alias
      }
    }
  }
  
  if(!foundAlias) { //If we haven't found an alias, put the current versions into the result list. Additionally we will compute the identifiers transformed through "using".
    if(identifier->next.isEmpty()) {
      accept(newElement); //We're at the end of a qualified identifier, accept it
    } else {
      FOREACH_ARRAY(SearchItem::Ptr next, identifier->next)
        applyAliases(&newElement, next, accept, position, canBeNamespace);
    }
  }
  
  /*if( !prefix.explicitlyGlobal() || !prefix.isEmpty() ) {*/ ///@todo check iso c++ if using-directives should be respected on top-level when explicitly global
  ///@todo this is bad for a very big repository(the chains should be walked for the top-context instead)
  
  //Find all namespace-imports at given scope
#ifdef DEBUG_SEARCH
  kDebug() << "checking imports in" << (backPointer ? backPointer->qualifiedIdentifier().toString() : QString("global"));
#endif

  {
    AliasChainElement importChainItem(backPointer, &globalImportIdentifier);
    
    QVarLengthArray<Declaration*> imports;
    QVector<DeclarationPointer>* createVisibleCache = 0;
    
    ///Eventually take a reduced list of declarations from the cache, instead of asking the symbol-store.
    if(accept.cache)
      eventuallyUseCache(importChainItem.hash, accept.cache, imports);
    
    if(imports.isEmpty()) {
      SymbolTable::self()->findDeclarationsByHash( importChainItem.hash, imports );
      
      if(imports.size() > visibilityCachingMargin && accept.cache)
        createVisibleCache = &(*accept.cache->visibleDeclarations.insert(std::make_pair( importChainItem.hash, QVector<DeclarationPointer>())).first).second;
    }
    
    if(!imports.isEmpty()) {
      DeclarationChecker check(this, position, AbstractType::Ptr(), NoSearchFlags, createVisibleCache);
      
      FOREACH_ARRAY( Declaration* importDecl, imports )
      {
        //We must never break or return from this loop, because else we might be creating a bad cache
#ifdef DEBUG_SEARCH
      kDebug() << "found" << imports.size() << "imports";
#endif
        if(!check(importDecl))
          continue;
        if(importDecl->identifier() != globalImportIdentifier) //We need to check, since we've only searched by hash
          continue;
        
        //Search for the identifier with the import-identifier prepended
        Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(importDecl));
        NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(importDecl);
        
#ifdef DEBUG_SEARCH
        kDebug() << "found import of" << alias->importIdentifier().toString();
#endif

        QualifiedIdentifier importIdentifier = alias->importIdentifier();
        
        if(importIdentifier.isEmpty()) {
          kDebug() << "found empty import";
          continue;
        }
        
        int count = importIdentifier.count();
        QVarLengthArray<AliasChainElement, 5> newChain;
        newChain.resize(importIdentifier.count());
        
        for(int a = 0; a < count; ++a)
          newChain[a] = AliasChainElement(a == 0 ? 0 : &newChain[a-1], &importIdentifier.at(a));
      
        AliasChainElement* newAliasedElement = &newChain[count-1];
        
#ifdef DEBUG_SEARCH
        kDebug() << "imported" << newAliasedElement->qualifiedIdentifier().toString();
#endif
        applyAliases(newAliasedElement, identifier, accept, position, canBeNamespace);
      }
    }
  }
}

template<class Acceptor>
void TopDUContext::applyAliases( const SearchItem::PtrList& identifiers, Acceptor& acceptor, const SimpleCursor& position, bool canBeNamespace ) const
{
  FOREACH_ARRAY(SearchItem::Ptr item, identifiers)
    applyAliases(0, item, acceptor, position, canBeNamespace);
}

struct TopDUContext::FindContextsAcceptor {
  FindContextsAcceptor(const TopDUContext* _top, QList<DUContext*>& _target, const ContextChecker& _check) : top(_top), target(_target), check(_check) {
    cache = _top->d_func()->currentCache();
  }
  
  void operator() (const AliasChainElement& element) {
#ifdef DEBUG_SEARCH
    kDebug() << "accepting" << element.qualifiedIdentifier().toString();
#endif
    QVarLengthArray<DUContext*> decls;
    
    SymbolTable::self()->findContextsByHash(element.hash, decls);
    FOREACH_ARRAY(DUContext* ctx, decls) {
      if(!check(ctx))
        continue;
      
      if(ctx->localScopeIdentifier().last() != *element.identifier) ///@todo eventually do more extensive checking
        continue;
      
      target << ctx;
    }
  }
  
  const TopDUContext* top;
  CacheData* cache;
  QList<DUContext*>& target;
  const ContextChecker& check;
};

void TopDUContext::findContextsInternal(ContextType contextType, const SearchItem::PtrList& baseIdentifiers, const SimpleCursor& position, QList<DUContext*>& ret, SearchFlags flags) const {

  ENSURE_CAN_READ
  ContextChecker check(this, position, contextType, flags & DUContext::NoImportsCheck);
  FindContextsAcceptor storer(this, ret, check);
  
  ///The actual scopes are found within applyAliases, and each complete qualified identifier is given to FindContextsAcceptor.
  ///That stores the found declaration to the output.
  applyAliases(baseIdentifiers, storer, position, contextType == Namespace);
}

TopDUContext * TopDUContext::topContext() const
{
  return const_cast<TopDUContext*>(this);
}

bool TopDUContext::deleting() const
{
  return d_func()->m_deleting;
}

QList<ProblemPointer> TopDUContext::problems() const
{
  ENSURE_CAN_READ
  return d_func()->m_problems;
}

void TopDUContext::addProblem(const ProblemPointer& problem)
{
  ENSURE_CAN_WRITE
  d_func()->m_problems << problem;
}

void TopDUContext::clearProblems()
{
  ENSURE_CAN_WRITE
  d_func()->m_problems.clear();
}


bool TopDUContext::imports(const DUContext * origin, const SimpleCursor& position) const
{
  ENSURE_CAN_READ

  return importsPrivate(origin, position);
 }

bool TopDUContext::importsPrivate(const DUContext * origin, const SimpleCursor& position) const
{
  Q_UNUSED(position);
    
  if( dynamic_cast<const TopDUContext*>(origin) ) {
    QMutexLocker lock(&importStructureMutex);
    d_func()->needImportStructure();
    return d_func()->m_recursiveImports.contains(static_cast<const TopDUContext*>(origin));
  } else {
    return DUContext::imports(origin, position);
  }
 }

void TopDUContext::addImportedParentContext(DUContext* context, const SimpleCursor& position, bool anonymous, bool temporary) {
  DUContext::addImportedParentContext(context, position, anonymous, temporary);
  d_func()->addImportedContextRecursively(static_cast<TopDUContext*>(context), temporary);
}

void TopDUContext::removeImportedParentContext(DUContext* context) {
  DUContext::removeImportedParentContext(context);
  d_func()->removeImportedContextRecursively(static_cast<TopDUContext*>(context));
}

void TopDUContext::addImportedParentContexts(const QList<QPair<TopDUContext*, SimpleCursor> >& contexts, bool temporary) {
  typedef QPair<TopDUContext*, SimpleCursor> Pair;

  foreach(Pair pair, contexts)
    addImportedParentContext(pair.first, pair.second, false, temporary);
}

void TopDUContext::removeImportedParentContexts(const QList<TopDUContext*>& contexts) {
  foreach(TopDUContext* context, contexts)
    DUContext::removeImportedParentContext(context);
  
  d_func()->removeImportedContextsRecursively(contexts);
}
 
/// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.)
bool TopDUContext::inDuChain() const {
  return d_func()->m_inDuChain;
}

/// This flag is only used by DUChain, never change it from outside.
void TopDUContext::setInDuChain(bool b) {
  d_func()->m_inDuChain = b;
}

TopDUContext::Flags TopDUContext::flags() const {
  return d_func()->m_flags;
}

void TopDUContext::setFlags(Flags f) {
  d_func()->m_flags = f;
}

void TopDUContext::clearDeclarationIndices() {
  ENSURE_CAN_WRITE
  for(int a = 0; a < d_func()->m_usedDeclarationIds.size(); ++a)
      DUChain::uses()->removeUse(d_func()->m_usedDeclarationIds[a], this);
  
  d_func()->m_usedDeclarations.clear();
  d_func()->m_usedDeclarationIds.clear();
  d_func()->m_usedLocalDeclarations.clear();
}

Declaration* TopDUContext::usedDeclarationForIndex(int declarationIndex) const {
  ENSURE_CAN_READ
  if(declarationIndex < 0) {
    declarationIndex = -(declarationIndex + 1);//Add one, because we have subtracted one in another place
    if(declarationIndex >= 0 && declarationIndex < d_func()->m_usedLocalDeclarations.size())
      return d_func()->m_usedLocalDeclarations[declarationIndex].data();
    else
      return 0;
  }else{
    if(declarationIndex >= 0 && declarationIndex < d_func()->m_usedDeclarationIds.size())
    {
      if(d_func()->m_usedDeclarations[declarationIndex])
        return d_func()->m_usedDeclarations[declarationIndex].data();

      //If no real declaration is available, we need to search the declaration. This is currently not used, we need to think about whether we need it.
      return d_func()->m_usedDeclarationIds[declarationIndex].getDeclaration(const_cast<TopDUContext*>(this));
    }else{
      return 0;
    }
  }
}

int TopDUContext::indexForUsedDeclaration(Declaration* declaration, bool create) {
  if(create) {
    ENSURE_CAN_WRITE
  }else{
    ENSURE_CAN_READ
  }
  DeclarationPointer declarationPtr(declaration);
  if(declaration->topContext() == this) {
    //It is a local declaration, so we need a negative index.
    int index = d_func()->m_usedLocalDeclarations.indexOf(declarationPtr);
    if(index != -1)
      return -index - 1; //Subtract one so it's always negative
    if(!create)
      return std::numeric_limits<int>::max();
    d_func()->m_usedLocalDeclarations.append(declarationPtr);
    return -(d_func()->m_usedLocalDeclarations.count()-1) - 1; //Subtract one so it's always negative
  }else{
    DeclarationId id = declaration->id();
    int index = d_func()->m_usedDeclarationIds.indexOf(id);

    if(index != -1)
      return index;
    if(!create)
      return std::numeric_limits<int>::max();

    d_func()->m_usedDeclarationIds.append(id);
    d_func()->m_usedDeclarations.append(declarationPtr);

    DUChain::uses()->addUse(id, this);
    
    return d_func()->m_usedDeclarationIds.count()-1;
  }
}

QList<KTextEditor::SmartRange*> allSmartUses(TopDUContext* context, Declaration* declaration) {
  QList<KTextEditor::SmartRange*> ret;
  int declarationIndex = context->indexForUsedDeclaration(declaration, false);
  if(declarationIndex == std::numeric_limits<int>::max())
    return ret;
  return allSmartUses(context, declarationIndex);
}

QList<SimpleRange> allUses(TopDUContext* context, Declaration* declaration) {
  QList<SimpleRange> ret;
  int declarationIndex = context->indexForUsedDeclaration(declaration, false);
  if(declarationIndex == std::numeric_limits<int>::max())
    return ret;
  return allUses(context, declarationIndex);
}

///@todo move this kind of caching into the symbol-table
TopDUContext::Cache::Cache(TopDUContextPointer context) : d(new CacheData(context)) {
  DUChainWriteLocker lock(DUChain::lock());
  if(d->context)
    d->context->d_func()->m_threadCaches.insert(QThread::currentThreadId(), d);
}

TopDUContext::Cache::~Cache() {
  DUChainWriteLocker lock(DUChain::lock());
  if(d->context && d->context->d_func()->m_threadCaches[QThread::currentThreadId()] == d)
    d->context->d_func()->m_threadCaches.remove(QThread::currentThreadId());
  
  delete d;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
