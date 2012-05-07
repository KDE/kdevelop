/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2006 Hamish Rodda <rodda@kde.org>                       *
 *   Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef ducontextdata_H
#define ducontextdata_H

#include <QtCore/QMutex>
#include <QtCore/QMultiHash>
#include <QtCore/QMap>

#include "../editor/cursorinrevision.h"

#include "duchainbase.h"
#include "ducontext.h"
#include "duchainpointer.h"
#include "declaration.h"
#include "use.h"
#include "../languageexport.h"
#include <util/google/dense_hash_map>

#include "localindexeddeclaration.h"

namespace KDevelop{
class DUContext;

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_childContexts, LocalIndexedDUContext)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_importers, IndexedDUContext)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_importedContexts, DUContext::Import)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_localDeclarations, LocalIndexedDeclaration)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(DUContextData, m_uses, Use)

///This class contains data that needs to be stored to disk
class KDEVPLATFORMLANGUAGE_EXPORT DUContextData : public DUChainBaseData
{
public:
  DUContextData();
  ~DUContextData();
  DUContextData(const DUContextData& rhs);
  DUContext::ContextType m_contextType;
  IndexedQualifiedIdentifier m_scopeIdentifier;
  IndexedDeclaration m_owner;
  typedef DUContext::Import Import;
  START_APPENDED_LISTS_BASE(DUContextData, DUChainBaseData);
  APPENDED_LIST_FIRST(DUContextData, Import, m_importedContexts);
  APPENDED_LIST(DUContextData, LocalIndexedDUContext, m_childContexts, m_importedContexts);
  
  ///@todo Create an additional structure for importing to/from "temporary" contexts and classes in a way that it persists while saving/loading,
  ///      and doesn't require changing a top-contexts data only because a class was derived from.
  APPENDED_LIST(DUContextData, IndexedDUContext, m_importers, m_childContexts);

  ///@warning: Whenever m_localDeclarations is read or written, DUContextDynamicData::m_localDeclarationsMutex must be locked.
  APPENDED_LIST(DUContextData, LocalIndexedDeclaration, m_localDeclarations, m_importers);
  /**
   * Vector of all uses in this context
   * Mutable for range synchronization
   * */
  APPENDED_LIST(DUContextData, Use, m_uses, m_localDeclarations);
  END_APPENDED_LISTS(DUContextData, m_uses);

  bool m_inSymbolTable : 1;
  bool m_anonymousInParent : 1; //Whether this context was added anonymously into the parent. This means that it cannot be found as child-context in the parent.
  bool m_propagateDeclarations : 1;
  private:
    DUContextData& operator=(const DUContextData&) {
      return *this;
    }
};

///This class contains data that is only runtime-dependant and does not need to be stored to disk
class DUContextDynamicData
{
private:
    inline const DUContextData* d_func() { return m_context->d_func(); }
    static inline const DUContextData* ctx_d_func(DUContext* ctx) { return ctx->d_func(); }
    static inline DUContextDynamicData* ctx_dynamicData(DUContext* ctx) { return ctx->m_dynamicData; }

public:
  DUContextDynamicData( DUContext* );
  DUContextPointer m_parentContext;

  TopDUContext* m_topContext;
  
  //Use DeclarationPointer instead of declaration, so we can locate management-problems
  typedef QMultiHash<Identifier, DeclarationPointer> DeclarationsHash;
  
  //Whether this context uses m_localDeclarationsHash
  bool m_hasLocalDeclarationsHash;
  
  static QMutex m_localDeclarationsMutex;
  ///@warning: Whenever m_localDeclarations is read or written, m_localDeclarationsHash must be locked.
  DeclarationsHash m_localDeclarationsHash; //This hash can contain more declarations than m_localDeclarations, due to declarations propagated up from children.
  
  uint m_indexInTopContext; //Index of this DUContext in the top-context
  
  DUContext* m_context;

  mutable bool m_rangesChanged : 1;
   /**
   * Adds a child context.
   *
   * \note Be sure to have set the text location first, so that
   * the chain is sorted correctly.
   */
  void addChildContext(DUContext* context);
  
  /**Removes the context from childContexts
   * @return Whether a context was removed
   * */
  bool removeChildContext(DUContext* context);

  void addImportedChildContext( DUContext * context );
  void removeImportedChildContext( DUContext * context );

  void addDeclaration(Declaration* declaration);
  
  /**Removes the declaration from localDeclarations
   * @return Whether a declaration was removed
   * */
  bool removeDeclaration(Declaration* declaration);

