/***************************************************************************
 *   Copyright (C) 2002 by Harald Fernengel                                *
 *   harry@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _EXECCOMMAND_H_
#define _EXECCOMMAND_H_

#include <qobject.h>
#include <qstringlist.h>

class KProcess;
class KProgressDialog;

/**
 * This class invokes a binary with the arguments passed in the constructor and
 * emits the signal finished() with the output. It also displays a progress dialog
 * with the possibility to cancel the command.
 * If there was an error or the user pressed cancel, finished will emit a QString::null,
 * otherwise QStrings containing the stdout/stderr.
 * The object will delete itself after the finished signal has been emitted.
 */

class ExecCommand : public QObject
{
    Q_OBJECT
public:
    ExecCommand( const QString& executable, const QStringList& args,
		 const QString& workingDir = QString::null, QObject* parent = 0, const char* name = 0 );
    ~ExecCommand();

signals:
    void finished( const QString& output, const QString& errorOutput );

private slots:
    void receivedStdout (KProcess *, char *buffer, int buflen);
    void receivedStderr (KProcess *, char *buffer, int buflen);
    void processExited();
    void cancelClicked();

private:
    KProcess* proc;
    KProgressDialog* progressDlg;
    QString   out;
    QString   err;
};

#endif
