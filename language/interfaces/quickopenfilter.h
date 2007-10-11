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

#ifndef QUICKOPEN_FILTER_H
#define QUICKOPEN_FILTER_H

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

namespace KDevelop {
template<class Item>
class Filter {
  public:
    virtual ~Filter() {
    }
    ///Clears the filter, but not the data.
    void clear() {
      m_filtered = m_items;
      m_oldFilterText = QString();
    }

    ///Clears the filter and sets new data. The filter-text will be lost.
    void setItems( const QList<Item>& data ) {
      m_items = data;
      clear();
    }

    const QList<Item>& items() const {
      return m_items;
    }

    ///Returns the data that is left after the filtering
    const QList<Item>& filteredItems() const {
      return m_filtered;
    }

    ///Changes the filter-text and refilters the data
    void setFilter( const QString& text )
    {
      QList<Item> filterBase = m_filtered;
      if( !text.startsWith( m_oldFilterText ) )
        filterBase = m_items; //Start filtering based on the whole data

      ///@todo Use regexps, and the other additional filter-logic from kdevelop-3.4

      m_filtered.clear();

      foreach( const Item& data, filterBase )
        if( itemText( data ).contains( text ) )
          m_filtered << data;
      
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

#endif