  //Files the scope identifier into target
  void scopeIdentifier(bool includeClasses, QualifiedIdentifier& target) const;
  
  /**
   * This propagates the declaration into the parent search-hashes,
   * up to the first parent that has m_propagateDeclarations set to false.
   * 
   * Must be called with m_localDeclarationsMutex locked
  */
  void addDeclarationToHash(const Identifier& identifer, Declaration* declaration);
  ///Must be called with m_localDeclarationsMutex locked
  void removeDeclarationFromHash(const Identifier& identifer, Declaration* declaration);

  ///Adds all declarations that should be in the hash into the hash
  void enableLocalDeclarationsHash(DUContext* ctx, const Identifier& currentIdentifier = Identifier(), Declaration* currentDecl = 0);
  
  void disableLocalDeclarationsHash();

  bool needsLocalDeclarationsHash();
  
  //Iterates through all visible declarations within a given context, including the ones propagated from sub-contexts
  class VisibleDeclarationIterator {
  public:
    struct StackEntry {
      StackEntry() : data(0), item(0), endItem(0), nextChild(0) {
      }
      
      DUContextDynamicData* data;
      const LocalIndexedDeclaration* item;
      const LocalIndexedDeclaration* endItem;
      uint nextChild;
    };
    
    VisibleDeclarationIterator(DUContextDynamicData* data) {
      current.data = data;
      current.item = data->d_func()->m_localDeclarations();
      current.endItem = current.item + data->d_func()->m_localDeclarationsSize();
      current.nextChild = 0;
      toValidPosition();
    }
    
    Declaration* operator*() const {
      return current.item->data(current.data->m_topContext);
    }
    
    VisibleDeclarationIterator& operator++() {
      ++current.item;
      toValidPosition();
      return *this;
    }
    
    operator bool() const {
      return (bool)current.data;
    }
    
    //Moves the cursor to the next valid position, from an invalid one(currentPos.back() == current.data->declarationCount())
    void toValidPosition() {
      if(current.item == current.endItem) {
        {
          const DUContextData* data = current.data->d_func();
          
          //Check if we can proceed into a propagating child-context
          uint childContextCount = data->m_childContextsSize();
          const LocalIndexedDUContext* childContexts = data->m_childContexts();
          
          for(unsigned int a = 0; a < childContextCount; ++a) {
            DUContext* child = childContexts[a].data(current.data->m_topContext);
            if(ctx_d_func(child)->m_propagateDeclarations) {
              current.nextChild = a+1;
              stack.append(current);
              current.data = ctx_dynamicData(child);
              current.item = ctx_d_func(child)->m_localDeclarations();
              current.endItem = current.item + ctx_d_func(child)->m_localDeclarationsSize();
              current.nextChild = 0;
              toValidPosition();
              return;
            }
          }
        }
        upwards:
        //Go up and into the next valid context
        if(stack.isEmpty()) {
          current = StackEntry();
          return;
        }

        current = stack.back();
        stack.pop_back();

        const DUContextData* data = current.data->d_func();
        uint childContextCount = data->m_childContextsSize();
        const LocalIndexedDUContext* childContexts = data->m_childContexts();

        for(unsigned int a = current.nextChild; a < childContextCount; ++a) {
          DUContext* child = childContexts[a].data(current.data->m_topContext);
          
          if(ctx_d_func(child)->m_propagateDeclarations) {

            current.nextChild = a+1;
            stack.append(current);
            
            current.data = ctx_dynamicData(child);
            current.item = ctx_d_func(child)->m_localDeclarations();
            current.endItem = current.item + ctx_d_func(child)->m_localDeclarationsSize();
            current.nextChild = 0;
            toValidPosition();
            return;
          }
        }
          
        goto upwards;
      }
    }
    
    StackEntry current;

    KDevVarLengthArray<StackEntry> stack;
  };
  
  /**
   * Returns true if this context is imported by the given one, on any level.
   * */
  bool imports(const DUContext* context, const TopDUContext* source, int maxDepth) const;

  /**
   * This can deal with endless recursion
   */
  
  struct ImportsHash_Op {
    size_t operator() (const DUContextDynamicData* data) const {
      return (size_t)data;
    }
  };
  
  typedef google::dense_hash_map<const DUContextDynamicData*, bool, ImportsHash_Op> ImportsHash;
  
  bool importsSafeButSlow(const DUContext* context, const TopDUContext* source, ImportsHash& checked) const;
};

}


#endif

//kate: space-indent on; indent-width 2; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
