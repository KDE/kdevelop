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

#include "kdevversioncontrol.h"

class Context;
class QPopupMenu;
class KDialogBase;
class KURL;
class KAction;
class CvsWidget;
class CvsForm;
class KProcess;

/**
* Implementation for a cvs command line tool wrapper: it let to do all common
* used cvs operations (add, commit, remove, ...).
* TODO: Additional slots for more complex stuff as status, revert, patch creation, ...
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
	* Adds a configuration widget (for properly configuring CVS command-line options)
	* and adds it to @p dlg.
	*/
	void projectConfigWidget( KDialogBase *dlg );
	/**
	 * Called when the user wishes to stop an operation.
	 */
	void slotStopButtonClicked( KDevPlugin* );

	void processExited();
	void receivedStdout( KProcess*, char*, int );
	void receivedStderr( KProcess*, char*, int );

private:
	struct Private;
	Private *d;
};

#endif
