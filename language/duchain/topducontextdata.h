/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef TOPDUCONTEXTDATA_H
#define TOPDUCONTEXTDATA_H

#include "ducontextdata.h"
#include "topducontext.h"
#include "indexedstring.h"
#include "declarationid.h"

namespace KDevelop {

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(TopDUContextData, m_usedDeclarationIds, DeclarationId);

class KDEVPLATFORMLANGUAGE_EXPORT TopDUContextData : public DUContextData
{
public:
  TopDUContextData(IndexedString url)
    : DUContextData(), m_flags(TopDUContext::NoFlags), m_inDuChain(false), m_url(url), m_currentUsedDeclarationIndex(0)
  {
    initializeAppendedLists();
  }
  TopDUContextData(const TopDUContextData& rhs) :DUContextData(rhs), m_deleting(false), m_inDuChain(false) {
    initializeAppendedLists();
    copyListsFrom(rhs);
    m_hasUses = rhs.m_hasUses;
  }
  ~TopDUContextData() {
    freeAppendedLists();
  }
  
  TopDUContext::Flags m_flags;

  bool m_hasUses  : 1;
  bool m_deleting : 1;
  bool m_inDuChain : 1;

  IndexedString m_url;

  ///Is used to count up the used declarations while building uses
  uint m_currentUsedDeclarationIndex;

  START_APPENDED_LISTS_BASE(TopDUContextData, DUContextData);
  ///Maps a declarationIndex to a DeclarationId, which is used when the entry in m_usedDeclaration is zero.
  APPENDED_LIST_FIRST(TopDUContextData, DeclarationId, m_usedDeclarationIds);
  END_APPENDED_LISTS(TopDUContextData, m_usedDeclarationIds);
};

}
#endif
