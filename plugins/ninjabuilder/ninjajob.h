/* This file is part of KDevelop
    Copyright 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright 2017 Kevin Funk <kfunk@kde.org>

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

#ifndef NINJAJOB_H
#define NINJAJOB_H

#include <outputview/outputexecutejob.h>

#include <QPointer>

namespace KDevelop {
class ProjectBaseItem;
}

class NinjaBuilder;

class QUrl;

class NinjaJob
    : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    enum CommandType {
        BuildCommand,
        CleanCommand,
        CustomTargetCommand,
        InstallCommand
    };

    enum ErrorTypes {
        Correct = 0,
        Failed
    };

public:
    NinjaJob(KDevelop::ProjectBaseItem* item, CommandType commandType, const QStringList& arguments,
             const QByteArray& signal, NinjaBuilder* parent);
    ~NinjaJob() override;

    void setIsInstalling(bool isInstalling);
    static QString ninjaExecutable();

    KDevelop::ProjectBaseItem* item() const;
    CommandType commandType() const;
    QUrl workingDirectory() const override;
    QStringList privilegedExecutionCommand() const override;

protected Q_SLOTS:
    void postProcessStdout(const QStringList& lines) override;
    void postProcessStderr(const QStringList& lines) override;

private Q_SLOTS:
    void emitProjectBuilderSignal(KJob* job);

private:
    bool m_isInstalling;
    QPersistentModelIndex m_idx;
    CommandType m_commandType;
    QByteArray m_signal;
    QPointer<NinjaBuilder> m_plugin;

    void appendLines(const QStringList& lines);
};

#endif  // NINJAJOB_H
