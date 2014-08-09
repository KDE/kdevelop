/* This file is part of KDevelop
    Copyright 2007 David Nolden <david.nolden@kdevelop.org>
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

#include "classdeclaration.h"
#include "identifier.h"
#include <language/duchain/declaration.h>
#include <language/duchain/appendedlist.h>
#include <language/duchain/duchainregister.h>
#include "types/structuretype.h"


namespace KDevelop {

DEFINE_LIST_MEMBER_HASH(ClassDeclarationData, baseClasses, BaseClassInstance)


ClassDeclaration::ClassDeclaration(const KDevelop::RangeInRevision& range, DUContext* context)
  : ClassMemberDeclaration(*new ClassDeclarationData, range)
{
  d_func_dynamic()->setClassId(this);
  setContext(context);
}

ClassDeclaration::ClassDeclaration( ClassDeclarationData& data, const KDevelop::RangeInRevision& range, DUContext* context )
  : ClassMemberDeclaration( data, range )
{
  setContext(context);
}

ClassDeclaration::ClassDeclaration(ClassDeclarationData& data)
  : ClassMemberDeclaration(data)
{
}

REGISTER_DUCHAIN_ITEM(ClassDeclaration);

void ClassDeclaration::clearBaseClasses() {
  bool wasInSymbolTable = inSymbolTable();
  setInSymbolTable(false);
  d_func_dynamic()->baseClassesList().clear();
  setInSymbolTable(wasInSymbolTable);
}

uint ClassDeclaration::baseClassesSize() const {
  return d_func()->baseClassesSize();
}

const BaseClassInstance* ClassDeclaration::baseClasses() const {
  return d_func()->baseClasses();
}

void ClassDeclaration::addBaseClass(BaseClassInstance klass) {
  bool wasInSymbolTable = inSymbolTable();
  setInSymbolTable(false);
  d_func_dynamic()->baseClassesList().append(klass);
  setInSymbolTable(wasInSymbolTable);
}

void ClassDeclaration::replaceBaseClass(uint n, BaseClassInstance klass) {
  Q_ASSERT(n <= d_func()->baseClassesSize());
  bool wasInSymbolTable = inSymbolTable();
  setInSymbolTable(false);
  d_func_dynamic()->baseClassesList()[n] = klass;
  setInSymbolTable(wasInSymbolTable);
}

ClassDeclaration::~ClassDeclaration()
{
}

ClassDeclaration::ClassDeclaration(const ClassDeclaration& rhs) : ClassMemberDeclaration(*new ClassDeclarationData(*rhs.d_func())) {
  d_func_dynamic()->setClassId(this);
}

Declaration* ClassDeclaration::clonePrivate() const {
  return new ClassDeclaration(*this);
}

bool isPublicBaseClassInternal( const ClassDeclaration* self, ClassDeclaration* base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels, int depth, QSet<const ClassDeclaration*>* checked ) {
  
  if(checked) {
    if(checked->contains(self))
      return false;
    checked->insert(self);
  }
  else if(depth > 3)
  {
    //Too much depth, to prevent endless recursion, we control the recursion using the 'checked' set
    QSet<const ClassDeclaration*> checkedSet;
    return isPublicBaseClassInternal(self, base, topContext, baseConversionLevels, depth, &checkedSet);
  }
  
  if( baseConversionLevels )
    *baseConversionLevels = 0;

  if( self->indexedType() == base->indexedType() )
    return true;

  FOREACH_FUNCTION(const BaseClassInstance& b, self->baseClasses)
  {
    if( baseConversionLevels )
      ++ (*baseConversionLevels);
    //kDebug(9007) << "public base of" << c->toString() << "is" << b.baseClass->toString();
    if( b.access != KDevelop::Declaration::Private ) {
      int nextBaseConversion = 0;
      if( StructureType::Ptr c = b.baseClass.type<StructureType>() ) {
        ClassDeclaration* decl = dynamic_cast<ClassDeclaration*>(c->declaration(topContext));
        if( decl && isPublicBaseClassInternal( decl, base, topContext, &nextBaseConversion, depth+1, checked ) ) {
          if ( baseConversionLevels )
            *baseConversionLevels += nextBaseConversion;
          return true;
        }
      }
    }
    if( baseConversionLevels )
      -- (*baseConversionLevels);
  }
  return false;
}

bool ClassDeclaration::isPublicBaseClass( ClassDeclaration* base, const KDevelop::TopDUContext* topContext, int* baseConversionLevels ) const {
  return isPublicBaseClassInternal( this, base, topContext, baseConversionLevels, 0, 0 );
}

QString ClassDeclaration::toString() const {
  QString ret;
  switch ( classModifier() ) {
    case ClassDeclarationData::None:
      //nothing
      break;
    case ClassDeclarationData::Abstract:
      ret += "abstract ";
      break;
    case ClassDeclarationData::Final:
      ret += "final ";
      break;
  }
  switch ( classType() ) {
    case ClassDeclarationData::Class:
      ret += "class ";
      break;
    case ClassDeclarationData::Interface:
      ret += "interface ";
      break;
    case ClassDeclarationData::Trait:
      ret += "trait ";
      break;
    case ClassDeclarationData::Union:
      ret += "union ";
      break;
    case ClassDeclarationData::Struct:
      ret += "struct ";
      break;
  }
  return ret + identifier().toString();
}

ClassDeclarationData::ClassType ClassDeclaration::classType() const {
  return d_func()->m_classType;
}

void ClassDeclaration::setClassType(ClassDeclarationData::ClassType type) {
  d_func_dynamic()->m_classType = type;
}

ClassDeclarationData::ClassModifier ClassDeclaration::classModifier() const {
  return d_func()->m_classModifier;
}

void ClassDeclaration::setClassModifier(ClassDeclarationData::ClassModifier modifier) {
  d_func_dynamic()->m_classModifier = modifier;
}

}
