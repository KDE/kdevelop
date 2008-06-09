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

#ifndef IRUNCONTROLLER_H
#define IRUNCONTROLLER_H

#include <QtCore/QObject>

#include <kjobtrackerinterface.h>

#include "interfacesexport.h"

namespace KDevelop
{

class IRun;

/**
 * The main controller for running processes.
 */
class KDEVPLATFORMINTERFACES_EXPORT IRunController : public KJobTrackerInterface
{
    Q_OBJECT

public:
    ///Constructor.
    IRunController(QObject *parent);

    /**
     * Request the provided \a run object to be executed.
     *
     * \return the serial number for the run job, or -1 if \a run could not be executed.
     */
    virtual KJob* execute(const IRun& run) = 0;

    /**
     * Provide the default run object.
     */
    virtual IRun defaultRun() const = 0;

    /**
     * Interrogate the current managed jobs
     */
    virtual QList<KJob*> currentJobs() const = 0;

    /**
     * An enumeration of the possible states for the run controller.
     */
    enum State {
        Idle     /**< No processes are currently running */,
        Running  /**< processes are currently running */
    };

public Q_SLOTS:
    /**
     * Request for all running processes to be killed.
     */
    virtual void stopAllProcesses() = 0;

Q_SIGNALS:
    /**
     * Notify that the state of the run controller has changed to \a {state}.
     */
    void runStateChanged(State state);

    /**
     * Notify that a new job has been registered.
     */
    void jobRegistered(KJob* job);

    /**
     * Notify that a job has been unregistered.
     */
    void jobUnregistered(KJob* job);
};

}

#endif // IRUNCONTROLLER_H
