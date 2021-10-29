/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PROJECT_ITEM_QUICKOPEN
#define PROJECT_ITEM_QUICKOPEN

#include "duchainitemquickopen.h"

#include <serialization/indexedstring.h>
#include <language/duchain/identifier.h>

#include <functional>
#include <type_traits>

template <typename Type>
class ResultCache
{
public:
    ResultCache(std::function<Type()> func)
        : m_func(func)
    {
    }

    /// Mark this cache dirty. A call to cachedResult() will need to refill the cache
    inline void markDirty() const
    {
        m_isDirty = true;
    }

    /**
     * If marked dirty, calls @p func and stores return value of @p func
     *
     * @return Cached result of @p func
     */
    inline Type cachedResult() const
    {
        if (m_isDirty) {
            m_result = m_func();
            m_isDirty = false;
        }
        return m_result;
    }
private:
    std::function<Type()> m_func;

    mutable Type m_result;
    mutable bool m_isDirty = true;
};

struct CodeModelViewItem
{
    CodeModelViewItem()
    {
    }
    CodeModelViewItem(const KDevelop::IndexedString& file, const KDevelop::QualifiedIdentifier& id)
        : m_file(file)
        , m_id(id)
    {
    }
    KDevelop::IndexedString m_file;
    KDevelop::QualifiedIdentifier m_id;
};

Q_DECLARE_TYPEINFO(CodeModelViewItem, Q_MOVABLE_TYPE);

using AddedItems = QMap<uint, QList<KDevelop::QuickOpenDataPointer>>;

class ProjectItemDataProvider
    : public KDevelop::QuickOpenDataProviderBase
{
    Q_OBJECT
public:
    enum ItemTypes {
        NoItems = 0,
        Classes = 1,
        Functions = 2,
        AllItemTypes = Classes + Functions
    };

    explicit ProjectItemDataProvider(KDevelop::IQuickOpen* quickopen);

    void enableData(const QStringList& items, const QStringList& scopes) override;

    void setFilterText(const QString& text) override;

    void reset() override;

    uint itemCount() const override;
    uint unfilteredItemCount() const override;

    static QStringList supportedItemTypes();
private:
    KDevelop::QuickOpenDataPointer data(uint pos) const override;

    ItemTypes m_itemTypes;
    KDevelop::IQuickOpen* m_quickopen;
    QSet<KDevelop::IndexedString> m_files;
    QVector<CodeModelViewItem> m_currentItems;
    QString m_currentFilter;
    QVector<CodeModelViewItem> m_filteredItems;

    //Maps positions to the additional items behind those positions
    //Here additional inserted items are stored, that are not represented in m_filteredItems.
    //This is needed at least to also show overloaded function declarations
    mutable AddedItems m_addedItems;
    ResultCache<uint> m_addedItemsCountCache;
};

#endif
