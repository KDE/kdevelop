/* This file is part of KDevelop
Copyright 2007-2008 Hamish Rodda <rodda@kde.org>

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

#ifndef RUNCONTROLLER_H
#define RUNCONTROLLER_H

#include <QPointer>

#include "irun.h"
#include "irunprovider.h"
#include "iruncontroller.h"
#include "outputjob.h"

namespace KDevelop
{

class IPlugin;
class IProject;

class RunController : public IRunController
{
    Q_OBJECT

public:
    RunController(QObject *parent);
    ~RunController();

    virtual void registerJob(KJob *job);
    virtual void unregisterJob(KJob *job);
    virtual QList<KJob*> currentJobs() const;

    virtual KJob* execute(const IRun& run);
    virtual IRun defaultRun() const;

    IRunProvider* findProvider(const QString& instrumentor);

    QItemDelegate* delegate() const;

public Q_SLOTS:
    virtual void stopAllProcesses();

protected Q_SLOTS:
    virtual void finished(KJob *job);
    virtual void suspended(KJob *job);
    virtual void resumed(KJob *job);

private Q_SLOTS:
    void slotExecute();
    void slotProjectOpened(KDevelop::IProject* project);
    void slotProjectClosing(KDevelop::IProject* project);
    void slotKillJob();

private:
    void setupActions();
    void checkState();

    class RunControllerPrivate;
    RunControllerPrivate* const d;
};

class RunJob : public OutputJob
{
    Q_OBJECT

public:
    RunJob(RunController* controller, const IRun& run);

    virtual void start();

    enum ErrorTypes {
        ErrorNoProvider = UserDefinedError,
        ErrorInvalidTarget
    };

protected:
    virtual bool doKill();

private Q_SLOTS:
    void slotOutput(KJob* job, const QString& line, KDevelop::IRunProvider::OutputTypes type);
    void slotFinished(KJob* job);

private:
    RunController* m_controller;
    IRunProvider* m_provider;
    IRun m_run;
};

}

#endif
