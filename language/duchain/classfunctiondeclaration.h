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
#include "classmemberdeclarationdata.h"

namespace KDevelop
{

struct QtFunctionEnumContainer {
  enum QtFunctionType
  {
    Normal /**< Indicates a normal function */,
    Signal /**< indicates a Qt slot */,
    Slot   /**< indicates a Qt signal */
  };
};

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(ClassFunctionDeclarationData, m_defaultParameters, IndexedString);

class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclarationData : public ClassMemberDeclarationData, public AbstractFunctionDeclarationData, public QtFunctionEnumContainer
{
public:
  ClassFunctionDeclarationData() {
    initializeAppendedLists();
    m_functionType = Normal;
  }
  ClassFunctionDeclarationData( const ClassFunctionDeclarationData& rhs )
      : ClassMemberDeclarationData( rhs ), AbstractFunctionDeclarationData(rhs)
  {
    initializeAppendedLists();
    copyListsFrom(rhs);
    m_functionType = rhs.m_functionType;
  }
  ~ClassFunctionDeclarationData() {
    freeAppendedLists();
  }
  QtFunctionType m_functionType;
  START_APPENDED_LISTS_BASE(ClassFunctionDeclarationData, ClassMemberDeclarationData);
  APPENDED_LIST_FIRST(ClassFunctionDeclarationData, IndexedString, m_defaultParameters);
  END_APPENDED_LISTS(ClassFunctionDeclarationData, m_defaultParameters);
};
/**
 * Represents a single variable definition in a definition-use chain.
 */
typedef MergeAbstractFunctionDeclaration<ClassMemberDeclaration, ClassFunctionDeclarationData> ClassFunctionDeclarationBase;
class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclaration : public ClassFunctionDeclarationBase, public QtFunctionEnumContainer
{
public:
  ClassFunctionDeclaration(const SimpleRange& range, DUContext* context);
  ClassFunctionDeclaration(ClassFunctionDeclarationData& data);
  ~ClassFunctionDeclaration();

  QtFunctionType functionType() const;
  void setFunctionType(QtFunctionType functionType);

  bool isConstructor() const;
  bool isDestructor() const;
  bool isConversionFunction() const;

  bool isFunctionDeclaration() const;

  void setIsExternalDefinition(bool);

  virtual QString toString() const;

  virtual void setAbstractType(AbstractType::Ptr type);

  virtual Declaration* clone() const;
  
  virtual uint additionalIdentity() const;
  
  virtual const IndexedString* defaultParameters() const;
  virtual int defaultParametersSize() const;
  virtual void addDefaultParameter(const IndexedString& str);
  virtual void clearDefaultParameters();
  
  enum {
    Identity = 14
  };
  
protected:
  ClassFunctionDeclaration(const ClassFunctionDeclaration& rhs);
private:
  DUCHAIN_DECLARE_DATA(ClassFunctionDeclaration)
};
}

#endif // CLASSFUNCTIONDECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
