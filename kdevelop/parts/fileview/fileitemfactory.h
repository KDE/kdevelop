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

#ifndef FILEITEMFACTORY_H
#define FILEITEMFACTORY_H

#include <kfiletreeview.h>

class FileTreeWidget;

/**
@author KDevelop Authors
*/
namespace filetreeview
{
    /**
    @author KDevelop Authors
    */
    class FileTreeViewItem : public KFileTreeViewItem
    {
        friend class FileTreeBranchItem;
    protected:
        FileTreeViewItem( KFileTreeViewItem* parent, KFileItem* item, KFileTreeBranch* branch, bool pf )
        : KFileTreeViewItem( parent, item, branch ), m_isProjectFile( pf )
        {
            hideOrShow();
        }
        FileTreeViewItem( KFileTreeView* parent, KFileItem* item, KFileTreeBranch* branch )
        : KFileTreeViewItem( parent, item, branch ), m_isProjectFile( false )
        {
            hideOrShow();
        }
        virtual ~FileTreeViewItem() {}

    public:
        virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
        FileTreeWidget* listView() const;
        void hideOrShow();
        bool isProjectFile() const { return m_isProjectFile; }
        bool setProjectFile( QString const &path, bool pf );

    protected:
        virtual int compare( QListViewItem *i, int col, bool ascending ) const;

    private:
        bool m_isProjectFile;
    };

    /**
    @author KDevelop Authors
    */
    class FileTreeBranchItem : public KFileTreeBranch
    {
    protected:
        FileTreeBranchItem( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix )
        : KFileTreeBranch( view, url, name, pix, false,
            new FileTreeViewItem( view, new KFileItem( url, "inode/directory", S_IFDIR ), this ) )
        {
        }
        virtual ~FileTreeBranchItem()
        {
//            if (root())
//                delete root()->fileItem();
        }

    };

    /**
    @author KDevelop Authors
    */
    class BranchItemFactory
    {
    public:
        virtual FileTreeBranchItem *makeBranchItem( KFileTreeView* view, const KURL& url, const QString& name, const QPixmap& pix ) = 0;
    };
}

#endif
