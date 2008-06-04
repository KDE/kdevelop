/* This file is part of KDevelop
    Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#ifndef ABSTRACTFUNCTIONDECLARATION_H
#define ABSTRACTFUNCTIONDECLARATION_H

#include <QtCore/QString>
#include "../languageexport.h"

namespace KDevelop
{

class AbstractFunctionDeclarationPrivate;

/**
 * Provides an interface to declarations which represent functions in a definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT AbstractFunctionDeclaration
{
public:
  AbstractFunctionDeclaration();
  AbstractFunctionDeclaration(const AbstractFunctionDeclaration& rhs);
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

  /**
   * Returns the default-parameters that are set. The last default-parameter matches the last
   * argument of the function, but the returned list will only contain default-values for those
   * arguments that have one, for performance-reasons.
   *
   * So the list may be empty or smaller than the list of function-arguments.
   * */
  const QList<QString>& defaultParameters() const;

  void addDefaultParameter(const QString& str);
  
  void clearDefaultParameters() ;

  private:
    AbstractFunctionDeclarationPrivate* const d;
};

}

#endif // ABSTRACTFUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
