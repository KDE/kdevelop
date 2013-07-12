/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_PLUGIN_NATIVEAPPJOB_H
#define KDEVPLATFORM_PLUGIN_NATIVEAPPJOB_H

#include <QtCore/QProcess>
#include <outputview/outputjob.h>

namespace KDevelop
{
class ILaunchConfiguration;
class ProcessLineMaker;
class OutputModel;
}

class KProcess;

class NativeAppJob : public KDevelop::OutputJob
{
Q_OBJECT
public:
    NativeAppJob( QObject* parent, KDevelop::ILaunchConfiguration* cfg );
    virtual void start();
    virtual bool doKill();
    KDevelop::OutputModel* model();
private slots:
    void processError(QProcess::ProcessError);
    void processFinished(int,QProcess::ExitStatus);
private:
    void appendLine(const QString &l);
    KProcess* proc;
    KDevelop::ProcessLineMaker* lineMaker;
};

#endif
