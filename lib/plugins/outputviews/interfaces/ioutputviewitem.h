/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef IOUTPUTVIEWITEM_H
#define IOUTPUTVIEWITEM_H

#include "kdevexport.h"
#include <QStandardItem>
class QAction;
template<typename T1> class QList;

#define OUTPUTVIEWITEM_TYPE ( (QStandardItem::UserType) + 1 )

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
    virtual void activated() = 0;

    /**
     * Provides the list of QActions when context menu was requested for this item.
     * @return List of QAction which will be inserted in context menu.
     */
    virtual QList<QAction*> contextMenuActions() = 0;

    /**
     * The default implementation returns OUTPUTVIEWITEM_TYPE
     */
    virtual int type() const;
};

/**
 * Default Item. Can be used if any special treatment is not required.
 */
class OUTPUTVIEWINTERFACES_EXPORT DefaultOutputItem : public IOutputViewItem
{
public:
    DefaultOutputItem( const QString& text );
    virtual ~DefaultOutputItem();

    void activated();
    QList<QAction*> contextMenuActions();
};

#endif
