/* This  is part of KDevelop
    Copyright 2007 David Nolden <david.nolden@kdevelop.org>

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

#include "classdeclaration.h"
#include <duchain/identifier.h>
#include <duchain/declaration.h>
#include <appendedlist.h>


using namespace KDevelop;

namespace Cpp {

DEFINE_LIST_MEMBER_HASH(ClassDeclarationPrivate, baseClasses, BaseClassInstance);
  
class ClassDeclarationPrivate
{
public:
  ClassDeclarationPrivate() {
    initializeAppendedLists(true);
  }
  
  ~ClassDeclarationPrivate() {
    freeAppendedLists();
  }
  
  ClassDeclarationPrivate(const ClassDeclarationPrivate& rhs) {
    initializeAppendedLists(true);
    copyListsFrom(rhs);
  }
  
  static size_t classSize() {
    return sizeof(ClassDeclarationPrivate);
  }
  
  START_APPENDED_LISTS(ClassDeclarationPrivate);
  APPENDED_LIST_FIRST(ClassDeclarationPrivate, BaseClassInstance, baseClasses);
  END_APPENDED_LISTS(ClassDeclarationPrivate, baseClasses);
};

ClassDeclaration::ClassDeclaration(const HashedString& url, const KDevelop::SimpleRange& range, DUContext* context)
  : Declaration(url, range, context), d_ptr(new ClassDeclarationPrivate)
{
}

void ClassDeclaration::clearBaseClasses() {
  d_ptr->baseClassesList().clear();
}

uint ClassDeclaration::baseClassesSize() const {
  return d_ptr->baseClassesSize();
}

const BaseClassInstance* ClassDeclaration::baseClasses() const {
  return d_ptr->baseClasses();
}

void ClassDeclaration::addBaseClass(BaseClassInstance klass) {
  d_ptr->baseClassesList().append(klass);
}

void ClassDeclaration::replaceBaseClass(uint n, BaseClassInstance klass) {
  Q_ASSERT(n <= d_ptr->baseClassesSize());
  d_ptr->baseClassesList()[n] = klass;
}

ClassDeclaration::~ClassDeclaration()
{
}

ClassDeclaration::ClassDeclaration(const ClassDeclaration& rhs) : Declaration(rhs), 
        d_ptr(new ClassDeclarationPrivate(*rhs.d_ptr)) {
}

Declaration* ClassDeclaration::clone() const {
  return new ClassDeclaration(*this);
}

bool ClassDeclaration::isPublicBaseClass( ClassDeclaration* base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels ) const {
  if( baseConversionLevels )
    *baseConversionLevels = 0;

  if( indexedType() == base->indexedType() )
    return true;
  
  FOREACH_FUNCTION(const Cpp::BaseClassInstance& b, baseClasses)
  {
    if( baseConversionLevels )
      ++ (*baseConversionLevels);
    //kDebug(9007) << "public base of" << c->toString() << "is" << b.baseClass->toString();
    if( b.access != KDevelop::Declaration::Private ) {
      int nextBaseConversion = 0;
      if( CppClassType::Ptr c = b.baseClass.type().cast<CppClassType>() ) {
        ClassDeclaration* decl = dynamic_cast<ClassDeclaration*>(c->declaration(topContext));
        if( decl && decl->isPublicBaseClass( base, topContext, &nextBaseConversion ) )
          *baseConversionLevels += nextBaseConversion;
          return true;
      }
    }
    if( baseConversionLevels )
      -- (*baseConversionLevels);
  }
  return false;
}

}
