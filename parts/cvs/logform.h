/***************************************************************************
 *   Copyright (C) 200?-2003 by KDevelop Authors                           *
 *   www.kdevelop.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOGFORM_H
#define LOGFORM_H

class QStringList;
class QProcess;

#include <logformbase.h>

/**
Implementation for the form displaying 'cvs log' output.

@author KDevelop Authors
*/
class LogForm : public LogFormBase
{
	Q_OBJECT
public:
    LogForm( QWidget *parent=0, const char *name=0, int flags=0 );
    ~LogForm();

	void start( const QString &workDir, const QStringList &pathsList );

public slots:
	void slotProcessExited();
	void slotReadStdout();
	void slotReadStderr();
	void slotLinkClicked( const QString & link );

private:
	void setText( const QString& text );

private:
	QProcess *process;
	QString pathName;
};

#endif
