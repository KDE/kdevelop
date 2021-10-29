/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_PLUGIN_DECLARATION_LIST_QUICKOPEN_H
#define KDEVPLATFORM_PLUGIN_DECLARATION_LIST_QUICKOPEN_H

#include "duchainitemquickopen.h"

namespace KDevelop {
class IQuickOpen;
}

class DeclarationListDataProvider
    : public DUChainItemDataProvider
{
    Q_OBJECT
public:
    DeclarationListDataProvider(KDevelop::IQuickOpen* quickopen,
                                const QVector<DUChainItem>& items,
                                bool openDefinitions = false);

    void reset() override;
private:
    QVector<DUChainItem> m_items;
};

#endif

