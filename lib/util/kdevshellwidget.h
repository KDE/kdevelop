/***************************************************************************
 *   Copyright (C) 2006 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef KDEVSHELLWIDGET_H
#define KDEVSHELLWIDGET_H

#include <qstrlist.h>
#include <qvbox.h>
#include <qguardedptr.h>

class KProcess;
namespace KParts
{
	class ReadOnlyPart;
}

class KDevShellWidget : public QVBox
{

Q_OBJECT

public:
	KDevShellWidget(QWidget *parent = 0, const char *name = 0);
	virtual ~KDevShellWidget();

	/**
	 * Stores the shell name and arguments, that will be used in @ref activate()
	 * @param shell The shell name, for example 'irb' or '/bin/bash'
	 * @param arguments Any optional arguments
	 */
	void setShell( const QString & shell, const QStrList & arguments = QStrList() );

	/**
	 * Executes the previously set shell. If @ref setShell wasn't called before
	 * konsolepart will decide what to use.
	 */
	void activate();

	/**
	 * Should we auto launch the shell again if it was terminated?
	 * @param doAutoActivate
	 */
	void setAutoReactivateOnClose( bool doAutoActivate );

	/**
	 * Send text to the running shell
	 * @param text The text to send to the shell
	 */
	void sendInput( const QString & text );

	/**
	 * Call to check if the shell is currently running
	 * @return true if the shell is currently running
	 */
	bool isRunning();

signals:
	/**
	 * This signal will be emmitted when the started shell exits normally
	 * @param exitcode The return code of the process
	 */
	void shellExited( int exitcode );

	/**
	 * This signal will be emitted when the started shell is terminated by a signal
	 * @param signalcode The signal the process was killed with
	 */
	void shellSignalled( int signalcode );

	/**
	 *  This signal will be emitted when the process receives data
	 * @param text received data
	 */
	void receivedData( const QString & );

private slots:
	void partDestroyed();
	void processExited( KProcess * );
	void setAutoReactivateOnCloseDelayed( );

private:
	QGuardedPtr<KParts::ReadOnlyPart> m_konsolePart;
	QString m_shellName;
	QStrList m_shellArguments;
	bool m_doAutoActivate;
	bool m_isRunning;
};

#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
