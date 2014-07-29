/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2006 Adam Treat <treat@kde.org>
    Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#ifndef KDEVPLATFORM_CLASSFUNCTIONDECLARATION_H
#define KDEVPLATFORM_CLASSFUNCTIONDECLARATION_H

#include "classmemberdeclaration.h"
#include "abstractfunctiondeclaration.h"
#include "classmemberdeclarationdata.h"

namespace KDevelop
{

enum ClassFunctionFlags
{
  FunctionFlagNormal = 0,
  FunctionSignalFlag = 1 <<  1,
  FunctionSlotFlag = 1 << 2,
  AbstractFunctionFlag = 1 << 3,
  FinalFunctionFlag = 1 << 4
};

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(ClassFunctionDeclarationData, m_defaultParameters, IndexedString)

class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclarationData : public ClassMemberDeclarationData, public AbstractFunctionDeclarationData
{
public:
  ClassFunctionDeclarationData() {
    initializeAppendedLists();
    m_functionFlags = FunctionFlagNormal;
  }
  ClassFunctionDeclarationData( const ClassFunctionDeclarationData& rhs )
      : ClassMemberDeclarationData( rhs ), AbstractFunctionDeclarationData(rhs)
  {
    initializeAppendedLists();
    copyListsFrom(rhs);
    m_functionFlags = rhs.m_functionFlags;
  }
  ~ClassFunctionDeclarationData() {
    freeAppendedLists();
  }
  ClassFunctionFlags m_functionFlags;
  START_APPENDED_LISTS_BASE(ClassFunctionDeclarationData, ClassMemberDeclarationData);
  APPENDED_LIST_FIRST(ClassFunctionDeclarationData, IndexedString, m_defaultParameters);
  END_APPENDED_LISTS(ClassFunctionDeclarationData, m_defaultParameters);
};
/**
 * Represents a single variable definition in a definition-use chain.
 */
typedef MergeAbstractFunctionDeclaration<ClassMemberDeclaration, ClassFunctionDeclarationData> ClassFunctionDeclarationBase;
class KDEVPLATFORMLANGUAGE_EXPORT ClassFunctionDeclaration : public ClassFunctionDeclarationBase
{
public:
  ClassFunctionDeclaration(const RangeInRevision& range, DUContext* context);
  ClassFunctionDeclaration(ClassFunctionDeclarationData& data, const RangeInRevision& range, DUContext* context);
  ClassFunctionDeclaration(ClassFunctionDeclarationData& data);
  ~ClassFunctionDeclaration();

  ///Whether this function is a signal, for example a C++ Qt signal
  bool isSignal() const;
  void setIsSignal(bool);
  
  ///Whether this function is a slot, for example a C++ Qt slot
  bool isSlot() const;
  void setIsSlot(bool);
  
  ///Whether this function is abstract
  bool isAbstract() const;
  void setIsAbstract(bool);
  
  ///Whether this function is final
  bool isFinal() const;
  void setIsFinal(bool);
  
  virtual bool isConstructor() const;
  virtual bool isDestructor() const;
  
  bool isConversionFunction() const;

  bool isFunctionDeclaration() const;

  virtual QString toString() const;

  virtual void setAbstractType(AbstractType::Ptr type);

  virtual Declaration* clonePrivate() const;
  
  virtual uint additionalIdentity() const;
  
  virtual const IndexedString* defaultParameters() const;
  virtual unsigned int defaultParametersSize() const;
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

#endif // KDEVPLATFORM_CLASSFUNCTIONDECLARATION_H
