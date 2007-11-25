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

static int s_runSerial = 0;

class IRun::IRunPrivate : public QSharedData
{
    public:
        int serial;
        QString executable, instrumentor, environmentKey;
        QStringList arguments;
};

IRun::IRun()
{
    d->serial = s_runSerial++;
    if (s_runSerial == INT_MAX)
        s_runSerial = 0;
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

int KDevelop::IRun::serial() const
{
    return d->serial;
}

KDevelop::IRunProvider::~ IRunProvider()
{
}

#include "irun.moc"
