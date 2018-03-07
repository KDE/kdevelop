/*
   Copyright 2018 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#ifndef ANDROIDDOCKERRUNTIME_H
#define ANDROIDDOCKERRUNTIME_H

#include "dockerruntime.h"

class AndroidDockerRuntime : public DockerRuntime
{
    Q_OBJECT
public:
    AndroidDockerRuntime();

    QString name() const override;

    /** offer a meaningful default install prefix */
    QByteArray getenv(const QByteArray & varname) const override;
    QStringList args() const;

    QStringList extraProcessArguments(KProcess * p) const override;
    QStringList extraProcessArguments(QProcess * p) const override;
    QStringList extraDockerArguments() const override;
};

#endif // ANDROIDDOCKERRUNTIME_H
