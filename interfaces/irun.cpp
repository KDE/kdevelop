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

#include "irun.h"

#include <QStringList>

#include <limits.h>

using namespace KDevelop;

class IRun::IRunPrivate : public QSharedData
{
    public:
        KUrl executable, workingDirectory;
        QString instrumentor, environmentKey;
        QStringList arguments;
};

IRun::IRun()
    : d(new IRunPrivate)
{
}

void IRun::setExecutable(const QString & executable)
{
    d->executable = executable;
}

KUrl IRun::executable() const
{
    return d->executable;
}

void IRun::setInstrumentor(const QString & instrumentor)
{
    d->instrumentor = instrumentor;
}

QString IRun::instrumentor() const
{
    return d->instrumentor;
}

void IRun::clearArguments()
{
    d->arguments.clear();
}

void IRun::setArguments(const QStringList & arguments)
{
    d->arguments = arguments;
}

void IRun::addArgument(const QString & argument)
{
    d->arguments << argument;
}

QStringList IRun::arguments() const
{
    return d->arguments;
}

void IRun::setEnvironmentKey(const QString& environmentKey)
{
    d->environmentKey = environmentKey;
}

QString IRun::environmentKey() const
{
    return d->environmentKey;
}

IRun::IRun(const IRun & rhs)
    : d(rhs.d)
{
}

IRun & KDevelop::IRun::operator =(const IRun & rhs)
{
    d.operator=(rhs.d);
    return *this;
}

IRun::~ IRun()
{
}

IRunController::IRunController(QObject * parent)
    : QObject(parent)
{
}

KUrl KDevelop::IRun::workingDirectory() const
{
    return d->workingDirectory;
}

void KDevelop::IRun::setWorkingDirectory(const QString & workingDirectory)
{
    d->workingDirectory = workingDirectory;
}

#include "irun.moc"
