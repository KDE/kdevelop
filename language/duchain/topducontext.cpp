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

#include <limits>
#include <ext/hash_map>

#include <QThread>

#include "../editor/hashedstring.h"
#include "../interfaces/iproblem.h"

#include "symboltable.h"
#include "declaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "parsingenvironment.h"
#include "duchainpointer.h"
#include "declarationid.h"
#include "namespacealiasdeclaration.h"
#include "aliasdeclaration.h"
#include "abstractfunctiondeclaration.h"
#include "uses.h"
#include "arrayhelpers.h"
#include "topducontextdata.h"
#include "duchainregister.h"
#include "topducontextdynamicdata.h"

//#define DEBUG_SEARCH

using namespace KTextEditor;

//Do visibility-caching when more then X items are found.
const int visibilityCachingMargin = 10;

namespace std {
  using namespace __gnu_cxx;
}

namespace KDevelop
{
ReferencedTopDUContext::ReferencedTopDUContext(TopDUContext* context) : m_topContext(context) {
  if(m_topContext)
    DUChain::self()->refCountUp(m_topContext);
}

ReferencedTopDUContext::ReferencedTopDUContext(const ReferencedTopDUContext& rhs) : m_topContext(rhs.m_topContext) {
  if(m_topContext)
    DUChain::self()->refCountUp(m_topContext);
}

ReferencedTopDUContext::~ReferencedTopDUContext() {
  if(m_topContext)
    DUChain::self()->refCountDown(m_topContext);
}

ReferencedTopDUContext& ReferencedTopDUContext::operator=(const ReferencedTopDUContext& rhs) {
  if(m_topContext == rhs.m_topContext)
    return;
  
  if(m_topContext)
    DUChain::self()->refCountDown(m_topContext);
  
  m_topContext = rhs.m_topContext;
  
  if(m_topContext)
    DUChain::self()->refCountUp(m_topContext);
  return *this;
}

IndexedTopDUContext::IndexedTopDUContext(TopDUContext* context) {
  if(context)
    m_index = context->ownIndex();
  else
    m_index = 0;
}

bool IndexedTopDUContext::isLoaded() const {
  if(m_index)
    return DUChain::self()->isInMemory(m_index);
  else
    return false;
}

TopDUContext* IndexedTopDUContext::data() const {
  if(m_index)
    return DUChain::self()->chainForIndex(m_index);
  else
    return 0;
}

DEFINE_LIST_MEMBER_HASH(TopDUContextData, m_usedDeclarationIds, DeclarationId)
REGISTER_DUCHAIN_ITEM(TopDUContext);

class TopDUContext::CacheData {
  public:
    CacheData(TopDUContextPointer _context) : context(_context) {
    }
    typedef std::hash_map<uint, QVector<DeclarationPointer> > HashType;
    HashType visibleDeclarations; //Contains cached visible declarations. Visible means that they are imported, and does not respect include-positions or similar
    TopDUContextPointer context;
};

struct TopDUContext::AliasChainElement {
  AliasChainElement() { //Creates invalid entries, but we need it fast because it's used to intialize all items in KDevVarLengthArray
  }
  AliasChainElement(const AliasChainElement* _prev, Identifier id) : previous(_prev), ownsPrevious(false), identifier(id), hash(0), length(0) {
    if(previous) {
      length = previous->length + 1;
      hash = QualifiedIdentifier::combineHash(previous->hash, previous->length, identifier);
    } else{
      length = 1;
      hash = QualifiedIdentifier::combineHash(0, 0, identifier);
    }
  }

