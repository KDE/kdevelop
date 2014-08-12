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

namespace KDevelop {

///This class contains data that is only runtime-dependant and does not need to be stored to disk
class DUContextDynamicData
{
private:
    inline const DUContextData* d_func() const { return m_context->d_func(); }
    inline DUContextData* d_func_dynamic() { return m_context->d_func_dynamic(); }
    static inline const DUContextData* ctx_d_func(DUContext* ctx) { return ctx->d_func(); }
    static inline DUContextDynamicData* ctx_dynamicData(DUContext* ctx) { return ctx->m_dynamicData; }

public:
  DUContextDynamicData( DUContext* );
  DUContextPointer m_parentContext;

  TopDUContext* m_topContext;

  uint m_indexInTopContext; //Index of this DUContext in the top-context
  
  DUContext* m_context;

  // cache of unserialized child contexts
  QVector<DUContext*> m_childContexts;

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
          //Check if we can proceed into a propagating child-context
          const auto childContexts = current.data->m_childContexts;
          for (int a = 0; a < childContexts.size(); ++a) {
            DUContext* child = childContexts[a];
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

        const auto childContexts = current.data->m_childContexts;
        for (int a = current.nextChild; a < childContexts.size(); ++a) {
          DUContext* child = childContexts[a];

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
  bool imports(const DUContext* context, const TopDUContext* source,
               QSet<const DUContextDynamicData*>* recursionGuard) const;
};

}

#endif
