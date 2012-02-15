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

#include "projectitemquickopen.h"
#include <language/duchain/topducontext.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>
#include <language/duchain/abstractfunctiondeclaration.h>
#include <language/duchain/types/structuretype.h>
#include <project/projectmodel.h>
#include <klocale.h>
#include <language/interfaces/iquickopen.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/codemodel.h>

using namespace KDevelop;

ProjectItemDataProvider::ProjectItemDataProvider( KDevelop::IQuickOpen* quickopen ) : m_quickopen(quickopen) {
}

struct SubstringCache {
    SubstringCache(QString string = QString()) : substring(string) {
    }

    inline int containedIn(Identifier id) {
        int index = id.index();
        QHash<int, int>::const_iterator it = cache.constFind(index);
        if(it != cache.constEnd())
            return *it;

        const QString idStr = id.identifier().str();

        int result = idStr.lastIndexOf(substring, -1, Qt::CaseInsensitive );

        //here we shift the values if the matched string is bigger than the substring, so closer matches will appear first
        if (result >= 0) {
            result = result + (idStr.size() - substring.size());
        }

        cache[index] = result;

        return result;
    }

    QString substring;
    QHash<int, int> cache;
};

struct ClosestMatchToText {
    ClosestMatchToText(const QHash<int , int >& _cache) : cache(_cache) {

    }

    /** @brief Calculates the distance to two pre-filtered match items
     *
     *  @param a The CodeModelView witch represents the first item to be tested
     *  @param b The CodeModelView witch represents the second item to be tested
     *
     *  @b
     */
    inline bool operator()(const CodeModelViewItem& a, const CodeModelViewItem& b) {
        const int height_a = cache[a.m_id.index()];
        const int height_b = cache[b.m_id.index()];

        return height_a < height_b;
    }

private:
    const QHash<int , int >& cache;
};

void ProjectItemDataProvider::setFilterText( const QString& text ) {
    m_addedItems.clear();

    QStringList search(text.split("::", QString::SkipEmptyParts));
    for(int a = 0; a < search.count(); ++a) {
        if(search[a].endsWith(':')) //Don't get confused while the :: is being typed
            search[a] = search[a].left(search[a].length()-1);
    }
    if(!search.isEmpty() && search.back().endsWith('(')) {
        search.back() = search.back().left(search.back().length()-1);
    }

    if(text.isEmpty() || search.isEmpty()) {
        m_filteredItems = m_currentItems;
        return;
    }
    KDevVarLengthArray<SubstringCache, 5> cache;
    for(int a = 0; a < search.count(); ++a) {
        cache.append(SubstringCache(search.at(a)));
    }

    if(!text.startsWith(m_currentFilter)) {
        m_filteredItems = m_currentItems;
    }

    m_currentFilter = text;

    QVector<CodeModelViewItem> oldFiltered = m_filteredItems;
    QHash<int , int > heights;

    m_filteredItems.clear();

    for(int a = 0; a < oldFiltered.size(); ++a) {
        QualifiedIdentifier currentId = oldFiltered[a].m_id;

        int last_pos = currentId.count() - 1, current_height = 0, distance = 0;

        //iter over each search item from last to first
        //this makes easier to calculate the distance based on where we hit the result or nothing
        for(int b = search.count() - 1; b >= 0; --b) {
            //iter over each id for the current identifier, from last to first, so we
            for(; last_pos >= 0; --last_pos, distance++) {
                // the more distant we are from the class definition, the less priority it will have
                current_height += distance * 10000;
                int result;
                //if the current search item is contained on the current identifier
                if((result = cache[b].containedIn( currentId.at(last_pos) )) >= 0) {
                    //when we find a hit, whe add the distance to the searched word.
                    //so the closest item will be displayed first
                    current_height += result;

                    if(b == 0) {
                        heights[currentId.index()] = current_height;
                        m_filteredItems << oldFiltered[a];
                    }
                    break;
                }
            }
        }
    }

    //then, for the last part, we use the already built cache to sort the items according with their distance
    qSort(m_filteredItems.begin(), m_filteredItems.end(), ClosestMatchToText(heights));
}

QList<KDevelop::QuickOpenDataPointer> ProjectItemDataProvider::data( uint start, uint end ) const {

    QList<KDevelop::QuickOpenDataPointer> ret;
    for(uint a = start; a < end; ++a)
        ret << data(a);
    return ret;
}

