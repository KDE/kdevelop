/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SVNPART_H_
#define _SVNPART_H_

#include <qguardedptr.h>

#include <kurl.h>

#include "kdevversioncontrol.h"

class Context;
class QPopupMenu;
class KDialogBase;
class KAction;
class SvnWidget;
class SvnForm;
class KProcess;
class SvnPart;

/**
* Implementation for the SvnPart command line tool wrapper: it let to do all common
* used svn operations (add, commit, remove, ...).
* TODO: Additional slots for more complex stuff as status, revert, patch creation, ...
*/
class SvnPart : public KDevVersionControl
{
	Q_OBJECT

public:
	// Standard constructor.
	SvnPart( QObject *parent, const char *name, const QStringList & );
	// Destructor.
	~SvnPart();

	/**
	* Returns the configuration widget (for properly configuring the project to
	* use SVN), child of @p parent.
	*/
	virtual QWidget *newProjectWidget( QWidget *parent );
	/**
	* Setup a directory tree for use with SVN.
	* (UNIMPLEMENTED)
	*/
	virtual void createNewProject( const QString& dir );

public slots:
	/**
	* Setup a directory tree for use with SVN.
	* (UNIMPLEMENTED)
	*/
	void slotImportSvn();

private slots:
	// Add menu items binded to svn operations' slots to @p popup, using
	// data in @p context.
	// Not that @p context _must_ be FileContext-type, otherwise will do
	// nothing.
	void contextMenu( QPopupMenu *popup, const Context *context );

	// Svn operations.
	void slotCommit();
	void slotUpdate();
	void slotAdd();
	void slotRemove();
	void slotRevert();
	void slotLog();
	void slotDiff();

        void slotActionCommit();
        void slotActionUpdate();
        void slotActionAdd();
        void slotActionRemove();
        void slotActionRevert();
        void slotActionLog();
        void slotActionDiff();

	void slotProjectOpened();
	void slotProjectClosed();

	// Adds a configuration widget (for properly configuring SVN command-line options)
	// and adds it to @p dlg.
	void projectConfigWidget( KDialogBase *dlg );
	// Called when the user wishes to stop an operation.
	void slotStopButtonClicked( KDevPlugin* );

	void receivedStdout( KProcess*, char*, int );
	void receivedStderr( KProcess*, char*, int );
	// Display "svn diff" results in the diff part (embedded views).
	void slotDiffFinished();

private:
	// This implements commit operation: it is reused in several parts.
	QString buildSvnCommand( const QString& fileName, const QString& cmd, const QString& options ) const;

	void init();

        // Svn operations.
        void commit( const QString& fileName );
        void update(  const QString& fileName );
        void add( const QString& fileName );
        void remove( const QString& fileName );
        void revert( const QString& fileName );
        void log( const QString& fileName );
        void diff( const QString& fileName );

	// Setup actions.
	void setupActions();
	// Updates Url forn the currently focused document
	QString currentDocument();
	// Retrieves the fileName and dirName from the pathUrl
	bool findPaths();
	// Returns true if the file or directory indicated in @p url has been registered in the SVN
	// (if not, returns false since it avoid performing SVN operation)
	bool isRegisteredInRepository();

	// The value for overriding the $SVN_RSH env variable
	QString svn_rsh() const;
	// Contains the url of the file for which the service has been invoked
	QString popupfile;
	// Reference to widget integrated in the "bottom tabbar" (IDEAL)
	QGuardedPtr<SvnWidget> m_widget;
	// This is a pointer to the d->form used for collecting data about SVN project creation (used
	// by the ApplicationWizard in example)
	QGuardedPtr<SvnForm> m_svnConfigurationForm;

	// Shell process reference (i.e. used by 'svn diff')
	KProcess* proc;
	// Buffers for process' standard output and error
	QString stdOut, stdErr;

	// Actions
	KAction *actionCommit,
		*actionDiff,
		*actionLog,
		*actionAdd,
		*actionRemove,
		*actionUpdate,
		*actionRevert;
};

#endif
