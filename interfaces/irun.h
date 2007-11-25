/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

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

#ifndef IRUN_H
#define IRUN_H

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>

#include <KUrl>

#include "interfacesexport.h"
#include "iextension.h"

namespace KDevelop
{

class KDEVPLATFORMINTERFACES_EXPORT IRun
{
public:
    IRun();

    int serial() const;

    KUrl executable() const;
    void setExecutable(const QString& executable);

    QString environmentKey() const;
    void setEnvironmentKey(const QString& environmentKey);

    QStringList arguments() const;
    void addArgument(const QString& argument);
    void setArguments(const QStringList& arguments);
    void clearArguments();

    QString instrumentor() const;
    void setInstrumentor(const QString& instrumentor);

private:
    class IRunPrivate;
    QSharedDataPointer<IRunPrivate> d;
};

class KDEVPLATFORMINTERFACES_EXPORT IRunController : public QObject
{
    Q_OBJECT
    
public:
    IRunController(QObject *parent);

    virtual void run(const IRun& run) = 0;
    virtual void abort(const IRun& run) = 0;
    virtual void abortAll() = 0;

    enum State {
        Running,
        Paused,
        Finished
    };

Q_SIGNALS:
    void runStateChanged(State state);
};

class KDEVPLATFORMINTERFACES_EXPORT IRunProvider
{
public:
    virtual ~IRunProvider();

    virtual QStringList instrumentorsProvided() const = 0;

    virtual void run(const IRun& run) = 0;
    virtual void abort(const IRun& run) = 0;
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS(KDevelop, IRunProvider, "org.kdevelop.IRunProvider")
Q_DECLARE_INTERFACE(KDevelop::IRunProvider, "org.kdevelop.IRunProvider")

#endif
