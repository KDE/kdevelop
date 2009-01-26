/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden@kdevelop.org>

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

#ifndef CLASSDECLARATION_H
#define CLASSDECLARATION_H

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include "cppduchainexport.h"
#include "cpptypes.h"
#include <language/duchain/classmemberdeclarationdata.h>
#include <language/duchain/classmemberdeclaration.h>

namespace KDevelop {
  class QualifiedIdentifier;
  class DUContext;
  class TopDUContext;
  class HashedString;
  class SimpleRange;
}

namespace Cpp {

struct BaseClassInstance
{
  KDevelop::IndexedType baseClass; //May either be CppClassType, or CppDelayedType
  KDevelop::Declaration::AccessPolicy access;
  bool virtualInheritance;
};

DECLARE_LIST_MEMBER_HASH(ClassDeclarationData, baseClasses, BaseClassInstance)

class ClassDeclarationData : public KDevelop::ClassMemberDeclarationData
{
public:
  enum ClassType {
    Class,
    Struct,
    Union,
    Interface
  };
  
  ClassDeclarationData() : m_classType(Class) {
    initializeAppendedLists();
  }

  ~ClassDeclarationData() {
    freeAppendedLists();
  }

  ClassDeclarationData(const ClassDeclarationData& rhs) : KDevelop::ClassMemberDeclarationData(rhs) {
    initializeAppendedLists();
    copyListsFrom(rhs);
    m_classType = rhs.m_classType;
  }
  
  /// Type of the class (struct, class, etc.)
  ClassType m_classType;

  START_APPENDED_LISTS_BASE(ClassDeclarationData, KDevelop::ClassMemberDeclarationData);
  APPENDED_LIST_FIRST(ClassDeclarationData, BaseClassInstance, baseClasses);
  END_APPENDED_LISTS(ClassDeclarationData, baseClasses);
};

/**
 * Represents a single template-parameter definition
 */
class KDEVCPPDUCHAIN_EXPORT ClassDeclaration : public KDevelop::ClassMemberDeclaration
{
public:
  
  ClassDeclaration(const ClassDeclaration& rhs);
  ClassDeclaration(ClassDeclarationData& data);
  ClassDeclaration(const KDevelop::SimpleRange& range, KDevelop::DUContext* context);
  ~ClassDeclaration();

  void clearBaseClasses();
  ///Count of base-classes
  uint baseClassesSize() const;
  ///The types this class is based on
  const BaseClassInstance* baseClasses() const;
  void addBaseClass(BaseClassInstance klass);
  //Replaces the n'th base-class with the given one. The replaced base-class must have existed.
  void replaceBaseClass(uint n, BaseClassInstance klass);

  /**Returns whether base is a public base-class of this class
   * @param baseConversionLevels If nonzero, this will count the distance of the classes.
   * */
  bool isPublicBaseClass( ClassDeclaration* base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels  = 0 ) const;

  QString toString() const;

  void setClassType(ClassDeclarationData::ClassType type);
  
  ClassDeclarationData::ClassType classType() const;
  
  enum {
    Identity = 17
  };

private:
  virtual KDevelop::Declaration* clonePrivate() const;
  DUCHAIN_DECLARE_DATA(ClassDeclaration)
};

}

#endif // FUNCTIONDECLARATION_H

