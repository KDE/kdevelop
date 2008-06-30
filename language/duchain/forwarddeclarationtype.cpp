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


#include "forwarddeclarationtype.h"
#include "forwarddeclaration.h"

using namespace KDevelop;

ForwardDeclarationType::ForwardDeclarationType(const ForwardDeclarationType& rhs) : AbstractType(rhs), IdentifiedType(rhs) {
}

ForwardDeclarationType::ForwardDeclarationType() {
}

bool ForwardDeclarationType::equals(const AbstractType* rhs) const {
  const IdentifiedType* other = dynamic_cast<const IdentifiedType*>(rhs);
  if(!other)
    return false;
  
  return IdentifiedType::equals(other);
}

AbstractType::WhichType ForwardDeclarationType::whichType() const
{
  return AbstractType::TypeForward;
}

QString ForwardDeclarationType::toString() const {
  return IdentifiedType::qualifiedIdentifier().toString();
}

AbstractType* ForwardDeclarationType::clone() const {
  return new ForwardDeclarationType(*this);
}

void ForwardDeclarationType::accept0 (TypeVisitor */*v*/) const {
}

AbstractType::Ptr ForwardDeclarationType::resolve(const TopDUContext* topContext) const {
  Declaration* bDecl = declaration(topContext);
  if(bDecl) {
    //kDebug() << "have basic decl:" << bDecl->toString();
    ForwardDeclaration* decl = dynamic_cast<ForwardDeclaration*>(bDecl);
    if(!decl) {
      //kWarning() << "Forward-declaration type is attached to non forward-declaration";
      return bDecl->abstractType();
    }else{
      Declaration* resolved = decl->resolve(topContext);
      if(resolved)
        return resolved->abstractType();
    }
  }else{
    //kDebug() << "no basic decl";
  }
  return AbstractType::Ptr(const_cast<ForwardDeclarationType*>(this));
}

uint ForwardDeclarationType::hash() const {
  return IdentifiedType::hash();
}
