/* This file is part of KDevelop
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

#ifndef KDEV_TYPEBUILDER_H
#define KDEV_TYPEBUILDER_H

#include "language/duchain/typesystem.h"
#include "language/duchain/declaration.h"
#include "language/duchain/repositories/ityperepository.h"

namespace KDevelop {
  
class ForwardDeclaration;

template<typename T, typename NameT>
class KDEVPLATFORMLANGUAGE_EXPORT AbstractTypeBuilder : public LangugageSpecificTypeBuilderBase
{
public:
  const QList< KDevelop::AbstractType::Ptr >& topTypes() const
  {
    return m_topTypes;
  }

protected:
  virtual ITypeRepository* typeRepository() const = 0;
  virtual DUContext* searchContext() const
  {
    return LangugageSpecificTypeBuilderBase::currentContext();
  }

  // Call at the beginning of processing a class-specifier, right after the type for the class was created.
  // The type can be retrieve through currentAbstractType().
  virtual void classTypeOpened(KDevelop::AbstractType::Ptr) {}

  virtual void supportBuild(T* node, DUContext* context = 0)
  {
    m_topTypes.clear();

    LangugageSpecificTypeBuilderBase::supportBuild(node, context);
    
    Q_ASSERT(m_typeStack.isEmpty());
  }
  
  KDevelop::AbstractType::Ptr lastType() const
  {
    return m_lastType;
  }

  void setLastType(KDevelop::AbstractType::Ptr ptr)
  {
    m_lastType = ptr;
  }
  
  void clearLastType()
  {
    m_lastType = 0;
  }

  /**Simulates that the given type was created.
   * After calling, the given type will be the last type.
   * */
  void injectType(const KDevelop::AbstractType::Ptr& type)
  {
    openType(type);
    closeType();
  }

  template <class T2>
  void injectType(const KSharedPtr<T2>& type)
  { injectType(KDevelop::AbstractType::Ptr::staticCast(type)); }

  template <class T2>
  void openType(KSharedPtr<T2> type)
  { openAbstractType(KDevelop::AbstractType::Ptr::staticCast(type)); }

  void openAbstractType(KDevelop::AbstractType::Ptr type)
  {
    m_typeStack.append(type);
  }

  void closeType()
  {
    // Check that this isn't the same as a previously existing type
    // If it is, it will get replaced
    m_lastType = typeRepository()->registerType(currentAbstractType());

    bool replaced = m_lastType != currentAbstractType();

    // And the reference will be lost...
    m_typeStack.pop();

    if (!hasCurrentType() && !replaced)
      m_topTypes.append(m_lastType);
  }

  inline bool hasCurrentType() { return !m_typeStack.isEmpty(); }

  // You must not use this in creating another type definition, as it may not be the registered type.
  inline KDevelop::AbstractType::Ptr currentAbstractType() { return m_typeStack.top(); }

  // You must not use this in creating another type definition, as it may not be the registered type.
  template <class T2>
  KSharedPtr<T2> currentType() { return KSharedPtr<T2>::dynamicCast(m_typeStack.top()); }

  ///Returns whether a type was opened
  bool openTypeFromName(NameT* name, bool needClass)
  {
    return openTypeFromName(identifierForNode(name), name, needClass);
  }
  
  bool openTypeFromName(QualifiedIdentifier id, T* typeNode, bool needClass)
  {
    bool openedType = false;

    bool delay = false;
    
    if(!delay) {
      SimpleCursor pos(editorFindRange(typeNode, typeNode).start());
      DUChainReadLocker lock(DUChain::lock());

      QList<Declaration*> dec = searchContext()->findDeclarations(id, pos);

      if ( dec.isEmpty() )
        delay = true;

      if(!delay) {
        foreach( Declaration* decl, dec ) {
          if( needClass && !dynamic_cast<StructureType*>(decl->abstractType().data()) )
            continue;

          if (decl->abstractType() ) {
            ///@todo only functions may have multiple declarations here
            //ifDebug( if( dec.count() > 1 ) kDebug(9007) << id.toString() << "was found" << dec.count() << "times" )
            //kDebug(9007) << "found for" << id.toString() << ":" << decl->toString() << "type:" << decl->abstractType()->toString() << "context:" << decl->context();
            openedType = true;
            openType(decl->abstractType());
            break;
          }
        }
      }

      if(!openedType)
        delay = true;
    }
      ///@todo What about position?

    /*if(delay) {
      //Either delay the resolution for template-dependent types, or create an unresolved type that stores the name.
    openedType = true;
    openDelayedType(id, name, templateDeclarationDepth() ? DelayedType::Delayed : DelayedType::Unresolved );

    ifDebug( if(templateDeclarationDepth() == 0) kDebug(9007) << "no declaration found for" << id.toString() << "in context \"" << searchContext()->scopeIdentifier(true).toString() << "\"" << "" << searchContext() )
    }*/
    return openedType;
  }

private:
  QStack<KDevelop::AbstractType::Ptr> m_typeStack;

  KDevelop::AbstractType::Ptr m_lastType;

  QList<KDevelop::AbstractType::Ptr> m_topTypes;
};

}

#endif // TYPEBUILDER_H

