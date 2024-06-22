/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "commandexecutor.h"

#include "processlinemaker.h"

#include <KProcess>
#include <KShell>

#include <QMap>
#include <QStringList>
#include <QString>

namespace KDevelop {

class CommandExecutorPrivate
{
public:
    explicit CommandExecutorPrivate(CommandExecutor* cmd)
        : m_exec(cmd)
        , m_useShell(false)
    {
    }
    CommandExecutor* m_exec;
    KProcess* m_process;
    ProcessLineMaker* m_lineMaker;
    QString m_command;
    QStringList m_args;
    QString m_workDir;
    QMap<QString, QString> m_env;
    bool m_useShell;
    void procError(QProcess::ProcessError error)
    {
        Q_UNUSED(error)
        m_lineMaker->flushBuffers();
        emit m_exec->failed(error);
    }
    void procFinished(int code, QProcess::ExitStatus status)
    {
        m_lineMaker->flushBuffers();
        if (status == QProcess::NormalExit)
            emit m_exec->completed(code);
    }
};

CommandExecutor::CommandExecutor(const QString& command, QObject* parent)
    : QObject(parent)
    , d_ptr(new CommandExecutorPrivate(this))
{
    Q_D(CommandExecutor);

    d->m_process = new KProcess(this);
    d->m_process->setOutputChannelMode(KProcess::SeparateChannels);
    d->m_lineMaker = new ProcessLineMaker(d->m_process);
    d->m_command = command;
    connect(d->m_lineMaker, &ProcessLineMaker::receivedStdoutLines,
            this, &CommandExecutor::receivedStandardOutput);
    connect(d->m_lineMaker, &ProcessLineMaker::receivedStderrLines,
            this, &CommandExecutor::receivedStandardError);
    connect(d->m_process, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
            Q_D(CommandExecutor);
            d->procError(error);
        });
    connect(d->m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus status) {
            Q_D(CommandExecutor);
            d->procFinished(code, status);
        });
}

CommandExecutor::~CommandExecutor()
{
    Q_D(CommandExecutor);

    delete d->m_process;
    delete d->m_lineMaker;
}

void CommandExecutor::setEnvironment(const QMap<QString, QString>& env)
{
    Q_D(CommandExecutor);

    d->m_env = env;
}

void CommandExecutor::setEnvironment(const QStringList& env)
{
    Q_D(CommandExecutor);

    QMap<QString, QString> envmap;
    for (const QString& var : env) {
        int sep = var.indexOf(QLatin1Char('='));
        envmap.insert(var.left(sep), var.mid(sep + 1));
    }

    d->m_env = envmap;
}

void CommandExecutor::setArguments(const QStringList& args)
{
    Q_D(CommandExecutor);

    d->m_args = args;
}

void CommandExecutor::setWorkingDirectory(const QString& dir)
{
    Q_D(CommandExecutor);

    d->m_workDir = dir;
}

bool CommandExecutor::useShell() const
{
    Q_D(const CommandExecutor);

    return d->m_useShell;
}

void CommandExecutor::setUseShell(bool shell)
{
    Q_D(CommandExecutor);

    d->m_useShell = shell;
}

void CommandExecutor::start()
{
    Q_D(CommandExecutor);

    for (auto it = d->m_env.constBegin(), itEnd = d->m_env.constEnd(); it != itEnd; ++it) {
        d->m_process->setEnv(it.key(), it.value());
    }

    d->m_process->setWorkingDirectory(d->m_workDir);
    if (!d->m_useShell) {
        d->m_process->setProgram(d->m_command, d->m_args);
    } else {
        QStringList arguments;
        arguments.reserve(d->m_args.size());
        for (const QString& a : std::as_const(d->m_args)) {
            arguments << KShell::quoteArg(a);
        }

        d->m_process->setShellCommand(d->m_command + QLatin1Char(' ') + arguments.join(QLatin1Char(' ')));
    }

    d->m_process->start();
}

void CommandExecutor::setCommand(const QString& command)
{
    Q_D(CommandExecutor);

    d->m_command = command;
}

void CommandExecutor::kill()
{
    Q_D(CommandExecutor);

    d->m_process->kill();
}

QString CommandExecutor::command() const
{
    Q_D(const CommandExecutor);

    return d->m_command;
}

QStringList CommandExecutor::arguments() const
{
    Q_D(const CommandExecutor);

    return d->m_args;
}

QString CommandExecutor::workingDirectory() const
{
    Q_D(const CommandExecutor);

    return d->m_workDir;
}

}

#include "moc_commandexecutor.cpp"
