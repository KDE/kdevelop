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

#ifndef KDEVPLATFORM_ABSTRACTTYPEBUILDER_H
#define KDEVPLATFORM_ABSTRACTTYPEBUILDER_H

#include "../types/structuretype.h"
#include "../declaration.h"
#include "../duchain.h"
#include "../duchainlock.h"

namespace KDevelop {

/**
 * \short Abstract definition-use chain type builder class
 *
 * The AbstractTypeBuilder is a convenience class template for creating customized
 * definition-use chain type builders from an AST.  It simplifies:
 * - use of your editor integrator
 * - creating and re-using types
 * - creating complex types in a stack
 * - referencing named types
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
template<typename T, typename NameT, typename LangugageSpecificTypeBuilderBase>
class AbstractTypeBuilder : public LangugageSpecificTypeBuilderBase
{
public:
  /**
   * Returns the list of types that were created in the parsing run, excluding subtypes
   * (ie. returns complete types, not the simple types and intermediate types which
   * went into creating any complex type)
   *
   * Used for unit tests only.
   */
  const QList< AbstractType::Ptr >& topTypes() const
  {
    return m_topTypes;
  }

protected:
  /**
   * Determine the context to search in when named types are requested.
   * You may reimplement this to return a different context if required.
   *
   * \returns the context in which to search for named types.
   */
  virtual DUContext* searchContext() const
  {
    return LangugageSpecificTypeBuilderBase::currentContext();
  }

  /**
   * Notify that a class type was opened.
   *
   * This should be called at the beginning of processing a class-specifier, right after the type for the class was created.
   * The type can be retrieved through currentAbstractType().
   */
  virtual void classTypeOpened(KDevelop::AbstractType::Ptr) {}

  /**
   * Perform initialisation at the start of a build, and check that all types
   * that were registered were also used.
   */
  virtual void supportBuild(T* node, DUContext* context = 0)
  {
    m_topTypes.clear();

    LangugageSpecificTypeBuilderBase::supportBuild(node, context);

    Q_ASSERT(m_typeStack.isEmpty());
  }

  /**
   * Retrieve the last type that was encountered.
   * \returns the last encountered type.
   */
  AbstractType::Ptr lastType() const
  {
    return m_lastType;
  }

  /**
   * Set the last encountered type.
   *
   * \param ptr pointer to the last encountered type.
   */
  void setLastType(const AbstractType::Ptr& ptr)
  {
    m_lastType = ptr;
  }

  /// Clear the last encountered type.
  void clearLastType()
  {
    m_lastType = 0;
  }

  /**
   * Simulates that the given type was created.
   * After calling, this type will be the last type.
   * */
  void injectType(const AbstractType::Ptr& type)
  {
    openType(type);
    closeType();
  }

  /**
   * Simulates that the given type was created.
   * After calling, this type will be the last type.
   * */
  template <class T2>
  void injectType(const TypePtr<T2>& type)
  { injectType(AbstractType::Ptr::staticCast(type)); }

  /**
   * Opens the given \a type, and sets it to be the current type.
   */
  template <class T2>
  void openType(const TypePtr<T2>& type)
  { openAbstractType(AbstractType::Ptr::staticCast(type)); }

  /**
   * Opens the given \a type, and sets it to be the current type.
   */
  void openAbstractType(const AbstractType::Ptr& type)
  {
    m_typeStack.append(type);
  }

  /**
   * Close the current type.
   */
  void closeType()
  {
    m_lastType = currentAbstractType();

    bool replaced = m_lastType != currentAbstractType();

    // And the reference will be lost...
    m_typeStack.pop();

    if (!hasCurrentType() && !replaced)
      m_topTypes.append(m_lastType);
  }

  /// Determine if the type builder is currently parsing a type. \returns true if there is a current type, else returns false.
  inline bool hasCurrentType() { return !m_typeStack.isEmpty(); }

  /**
   * Retrieve the current type being parsed.
   *
   * \warning You must not use this in creating another type definition, as it may not be a registered type.
   *
   * \returns the current abstract type being parsed.
   */
  inline AbstractType::Ptr currentAbstractType()
  {
    if (m_typeStack.isEmpty()) {
      return AbstractType::Ptr();
    } else {
      return m_typeStack.top();
    }
  }

  /**
   * Retrieve the current type being parsed.
   *
   * \warning You must not use this in creating another type definition, as it may not be a registered type.
   *
   * \returns the current type being parsed.
   */
  template <class T2>
  TypePtr<T2> currentType() { return currentAbstractType().template cast<T2>(); }

  /**
   * Search for a type with the identifier given by \a name.
   *
   * \param name the AST node representing the name of the type to open.
   * \param needClass if true, only class types will be searched, if false all named types will be searched.
   *
   * \returns whether a type was found (and thus opened).
   */
  bool openTypeFromName(NameT* name, bool needClass)
  {
    return openTypeFromName(identifierForNode(name), name, needClass);
  }

  /**
   * Search for a type with the identifier given by \a name.
   *
   * \param id the identifier of the type for which to search.
   * \param typeNode the AST node representing the type to open.
   * \param needClass if true, only class types will be searched, if false all named types will be searched.
   *
   * \returns whether a type was found (and thus opened).
   */
  bool openTypeFromName(QualifiedIdentifier id, T* typeNode, bool needClass)
  {
    bool openedType = false;

    bool delay = false;

    if(!delay) {
      CursorInRevision pos(this->editorFindRange(typeNode, typeNode).start);
      DUChainReadLocker lock(DUChain::lock());

      QList<Declaration*> dec = searchContext()->findDeclarations(id, pos);

      if ( dec.isEmpty() )
        delay = true;

      if(!delay) {
        foreach( Declaration* decl, dec ) {
          // gcc 4.0.1 doesn't eath this // if( needClass && !decl->abstractType().cast<StructureType>() )
          if( needClass && !decl->abstractType().cast(static_cast<StructureType *>(0)) )
            continue;

          if (decl->abstractType() ) {
            ///@todo only functions may have multiple declarations here
            //ifDebug( if( dec.count() > 1 ) kDebug() << id.toString() << "was found" << dec.count() << "times" )
            //kDebug() << "found for" << id.toString() << ":" << decl->toString() << "type:" << decl->abstractType()->toString() << "context:" << decl->context();
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

    ifDebug( if(templateDeclarationDepth() == 0) kDebug() << "no declaration found for" << id.toString() << "in context \"" << searchContext()->scopeIdentifier(true).toString() << "\"" << "" << searchContext() )
    }*/
    return openedType;
  }

private:
  QStack<AbstractType::Ptr> m_typeStack;

  AbstractType::Ptr m_lastType;

  QList<AbstractType::Ptr> m_topTypes;
};

}

#endif // KDEVPLATFORM_ABSTRACTTYPEBUILDER_H

