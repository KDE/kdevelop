/* This file is part of KDevelop
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

#include "localindexeddeclaration.h"

#include "declaration.h"
#include "topducontextdynamicdata.h"
#include "topducontext.h"

using namespace KDevelop;

LocalIndexedDeclaration::LocalIndexedDeclaration(Declaration* decl)
  : m_declarationIndex(decl ? decl->m_indexInTopContext : 0)
{
}

LocalIndexedDeclaration::LocalIndexedDeclaration(uint declarationIndex)
  : m_declarationIndex(declarationIndex)
{
}

Declaration* LocalIndexedDeclaration::data(TopDUContext* top) const
{
  if(!m_declarationIndex)
    return 0;
  Q_ASSERT(top);
  return top->m_dynamicData->getDeclarationForIndex(m_declarationIndex);
}

bool LocalIndexedDeclaration::isLoaded(TopDUContext* top) const
{
  if(!m_declarationIndex)
    return false;

  Q_ASSERT(top);
  return top->m_dynamicData->isDeclarationForIndexLoaded(m_declarationIndex);
}
