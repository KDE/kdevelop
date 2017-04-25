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
#include <QtCore/QObject>
#include <QtCore/QVector>

namespace KDevelop {
class IRuntime;

/**
 * Exposes runtimes.
 *
 * @author Aleix Pol <aleixpol@kde.org>
 */
class KDEVPLATFORMINTERFACES_EXPORT IRuntimeController: public QObject
{
    Q_OBJECT
public:
    IRuntimeController();
    ~IRuntimeController() override;

    virtual void addRuntimes(const QVector<KDevelop::IRuntime*> &runtimes) = 0;

    virtual QVector<IRuntime*> availableRuntimes() const = 0;

    virtual void setCurrentRuntime(IRuntime* doc) = 0;

    virtual IRuntime* currentRuntime() const = 0;

Q_SIGNALS:
    void currentRuntimeChanged(IRuntime* currentRuntime);
};

}

#endif
