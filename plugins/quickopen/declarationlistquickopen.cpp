
/* This file is part of the KDE libraries
   Copyright (C) 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "declarationlistquickopen.h"

using namespace KDevelop;

DeclarationListDataProvider::DeclarationListDataProvider(IQuickOpen* quickopen,
                                                         const QList<DUChainItem>& items,
                                                         bool openDefinitions)
: DUChainItemDataProvider( quickopen, openDefinitions )
, m_items(items)
{
  reset();
}

void DeclarationListDataProvider::reset()
{
  DUChainItemDataProvider::clearFilter();
  setItems(m_items);
}
