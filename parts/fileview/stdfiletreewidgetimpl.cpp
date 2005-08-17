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

 #include <q3header.h>
//Added by qt3to4:
#include <QPixmap>
 #include <kdebug.h>

#include "fileitemfactory.h"
#include "filetreewidget.h"
#include "stdfiletreewidgetimpl.h"

///////////////////////////////////////////////////////////////////////////////
// class StdFileTreeViewItem
///////////////////////////////////////////////////////////////////////////////

class StdFileTreeViewItem : public filetreeview::FileTreeViewItem
{
public:
    StdFileTreeViewItem( KFileTreeViewItem* parent, KFileItem* item, KFileTreeBranch* branch, bool pf )
        : FileTreeViewItem( parent, item, branch, pf ) {}
    StdFileTreeViewItem( KFileTreeView* parent, KFileItem* item, KFileTreeBranch* branch )
        : FileTreeViewItem( parent, item, branch ) {}
};

///////////////////////////////////////////////////////////////////////////////
// class StdFileTreeBranchItem
///////////////////////////////////////////////////////////////////////////////

class StdFileTreeBranchItem : public filetreeview::FileTreeBranchItem
{
public:
    StdFileTreeBranchItem( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix )
        : FileTreeBranchItem( view, url, name, pix ) {}
    virtual ~StdFileTreeBranchItem() {}
    virtual KFileTreeViewItem* createTreeViewItem( KFileTreeViewItem* parent, KFileItem* fileItem );
};

///////////////////////////////////////////////////////////////////////////////

KFileTreeViewItem* StdFileTreeBranchItem::createTreeViewItem( KFileTreeViewItem* parent, KFileItem* fileItem )
{
    if (!parent || !fileItem)
        return 0;

    FileTreeWidget *lv = static_cast<StdFileTreeViewItem*>( parent )->listView();
    const KURL fileURL = fileItem->url();
    bool isDirectory = lv->projectFiles().contains( fileURL.path() ) > 0;

    return new StdFileTreeViewItem( parent, fileItem, this, isDirectory );
}

///////////////////////////////////////////////////////////////////////////////
// class StdFileItemFactory
///////////////////////////////////////////////////////////////////////////////

class StdBranchItemFactory : public filetreeview::BranchItemFactory
{
public:
    virtual filetreeview::FileTreeBranchItem *makeBranchItem( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix )
    {
        return new StdFileTreeBranchItem( view, url, name, pix );
    }
};

///////////////////////////////////////////////////////////////////////////////
// class StdFileTreeWidgetImpl
///////////////////////////////////////////////////////////////////////////////

StdFileTreeWidgetImpl::StdFileTreeWidgetImpl( FileTreeWidget *parent )
    : FileTreeViewWidgetImpl( parent, "stdfiletreewidgetimpl" )
{
    kdDebug(9017) << "StdFileTreeWidgetImpl::StdFileTreeWidgetImpl()" << endl;

    setBranchItemFactory( new StdBranchItemFactory );

    parent->addColumn( QString() );
    header()->hide();
}

///////////////////////////////////////////////////////////////////////////////

StdFileTreeWidgetImpl::~StdFileTreeWidgetImpl()
{
    kdDebug(9017) << "StdFileTreeWidgetImpl::~StdFileTreeWidgetImpl()" << endl;
}

#include "stdfiletreewidgetimpl.moc"
