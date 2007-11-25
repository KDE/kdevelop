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

namespace KDevelop
{

class KDEVPLATFORMINTERFACES_EXPORT IRun
{
public:
    IRun();
    IRun(const IRun& rhs);
    IRun& operator=(const IRun& rhs);
    ~IRun();

    int serial() const;

    KUrl executable() const;
    void setExecutable(const QString& executable);

    QString environmentKey() const;
    void setEnvironmentKey(const QString& environmentKey);

    QStringList arguments() const;
    void addArgument(const QString& argument);
    void setArguments(const QStringList& arguments);
    void clearArguments();

    /**
     * The requested instrumentor, usually one of 'default', 'gdb', 'memcheck' etc.
     */
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

    /**
     * Request for the provided \a run object to be executed.
     */
    virtual bool run(const IRun& run) = 0;
    virtual void abort(const IRun& run) = 0;
    virtual void abortAll() = 0;

    enum State {
        Idle,
        Running,
        Paused
    };

Q_SIGNALS:
    void runStateChanged(State state);
};

}

#endif
