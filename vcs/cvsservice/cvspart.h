/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CVSPART_H_
#define _CVSPART_H_

#include <qpointer.h>
#include <q3cstring.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include "kdevversioncontrol.h"

class Context;
class Q3PopupMenu;
class QDir;
class KDialogBase;
class KURL;
class KURL::List;
class KAction;

class CvsProcessWidget;
class CvsForm;
class CheckoutDialog;

class CvsService_stub;
class Repository_stub;
class CvsServicePartImpl;

class CvsServicePart : public KDevVersionControl
{
    Q_OBJECT

    friend class CvsServicePartImpl;

public:
    //! Standard constructor.
    CvsServicePart( QObject *parent, const char *name, const QStringList & );
    //! Destructor.
    virtual ~CvsServicePart();

    /**
    * Returns the configuration widget (for properly configuring the project to
    * use CVS), child of @p parent.
    */
    virtual QWidget *newProjectWidget( QWidget *parent );
    /**
    * Setup a directory tree for use with CVS.
    */
    virtual void createNewProject( const QString& dir );
    /**
    * Fetch a module from remote repository, so it can be used for importing
    */
    virtual bool fetchFromRepository();
    /**
    * @return the info provider for VCS sandboxes
    */
    virtual KDevVCSFileInfoProvider *fileInfoProvider() const;
    /**
    * @param dirPath absolute path of the directory
    * @return true if the the directory is a valid CVS sandbox
    */
    virtual bool isValidDirectory( const QString &dirPath ) const;

private slots:
    /** Add menu items binded to cvs operations' slots to @p popup, using
     * data in @p context.
     * Not that @p context _must_ be FileContext-type, otherwise will do
     * nothing.
     */
    void contextMenu( Q3PopupMenu *popup, const Context *context );

    // Cvs operations (menubar)
    void slotActionLogin();
    void slotActionLogout();

    void slotActionCommit();
    void slotActionUpdate();
    void slotActionEditors();
    void slotActionEdit();
    void slotActionUnEdit();
    void slotActionAdd();
    void slotActionAnnotate();
    void slotActionAddBinary();
    void slotActionRemove();
    void slotActionRemoveSticky();
    void slotActionLog();
    void slotActionDiff();
    void slotActionTag();
    void slotActionUnTag();
    void slotActionAddToIgnoreList();
    void slotActionRemoveFromIgnoreList();

    // Cvs operations (context menu)
    void slotCommit();
    void slotUpdate();
    void slotEditors();
    void slotEdit();
    void slotUnEdit();
    void slotAdd();
    void slotAnnotate();
    void slotAddBinary();
    void slotRemove();
    void slotRemoveSticky();
    void slotLog();
    void slotDiff();
    void slotTag();
    void slotUnTag();
    void slotAddToIgnoreList();
    void slotRemoveFromIgnoreList();

    void slotProjectOpened();
    void slotProjectClosed();

    void slotAddFilesToProject(const QStringList &);
    void slotRemovedFilesFromProject(const QStringList &);

    /** Adds a configuration widget (for properly configuring CVS command-line options)
     * and adds it to @p dlg.
     */
    void projectConfigWidget( KDialogBase *dlg );

    //! Called when the user wishes to stop an operation.
    void slotStopButtonClicked( KDevPlugin* );

private slots:
    void init();

private:
    void setupActions();
    //! Returns the KURL for the currently focused document, if there is any
    bool urlFocusedDocument( KURL &url );

    //! A list of KURLs of the files to be "operated" on (to be committed, added, removed, ...)
    KURL::List m_urls;

    /** This is a pointer to the d->form used for collecting data about CVS project creation (used
     * by the ApplicationWizard in example)
     */
    QPointer<CvsForm> m_cvsConfigurationForm;

    // Actions
    KAction *actionCommit,
        *actionDiff,
        *actionLog,
        *actionAnnotate,
        *actionAdd,
        *actionAddBinary,
        *actionRemove,
        *actionUpdate,
        *actionRemoveSticky,
        *actionEdit,
        *actionEditors,
        *actionUnEdit,
        *actionAddToIgnoreList,
        *actionRemoveFromIgnoreList,
        *actionTag,
        *actionUnTag,
        *actionLogin,
        *actionLogout;

    CvsServicePartImpl *m_impl;
};

#endif