  //Computes the identifier represented by this chain element(generally the the identifiers across the "previous" chain reversed
  QualifiedIdentifier qualifiedIdentifier() const {
    QualifiedIdentifier ret;
    if(previous)
      ret = previous->qualifiedIdentifier();
    ret.push(identifier);
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
  Identifier identifier;
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

template <class T>
void removeFromVector(QVector<T>& vec, const T& t) {
  for(int a  =0; a < vec.size(); ++a) {
    if(vec[a] == t) {
      vec.remove(a);
      removeFromVector(vec, t);
      return;
    }
  }
}

QMutex importStructureMutex(QMutex::Recursive);

template<class Container>
bool removeOneImport(Container& container, const DUContext* value) {
  for(int a = 0; a < container.size(); ++a) {
    if(container[a].context() == value) {
      removeFromArray(container, a);
      return true;
    }
  }
  return false;
}

//Contains data that is not shared among top-contexts that share their duchain entries
class TopDUContextLocalPrivate {
public:
  TopDUContextLocalPrivate (TopDUContext* ctxt, TopDUContext* sharedDataOwner, uint index) :
    m_ctxt(ctxt), m_sharedDataOwner(sharedDataOwner), m_ownIndex(index), m_inDuChain(false)
  {
    if(sharedDataOwner) {
      Q_ASSERT(!sharedDataOwner->m_local->m_sharedDataOwner);
      sharedDataOwner->m_local->m_dataUsers.insert(m_ctxt);
      
      foreach(const DUContext::Import& import, m_sharedDataOwner->m_local->m_importedContexts)
        if(dynamic_cast<TopDUContext*>(import.context()))
          dynamic_cast<TopDUContext*>(import.context())->m_local->m_directImporters.insert(m_ctxt);
    }
  }
  
  mutable QHash<Qt::HANDLE,TopDUContext::CacheData*> m_threadCaches;
  
  TopDUContext::CacheData* currentCache() const {
    QHash<Qt::HANDLE,TopDUContext::CacheData*>::iterator it = m_threadCaches.find(QThread::currentThreadId());
    if(it != m_threadCaches.end())
      return *it;
    else
      return 0;
  }
  
  ~TopDUContextLocalPrivate() {
    //Either we use some other contexts data and have no users, or we own the data and have users that share it.
    QMutexLocker lock(&importStructureMutex);
    
    Q_ASSERT(!m_sharedDataOwner || m_dataUsers.isEmpty());
    
    if(m_sharedDataOwner) {
      Q_ASSERT(m_sharedDataOwner->m_local->m_dataUsers.contains(m_ctxt));
      m_sharedDataOwner->m_local->m_dataUsers.remove(m_ctxt);
      
      if(!m_sharedDataOwner->m_local->m_dataUsers.isEmpty()) {
        //this should not happen, because the users should always be deleted before the owner itself is deleted.
        Q_ASSERT(0);
      }
      
      foreach(const DUContext::Import& import, m_sharedDataOwner->m_local->m_importedContexts)
        if(DUChain::self()->isInMemory(import.topContextIndex()) && dynamic_cast<TopDUContext*>(import.context()))
          dynamic_cast<TopDUContext*>(import.context())->m_local->m_directImporters.remove(m_ctxt);
    }
    foreach(const DUContext::Import& import, m_importedContexts)
      if(DUChain::self()->isInMemory(import.topContextIndex()) && dynamic_cast<TopDUContext*>(import.context()))
        dynamic_cast<TopDUContext*>(import.context())->m_local->m_directImporters.remove(m_ctxt);
  }
  
  //After loading, should rebuild the links
  void rebuildDynamicImportStructure() {
    //Currently we do not store the whole data in TopDUContextLocalPrivate, so we reconstruct it from what was stored by DUContext.
    ///@todo sharing
    Q_ASSERT(m_importedContexts.isEmpty());
    FOREACH_FUNCTION(const DUContext::Import& import, m_ctxt->d_func()->m_importedContexts) {
      if(DUChain::self()->isInMemory(import.topContextIndex())) {
        TopDUContext* top = dynamic_cast<TopDUContext*>(import.context());
        Q_ASSERT(top);
        addImportedContextRecursively(top, false, true);
      }
    }
    FOREACH_FUNCTION(IndexedDUContext importer, m_ctxt->d_func()->m_importers) {
      if(DUChain::self()->isInMemory(importer.topContextIndex())) {
        TopDUContext* top = dynamic_cast<TopDUContext*>(importer.context());
        Q_ASSERT(top);
        top->m_local->addImportedContextRecursively(m_ctxt, false, true);
      }
    }
  }
  
  //Index of this top-context within the duchain
  //Since the data of top-contexts can be shared among multiple, this can be used to add imports that are local to this top-context.
  QVector<DUContext::Import> m_importedContexts;
//   mutable bool m_haveImportStructure : 1;
  TopDUContext* m_ctxt;
  TopDUContext* m_sharedDataOwner; //Either the owner of the shared data, or zero if this context owns the data
  QSet<TopDUContext*> m_dataUsers; //Set of all context that use the data of this context.
  
  QSet<DUContext*> m_directImporters;
  
  ParsingEnvironmentFilePointer m_file;
  QList<ProblemPointer> m_problems;

  uint m_ownIndex;
  
  bool m_inDuChain;
  
  
  void clearImportedContextsRecursively() {
    QMutexLocker lock(&importStructureMutex);
  
    QSet<QPair<TopDUContext*, const TopDUContext*> > rebuild;

    foreach(DUContext::Import import, m_importedContexts) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(import.context());
      if(top) {
        top->m_local->m_directImporters.remove(m_ctxt);
        
        foreach(TopDUContext* user, m_dataUsers)
          user->m_local->removeImportedContextRecursively(top, false);
        
        removeImportedContextRecursion(top, top, 1, rebuild);
        
        QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = top->m_local->m_recursiveImports;
        for(RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
          if(m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == top)
            removeImportedContextRecursion(top, it.key(), it->first+1, rebuild); //Remove all contexts that are imported through the context
        }
      }
    }
    
    m_importedContexts.clear();

    rebuildImportStructureRecursion(rebuild);
  }
  
  //Adds the context to this and all contexts that import this, and manages m_recursiveImports
  void addImportedContextRecursively(TopDUContext* context, bool temporary, bool local) {
    QMutexLocker lock(&importStructureMutex);

    context->m_local->m_directImporters.insert(m_ctxt);
    
    if(local)
      m_importedContexts << DUContext::Import(context);
    
//     if(!m_haveImportStructure)
//       return;
    foreach(TopDUContext* user, m_dataUsers)
      user->m_local->addImportedContextRecursively(context, temporary, false);

//     context->m_local->needImportStructure();

    addImportedContextRecursion(context, context, 1, temporary);

    QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->m_local->m_recursiveImports;
    for(RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it)
      addImportedContextRecursion(context, it.key(), (*it).first+1, temporary); //Add contexts that were imported earlier into the given one
  }

  //Removes the context from this and all contexts that import this, and manages m_recursiveImports
  void removeImportedContextRecursively(TopDUContext* context, bool local) {
    QMutexLocker lock(&importStructureMutex);

    context->m_local->m_directImporters.remove(m_ctxt);
    
    if(local)
      removeFromVector(m_importedContexts, DUContext::Import(context));
    
//     if(!m_haveImportStructure)
//       return;

    foreach(TopDUContext* user, m_dataUsers)
      user->m_local->removeImportedContextRecursively(context, false);
    
    QSet<QPair<TopDUContext*, const TopDUContext*> > rebuild;
    removeImportedContextRecursion(context, context, 1, rebuild);

    QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->m_local->m_recursiveImports;
    for(RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
      if(m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == context)
        removeImportedContextRecursion(context, it.key(), it->first+1, rebuild); //Remove all contexts that are imported through the context
    }

    rebuildImportStructureRecursion(rebuild);
  }

  void removeImportedContextsRecursively(const QList<TopDUContext*>& contexts, bool local) {
    QMutexLocker lock(&importStructureMutex);

//     if(!m_haveImportStructure)
//       return;
    foreach(TopDUContext* user, m_dataUsers)
      user->m_local->removeImportedContextsRecursively(contexts, false);

    QSet<QPair<TopDUContext*, const TopDUContext*> > rebuild;
    foreach(TopDUContext* context, contexts) {
      
      context->m_local->m_directImporters.remove(m_ctxt);
      
      if(local)
        removeFromVector(m_importedContexts, DUContext::Import(context));
      
      removeImportedContextRecursion(context, context, 1, rebuild);

      QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->m_local->m_recursiveImports;
      for(RecursiveImports::const_iterator it = b.constBegin(); it != b.constEnd(); ++it) {
        if(m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == context)
          removeImportedContextRecursion(context, it.key(), it->first+1, rebuild); //Remove all contexts that are imported through the context
      }
    }

    rebuildImportStructureRecursion(rebuild);
  }

/*  void needImportStructure() const {
    return; //We always have an import-structure now*/
//     if(m_haveImportStructure)
//       return;

/*    for(QVector<DUContextPointer>::const_iterator parentIt = m_importedParentContexts.constBegin(); parentIt != m_importedParentContexts.constEnd(); ++parentIt) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(parentIt->data())); //To avoid detaching, use const iterator
      if(top) {
        RecursiveImports::iterator it = m_recursiveImports.find(top);
        if(it == m_recursiveImports.end() || it->first != 1) {

          if(it == m_recursiveImports.end())
            m_recursiveImports.insert(top, qMakePair(1, const_cast<const TopDUContext*>(top)));
          else
            *it = qMakePair(1, const_cast<const TopDUContext*>(top));

          top->m_local->needImportStructure();

          for(RecursiveImports::const_iterator importIt = top->m_local->m_recursiveImports.constBegin(); importIt != top->m_local->m_recursiveImports.constEnd(); ++importIt) {
            it = m_recursiveImports.find(importIt.key());
            if(it == m_recursiveImports.end())
              m_recursiveImports.insert(importIt.key(), qMakePair(importIt->first+1, const_cast<const TopDUContext*>(top)));
            else if(it->first > importIt->first+1)
              *it = qMakePair(importIt->first+1, const_cast<const TopDUContext*>(top)); //Found a shorter path
          }
        }
      }
    }*/
//     m_haveImportStructure = true;
//   }

  //Has an entry for every single recursively imported file, that contains the shortest path, and the next context on that path to the imported context.
  //This does not need to be stored to disk, because it is defined implicitly.
  //What makes this most complicated is the fact that loops are allowed in the import structure.
  typedef QHash<const TopDUContext*, QPair<int, const TopDUContext*> > RecursiveImports;
  mutable RecursiveImports m_recursiveImports;
  private:

//     void childClosure(QSet<TopDUContext*>& children) {
//       if(children.contains(m_ctxt))
//         return;
//       children.insert(m_ctxt);
//       for(QVector<DUContext*>::const_iterator it = m_importedChildContexts.constBegin(); it != m_importedChildContexts.constEnd(); ++it) {
//         TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(*it)); //We need to do const cast, to avoid senseless detaching
//         if(top)
//           top->m_local->childClosure(children);
//       }
//     }

  void addImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, int depth, bool temporary = false) {

//     if(!m_haveImportStructure)
//       return;

    if(imported == m_ctxt)
      return;

    const bool computeShortestPaths = false;  ///@todo We do not compute the shortest path. Think what's right.

//     traceNext->m_local->needImportStructure();
//     imported->m_local->needImportStructure();

    RecursiveImports::iterator it = m_recursiveImports.find(imported);
    if(it == m_recursiveImports.end()) {
      //Insert new path to "imported"
      m_recursiveImports[imported] = qMakePair(depth, traceNext);
      Q_ASSERT(traceNext != m_ctxt);
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
        Q_ASSERT(traceNext == imported || (traceNext->m_local->m_recursiveImports.contains(imported) && traceNext->m_local->m_recursiveImports[imported].first < (*it).first));
      }else{
        //The imported context is already imported through a same/better path, so we can just stop processing. This saves us from endless recursion.
        return;
      }
    }

