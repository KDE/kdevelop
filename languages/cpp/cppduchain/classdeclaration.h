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

#include <declaration.h>
#include "cppduchainexport.h"
#include "cpptypes.h"

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

class ClassDeclarationData;

/**
 * Represents a single template-parameter definition
 */
class KDEVCPPDUCHAIN_EXPORT ClassDeclaration : public KDevelop::Declaration
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

  virtual KDevelop::Declaration* clone() const;

  enum {
    Identity = 17
  };

private:
  DUCHAIN_DECLARE_DATA(ClassDeclaration)
};

}

#endif // FUNCTIONDECLARATION_H

