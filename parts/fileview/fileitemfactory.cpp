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

#include <qpainter.h>
#include <kdebug.h>

#include "filetreewidget.h"
#include "fileitemfactory.h"

using namespace filetreeview;

///////////////////////////////////////////////////////////////////////////////
// class FileTreeViewItem
///////////////////////////////////////////////////////////////////////////////

FileTreeWidget* FileTreeViewItem::listView() const
{
    return static_cast<FileTreeWidget*>( Q3ListViewItem::listView() );
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeViewItem::hideOrShow()
{
    kdDebug( 9017 ) << "MyFileTreeViewItem::hideOrShow(): " + path() << endl;
    setVisible( listView()->shouldBeShown( this ) );

    FileTreeViewItem* item = static_cast<FileTreeViewItem*>( firstChild() );
    while (item)
    {
        //kdDebug( 9017 ) << "MyFileTreeViewItem::hideOrShow(): " + item->path() << endl;
        item->hideOrShow();
        item = static_cast<FileTreeViewItem*>( item->nextSibling() );
    }
}

///////////////////////////////////////////////////////////////////////////////

bool FileTreeViewItem::setProjectFile( QString const & path, bool pf )
{
    kdDebug( 9017 ) << "FileTreeViewItem::setProjectFile(): " + path << " projectfile: " << pf << endl;

    if ( this->path() == path )
    {
        m_isProjectFile = pf;
        setVisible( listView()->shouldBeShown( this ) );
        repaint();
        return true;
    }

    FileTreeViewItem* item = static_cast<FileTreeViewItem*>( firstChild() );
    while( item )
    {
        if ( item->setProjectFile( path, pf ) )
            return true;
        else
            item = static_cast<FileTreeViewItem*>(item->nextSibling());
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void FileTreeViewItem::paintCell(QPainter *p, const QColorGroup &cg,
    int column, int width, int alignment)
{
    if ( listView()->showNonProjectFiles() && isProjectFile() )
    {
        QFont font( p->font() );
        font.setBold( true );
        p->setFont( font );
    }

    Q3ListViewItem::paintCell( p, cg, column, width, alignment );
}


///////////////////////////////////////////////////////////////////////////////

int FileTreeViewItem::compare( Q3ListViewItem *i, int col, bool ascending ) const
{
    KFileTreeViewItem* rhs = dynamic_cast<KFileTreeViewItem*>( i );
    if (rhs)
    {
        if (rhs->isDir() && !isDir())
            return (ascending) ? 1 : -1;
        else
            if (!rhs->isDir() && isDir())
                return (ascending) ? -1 : 1;
    }

    return Q3ListViewItem::compare( i, col, ascending );
}

///////////////////////////////////////////////////////////////////////////////
// class BranchItemFactory
///////////////////////////////////////////////////////////////////////////////
