/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef STDFILETREEWIDGETIMPL_H
#define STDFILETREEWIDGETIMPL_H

#include "filetreeviewwidgetimpl.h"

/**
* @author Mario Scalas
* Provides the usual feedback of a KFileTreeView-widget (yeah, it does nothing other
* than providing almost standard KFileTreeViewItems). Adds 1 (one) column to the
* tree view and hides the header.
*/
class StdFileTreeWidgetImpl : public FileTreeViewWidgetImpl
{
    Q_OBJECT
public:
    StdFileTreeWidgetImpl( FileTreeWidget *parent );
    virtual ~StdFileTreeWidgetImpl();

    /*
    * We have no reason not to allow it here
    */
    virtual bool canReloadTree() const { return true; }
};

#endif
