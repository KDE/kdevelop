/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "declarationlistquickopen.h"

using namespace KDevelop;

DeclarationListDataProvider::DeclarationListDataProvider(IQuickOpen* quickopen,
                                                         const QVector<DUChainItem>& items,
                                                         bool openDefinitions)
    : DUChainItemDataProvider(quickopen, openDefinitions)
    , m_items(items)
{
    reset();
}

void DeclarationListDataProvider::reset()
{
    DUChainItemDataProvider::clearFilter();
    setItems(m_items);
}

#include "moc_declarationlistquickopen.cpp"
