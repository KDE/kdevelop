/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden@kdevelop.org>
    Copyright 2009 Lior Mualem <lior.m.kde@gmail.com>
    
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

#ifndef KDEVPLATFORM_CLASSDECLARATION_H
#define KDEVPLATFORM_CLASSDECLARATION_H

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/classmemberdeclarationdata.h>
#include <language/duchain/classmemberdeclaration.h>

namespace KDevelop {
  class QualifiedIdentifier;
  class DUContext;
  class TopDUContext;
  class IndexedString;
  class SimpleRange;
}

namespace KDevelop {

struct KDEVPLATFORMLANGUAGE_EXPORT BaseClassInstance
{
  KDevelop::IndexedType baseClass; //May either be StructureType, or DelayedType
  KDevelop::Declaration::AccessPolicy access;
  bool virtualInheritance;
};

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(ClassDeclarationData, baseClasses, BaseClassInstance)

class KDEVPLATFORMLANGUAGE_EXPORT ClassDeclarationData : public KDevelop::ClassMemberDeclarationData
{
public:
  enum ClassType {
    Class,
    Struct,
    Union,
    Interface
  };
  
  enum ClassModifier {
    Final,
    Abstract,
    None
  };
  
  ClassDeclarationData() : m_classType(Class), m_classModifier(None) {
    initializeAppendedLists();
  }

  ~ClassDeclarationData() {
    freeAppendedLists();
  }

  ClassDeclarationData(const ClassDeclarationData& rhs) : KDevelop::ClassMemberDeclarationData(rhs) {
    initializeAppendedLists();
    copyListsFrom(rhs);
    m_classType = rhs.m_classType;
    m_classModifier = rhs.m_classModifier;
  }
  
  /// Type of the class (struct, class, etc.)
  ClassType m_classType;
  /// Modifier of the class (final, abstract, etc.)
  ClassModifier m_classModifier;

  START_APPENDED_LISTS_BASE(ClassDeclarationData, KDevelop::ClassMemberDeclarationData);
  APPENDED_LIST_FIRST(ClassDeclarationData, BaseClassInstance, baseClasses);
  END_APPENDED_LISTS(ClassDeclarationData, baseClasses);
};

/**
 * Represents a single template-parameter definition
 */
class KDEVPLATFORMLANGUAGE_EXPORT ClassDeclaration : public KDevelop::ClassMemberDeclaration
{
public:
  
  ClassDeclaration(const ClassDeclaration& rhs);
  ClassDeclaration(ClassDeclarationData& data);
  ClassDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
  ClassDeclaration(ClassDeclarationData& data, const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
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
  
  void setClassModifier(ClassDeclarationData::ClassModifier modifier);
  
  ClassDeclarationData::ClassModifier classModifier() const;
  
  enum {
    Identity = 17
  };

private:
  virtual KDevelop::Declaration* clonePrivate() const;
  DUCHAIN_DECLARE_DATA(ClassDeclaration)
};

}

Q_DECLARE_TYPEINFO(KDevelop::BaseClassInstance, Q_MOVABLE_TYPE);

#endif // KDEVPLATFORM_CLASSDECLARATION_H

