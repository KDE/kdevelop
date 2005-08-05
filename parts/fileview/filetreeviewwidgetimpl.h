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

#ifndef FILETREEVIEWWIDGETIMPL_H
#define FILETREEVIEWWIDGETIMPL_H

#include <qobject.h>
#include <qvaluelist.h>
#include <kfiletreeview.h>
#include <qdom.h>

#include "fileitemfactory.h"

class FileTreeWidget;
namespace filetreeview
{
    class FileTreeViewItem;
    class FileTreeBranchItem;
    class BranchItemFactory;
}
class FileViewPart;
class QHeader;
class KToggleAction;
class QPopupMenu;

/**
* @author Mario Scalas
* A base class for providing additional features to the standard KFileTreeViewItem-based widget we
* use for listing files in project directory.
*/
class FileTreeViewWidgetImpl : public QObject
{
    Q_OBJECT
public:
    FileTreeViewWidgetImpl( FileTreeWidget *parent, const char *name );
    virtual ~FileTreeViewWidgetImpl();

    //! return a list containing the filenames of the currently selected items.
    KURL::List selectedPathUrls();
    // shortcuts
    FileTreeWidget *fileTree() const;
    QDomDocument &projectDom() const;
    QHeader *header() const { return fileTree()->header(); }
    void setColumnWidth( int column, int w ) { fileTree()->setColumnWidth( column, w ); }
    int contentsWidth() const { return fileTree()->contentsWidth(); }
    void triggerUpdate() { fileTree()->triggerUpdate(); }
    FileViewPart *part() const { return m_part; }
    filetreeview::FileTreeViewItem *currentItem() const { return  static_cast<filetreeview::FileTreeViewItem*>( fileTree()->currentItem() ); }
    /**
    * @return a reference to a new filetreeview::BranchItemFactory object which can be used for filling
    * the tree view.
    */
    filetreeview::BranchItemFactory *branchItemFactory() const { return m_branchItemFactory; }
    /**
    * Costraints that must be satisfied to start a reload of the the tree.
    * @return
    */
    virtual bool canReloadTree() const = 0;
    /**
    * Here the popup menu is filled: by standard only the "reload tree" (only if the above function
    * returns true) and "show project files" options are added
    * @param popupMenu the menu to fill
    * @param item the QListViewItem which this menu has been requested upon
    */
    virtual void fillPopupMenu( QPopupMenu *popupMenu, QListViewItem *item ) const;
    /**
    * @return true if non project files are to be shown, false otherwise
    */
    bool showNonProjectFiles() const;

signals:
    /**
    * Emitted when the current implementation "recognizes" it cannot work anymore: for example the VCS impl.
    * can ask to be relieved from work when the VCS plug-in it uses has been unloaded!
    */
    void implementationInvalidated();

private slots:
    void slotReloadTree();
    void slotToggleShowNonProjectFiles();

protected:
    void setBranchItemFactory( filetreeview::BranchItemFactory *aFactory ) { m_branchItemFactory = aFactory; }
    QString projectDirectory() const;

private:
	QValueList<QListViewItem*> allSelectedItems( QListViewItem * item ) const;
    filetreeview::BranchItemFactory *m_branchItemFactory;

    FileViewPart *m_part;

    bool m_isReloadingTree;

    KToggleAction *m_actionToggleShowNonProjectFiles;
};

#endif
