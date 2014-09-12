/* This file is part of KDevelop
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

#include "forwarddeclaration.h"

#include <ktexteditor/document.h>
#include <KLocalizedString>

#include "duchain.h"
#include "duchainlock.h"
#include "ducontext.h"
#include "use.h"
#include "duchainregister.h"
#include "types/identifiedtype.h"

using namespace KTextEditor;

namespace KDevelop
{
REGISTER_DUCHAIN_ITEM(ForwardDeclaration);

ForwardDeclaration::ForwardDeclaration(const ForwardDeclaration& rhs) : Declaration(*new ForwardDeclarationData(*rhs.d_func())) {
}

ForwardDeclaration::ForwardDeclaration(ForwardDeclarationData& data) : Declaration(data) {
}

ForwardDeclaration::ForwardDeclaration(const RangeInRevision& range, DUContext* context )
  : Declaration(*new ForwardDeclarationData, range)
{
  d_func_dynamic()->setClassId(this);
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
    return i18n("context-free forward-declaration %1", identifier().toString());
}

Declaration * ForwardDeclaration::resolve(const TopDUContext* topContext) const
{
  ENSURE_CAN_READ

  //If we've got a type assigned, that counts as a way of resolution.
  AbstractType::Ptr t = abstractType();
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(t.data());
  if( idType  ) {
    Declaration* decl = idType->declaration(topContext);
    if(decl && !decl->isForwardDeclaration())
      return decl;
    else
      return 0;
  }
  
  if(!topContext)
      topContext = this->topContext();
  
  QualifiedIdentifier globalIdentifier = qualifiedIdentifier();
  globalIdentifier.setExplicitlyGlobal(true);

  //We've got to use DUContext::DirectQualifiedLookup so C++ works correctly.
  QList<Declaration*> declarations = topContext->findDeclarations(globalIdentifier, CursorInRevision::invalid(), AbstractType::Ptr(), 0, DUContext::DirectQualifiedLookup);

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
  if(resolved && resolved != this)
    return resolved->logicalInternalContext(topContext);
  else
    return Declaration::logicalInternalContext(topContext);
}

bool ForwardDeclaration::isForwardDeclaration() const
{
  return true;
}

Declaration* ForwardDeclaration::clonePrivate() const {
  return new ForwardDeclaration(*this);
}

}
