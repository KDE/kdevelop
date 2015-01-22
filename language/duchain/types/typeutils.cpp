/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
   Copyright 2014 Sven Brauch <svenbrauch@gmail.com>

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

#include "typeutils.h"
#include "referencetype.h"
#include "pointertype.h"
#include "typealiastype.h"
#include "unsuretype.h"
#include "integraltype.h"

namespace TypeUtils {
  using namespace KDevelop;

  TypePtr< KDevelop::AbstractType > unAliasedType(const TypePtr< KDevelop::AbstractType >& _type) {
    TypePtr< KDevelop::AbstractType > type = _type;
    
    TypePtr< KDevelop::TypeAliasType > alias = type.cast<KDevelop::TypeAliasType>();
    
    int depth = 0; //Prevent endless recursion
    while(alias && depth < 20) {
      
      uint hadModifiers = alias->modifiers();
      
      type = alias->type();
      
      if(hadModifiers && type)
        type->setModifiers(type->modifiers() | hadModifiers);
      
      alias = type.cast<KDevelop::TypeAliasType>();
      ++depth;
    }
    
    return type;
  }
  
  ///@todo remove constant and topContext
  AbstractType::Ptr targetType(const AbstractType::Ptr& _base, const TopDUContext* /*topContext*/, bool* /*constant*/) {

    AbstractType::Ptr base(_base);

    ReferenceType::Ptr ref = base.cast<ReferenceType>();
    PointerType::Ptr pnt = base.cast<PointerType>();
    TypeAliasType::Ptr alias = base.cast<TypeAliasType>();

    while( ref || pnt || alias ) {
      
      uint hadModifiers = base->modifiers();
      
      if( ref ) {
        base = ref->baseType();
      } else if(pnt) {
        base = pnt->baseType();
      }else{
        base = alias->type();
      }
      if((alias || ref) && hadModifiers && base)
        base->setModifiers(base->modifiers() | hadModifiers);
      
      ref = base.cast<ReferenceType>();
      pnt = base.cast<PointerType>();
      alias = base.cast<TypeAliasType>();
    }

    return base;
  }

  AbstractType::Ptr targetTypeKeepAliases(const AbstractType::Ptr& _base, const TopDUContext* /*topContext*/, bool* /*constant*/) {

    AbstractType::Ptr base(_base);

    ReferenceType::Ptr ref = base.cast<ReferenceType>();
    PointerType::Ptr pnt = base.cast<PointerType>();

    while( ref || pnt ) {
      
      if( ref ) {
        uint hadModifiers = ref->modifiers();
        base = ref->baseType();
        if(hadModifiers && base)
          base->setModifiers(base->modifiers() | hadModifiers);
      } else if(pnt) {
        base = pnt->baseType();
      }
      ref = base.cast<ReferenceType>();
      pnt = base.cast<PointerType>();
    }

    return base;
  }

  AbstractType::Ptr resolveAliasType(const AbstractType::Ptr& eventualAlias)
  {
    if ( eventualAlias && eventualAlias->whichType() == KDevelop::AbstractType::TypeAlias ) {
      return eventualAlias.cast<TypeAliasType>()->type();
    }
    return eventualAlias;
  }

  bool isUsefulType(AbstractType::Ptr type)
  {
    type = resolveAliasType(type);
    if ( ! type ) {
      return false;
    }
    if ( type->whichType() != AbstractType::TypeIntegral ) {
      return true;
    }
    auto dtype = type.cast<IntegralType>()->dataType();
    if ( dtype != IntegralType::TypeMixed && dtype != IntegralType::TypeNull ) {
      return true;
    }
    return false;
  }
} // namespace TypeUtils
