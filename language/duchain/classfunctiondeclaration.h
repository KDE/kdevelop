/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef CLASSFUNCTIONDECLARATION_H
#define CLASSFUNCTIONDECLARATION_H

#include "language/duchain/classmemberdeclaration.h"
#include "language/duchain/abstractfunctiondeclaration.h"

namespace KDevelop
{
class ClassFunctionDeclarationPrivate;
/**
 * Represents a single variable definition in a definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclaration : public ClassMemberDeclaration, public AbstractFunctionDeclaration
{
public:
  ClassFunctionDeclaration(const HashedString& url, const SimpleRange& range, DUContext* context);
  ~ClassFunctionDeclaration();

  enum QtFunctionType
  {
    Normal /**< Indicates a normal function */,
    Signal /**< indicates a Qt slot */,
    Slot   /**< indicates a Qt signal */
  };

  QtFunctionType functionType() const;
  void setFunctionType(QtFunctionType functionType);

  bool isConstructor() const;
  bool isDestructor() const;
  bool isConversionFunction() const;
//  bool isExternalDefinition() const; //Whether this declaration is an external definition of a class-member(Mainly for C++)

  void setIsExternalDefinition(bool);

  virtual QString toString() const;

  virtual void setAbstractType(AbstractType::Ptr type);

  virtual Declaration* clone() const;
  
  virtual uint additionalIdentity() const;
protected:
  ClassFunctionDeclaration(const ClassFunctionDeclaration& rhs);
private:
  Q_DECLARE_PRIVATE(ClassFunctionDeclaration)
};
}
Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::ClassFunctionDeclaration::FunctionSpecifiers)

#endif // CLASSFUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
