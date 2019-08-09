/* This file is part of the KDE libraries
   Copyright (C) 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef DUCHAIN_ITEM_QUICKOPEN
#define DUCHAIN_ITEM_QUICKOPEN

#include <language/interfaces/quickopendataprovider.h>
#include <language/interfaces/quickopenfilter.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/declaration.h>

namespace KDevelop {
class IQuickOpen;
}

struct DUChainItem
{
    DUChainItem()
    {
    }
    KDevelop::IndexedDeclaration m_item;
    QString m_text;
    KDevelop::Path m_projectPath;
    bool m_noHtmlDestription = false;
};

Q_DECLARE_TYPEINFO(DUChainItem, Q_MOVABLE_TYPE);

class DUChainItemData
    : public KDevelop::QuickOpenDataBase
{
public:
    explicit DUChainItemData(const DUChainItem& item, bool openDefinition = false);

    QString text() const override;
    QString htmlDescription() const override;
    QList<QVariant> highlighting() const override;

    bool execute(QString& filterText) override;

    bool isExpandable() const override;
    QWidget* expandingWidget() const override;

    QIcon icon() const override;
    KDevelop::Path projectPath() const;

private:
    const DUChainItem m_item;
    const bool m_openDefinition;
};

/**
 * A QuickOpenDataProvider that presents a list of declarations.
 * The declarations need to be set using setItems(..) in a re-implemented reset() function.
 * */

class DUChainItemDataProvider
    : public KDevelop::QuickOpenDataProviderBase
    , public KDevelop::Filter<DUChainItem>
{
    Q_OBJECT
public:
    using Base = KDevelop::Filter<DUChainItem>;

    /// When openDefinitions is true, the definitions will be opened if available on execute().
    explicit DUChainItemDataProvider(KDevelop::IQuickOpen* quickopen, bool openDefinitions = false);
    void setFilterText(const QString& text) override;
    uint itemCount() const override;
    uint unfilteredItemCount() const override;
    KDevelop::QuickOpenDataPointer data(uint row) const override;

    void reset() override;
protected:
    //Override to create own DUChainItemData derived classes
    DUChainItemData* createData(const DUChainItem& item) const;

    //Reimplemented from Base<..>
    QString itemText(const DUChainItem& data) const override;

    KDevelop::IQuickOpen* m_quickopen;
private:
    bool m_openDefinitions;
};

#endif

