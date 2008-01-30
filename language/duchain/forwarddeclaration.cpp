/* This file is part of KDevelop
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

#include "forwarddeclaration.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>
#include <klocale.h>

#include "duchain.h"
#include "duchainlock.h"
#include "ducontext.h"
#include "use.h"
#include "definition.h"
#include "symboltable.h"
#include "use_p.h"
#include "declaration_p.h"
#include "identifiedtype.h"
#include "forwarddeclarationtype.h"

using namespace KTextEditor;

namespace KDevelop
{

class ForwardDeclarationPrivate : public DeclarationPrivate
{
public:
  ForwardDeclarationPrivate() {
  }
  ForwardDeclarationPrivate( const ForwardDeclarationPrivate& rhs ) 
      : DeclarationPrivate( rhs )
  {
  }
};

ForwardDeclaration::ForwardDeclaration(const ForwardDeclaration& rhs) : Declaration(*new ForwardDeclarationPrivate(*rhs.d_func())) {
  if( abstractType() ) {
    //Also clone the foward-declaration type and assign it to the new forward-declaration
    ForwardDeclarationType* t = dynamic_cast<ForwardDeclarationType*>(abstractType().data());
    if( t ) {
      setAbstractType( AbstractType::Ptr(t->clone()) );
      Q_ASSERT(dynamic_cast<ForwardDeclarationType*>(abstractType().data()));
      static_cast<ForwardDeclarationType*>(abstractType().data())->setDeclaration(this);
    }
  }
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

ForwardDeclaration::ForwardDeclaration(const HashedString& url, const SimpleRange& range, Scope scope, DUContext* context )
  : Declaration(*new ForwardDeclarationPrivate, url, range, scope)
{
  if( context )
    setContext( context );
}

ForwardDeclaration::~ForwardDeclaration()
{
}
  
QString ForwardDeclaration::toString() const
{
  if(context() )
    return qualifiedIdentifier().toString();
  else
    return i18n("context-free forward-declaration") + " " + identifier().toString();
}

Declaration * ForwardDeclaration::resolve(const TopDUContext* topContext) const
{
  ENSURE_CAN_READ

  //If we've got a type assigned, that counts as a way of resolution.
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(abstractType().data());
  if( idType && !dynamic_cast<ForwardDeclarationType*>(abstractType().data()) ) {
    return idType->declaration();
  }
  
  if(!topContext)
      topContext = this->topContext();
  
  QualifiedIdentifier globalIdentifier = qualifiedIdentifier();
  globalIdentifier.setExplicitlyGlobal(true);

  //We've got to use DUContext::DirectQualifiedLookup so C++ works correctly.
  QList<Declaration*> declarations = topContext->findDeclarations(globalIdentifier, SimpleCursor::invalid(), AbstractType::Ptr(), 0, DUContext::DirectQualifiedLookup);

  foreach(Declaration* decl, declarations) {
    if( !decl->isForwardDeclaration() )
      return decl;
  }

  return 0;
}

DUContext * ForwardDeclaration::logicalInternalContext(const TopDUContext* topContext) const
{
  ENSURE_CAN_READ
  Declaration* resolved = resolve(topContext);
  if(resolved)
    return resolved->logicalInternalContext(topContext);
  else
    return Declaration::logicalInternalContext(topContext);
}

bool ForwardDeclaration::isForwardDeclaration() const
{
  return true;
}

Declaration* ForwardDeclaration::clone() const {
  return new ForwardDeclaration(*this);
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