    if(temporary)
      return;

    for(QSet<DUContext*>::const_iterator it = m_directImporters.constBegin(); it != m_directImporters.constEnd(); ++it) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(*it)); //Avoid detaching, so use const_cast
      if(top) ///@todo also record this for local imports
        top->m_local->addImportedContextRecursion(m_ctxt, imported, depth+1);
    }
  }

  void removeImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, int distance, QSet<QPair<TopDUContext*, const TopDUContext*> >& rebuild) {

    if(imported == m_ctxt)
      return;

//     if(!m_haveImportStructure)
//       return;

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
        for(QSet<DUContext*>::const_iterator childIt = m_directImporters.constBegin(); childIt != m_directImporters.constEnd(); ++childIt) {
          TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(*childIt)); //Avoid detaching, so use const iterator
          if(top)
            top->m_local->removeImportedContextRecursion(m_ctxt, imported, distance+1, rebuild); //Don't use 'it' from here on, it may be invalid
        }
      }
    }
  }

  //Updates the trace to 'imported'
  void rebuildStructure(const TopDUContext* imported);

  void rebuildImportStructureRecursion(const QSet<QPair<TopDUContext*, const TopDUContext*> >& rebuild) {
    for(QSet<QPair<TopDUContext*, const TopDUContext*> >::const_iterator it = rebuild.constBegin(); it != rebuild.constEnd(); ++it) {
    //for(int a = rebuild.size()-1; a >= 0; --a) {
      //Find the best imported parent
      it->first->m_local->rebuildStructure(it->second);
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
      bool visible = top->m_local->m_recursiveImports.contains(static_cast<const TopDUContext*>(otherTop));
      if(createVisibleCache && visible)
        createVisibleCache->append(DeclarationPointer(dec));

      if (dataType && dec->abstractType() != dataType)
        // The declaration doesn't match the type filter we are applying
        return false;

      // Make sure that this declaration is accessible
      if (!(flags & DUContext::NoImportsCheck) && !visible)
        return false;
    } else {
      if(createVisibleCache)
        createVisibleCache->append(DeclarationPointer(dec));

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
  const TopDUContextData* topDFunc;
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
//       current->d_func()->needImportStructure();

      TopDUContextLocalPrivate::RecursiveImports::const_iterator it = current->m_local->m_recursiveImports.constFind(target);

      if(it == current->m_local->m_recursiveImports.constEnd())
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

RecursiveImports TopDUContext::recursiveImports() const
{
  ENSURE_CAN_READ
  QMutexLocker lock(&importStructureMutex);
  return m_local->m_recursiveImports;
}

SimpleCursor TopDUContext::importPosition(const DUContext* target) const
{
  ENSURE_CAN_READ
  DUCHAIN_D(DUContext);
  for(unsigned int a = 0; a < d->m_importedContextsSize(); ++a)
    if(d->m_importedContexts()[a].context() == target)
      return d->m_importedContexts()[a].position;
  
  return DUContext::importPosition(target);
}


void TopDUContextLocalPrivate::rebuildStructure(const TopDUContext* imported) {
  if(m_ctxt == imported)
    return;

  for(QVector<DUContext::Import>::const_iterator parentIt = m_importedContexts.constBegin(); parentIt != m_importedContexts.constEnd(); ++parentIt) {
    TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(parentIt->context())); //To avoid detaching, use const iterator
    if(top) {
//       top->m_local->needImportStructure();
      if(top == imported) {
        addImportedContextRecursion(top, imported, 1);
      }else{
        RecursiveImports::const_iterator it2 = top->m_local->m_recursiveImports.find(imported);
        if(it2 != top->m_local->m_recursiveImports.end()) {

          addImportedContextRecursion(top, imported, (*it2).first + 1);
        }
      }
    }
  }
  
  for(unsigned int a = 0; a < m_ctxt->d_func()->m_importedContextsSize(); ++a) {
  TopDUContext* top = dynamic_cast<TopDUContext*>(const_cast<DUContext*>(m_ctxt->d_func()->m_importedContexts()[a].context())); //To avoid detaching, use const iterator
    if(top) {
//       top->m_local->needImportStructure();
      if(top == imported) {
        addImportedContextRecursion(top, imported, 1);
      }else{
        RecursiveImports::const_iterator it2 = top->m_local->m_recursiveImports.find(imported);
        if(it2 != top->m_local->m_recursiveImports.end()) {

          addImportedContextRecursion(top, imported, (*it2).first + 1);
        }
      }
    }
  }
}

void TopDUContext::rebuildDynamicImportStructure() {
  m_local->rebuildDynamicImportStructure();
}

void TopDUContext::rebuildDynamicData(DUContext* parent, uint ownIndex) {
  Q_ASSERT(parent == 0 && ownIndex != 0);
  m_local->m_ownIndex = ownIndex;
  DUContext::rebuildDynamicData(parent, 0);
}

uint TopDUContext::ownIndex() const
{
  return m_local->m_ownIndex;
}

TopDUContext::TopDUContext(TopDUContextData& data) : DUContext(data), m_local(new TopDUContextLocalPrivate(this, 0, DUChain::newTopContextIndex())), m_dynamicData(new TopDUContextDynamicData(this)) {
}

TopDUContext::TopDUContext(const IndexedString& url, const SimpleRange& range, ParsingEnvironmentFile* file)
  : DUContext(*new TopDUContextData(url), range), m_local(new TopDUContextLocalPrivate(this, 0, DUChain::newTopContextIndex())), m_dynamicData(new TopDUContextDynamicData(this))
{
  d_func_dynamic()->setClassId(this);
  
  DUCHAIN_D_DYNAMIC(TopDUContext);
  d_func_dynamic()->setClassId(this);
  d->m_hasUses = false;
  d->m_deleting = false;
  m_local->m_file = ParsingEnvironmentFilePointer(file);
  setInSymbolTable(true);
}

TopDUContext::TopDUContext(TopDUContext* sharedDataOwner, ParsingEnvironmentFile* file)
  : DUContext(*sharedDataOwner), m_local(new TopDUContextLocalPrivate(this, sharedDataOwner, DUChain::newTopContextIndex())), m_dynamicData(sharedDataOwner->m_dynamicData)
{
  m_local->m_file = ParsingEnvironmentFilePointer(file);
}

KSharedPtr<ParsingEnvironmentFile> TopDUContext::parsingEnvironmentFile() const {
  return m_local->m_file;
}

TopDUContext::~TopDUContext( )
{
  if(!m_local->m_sharedDataOwner) {
    d_func_dynamic()->m_deleting = true;
    if(!isOnDisk())
      clearUsedDeclarationIndices();
  }
}

void TopDUContext::deleteSelf() {
  //We've got to make sure that m_dynamicData and m_local are still valid while all the sub-contexts are destroyed
  TopDUContextLocalPrivate* local = m_local;
  TopDUContextDynamicData* dynamicData = m_dynamicData;

  delete this;
  
  delete local;
  delete dynamicData;
}


void TopDUContext::setHasUses(bool hasUses)
{
  d_func_dynamic()->m_hasUses = hasUses;
}

bool TopDUContext::hasUses() const
{
  return d_func()->m_hasUses;
}

void TopDUContext::setParsingEnvironmentFile(ParsingEnvironmentFile* file) {
  m_local->m_file = KSharedPtr<ParsingEnvironmentFile>(file);
}

///Decides whether the cache contains a valid list of visible declarations for the given hash.
///@param hash The hash-value, @param data The cache @param items Will be filled with the cached declarations. Will be left alone if none were found.
void eventuallyUseCache(uint hash, TopDUContext::CacheData* cache, KDevVarLengthArray<Declaration*>& items) {
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
    cache = _top->m_local->currentCache();
  }

  void operator() (const AliasChainElement& element) {
    KDevVarLengthArray<Declaration*> decls;
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
      if(decl->identifier() != element.identifier) ///@todo eventually do more extensive checking
        continue;

      if( decl->kind() == Declaration::Alias ) {
        //Apply alias declarations
        AliasDeclaration* alias = static_cast<AliasDeclaration*>(decl);
        if(alias->aliasedDeclaration().isValid()) {
          decl = alias->aliasedDeclaration().declaration();
        } else {
          kDebug() << "lost aliased declaration";
        }
      }


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
//   d_func()->needImportStructure();

  applyAliases(identifiers, storer, position, false);

  return true;
}

///@todo Implement a cache so at least the global import checks don't need to be done repeatedly. The cache should be thread-local, using DUChainPointer for the hashed items, and when an item was deleted, it should be discarded
template<class Acceptor>
void TopDUContext::applyAliases( const AliasChainElement* backPointer, const SearchItem::Ptr& identifier, Acceptor& accept, const SimpleCursor& position, bool canBeNamespace ) const
{
  ///@todo explicitlyGlobal if the first identifier los global
  bool foundAlias = false;

  AliasChainElement newElement(backPointer, identifier->identifier); //Back-pointer for following elements. Also contains current hash and length.

#ifdef DEBUG_SEARCH
  kDebug() << "checking" << newElement.qualifiedIdentifier().toString();
#endif

  if( !identifier->next.isEmpty() || canBeNamespace ) { //If it cannot be a namespace, the last part of the scope will be ignored

    //Find namespace  aliases
    KDevVarLengthArray<Declaration*> aliases;
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
      DeclarationChecker check(this, position, AbstractType::Ptr(), NoSearchFlags, createVisibleCache);

      //The first part of the identifier has been found as a namespace-alias.
      //In c++, we only need the first alias. However, just to be correct, follow them all for now.
      FOREACH_ARRAY( Declaration* aliasDecl, aliases )
      {
        //Since the visible-cache is computed within check(...), we must always call it when we're computing the cache
        if(!createVisibleCache && aliasDecl->kind() != Declaration::NamespaceAlias)
          continue;

        if(!check(aliasDecl))
          continue;

        if(aliasDecl->kind() != Declaration::NamespaceAlias)
          continue;

        if(foundAlias) {
          if(createVisibleCache) //We've got to walk through all declarations so we create a correct visible-cache
            continue;
          else
            break;
        }

        if(aliasDecl->identifier() != newElement.identifier)  //Since we have retrieved the aliases by hash only, we still need to compare the name
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
        uint count = importIdentifier.count();

        KDevVarLengthArray<AliasChainElement, 5> newChain;
        newChain.resize(count);
        for(uint a = 0; a < count; ++a)
          newChain[a] = AliasChainElement(a == 0 ? 0 : &newChain[a-1], importIdentifier.at(a));

        AliasChainElement* newAliasedElement = &newChain[importIdentifier.count()-1];

        if(identifier->next.isEmpty()) {
          //Just insert the aliased namespace identifier
          accept(*newAliasedElement);
        }else{
          //Create an identifiers where namespace-alias part is replaced with the alias target
          FOREACH_ARRAY(SearchItem::Ptr item, identifier->next)
            applyAliases(newAliasedElement, item, accept, position, canBeNamespace);
        }
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
    AliasChainElement importChainItem(backPointer, globalImportIdentifier);

    KDevVarLengthArray<Declaration*> imports;
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
        KDevVarLengthArray<AliasChainElement, 5> newChain;
        newChain.resize(importIdentifier.count());

        for(int a = 0; a < count; ++a)
          newChain[a] = AliasChainElement(a == 0 ? 0 : &newChain[a-1], importIdentifier.at(a));

        AliasChainElement* newAliasedElement = &newChain[count-1];

#ifdef DEBUG_SEARCH
        kDebug() << "imported" << newAliasedElement->qualifiedIdentifier().toString();
#endif
        //Prevent endless recursion by checking whether we're actually doing a change
        if(!backPointer || newAliasedElement->hash != backPointer->hash || newAliasedElement->qualifiedIdentifier() != backPointer->qualifiedIdentifier())
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
    cache = _top->m_local->currentCache();
  }

  void operator() (const AliasChainElement& element) {
#ifdef DEBUG_SEARCH
    kDebug() << "accepting" << element.qualifiedIdentifier().toString();
#endif
    KDevVarLengthArray<DUContext*> decls;

    SymbolTable::self()->findContextsByHash(element.hash, decls);
    FOREACH_ARRAY(DUContext* ctx, decls) {
      if(!check(ctx))
        continue;

      if(ctx->localScopeIdentifier().last() != element.identifier) ///@todo eventually do more extensive checking
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

TopDUContext* TopDUContext::sharedDataOwner() const
{
  return m_local->m_sharedDataOwner;
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
  if(m_local->m_sharedDataOwner)
    return m_local->m_problems + m_local->m_sharedDataOwner->m_local->m_problems;
  else
    return m_local->m_problems;
}

void TopDUContext::addProblem(const ProblemPointer& problem)
{
  ENSURE_CAN_WRITE
  m_local->m_problems << problem;
}

void TopDUContext::clearProblems()
{
  ENSURE_CAN_WRITE
  m_local->m_problems.clear();
}

QVector<DUContext*> TopDUContext::importers() const
{
  ENSURE_CAN_READ
  return QVector<DUContext*>::fromList( m_local->m_directImporters.toList() );
}

QVector<DUContext::Import> TopDUContext::importedParentContexts() const
{
  ENSURE_CAN_READ

  if(m_local->m_sharedDataOwner)
    return m_local->m_importedContexts + m_local->m_sharedDataOwner->m_local->m_importedContexts;
  else
    return m_local->m_importedContexts;
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
//     d_func()->needImportStructure();
    return m_local->m_recursiveImports.contains(static_cast<const TopDUContext*>(origin));
  } else {
    return DUContext::imports(origin, position);
  }
 }

void TopDUContext::clearImportedParentContexts() {
  m_local->clearImportedContextsRecursively();
  
  if(!m_local->m_sharedDataOwner)
    DUContext::clearImportedParentContexts();
  else {
    foreach (Import parent, m_local->m_importedContexts)
      if( parent.context() )
        removeImportedParentContext(parent.context());
    Q_ASSERT(m_local->m_importedContexts.isEmpty());
  }
  
}

void TopDUContext::addImportedParentContext(DUContext* context, const SimpleCursor& position, bool /*anonymous*/, bool temporary) {
  if(context == this)
    return;
  if(!m_local->m_sharedDataOwner) //Always make the contexts anonymous, because we care about importers in TopDUContextLocalPrivate
    DUContext::addImportedParentContext(context, position, true, temporary); 
  
  m_local->addImportedContextRecursively(static_cast<TopDUContext*>(context), temporary, true);
}

void TopDUContext::removeImportedParentContext(DUContext* context) {
  if(!m_local->m_sharedDataOwner)
    DUContext::removeImportedParentContext(context);
  
  m_local->removeImportedContextRecursively(static_cast<TopDUContext*>(context), true);
}

void TopDUContext::addImportedParentContexts(const QList<QPair<TopDUContext*, SimpleCursor> >& contexts, bool temporary) {
  typedef QPair<TopDUContext*, SimpleCursor> Pair;

  foreach(Pair pair, contexts)
    addImportedParentContext(pair.first, pair.second, false, temporary);
}

void TopDUContext::removeImportedParentContexts(const QList<TopDUContext*>& contexts) {
  foreach(TopDUContext* context, contexts)
    DUContext::removeImportedParentContext(context);

  if(!m_local->m_sharedDataOwner)
    m_local->removeImportedContextsRecursively(contexts, true);
}

/// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.)
bool TopDUContext::inDuChain() const {
  return m_local->m_inDuChain;
}

/// This flag is only used by DUChain, never change it from outside.
void TopDUContext::setInDuChain(bool b) {
  m_local->m_inDuChain = b;
}

TopDUContext::Flags TopDUContext::flags() const {
  return d_func()->m_flags;
}

void TopDUContext::setFlags(Flags f) {
  d_func_dynamic()->m_flags = f;
}

bool TopDUContext::isOnDisk() const {
  ///@todo Change this to releasingToDisk, and only enable it while saving a top-context to disk.
  return m_dynamicData->isOnDisk();
}

void TopDUContext::clearUsedDeclarationIndices() {
  ENSURE_CAN_WRITE
  for(unsigned int a = 0; a < d_func()->m_usedDeclarationIdsSize(); ++a)
      DUChain::uses()->removeUse(d_func()->m_usedDeclarationIds()[a], this);

  d_func_dynamic()->m_usedDeclarationIdsList().clear();
}

Declaration* TopDUContext::usedDeclarationForIndex(unsigned int declarationIndex) const {
  ENSURE_CAN_READ
  if(declarationIndex < d_func()->m_usedDeclarationIdsSize())
    return d_func()->m_usedDeclarationIds()[declarationIndex].getDeclaration(this);
  else
    return 0;
}

int TopDUContext::indexForUsedDeclaration(Declaration* declaration, bool create) {
  if(create) {
    ENSURE_CAN_WRITE
  }else{
    ENSURE_CAN_READ
  }
  DeclarationId id(declaration->id());

  int index = -1;
  for(unsigned int a = 0; a < d_func()->m_usedDeclarationIdsSize(); ++a)
    if(d_func()->m_usedDeclarationIds()[a] == id) {
      index = a;
      break;
    }

  if(index != -1)
    return index;
  if(!create)
    return std::numeric_limits<int>::max();

  d_func_dynamic()->m_usedDeclarationIdsList().append(id);

  if(declaration->topContext() != this)
    DUChain::uses()->addUse(id, this);

  return d_func()->m_usedDeclarationIdsSize()-1;
}

QList<KTextEditor::SmartRange*> allSmartUses(TopDUContext* context, Declaration* declaration) {
  QList<KTextEditor::SmartRange*> ret;
  int declarationIndex = context->indexForUsedDeclaration(declaration, false);
  if(declarationIndex == std::numeric_limits<int>::max())
    return ret;
  return allSmartUses(context, declarationIndex);
}

QList<SimpleRange> allUses(TopDUContext* context, Declaration* declaration, bool noEmptyRanges) {
  QList<SimpleRange> ret;
  int declarationIndex = context->indexForUsedDeclaration(declaration, false);
  if(declarationIndex == std::numeric_limits<int>::max())
    return ret;
  return allUses(context, declarationIndex, noEmptyRanges);
}

///@todo move this kind of caching into the symbol-table
TopDUContext::Cache::Cache(TopDUContextPointer context) : d(new CacheData(context)) {
  DUChainWriteLocker lock(DUChain::lock());
  if(d->context)
    d->context->m_local->m_threadCaches.insert(QThread::currentThreadId(), d);
}

TopDUContext::Cache::~Cache() {
  DUChainWriteLocker lock(DUChain::lock());
  if(d->context && d->context->m_local->m_threadCaches[QThread::currentThreadId()] == d)
    d->context->m_local->m_threadCaches.remove(QThread::currentThreadId());

  delete d;
}

IndexedString TopDUContext::url() const {
  return d_func()->m_url;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
