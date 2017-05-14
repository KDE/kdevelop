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

#ifndef ANDROIDRUNTIME_H
#define ANDROIDRUNTIME_H

#include <interfaces/iruntime.h>
#include <util/path.h>
#include <QAction>

class KJob;
class AndroidPreferencesSettings;

class AndroidRuntime : public KDevelop::IRuntime
{
    Q_OBJECT
public:
    AndroidRuntime();
    ~AndroidRuntime() override;

    QString name() const override { return QStringLiteral("Android"); }

    void setEnabled(bool enabled) override;

    void startProcess(KProcess *process) override;
    void startProcess(QProcess *process) override;
    KDevelop::Path pathInHost(const KDevelop::Path & runtimePath) override { return runtimePath; }
    KDevelop::Path pathInRuntime(const KDevelop::Path & localPath) override { return localPath; }
    QByteArray getenv(const QByteArray &varname) const override;

    static AndroidPreferencesSettings* s_settings;
};

#endif
