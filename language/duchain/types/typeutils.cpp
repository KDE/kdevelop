/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

namespace TypeUtils {
  using namespace KDevelop;

  TypePtr< KDevelop::AbstractType > unAliasedType(const TypePtr< KDevelop::AbstractType >& _type) {
    TypePtr< KDevelop::AbstractType > type = _type;
    
    TypePtr< KDevelop::TypeAliasType > alias = type.cast<KDevelop::TypeAliasType>();
    
    int depth = 0; //Prevent endless recursion
    while(alias && depth < 20) {
      type = alias->type();
      alias = type.cast<KDevelop::TypeAliasType>();
      ++depth;
    }
    
    return type;
  }
  
  AbstractType::Ptr targetType(const AbstractType::Ptr& _base, const TopDUContext* /*topContext*/, bool* constant) {

    AbstractType::Ptr base(_base);

    ReferenceType::Ptr ref = base.cast<ReferenceType>();
    PointerType::Ptr pnt = base.cast<PointerType>();
    TypeAliasType::Ptr alias = base.cast<TypeAliasType>();

    while( ref || pnt || alias ) {
      if( ref ) {
        if( constant )
          (*constant) |= static_cast<bool>(ref->modifiers() & AbstractType::ConstModifier);
        base = ref->baseType();
      } else if(pnt) {
        if( constant )
          (*constant) |= static_cast<bool>(pnt->modifiers() & AbstractType::ConstModifier);
        base = pnt->baseType();
      }else{
        base = alias->type();
      }
      ref = base.cast<ReferenceType>();
      pnt = base.cast<PointerType>();
      alias = base.cast<TypeAliasType>();
    }

    return base;
  }

  AbstractType::Ptr targetTypeKeepAliases(const AbstractType::Ptr& _base, const TopDUContext* /*topContext*/, bool* constant) {

    AbstractType::Ptr base(_base);

    ReferenceType::Ptr ref = base.cast<ReferenceType>();
    PointerType::Ptr pnt = base.cast<PointerType>();

    while( ref || pnt ) {
      if( ref ) {
        if( constant )
          (*constant) |= static_cast<bool>(ref->modifiers() & AbstractType::ConstModifier);
        base = ref->baseType();
      } else if(pnt) {
        if( constant )
          (*constant) |= static_cast<bool>(pnt->modifiers() & AbstractType::ConstModifier);
        base = pnt->baseType();
      }
      ref = base.cast<ReferenceType>();
      pnt = base.cast<PointerType>();
    }

    return base;
  }
}
