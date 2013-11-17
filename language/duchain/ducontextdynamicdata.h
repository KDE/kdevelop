/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2006 Hamish Rodda <rodda@kde.org>                           *
 *   Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>*
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

#ifndef DUCONTEXTDYNAMICDATA_H
#define DUCONTEXTDYNAMICDATA_H

#include "ducontextdata.h"
#include <util/google/dense_hash_map>

namespace KDevelop {

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
  
  ///@warning: Whenever m_localDeclarations is read or written, the duchain must be locked
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
   * Must be called with duchain locked
  */
  void addDeclarationToHash(const Identifier& identifer, Declaration* declaration);
  ///Must be called with duchain locked
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
