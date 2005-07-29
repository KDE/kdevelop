/* This file is part of the KDE project
   Copyright (C) 2002 Harald Fernengel <harry@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _EXECCOMMAND_H_
#define _EXECCOMMAND_H_

#include <qobject.h>
#include <qstringlist.h>

class KProcess;
class KProgressDialog;

/**
@file execcommand.h
Command execution facilities.
*/

/**
 * This class invokes a binary with the arguments passed in the constructor and
 * emits the signal finished() with the output. It also displays
 * a progress dialog with the possibility to cancel the command.
 *
 * If there was an error or the user pressed cancel, finished ()
 * will emit a QString::null, otherwise QStrings containing the stdout/stderr.
 *
 * The object will delete itself after the finished signal has been emitted.
 * Additional environment can be set in the QStringList env via QStrings with the format "foo=blah"
 */
class ExecCommand : public QObject
{
    Q_OBJECT
public:
    ExecCommand( const QString& executable, const QStringList& args,
		 const QString& workingDir = QString::null,
                 const QStringList& env = QStringList(), QObject* parent = 0, const char* name = 0 );
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
