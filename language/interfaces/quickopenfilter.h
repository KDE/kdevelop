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

#include <QtCore/QRegExp>
#include <QtCore/QStringList>

#include "abbreviations.h"

#include <project/path.h>

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
 * based on QuickOpenDataProviderBase and KDevelop::Filter<YourType>.
 *
 * YourType should be the type that holds all the information you need.
 * The filter will hold the data, and you can access it through "items()".
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
 * */
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
    void setItems( const QList<Item>& data )
    {
        m_items = data;
        clearFilter();
    }

    const QList<Item>& items() const
    {
        return m_items;
    }

    ///Returns the data that is left after the filtering
    const QList<Item>& filteredItems() const
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

        QList<Item> filterBase = m_filtered;
        if( !text.startsWith( m_oldFilterText ) ) {
            filterBase = m_items; //Start filtering based on the whole data
        }

        m_filtered.clear();

        QStringList typedFragments = text.split("::", QString::SkipEmptyParts);
        if ( typedFragments.last().endsWith(':') ) {
            // remove the trailing colon if there's only one; otherwise,
            // this breaks incremental filtering
            typedFragments.last().chop(1);
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
    QList<Item> m_filtered;
    QList<Item> m_items;
};
}

namespace KDevelop {

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
    void setItems( const QList<Item>& data )
    {
        m_items = data;
        clearFilter();
    }

    const QList<Item>& items() const
    {
        return m_items;
    }

    ///Returns the data that is left after the filtering
    const QList<Item>& filteredItems() const
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

        const QString joinedText = text.join(QString());

        QList<Item> filterBase = m_filtered;

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

        // filterBase is correctly sorted, to keep it that way we add
        // exact matches to this list in sorted way and then prepend the whole list in one go.
        QList<Item> exactMatches;
        // similar for starting matches
        QList<Item> startMatches;
        // all other matches
        QList<Item> otherMatches;
        foreach( const Item& data, filterBase ) {
            const Path toFilter = static_cast<Parent*>(this)->itemPath(data);
            const QVector<QString>& segments = toFilter.segments();

            if (text.count() > segments.count()) {
                // number of segments mismatches, thus item cannot match
                continue;
            }
            {
                bool allMatched = true;
                // try to put exact matches up front
                for(int i = segments.count() - 1, j = text.count() - 1;
                    i >= 0 && j >= 0; --i, --j)
                {
                    if (segments.at(i) != text.at(j)) {
                        allMatched = false;
                        break;
                    }
                }
                if (allMatched) {
                    exactMatches << data;
                    continue;
                }
            }

            int searchIndex = 0;
            int pathIndex = 0;
            int lastMatchIndex = -1;
            // stop early if more search fragments remain than available after path index
            while (pathIndex < segments.size() && searchIndex < text.size()
                    && (pathIndex + text.size() - searchIndex - 1) < segments.size() )
            {
                const QString& segment = segments.at(pathIndex);
                const QString& typedSegment = text.at(searchIndex);
                lastMatchIndex = segment.indexOf(typedSegment, 0, Qt::CaseInsensitive);
                if (lastMatchIndex == -1 && !matchesAbbreviation(segment.midRef(0), typedSegment)) {
                    // no match, try with next path segment
                    ++pathIndex;
                    continue;
                }
                // else we matched
                ++searchIndex;
                ++pathIndex;
            }

            AbbreviationMatchQuality fuzzyMatch = NoMatch;
            if (searchIndex != text.size()) {
                if ( (fuzzyMatch = matchesPath(segments, joinedText)) == NoMatch ) {
                    continue;
                }
            }

            // prefer matches whose last element starts with the filter
            if ((pathIndex == segments.size() && lastMatchIndex == 0) || fuzzyMatch == MatchesSequentially) {
                startMatches << data;
            } else {
                otherMatches << data;
            }
        }

        m_filtered = exactMatches + startMatches + otherMatches;
        m_oldFilterText = text;
    }

private:
    QStringList m_oldFilterText;
    QList<Item> m_filtered;
    QList<Item> m_items;
};

}

#endif
