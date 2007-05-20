/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn  <dukjuahn@gmail.com>                 *
 *   Copyright (C) 2007 by Andreas Pakulat  <apaku@gmx.de>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef IOUTPUTVIEWITEM_H
#define IOUTPUTVIEWITEM_H

#include "kdevexport.h"
#include <QtGui/QStandardItem>
class QAction;
template<typename T1> class QList;

/**
 * Represents one line at OutputView.
 * Generated in IOutputViewItemFactory.
 * @sa IOutputViewItemFactory
 */
class OUTPUTVIEWINTERFACES_EXPORT IOutputViewItem : public QStandardItem
{
public:
    /**
     * constructor
     * @param text The text which will be displayed in listview.
     */
    IOutputViewItem( const QString& text );

    /// Destructor
    virtual ~IOutputViewItem();

    /**
     * Called when ListView emits signals such as activated(), clicked() and doubleClicked().
     * Each item can decide which action to be done when item was activated. For example,
     * compile error item can open the file that caused compile error.
     */
    virtual void activate();

    /**
     * Provides the list of QActions when context menu was requested for this item.
     * @return List of QAction which will be inserted in context menu.
     */
    virtual QList<QAction*> contextMenuActions();
private:
    class IOutputViewItemPrivate* const d;
};


#endif
//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
