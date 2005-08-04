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

#ifndef VCSFILETREEWIDGETIMPL_H
#define VCSFILETREEWIDGETIMPL_H

#include "filetreeviewwidgetimpl.h"
//Added by qt3to4:
#include <Q3PopupMenu>

class KDevVCSFileInfoProvider;
class KAction;
class KToggleAction;

/**
* @author Mario Scalas
* Provides VCS feedback to the standard features.
*/
class VCSFileTreeWidgetImpl : public FileTreeViewWidgetImpl
{
    Q_OBJECT
public:
    VCSFileTreeWidgetImpl( FileTreeWidget *parent, KDevVCSFileInfoProvider *infoProvider );
    virtual ~VCSFileTreeWidgetImpl();

    /*
    * We must avoid a reload operation while we syncing with the repository since this
    * will break our job.
    */
    virtual bool canReloadTree() const;
    /*
    * Add "sync with repository" and "show VCS fields" options to the context menu.
    */
    virtual void fillPopupMenu( Q3PopupMenu *popupMenu, Q3ListViewItem *item ) const;

private slots:
    //! show/hide the header and the additional fields (status and versions)
    void slotToggleShowVCSFields( bool checked );
    //! start a sync operation with the remote repository: this may take a while so we use
    //! the async interface provided by the KDevVCSFileInfoProvider interface
    void slotSyncWithRepository();
    //! when the requested info are ready the infoProvider object will be so kind the
    //! to give them to us in a ready-to-use format: we'll use that info to update all
    //! the items which are childs of the @see filetreeview::FileTreeViewItem stored in
    //! @p callerData.
    void vcsDirStatusReady( const VCSFileInfoMap &modifiedFiles, void *callerData );

private:
    bool showVCSFields() const;

    KToggleAction *m_actionToggleShowVCSFields;
    KAction *m_actionSyncWithRepository;
    KDevVCSFileInfoProvider *m_vcsInfoProvider;
    //! We use this guard to avoid enabling the "reload tree" action when performing
    //! syncing with remote repositories.
    bool m_isSyncingWithRepository;
    mutable KFileTreeViewItem *m_vcsStatusRequestedItem;
};

#endif
