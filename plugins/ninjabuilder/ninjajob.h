/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2017 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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
