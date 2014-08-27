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

#ifndef KDEVPLATFORM_TOPDUCONTEXTDATA_H
#define KDEVPLATFORM_TOPDUCONTEXTDATA_H

#include "ducontextdata.h"
#include "topducontext.h"
#include "indexedstring.h"
#include "declarationid.h"
#include "problem.h"

namespace KDevelop {

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(TopDUContextData, m_usedDeclarationIds, DeclarationId)
KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(TopDUContextData, m_problems, LocalIndexedProblem)

class KDEVPLATFORMLANGUAGE_EXPORT TopDUContextData : public DUContextData
{
public:
  TopDUContextData(IndexedString url)
    : DUContextData()
    , m_deleting(false)
    , m_url(url)
    , m_ownIndex(0)
    , m_currentUsedDeclarationIndex(0)
  {
    initializeAppendedLists();
  }

  TopDUContextData(const TopDUContextData& rhs)
    : DUContextData(rhs)
    , m_deleting(false)
  {
    initializeAppendedLists();
    copyListsFrom(rhs);
    m_features = rhs.m_features;
    m_url = rhs.m_url;
    m_currentUsedDeclarationIndex = rhs.m_currentUsedDeclarationIndex;
    m_ownIndex = rhs.m_ownIndex;
    m_importsCache = rhs.m_importsCache;
  }

  ~TopDUContextData() {
    freeAppendedLists();
  }

  TopDUContext::Features m_features;

  bool m_deleting : 1; ///@todo remove

  IndexedString m_url;
  uint m_ownIndex;

  ///If this is not empty, it means that the cache is used instead of the implicit structure.
  TopDUContext::IndexedRecursiveImports m_importsCache;

  ///Is used to count up the used declarations while building uses
  uint m_currentUsedDeclarationIndex;

  START_APPENDED_LISTS_BASE(TopDUContextData, DUContextData);
  ///Maps a declarationIndex to a DeclarationId, which is used when the entry in m_usedDeclaration is zero.
  APPENDED_LIST_FIRST(TopDUContextData, DeclarationId, m_usedDeclarationIds);
  APPENDED_LIST(TopDUContextData, LocalIndexedProblem, m_problems, m_usedDeclarationIds);
  END_APPENDED_LISTS(TopDUContextData, m_problems);

  private:
  static void updateImportCacheRecursion(IndexedTopDUContext currentContext, std::set<uint>& visited);
  static void updateImportCacheRecursion(uint baseIndex, IndexedTopDUContext currentContext, TopDUContext::IndexedRecursiveImports& imports);
  friend class TopDUContext;
};

}
#endif
