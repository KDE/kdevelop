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
class KURL;
class KAction;
class CvsWidget;
class CvsForm;

/**
* Implementation for a cvs command line tool wrapper: it let to do all common
* used cvs operations (add, commit, remove, ...).
* Note that all cvs operations
* TODO: Additional slots for complex stuff as revert, patch application, ...
*/
class CvsPart : public KDevVersionControl
{
	Q_OBJECT

public:
	/**
	* Standard constructor.
	*/
	CvsPart( QObject *parent, const char *name, const QStringList & );
	/**
	* Destructor.
	*/
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
	/**
	* Add menu items binded to cvs operations' slots to @p popup, using
	* data in @p context.
	* Not that @p context _must_ be FileContext-type, otherwise will do
	* nothing.
	*/
	void contextMenu( QPopupMenu *popup, const Context *context );

	/**
	* Cvs operations.
	*/
	void slotCommit();
	void slotUpdate();
	void slotAdd();
	void slotRemove();
	void slotReplace();
	void slotLog();
	void slotDiff();
	/**
	* Diss is called by slotDiff() to display "cvs diff" results in the
	* diff part, if available.
	*/
	void slotDiffFinished( const QString& diff, const QString& err );
	/**
	* Adds a configuration widget (for properly configuring CVS command-line options)
	* and adds it to @p dlg.
	*/
	void projectConfigWidget( KDialogBase *dlg );
	/**
	* Call this every time a slot for cvs operations starts!! (It will setup the
	* state (file/dir URL, ...).
	* It will also display proper error messages so the caller must only exit if
	* it fails (return false); if return true than basic requisites for cvs operation
	* are satisfied.
	*/
	bool prepareOperation();
	/**
	* Call this every time a slot for cvs operations ends!! (It will restore the state for a new
	* operation).
	*/
	void doneOperation();

private:
	/**
	* Setup actions.
	*/
	void setupActions();
	/**
	* Updates Url forn the currently focused document
	*/
	bool retrieveUrlFocusedDocument();
	/**
	* Retrieves the fileName and dirName from the pathUrl
	*/
	bool findPaths();
	/**
	* Returns true if the file or directory indicated in @p url has been registered in the CVS
	* (if not, returns false since it avoid performing CVS operation)
	*/
	bool isRegisteredInRepository();

private:
	// The value for overriding the $CVS_RSH env variable
	QString cvs_rsh() const;
	// Contains the url of the file or direcly for which the service has been invoked
	KURL pathUrl;
	// Contains the directory name
	QString dirName;
	// Contains the fileName (relative to dir)
	QString fileName;
	// Reference to widget integrated in the "bottom tabbar" (IDEAL)
	QGuardedPtr<CvsWidget> m_widget;
	// This is a pointer to the form used for collecting data about CVS project creation (used
	// by the ApplicationWizard in example)
	CvsForm *form;
	// True if invoked from menu, false otherwise (i.e. called from context menu)
	// Ok this is a very bad hack but I see no other solution for now.
	bool invokedFromMenu;


	// Actions
	KAction *actionCommit,
		*actionDiff,
		*actionLog,
		*actionAdd,
		*actionRemove,
		*actionUpdate,
		*actionReplace;
};

#endif
