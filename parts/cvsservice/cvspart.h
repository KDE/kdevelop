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

#include <qguardedptr.h>
#include <qcstring.h>

#include "kdevversioncontrol.h"

class Context;
class QPopupMenu;
class KDialogBase;
class KURL;
class KURL::List;
class KAction;

class CvsProcessWidget;
class CvsForm;
class CheckoutDialog;

class CvsService_stub;
class Repository_stub;
class CvsPartImpl;

/**
* Implementation for the CvsPart command line tool wrapper: it let to do all common
* used cvs operations (add, commit, remove, ...).
* @todo Additional slots for more complex stuff as status, revert, patch creation, ...
*/
class CvsPart : public KDevVersionControl
{
    Q_OBJECT

    friend class CvsPartImpl;

public:
    // Standard constructor.
    CvsPart( QObject *parent, const char *name, const QStringList & );
    // Destructor.
    virtual ~CvsPart();

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
    virtual void fetchFromRepository();

private slots:
    // Add menu items binded to cvs operations' slots to @p popup, using
    // data in @p context.
    // Not that @p context _must_ be FileContext-type, otherwise will do
    // nothing.
    void contextMenu( QPopupMenu *popup, const Context *context );

    // Cvs operations (menubar)
    void slotActionLogin();
    void slotActionLogout();

    void slotActionCommit();
    void slotActionUpdate();
    void slotActionAdd();
    void slotActionAddBinary();
    void slotActionRemove();
    void slotActionRevert();
    void slotActionLog();
    void slotActionDiff();
    void slotActionTag();
    void slotActionUnTag();
    void slotActionAddToIgnoreList();
    void slotActionRemoveFromIgnoreList();

    // Cvs operations (context menu)
    void slotCommit();
    void slotUpdate();
    void slotAdd();
    void slotAddBinary();
    void slotRemove();
    void slotRevert();
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

    // Adds a configuration widget (for properly configuring CVS command-line options)
    // and adds it to @p dlg.
    void projectConfigWidget( KDialogBase *dlg );

    // Called when the user wishes to stop an operation.
    void slotStopButtonClicked( KDevPlugin* );

private:
    void init();
    void setupActions();
    // Returns the KURL for the currently focused document, if there is any
    bool urlFocusedDocument( KURL &url );

    // A list of KURLs of the files to be "operated" on (to be committed, added, removed, ...)
    KURL::List m_urls;

    // This is a pointer to the d->form used for collecting data about CVS project creation (used
    // by the ApplicationWizard in example)
    QGuardedPtr<CvsForm> m_cvsConfigurationForm;

    // Actions
    KAction *actionCommit,
        *actionDiff,
        *actionLog,
        *actionAdd,
        *actionAddBinary,
        *actionRemove,
        *actionUpdate,
        *actionRevert,
        *actionAddToIgnoreList,
        *actionRemoveFromIgnoreList,
        *actionTag,
        *actionUnTag,
        *actionLogin,
        *actionLogout;

    CvsPartImpl *m_impl;
};

#endif
