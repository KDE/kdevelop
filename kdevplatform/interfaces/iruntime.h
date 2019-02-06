/*  This file is part of KDevelop
    Copyright 2017 Aleix Pol <aleixpol@kde.org>

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

#ifndef KDEVPLATFORM_IRUNTIME_H
#define KDEVPLATFORM_IRUNTIME_H

#include "interfacesexport.h"
#include <QObject>
#include <QString>

class QProcess;
class KProcess;

namespace KDevelop
{
class Path;

/**
 * A runtime represents an environment we develop against
 *
 * It allows the IDE to interact with systems that differ from process where
 * the process we are running in.
 *
 * It allows to execute processes into them and translate the paths these runtimes
 * offer into ones that will be visible to our process so we can introspect the
 * platform we are developing for as well.
 */
class KDEVPLATFORMINTERFACES_EXPORT IRuntime : public QObject
{
    Q_OBJECT
public:
    ~IRuntime() override;

    /** @returns a display string that identifies the runtime */
    virtual QString name() const = 0;

    /**
     * Adapts the @p process and starts it within the environment
     *
     * Gives an opportunity to the runtime to set up environment variables
     * or process the execution in any way necessary.
     */
    virtual void startProcess(QProcess* process) const = 0;

    /**
     * @see startProcess(QProcess*)
     */
    virtual void startProcess(KProcess* process) const = 0;

    /**
     * Given a @p localPath from our process's file system
     * @returns the path that the runtime's environment can use
     */
    virtual Path pathInRuntime(const Path& localPath) const = 0;

    /**
     * Given a @p runtimePath from the runtime
     * @returns the path in our file system scope that maps to the runtime's
     */
    virtual Path pathInHost(const Path& runtimePath) const = 0;

    /**
     * @returns the value for an environment variable in the runtime
     */
    virtual QByteArray getenv(const QByteArray& varname) const = 0;

    /**
     * @returns a path for binary directories or empty if there's no such thing.
     *
     * Some runtime systems won't be benefiting much from having custom build directories.
     * For such systems, the project manager will offer to initialise the build directories
     * there instead of asking where to create them.
     */
    virtual Path buildPath() const = 0;

protected:
    friend class RuntimeController;

    /**
     * notifies the runtime about its availability.
     *
     * This will be called exclusively from the IRuntimeController implementation.
     *
     * @see IRuntimeController::setCurrentRuntime
     */
    virtual void setEnabled(bool enabled) = 0;

};

}

#endif