KDevelop::QuickOpenDataPointer ProjectItemDataProvider::data( uint pos ) const {

    //Check whether this position falls into an appended item-list, else apply the offset
    uint filteredItemOffset = 0;
    for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::const_iterator it = m_addedItems.constBegin(); it != m_addedItems.constEnd(); ++it) {
        int offsetInAppended = pos - (it.key()+1);
        if(offsetInAppended >= 0 && offsetInAppended < it.value().count()) {
            return it.value()[offsetInAppended];
        }
        if(it.key() >= pos)
            break;
        filteredItemOffset += it.value().count();
    }
//
//     add += it.value().count();

    uint a = pos - filteredItemOffset;
    if(a > (uint)m_filteredItems.size())
        return KDevelop::QuickOpenDataPointer();

    QList<KDevelop::QuickOpenDataPointer> ret;
    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    TopDUContext* ctx = DUChainUtils::standardContextForUrl(m_filteredItems[a].m_file.toUrl());
    if(ctx) {
        QList<Declaration*> decls = ctx->findDeclarations(m_filteredItems[a].m_id, CursorInRevision::invalid(), AbstractType::Ptr(), 0, DUContext::DirectQualifiedLookup);

        //Filter out forward-declarations
        foreach(Declaration* decl, decls)
        if(decl->isForwardDeclaration() && decls.size() > 1)
            decls.removeAll(decl);

        foreach(Declaration* decl, decls) {
            DUChainItem item;
            item.m_item = decl;
            item.m_text = decl->qualifiedIdentifier().toString();
            //item.m_project =  .. @todo fill
            ret << QuickOpenDataPointer(new DUChainItemData(item, true));
        }
        if(decls.isEmpty()) {
            DUChainItem item;
            item.m_text = m_filteredItems[a].m_id.toString();
            //item.m_project =  .. @todo fill
            ret << QuickOpenDataPointer(new DUChainItemData(item));
        }
    }else{
        kDebug() << "Could not find standard-context";
    }

    if(!ret.isEmpty()) {
        QList<KDevelop::QuickOpenDataPointer> append = ret.mid(1);
        if(!append.isEmpty()) {
            QMap<uint, QList<KDevelop::QuickOpenDataPointer> > addMap;
            for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::iterator it = m_addedItems.begin(); it != m_addedItems.end();) {
                if(it.key() == pos) //There already is appended data stored, nothing to do
                    return ret.first();
                else if(it.key() > pos) {
                    addMap[it.key() + append.count()] = it.value();
                    it = m_addedItems.erase(it);
                }else{
                    ++it;
                }
            }

            m_addedItems.insert(pos, append);

            for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::const_iterator it = addMap.constBegin(); it != addMap.constEnd(); ++it)
                m_addedItems.insert(it.key(), it.value());
        }
        return ret.first();
    } else
        return KDevelop::QuickOpenDataPointer();
}

void ProjectItemDataProvider::reset() {
    m_usingFiles = m_quickopen->fileSet();
    m_currentItems.clear();
    m_addedItems.clear();

    KDevelop::DUChainReadLocker lock( DUChain::lock() );
    foreach( const IndexedString &u, m_usingFiles ) {
        uint count;
        const KDevelop::CodeModelItem* items;
        CodeModel::self().items( u, count, items );

        for(uint a = 0; a < count; ++a) {
            if(!items[a].id.isValid() || items[a].kind & CodeModelItem::ForwardDeclaration)
                continue;
            if(((m_itemTypes & Classes) && (items[a].kind & CodeModelItem::Class)) ||
                    ((m_itemTypes & Functions) && (items[a].kind & CodeModelItem::Function)))
            {
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

uint ProjectItemDataProvider::itemCount() const {
    uint add = 0;
    for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::const_iterator it = m_addedItems.constBegin(); it != m_addedItems.constEnd(); ++it)
        add += it.value().count();
    return m_filteredItems.count() + add;
}

uint ProjectItemDataProvider::unfilteredItemCount() const
{
  uint add = 0;
  for(QMap<uint, QList<KDevelop::QuickOpenDataPointer> >::const_iterator it = m_addedItems.constBegin(); it != m_addedItems.constEnd(); ++it)
    add += it.value().count();
  return m_currentItems.count() + add;
}

QStringList ProjectItemDataProvider::supportedItemTypes() {
    QStringList ret;
    ret << i18n("Classes");
    ret << i18n("Functions");
    return ret;
}

void ProjectItemDataProvider::enableData( const QStringList& items, const QStringList& scopes ) {
    if( scopes.contains(i18n("Project")) ) {
        m_itemTypes = NoItems;
        if( items.contains( i18n("Classes") ) )
            m_itemTypes = (ItemTypes)(m_itemTypes | Classes);
        if( items.contains( i18n("Functions") ) )
            m_itemTypes = (ItemTypes)(m_itemTypes | Functions);

    } else {
        m_itemTypes = NoItems;
    }
}
