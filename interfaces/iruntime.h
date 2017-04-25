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
 * A runtime represents an environment that will be targetted
 *
 * It allows the IDE to interact with virtual systems that live in different
 * namespaces.
 *
 * It allows to execute processes in them and translate the paths these runtimes
 * offer into ones that will be visible to our process to interact with.
 */
class KDEVPLATFORMINTERFACES_EXPORT IRuntime : public QObject
{
    Q_OBJECT
public:
    ~IRuntime() override;

    /** @returns a display string that identifies the runtime */
    virtual QString name() const = 0;

    virtual void startProcess(QProcess* process) = 0;
    virtual void startProcess(KProcess* process) = 0;

    virtual Path pathInRuntime(const Path& localPath) = 0;
    virtual Path pathInHost(const Path& runtimePath) = 0;

    virtual void setEnabled(bool enabled) = 0;
};

}

#endif

