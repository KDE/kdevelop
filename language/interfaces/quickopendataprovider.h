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
#ifndef QUICKOPENDATAPROVIDER_H
#define QUICKOPENDATAPROVIDER_H

#include <QList>
#include <QObject>

#include <ksharedptr.h>
#include <languageexport.h>
class QString;
class QStringList;

namespace KDevelop {

/**
 * Reimplement his to represent single entries within the quickopen list.
 * */
class QuickOpenDataBase : public KShared {
  public:
    virtual ~QuickOpenDataBase();

    ///Return the text to be shown in the list for this item
    virtual QString text() const = 0;

    virtual QString htmlDescription() const = 0;
    
    /**Can return Custom highlighting triplets as explained in
     * the kde header ktexteditor/codecompletionmodel.h
     * The default-implementation returns an empty list, which means no
     * special highlighting will be applied.
     * */
    virtual QList<QVariant> highlighting() const;
};

typedef KSharedPtr<QuickOpenDataBase> QuickOpenDataPointer;

/**
 * Use this interface to provide custom quickopen-data to the quickopen-widget.
 *
 * One general assumption is always made:
 * After refiltering
 * */

class KDEVPLATFORMLANGUAGE_EXPORT QuickOpenDataProviderBase : public QObject {
  Q_OBJECT;
  public:
    virtual ~QuickOpenDataProviderBase();

    /**
     * For efficiency, all changes to the filter-text are provided by the following 3 difference-operations.
     * */
    
    ///New search-text was appended
    virtual void appendFilterText( const QString& text ) = 0;
    ///Search-text of length @param length was removed at the end
    virtual void removeFilterText( int length ) = 0;
    ///Filter-text should be completely reset
    virtual void clearFilterText() = 0;

    ///Returns the count of items this provider currently represents
    virtual uint itemCount() const = 0;

    /**
     * Returns the data-items for a given range.
     * Generally, the items must addressed alphabetically,
     * they will be displayed in the same order in the
     * quickopen list.
     *
     * For performance-reasons the underlying models should
     * create the QuickOpenDataBase items on demand, because only
     * those that will really be shown will be requested.
     * 
     * @param start First item to be returned
     * @param end Return items until here. end <= itemCount()
     * */
    virtual QList<QuickOpenDataPointer> data( uint start, uint end ) const = 0;
};

}

#endif
