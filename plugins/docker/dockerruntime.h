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

#ifndef DOCKERRUNTIME_H
#define DOCKERRUNTIME_H

#include <interfaces/iruntime.h>
#include <util/path.h>
#include <QAction>

class KJob;
class DockerPreferencesSettings;

class DockerRuntime : public KDevelop::IRuntime
{
    Q_OBJECT
public:
    DockerRuntime(const QString& tag);
    ~DockerRuntime() override;

    QString name() const override { return m_tag; }

    void setEnabled(bool enabled) override;

    void startProcess(KProcess *process) override;
    void startProcess(QProcess *process) override;
    KDevelop::Path pathInHost(const KDevelop::Path & runtimePath) override;
    KDevelop::Path pathInRuntime(const KDevelop::Path & localPath) override;

    static DockerPreferencesSettings* s_settings;

private:
    void inspectImage();

    const QString m_tag;
    KDevelop::Path m_upperDir;
    KDevelop::Path m_userUpperDir;
};

#endif
