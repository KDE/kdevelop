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

#ifndef _CVSPART_H_
#define _CVSPART_H_

#include <qguardedptr.h>

#include <kurl.h>

#include "kdevversioncontrol.h"

class Context;
class QPopupMenu;
class KDialogBase;
class KAction;
class CvsWidget;
class CvsForm;
class KProcess;
class CvsPart;

/**
* Implementation for the CvsPart command line tool wrapper: it let to do all common
* used cvs operations (add, commit, remove, ...).
* TODO: Additional slots for more complex stuff as status, revert, patch creation, ...
*/
class CvsPart : public KDevVersionControl
{
	Q_OBJECT

public:
	// Standard constructor.
	CvsPart( QObject *parent, const char *name, const QStringList & );
	// Destructor.
	~CvsPart();

	/**
	* Returns the configuration widget (for properly configuring the project to
	* use CVS), child of @p parent.
	*/
	virtual QWidget *newProjectWidget( QWidget *parent );
	/**
	* Setup a directory tree for use with CVS.
	* (UNIMPLEMENTED)
	*/
	virtual void createNewProject( const QString& dir );

public slots:
	/**
	* Setup a directory tree for use with CVS.
	* (UNIMPLEMENTED)
	*/
	void slotImportCvs();

private slots:
	// Add menu items binded to cvs operations' slots to @p popup, using
	// data in @p context.
	// Not that @p context _must_ be FileContext-type, otherwise will do
	// nothing.
	void contextMenu( QPopupMenu *popup, const Context *context );

	// Cvs operations.
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

	// Adds a configuration widget (for properly configuring CVS command-line options)
	// and adds it to @p dlg.
	void projectConfigWidget( KDialogBase *dlg );
	// Called when the user wishes to stop an operation.
	void slotStopButtonClicked( KDevPlugin* );

	void receivedStdout( KProcess*, char*, int );
	void receivedStderr( KProcess*, char*, int );
	// Display "cvs diff" results in the diff part (embedded views).
	void slotDiffFinished();

private:
	// This implements commit operation: it is reused in several parts.
	QString buildCvsCommand( const QString& fileName, const QString& cmd, const QString& options ) const;

	void init();

        // Cvs operations.
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
	// Returns true if the file or directory indicated in @p url has been registered in the CVS
	// (if not, returns false since it avoid performing CVS operation)
	bool isRegisteredInRepository();

	// The value for overriding the $CVS_RSH env variable
	QString cvs_rsh() const;
	// Contains the url of the file for which the service has been invoked
	QString popupfile;
	// Reference to widget integrated in the "bottom tabbar" (IDEAL)
	QGuardedPtr<CvsWidget> m_widget;
	// This is a pointer to the d->form used for collecting data about CVS project creation (used
	// by the ApplicationWizard in example)
	QGuardedPtr<CvsForm> m_cvsConfigurationForm;

	// Shell process reference (i.e. used by 'cvs diff')
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
