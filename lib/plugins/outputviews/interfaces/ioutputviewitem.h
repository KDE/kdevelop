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
class QAction;
template<typename T1> class QList;


/**
 * Represents one line at OutputView.
 * Generated in IOutputViewItemFactory.
 * @sa IOutputViewItemFactory
 */
class OUTPUTVIEWINTERFACES_EXPORT IOutputViewItem
{
public:

    /// Destructor
    virtual ~IOutputViewItem()
    {}

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
};


#endif
