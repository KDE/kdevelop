/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2006 Hamish Rodda <rodda@kde.org>                       *
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

#ifndef ducontext_p_H
#define ducontext_p_H

#include <QtCore/QMutex>
#include <QtCore/QMultiHash>
#include <QtCore/QMap>

#include "../editor/simplecursor.h"

#include "duchainbase.h"
#include "ducontext.h"
#include "duchainpointer.h"
#include "declaration.h"
#include "use.h"
#include "../languageexport.h"

namespace KTextEditor {
  class SmartRange;
}


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
  
  START_APPENDED_LISTS_BASE(DUContextData, DUChainBaseData);
  APPENDED_LIST_FIRST(DUContextData, DUContext::Import, m_importedContexts);
  APPENDED_LIST(DUContextData, LocalIndexedDUContext, m_childContexts, m_importedContexts);
  
  ///@todo eventually move the importers into some separate structure
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
  
  /**
   * If this document is loaded, this contains a smart-range for each use.
   * This may temporarily contain zero ranges.
   * */
  mutable QVector<KTextEditor::SmartRange*> m_rangesForUses;
  
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
  struct VisibleDeclarationIterator {
    
    VisibleDeclarationIterator(DUContextDynamicData* data) {
      currentPos.append(0);
      currentData.append(data);
      toValidPosition();
    }
    
    Declaration* operator*() const {
      return currentData.back()->m_context->d_func()->m_localDeclarations()[currentPos.back()].data(currentData.back()->m_topContext);
    }
    
    VisibleDeclarationIterator& operator++() {
      Q_ASSERT(!currentPos.isEmpty());
      Q_ASSERT(currentPos.size() == currentData.size());
      ++currentPos.back();
      toValidPosition();
      return *this;
    }
    
    operator bool() const {
      return !currentData.isEmpty();
    }
    
    //Moves the cursor to the next valid position, from an invalid one(currentPos.back() == currentData->declarationCount())
    void toValidPosition() {
      const DUContextData* data = currentData.back()->m_context->d_func();
      if(currentPos.back() == data->m_localDeclarationsSize()) {
        //Check if we can proceed into a propagating child-context
        for(int a = 0; a < data->m_childContextsSize(); ++a) {
          DUContext* child = data->m_childContexts()[a].data(currentData.back()->m_topContext);
          if(child->d_func()->m_propagateDeclarations) {
            currentPos.back() = a;
            currentData.append(child->m_dynamicData);
            currentPos.append(0);
            toValidPosition();
            return;
          }
        }
        upwards:
        //Check if the parent has a follower child context
        currentPos.pop_back();
        currentData.pop_back();
        while(!currentPos.isEmpty() && currentPos.back() == currentData.back()->m_context->d_func()->m_childContextsSize()) {
          currentPos.pop_back();
          currentData.pop_back();
        }
        
        if(!currentPos.isEmpty()) {
          //We've found a next child that we can iterate into
          data = currentData.back()->m_context->d_func();
          for(int a = currentPos.back(); a < data->m_childContextsSize(); ++a) {
            if(data->m_childContexts()[a].data(currentData.back()->m_topContext)->d_func()->m_propagateDeclarations) {
              currentPos.back() = a+1;
              currentData.append(data->m_childContexts()[a].data(currentData.back()->m_topContext)->m_dynamicData);
              currentPos.append(0);
              toValidPosition();
              return;
            }
          }
        }else{
          return;
        }
        goto upwards;
      }
    }
    
    KDevVarLengthArray<DUContextDynamicData*> currentData; //Current data that is being iterated through, if inHash is false
    KDevVarLengthArray<int> currentPos; //Current position in the given data. back() is always within the declaration-vector, the other indices are within the child-contexts
  };
  
  /**
   * Returns true if this context is imported by the given one, on any level.
   * */
  bool isThisImportedBy(const DUContext* context) const;
};

}


#endif

//kate: space-indent on; indent-width 2; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
