/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "projectitemquickopen.h"
#include "debug.h"

#include <language/duchain/topducontext.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/codemodel.h>
#include <language/interfaces/iquickopen.h>
#include <language/interfaces/abbreviations.h>

#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/icore.h>

#include <project/projectmodel.h>
#include <serialization/indexedstringview.h>

#include <KLocalizedString>

using namespace KDevelop;

namespace {
struct SubstringCache
{
    explicit SubstringCache(const QString& string = QString())
        : substring(string)
    {
    }

    inline int containedIn(const Identifier& id) const
    {
        int index = id.index();
        QHash<int, int>::const_iterator it = cache.constFind(index);
        if (it != cache.constEnd()) {
            return *it;
        }

        const QString idStr = id.identifier().str();

        int result = idStr.lastIndexOf(substring, -1, Qt::CaseInsensitive);
        if (result < 0 && !idStr.isEmpty() && !substring.isEmpty()) {
            // no match; try abbreviations
            result = matchesAbbreviation(idStr, substring) ? 0 : -1;
        }

        //here we shift the values if the matched string is bigger than the substring,
        //so closer matches will appear first
        if (result >= 0) {
            result = result + (idStr.size() - substring.size());
        }

        cache[index] = result;

        return result;
    }

    QString substring;
    mutable QHash<int, int> cache;
};

struct ClosestMatchToText
{
    explicit ClosestMatchToText(const QHash<int, int>& _cache)
        : cache(_cache)
    {
    }

    /** @brief Calculates the distance to two pre-filtered match items
     *
     *  @param a The CodeModelView witch represents the first item to be tested
     *  @param b The CodeModelView witch represents the second item to be tested
     *
     *  @b
     */
    inline bool operator()(const CodeModelViewItem& a, const CodeModelViewItem& b) const
    {
        const int height_a = cache.value(a.m_id.index(), -1);
        const int height_b = cache.value(b.m_id.index(), -1);

        Q_ASSERT(height_a != -1);
        Q_ASSERT(height_b != -1);

        if (height_a == height_b) {
            // stable sorting for equal items based on index
            // TODO: fast string-based sorting in such cases?
            return a.m_id.index() < b.m_id.index();
        }

        return height_a < height_b;
    }
private:
    const QHash<int, int>& cache;
};

Path findProjectForForPath(const IndexedString& path)
{
    const auto model = ICore::self()->projectController()->projectModel();
    const auto item = model->itemForPath(IndexedStringView::fromString(path));
    return item ? item->project()->path() : Path();
}
uint addedItems(const AddedItems& items)
{
    uint add = 0;
    for (auto& item : items) {
        add += item.count();
    }
    return add;
}

}

ProjectItemDataProvider::ProjectItemDataProvider(KDevelop::IQuickOpen* quickopen)
    : m_itemTypes(NoItems)
    , m_quickopen(quickopen)
    , m_addedItemsCountCache([this]() { return addedItems(m_addedItems); })
{
}

void ProjectItemDataProvider::setFilterText(const QString& text)
{
    m_addedItems.clear();
    m_addedItemsCountCache.markDirty();

    QStringList search(text.split(QStringLiteral("::"), Qt::SkipEmptyParts));
    for (auto& s : search) {
        if (s.endsWith(QLatin1Char(':'))) { //Don't get confused while the :: is being typed
            s.chop(1);
        }
    }

    if (!search.isEmpty() && search.back().endsWith(QLatin1Char('('))) {
        search.back().chop(1);
    }

    if (text.isEmpty() || search.isEmpty()) {
        m_filteredItems = m_currentItems;
        return;
    }

    KDevVarLengthArray<SubstringCache, 5> cache;
    for (const QString& searchPart : std::as_const(search)) {
        cache.append(SubstringCache(searchPart));
    }

    if (!text.startsWith(m_currentFilter)) {
        m_filteredItems = m_currentItems;
    }

    m_currentFilter = text;

    const QVector<CodeModelViewItem> oldFiltered = m_filteredItems;
    QHash<int, int> heights;

    m_filteredItems.clear();

    for (const CodeModelViewItem& item : oldFiltered) {
        const QualifiedIdentifier& currentId = item.m_id;

        int last_pos = currentId.count() - 1;
        int current_height = 0;
        int distance = 0;

        //iter over each search item from last to first
        //this makes easier to calculate the distance based on where we hit the result or nothing
        //Iterating from the last item to the first is more efficient, as we want to match the
        //class/function name, which is the last item on the search fields and on the identifier.
        for (int b = search.count() - 1; b >= 0; --b) {
            //iter over each id for the current identifier, from last to first
            for (; last_pos >= 0; --last_pos, distance++) {
                // the more distant we are from the class definition, the less priority it will have
                current_height += distance * 10000;
                int result;
                //if the current search item is contained on the current identifier
                if ((result = cache[b].containedIn(currentId.at(last_pos))) >= 0) {
                    //when we find a hit, whe add the distance to the searched word.
                    //so the closest item will be displayed first
                    current_height += result;

                    if (b == 0) {
                        heights[currentId.index()] = current_height;
                        m_filteredItems << item;
                    }
                    break;
                }
            }
        }
    }

    //then, for the last part, we use the already built cache to sort the items according with their distance
    std::sort(m_filteredItems.begin(), m_filteredItems.end(), ClosestMatchToText(heights));
}


