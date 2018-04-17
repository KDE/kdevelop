/*
 * This file is part of KDevelop
 *
 * Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KDEVPLATFORM_QUICKOPEN_FILTER_H
#define KDEVPLATFORM_QUICKOPEN_FILTER_H

#include <QStringList>

#include "abbreviations.h"

#include <util/path.h>

namespace KDevelop {

/**
 * This is a simple filter-implementation that helps you implementing own quickopen data-providers.
 * You should use it when possible, because that way additional features(like regexp filtering) can
 * be implemented in a central place.
 *
 * This implementation does incremental filtering while
 * typing text, so it quite efficient for the most common case.
 *
 * The simplest way of using this is by reimplementing your data-provider
 * based on QuickOpenDataProviderBase and KDevelop::Filter\<Item\>.
 *
 * What you need to do to use it:
 *
 * Reimplement itemText(..) to provide the text filtering
 * should be performend on(This must be efficient).
 *
 * Call setItems(..) when starting a new quickopen session, or when the content
 * changes, to initialize the filter with your data.
 *
 * Call setFilter(..) with the text that should be filtered for on user-input.
 *
 * Use filteredItems() to provide data to quickopen.
 *
 * @tparam Item should be the type that holds all the information you need.
 * The filter will hold the data, and you can access it through "items()".
 */
template<class Item>
class Filter
{
public:
    virtual ~Filter()
    {
    }
    ///Clears the filter, but not the data.
    void clearFilter()
    {
        m_filtered = m_items;
        m_oldFilterText.clear();
    }

    ///Clears the filter and sets new data. The filter-text will be lost.
    void setItems(const QVector<Item>& data)
    {
        m_items = data;
        clearFilter();
    }

    const QVector<Item>& items() const
    {
        return m_items;
    }

    ///Returns the data that is left after the filtering
    const QVector<Item>& filteredItems() const
    {
        return m_filtered;
    }

    ///Changes the filter-text and refilters the data
    void setFilter( const QString& text )
    {
        if (m_oldFilterText == text) {
            return;
        }
        if (text.isEmpty()) {
            clearFilter();
            return;
        }

        QVector<Item> filterBase = m_filtered;
        if( !text.startsWith( m_oldFilterText ) ) {
            filterBase = m_items; //Start filtering based on the whole data
        }

        m_filtered.clear();

        QStringList typedFragments = text.split(QStringLiteral("::"), QString::SkipEmptyParts);
        if (typedFragments.isEmpty()) {
            clearFilter();
            return;
        }
        if ( typedFragments.last().endsWith(':') ) {
            // remove the trailing colon if there's only one; otherwise,
            // this breaks incremental filtering
            typedFragments.last().chop(1);
        }
        if (typedFragments.size() == 1 && typedFragments.last().isEmpty()) {
            clearFilter();
            return;
        }
        foreach( const Item& data, filterBase ) {
            const QString& itemData = itemText( data );
            if( itemData.contains(text, Qt::CaseInsensitive) || matchesAbbreviationMulti(itemData, typedFragments) ) {
                m_filtered << data;
            }
        }

        m_oldFilterText = text;
    }

protected:
    ///Should return the text an item should be filtered by.
    virtual QString itemText( const Item& data ) const = 0;

private:
    QString m_oldFilterText;
    QVector<Item> m_filtered;
    QVector<Item> m_items;
};

template<class Item, class Parent>
class PathFilter
{
public:
    ///Clears the filter, but not the data.
    void clearFilter()
    {
        m_filtered = m_items;
        m_oldFilterText.clear();
    }

    ///Clears the filter and sets new data. The filter-text will be lost.
    void setItems(const QVector<Item>& data)
    {
        m_items = data;
        clearFilter();
    }

    const QVector<Item>& items() const
    {
        return m_items;
    }

    ///Returns the data that is left after the filtering
    const QVector<Item>& filteredItems() const
    {
        return m_filtered;
    }

    ///Changes the filter-text and refilters the data
    void setFilter( const QStringList& text )
    {
        if (m_oldFilterText == text) {
            return;
        }
        if (text.isEmpty()) {
            clearFilter();
            return;
        }

        QVector<Item> filterBase = m_filtered;

        if ( m_oldFilterText.isEmpty()) {
            filterBase = m_items;
        } else if (m_oldFilterText.mid(0, m_oldFilterText.count() - 1) == text.mid(0, text.count() - 1)
                   && text.last().startsWith(m_oldFilterText.last())) {
            //Good, the prefix is the same, and the last item has been extended
        } else if (m_oldFilterText.size() == text.size() - 1 && m_oldFilterText == text.mid(0, text.size() - 1)) {
            //Good, an item has been added
        } else {
            //Start filtering based on the whole data, there was a big change to the filter
            filterBase = m_items;
        }

        QVector<QPair<int, int>> matches;
        for (int i = 0, c = filterBase.size(); i < c; ++i) {
            const auto& data = filterBase.at(i);
            const auto matchQuality = matchPathFilter(static_cast<Parent*>(this)->itemPath(data), text,
                                                      static_cast<Parent*>(this)->itemPrefixPath(data));
            if (matchQuality == -1) {
                continue;
            }
            matches.push_back({matchQuality, i});
        }
        std::stable_sort(matches.begin(), matches.end(),
                  [](const QPair<int, int>& lhs, const QPair<int, int>& rhs)
                  {
                    return lhs.first < rhs.first;
                  });
        m_filtered.resize(matches.size());
        std::transform(matches.begin(), matches.end(), m_filtered.begin(),
                       [&filterBase](const QPair<int, int>& match) {
                            return filterBase.at(match.second);
                       });
        m_oldFilterText = text;
    }

private:
    QStringList m_oldFilterText;
    QVector<Item> m_filtered;
    QVector<Item> m_items;
};

}

#endif
