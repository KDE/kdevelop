/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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
     * If @p runtimes has no parent, the controller will become its parent
     * and will remove it when needed.
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