KDevelop::QuickOpenDataPointer ProjectItemDataProvider::data(uint pos) const
{
    //Check whether this position falls into an appended item-list, else apply the offset
    uint filteredItemOffset = 0;
    for (AddedItems::const_iterator it = m_addedItems.constBegin(); it != m_addedItems.constEnd(); ++it) {
        int offsetInAppended = pos - (it.key() + 1);
        if (offsetInAppended >= 0 && offsetInAppended < it.value().count()) {
            return it.value()[offsetInAppended];
        }
        if (it.key() >= pos) {
            break;
        }
        filteredItemOffset += it.value().count();
    }

    const uint a = pos - filteredItemOffset;
    if (a > ( uint )m_filteredItems.size()) {
        return KDevelop::QuickOpenDataPointer();
    }

    const auto& filteredItem = m_filteredItems[a];

    QList<KDevelop::QuickOpenDataPointer> ret;
    KDevelop::DUChainReadLocker lock(DUChain::lock());
    TopDUContext* ctx = DUChainUtils::standardContextForUrl(filteredItem.m_file.toUrl());
    if (ctx) {
        QList<Declaration*> decls = ctx->findDeclarations(filteredItem.m_id, CursorInRevision::invalid(), AbstractType::Ptr(), nullptr, DUContext::DirectQualifiedLookup);

        //Filter out forward-declarations or duplicate imported declarations
        const auto unfilteredDecls = decls;
        for (Declaration* decl : unfilteredDecls) {
            bool filter = false;
            if (decls.size() > 1 && decl->isForwardDeclaration()) {
                filter = true;
            } else if (decl->url() != filteredItem.m_file && m_files.contains(decl->url())) {
                filter = true;
            }
            if (filter) {
                decls.removeOne(decl);
            }
        }

        ret.reserve(ret.size() + decls.size());
        for (Declaration* decl : std::as_const(decls)) {
            DUChainItem item;
            item.m_item = decl;
            item.m_text = decl->qualifiedIdentifier().toString();
            item.m_projectPath = findProjectForForPath(filteredItem.m_file);
            ret << QuickOpenDataPointer(new DUChainItemData(item));
        }

        if (decls.isEmpty()) {
            DUChainItem item;
            item.m_text = filteredItem.m_id.toString();
            item.m_projectPath = findProjectForForPath(filteredItem.m_file);
            ret << QuickOpenDataPointer(new DUChainItemData(item));
        }
    } else {
        qCDebug(PLUGIN_QUICKOPEN) << "Could not find standard-context";
    }

    if (!ret.isEmpty()) {
        QList<KDevelop::QuickOpenDataPointer> append = ret.mid(1);
        if (!append.isEmpty()) {
            m_addedItemsCountCache.markDirty();

            AddedItems addMap;
            for (AddedItems::iterator it = m_addedItems.begin(); it != m_addedItems.end(); ) {
                if (it.key() == pos) { //There already is appended data stored, nothing to do
                    return ret.first();
                } else if (it.key() > pos) {
                    addMap[it.key() + append.count()] = it.value();
                    it = m_addedItems.erase(it);
                } else {
                    ++it;
                }
            }

            m_addedItems.insert(pos, append);

            for (AddedItems::const_iterator it = addMap.constBegin(); it != addMap.constEnd(); ++it) {
                m_addedItems.insert(it.key(), it.value());
            }
        }
        return ret.first();
    } else {
        return KDevelop::QuickOpenDataPointer();
    }
}

void ProjectItemDataProvider::reset()
{
    m_files = m_quickopen->fileSet();
    m_currentItems.clear();
    m_addedItems.clear();
    m_addedItemsCountCache.markDirty();

    KDevelop::DUChainReadLocker lock(DUChain::lock());
    for (const IndexedString& u : std::as_const(m_files)) {
        uint count;
        const KDevelop::CodeModelItem* items;
        CodeModel::self().items(u, count, items);

        for (uint a = 0; a < count; ++a) {
            if (!items[a].id.isValid() || items[a].kind & CodeModelItem::ForwardDeclaration) {
                continue;
            }
            if (((m_itemTypes & Classes) && (items[a].kind & CodeModelItem::Class)) ||
                ((m_itemTypes & Functions) && (items[a].kind & CodeModelItem::Function))) {
                QualifiedIdentifier id = items[a].id.identifier();

                if (id.isEmpty() || id.at(0).identifier().isEmpty()) {
                    // id.isEmpty() not always hit when .toString() is actually empty...
                    // anyhow, this makes sure that we don't show duchain items without
                    // any name that could be searched for. This happens e.g. in the c++
                    // plugin for anonymous structs or sometimes for declarations in macro
                    // expressions
                    continue;
                }
                m_currentItems << CodeModelViewItem(u, id);
            }
        }
    }

    m_filteredItems = m_currentItems;
    m_currentFilter.clear();
}


uint ProjectItemDataProvider::itemCount() const
{
    return m_filteredItems.count() + m_addedItemsCountCache.cachedResult();
}

uint ProjectItemDataProvider::unfilteredItemCount() const
{
    return m_currentItems.count() + m_addedItemsCountCache.cachedResult();
}

QStringList ProjectItemDataProvider::supportedItemTypes()
{
    const QStringList ret{
        i18nc("@item quick open item type", "Classes"),
        i18nc("@item quick open item type", "Functions"),
    };
    return ret;
}

void ProjectItemDataProvider::enableData(const QStringList& items, const QStringList& scopes)
{
    //FIXME: property support different scopes
    if (scopes.contains(i18nc("@item quick open scope", "Project"))) {
        m_itemTypes = NoItems;
        if (items.contains(i18nc("@item quick open item type", "Classes"))) {
            m_itemTypes = ( ItemTypes )(m_itemTypes | Classes);
        }
        if (items.contains(i18nc("@item quick open item type", "Functions"))) {
            m_itemTypes = ( ItemTypes )(m_itemTypes | Functions);
        }
    } else {
        m_itemTypes = NoItems;
    }
}

#include "moc_projectitemquickopen.cpp"
