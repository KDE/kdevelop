
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

#include "topducontextutils.h"

#include "abstractfunctiondeclaration.h"
#include "declaration.h"

using namespace KDevelop;

///Takes a set of conditions in the constructors, and checks with each call to operator() whether these conditions are fulfilled on the given declaration.
///The import-structure needs to be constructed and locked when this is used
TopDUContext::DeclarationChecker::DeclarationChecker(const TopDUContext* _top, const CursorInRevision& _position, const AbstractType::Ptr& _dataType, DUContext::SearchFlags _flags, KDevVarLengthArray<IndexedDeclaration>* _createVisibleCache)
  : createVisibleCache(_createVisibleCache)
  , top(_top)
  , topDFunc(_top->d_func())
  , position(_position)
  , dataType(_dataType)
  , flags(_flags)
{
}

bool TopDUContext::DeclarationChecker::operator()(const Declaration* decl) const
{
  if(!decl)
    return false;

  if (top != decl->topContext()) {

    if((flags & DUContext::OnlyFunctions) && !dynamic_cast<const AbstractFunctionDeclaration*>(decl))
      return false;

    if (dataType && decl->abstractType()->indexed() != dataType->indexed())
      // The declaration doesn't match the type filter we are applying
      return false;

  } else {
    if((flags & DUContext::OnlyFunctions) && !dynamic_cast<const AbstractFunctionDeclaration*>(decl))
      return false;

    if (dataType && decl->abstractType() != dataType)
      // The declaration doesn't match the type filter we are applying
      return false;

    if (decl->range().start >= position)
      if(!decl->context() || decl->context()->type() != DUContext::Class)
          return false; // The declaration is behind the position we're searching from, therefore not accessible
  }
  // Success, this declaration is accessible
  return true;
}
