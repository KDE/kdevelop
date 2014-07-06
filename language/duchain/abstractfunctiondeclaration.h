/* This file is part of KDevelop
    Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_ABSTRACTFUNCTIONDECLARATION_H
#define KDEVPLATFORM_ABSTRACTFUNCTIONDECLARATION_H

#include <QtCore/QString>
#include <language/languageexport.h>
#include "indexedducontext.h"

namespace KDevelop
{
class DUContext;
class IndexedString;

class AbstractFunctionDeclarationData
{
public:
  AbstractFunctionDeclarationData() : m_isVirtual(false), m_isInline(false), m_isExplicit(false) {
  }
  IndexedDUContext m_functionContext;
  bool m_isVirtual: 1; ///@todo move into ClassFunctionDeclaration(Only valid for class-functions)
  bool m_isInline: 1;
  bool m_isExplicit: 1; ///@todo move into ClassFunctionDeclaration(Only valid for class-functions)
};

/**
 * Provides an interface to declarations which represent functions in a definition-use chain.
 * Don't inherit from this directly, use MergeAbstractFunctionDeclaration instead.
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractFunctionDeclaration
{
public:
  virtual ~AbstractFunctionDeclaration();

  enum FunctionSpecifier {
    VirtualSpecifier  = 0x1 /**< indicates a virtual function */,
    InlineSpecifier   = 0x2 /**< indicates a inline function */,
    ExplicitSpecifier = 0x4 /**< indicates a explicit function */
  };
  Q_DECLARE_FLAGS(FunctionSpecifiers, FunctionSpecifier)

  void setFunctionSpecifiers(FunctionSpecifiers specifiers);

  bool isInline() const;
  void setInline(bool isInline);

  ///Only used for class-member function declarations(see ClassFunctionDeclaration)
  bool isVirtual() const;
  void setVirtual(bool isVirtual);

  ///Only used for class-member function declarations(see ClassFunctionDeclaration)
  bool isExplicit() const;
  void setExplicit(bool isExplicit);
  
  ///Return the DUContext::Function type ducontext (the function parameter context) of this function
  ///Same as internalContext if the function has no definition
  DUContext* internalFunctionContext() const;
  void setInternalFunctionContext(DUContext *context);

  /**
   * Returns the default-parameters that are set. The last default-parameter matches the last
   * argument of the function, but the returned vector will only contain default-values for those
   * arguments that have one, for performance-reasons.
   *
   * So the vector may be empty or smaller than the count of function-arguments.
   * */
  virtual const IndexedString* defaultParameters() const = 0;
  virtual unsigned int defaultParametersSize() const = 0;
  virtual void addDefaultParameter(const IndexedString& str) = 0;
  virtual void clearDefaultParameters()  = 0;
  ///Returns the default parameter assigned to the given argument number.
  ///This is a convenience-function.
  IndexedString defaultParameterForArgument(int index) const;
  
private:
  //Must be implemented by sub-classes to provide a pointer to the data
  virtual const AbstractFunctionDeclarationData* data() const = 0;
  virtual AbstractFunctionDeclarationData* dynamicData() = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::AbstractFunctionDeclaration::FunctionSpecifiers)

///Use this to merge AbstractFunctionDeclaration into the class hierarchy. Base must be the base-class
///in the hierarchy, and Data must be the Data class of the following Declaration, and must be based on AbstractFunctionDeclarationData
///and BaseData.
template<class Base, class _Data>
class MergeAbstractFunctionDeclaration : public Base, public AbstractFunctionDeclaration {
  public:
  template<class BaseData>
  MergeAbstractFunctionDeclaration(BaseData& data) : Base(data) {
  }
  template<class BaseData, class Arg2>
  MergeAbstractFunctionDeclaration(BaseData& data, const Arg2& arg2) : Base(data, arg2) {
  }
  template<class BaseData, class Arg2, class Arg3>
  MergeAbstractFunctionDeclaration(BaseData& data, const Arg2& arg2, const Arg3& arg3) : Base(data, arg2, arg3) {
  }
  
  private:
  virtual const AbstractFunctionDeclarationData* data() const {
    return static_cast<const _Data*>(Base::d_func());
  }
  virtual AbstractFunctionDeclarationData* dynamicData() {
    return static_cast<_Data*>(Base::d_func_dynamic());
  }
};

}

#endif // KDEVPLATFORM_ABSTRACTFUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
