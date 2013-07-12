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

        foreach( const Item& data, filterBase ) {
            if( itemText( data ).contains(text, Qt::CaseInsensitive) ) {
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

///Return the length of the separator
template<class SeparatorType>
inline int separatorLength(const SeparatorType& separator)
{
    return separator.length();
}
///Specialization for QChar (always length of 1)
template<>
inline int separatorLength(const QChar& /*separator*/)
{
    return 1;
}

template<class Item>
class FilterWithSeparator
{
public:
    virtual ~FilterWithSeparator()
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
    ///@param separator Should be a QChar, or if needed a QString
    template<class SeparatorType>
    void setFilter( const QStringList& text, const SeparatorType& separator )
    {
        if (m_oldFilterText == text) {
            return;
        }
        if (text.isEmpty()) {
            clearFilter();
            return;
        }

        QList<Item> filterBase = m_filtered;

        if (text.isEmpty() || m_oldFilterText.isEmpty()) {
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

        QString exactNeedle;
        if (!text.isEmpty()) {
            exactNeedle = separator + text.join(separator);
        }

        // filterBase is correctly sorted, to keep it that way we add
        // exact matches to this list in sorted way and then prepend the whole list in one go.
        QList<Item> exactMatches;
        // similar for starting matches
        QList<Item> startMatches;
        // all other matches
        QList<Item> otherMatches;
        foreach( const Item& data, filterBase ) {
            QString toFilter = itemText(data);

            if (!exactNeedle.isEmpty() && toFilter.endsWith(exactNeedle)) {
                // put exact matches up front
                exactMatches << data;
                continue;
            }

            int searchStart = 0;
            for(QStringList::const_iterator it = text.constBegin(); it != text.constEnd(); ++it) {
                if (searchStart != 0) {
                    searchStart = toFilter.indexOf(separator, searchStart);
                    if (searchStart == -1) {
                        break;
                    }
                    ++searchStart;
                }

                searchStart = toFilter.indexOf(*it, searchStart, Qt::CaseInsensitive);
                if (searchStart == -1) {
                    break;
                }

                ++searchStart;
            }

            if (searchStart == -1) {
                continue;
            }

            // prefer matches whose last element starts with the filter
            int lastSeparator = toFilter.lastIndexOf(separator);
            if (lastSeparator + 1 + separatorLength(separator) == searchStart) {
                startMatches << data;
            } else {
                otherMatches << data;
            }
        }

        m_filtered = exactMatches + startMatches + otherMatches;
        m_oldFilterText = text;
    }

protected:
    ///Should return the text an item should be filtered by.
    virtual QString itemText( const Item& data ) const = 0;

private:
    QStringList m_oldFilterText;
    QList<Item> m_filtered;
    QList<Item> m_items;
};

}

#endif
