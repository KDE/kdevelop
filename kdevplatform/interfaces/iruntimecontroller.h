/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_IRUNTIMECONTROLLER_H
#define KDEVPLATFORM_IRUNTIMECONTROLLER_H

#include <interfaces/idocumentation.h>
#include <QObject>
#include <QVector>

namespace KDevelop {
class IRuntime;

/**
 * @brief Exposes runtimes
 *
 * Makes it possible to have the IDE develop for different platforms other
 * than the local host by having available different runtimes that will give
 * access to the target systems.
 *
 * Allows to add runtimes, list them and offer a currentRuntime.
 * The currentRuntime will be the runtime towards which all the runtime and build
 * information * gathered by the IDE will be fetched from.
 *
 * @see IRuntime
 */
class KDEVPLATFORMINTERFACES_EXPORT IRuntimeController: public QObject
{
    Q_OBJECT
public:
    IRuntimeController();
    ~IRuntimeController() override;

    /**
     * Makes @p runtimes available to be used.
     */
    virtual void addRuntimes(KDevelop::IRuntime* runtimes) = 0;

    /**
     * Lists available runtimes
     */
    virtual QVector<IRuntime*> availableRuntimes() const = 0;

    /**
     * Sets @p runtime as the currently used runtime and emits currentRuntimeChanged
     * so the IDE can adapt, if necessary.
     */
    virtual void setCurrentRuntime(IRuntime* runtime) = 0;

    /**
     * @returns the current runtime
     */
    virtual IRuntime* currentRuntime() const = 0;

Q_SIGNALS:
    void currentRuntimeChanged(IRuntime* currentRuntime);
};

}

#endif
