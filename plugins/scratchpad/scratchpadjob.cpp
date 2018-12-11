/* This file is part of KDevelop
 *
 * Copyright 2018 Amish K. Naidu <amhndu@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "scratchpadjob.h"
#include "scratchpad.h"

#include <debug.h>

#include <outputview/outputmodel.h>
#include <util/processlinemaker.h>

#include <KProcess>
#include <KLocalizedString>

#include <QMetaEnum>

ScratchpadJob::ScratchpadJob(const QString& command, const QString& title, QObject* parent)
    : KDevelop::OutputJob(parent)
    , m_process(new KProcess(this))
    , m_lineMaker(new KDevelop::ProcessLineMaker(m_process, this))
{
    qCDebug(PLUGIN_SCRATCHPAD) << "Creating job for" << title;

    setCapabilities(Killable);

    if (!command.isEmpty()) {
        m_process->setShellCommand(command);

        setStandardToolView(KDevelop::IOutputView::RunView);
        setTitle(i18nc("prefix to distinguish scratch tabs", "scratch:%1", title));

        auto* model = new KDevelop::OutputModel(this);
        setModel(model);

        connect(m_lineMaker, &KDevelop::ProcessLineMaker::receivedStdoutLines,
                model, &KDevelop::OutputModel::appendLines);
        connect(m_lineMaker, &KDevelop::ProcessLineMaker::receivedStderrLines,
                model, &KDevelop::OutputModel::appendLines);
        m_process->setOutputChannelMode(KProcess::MergedChannels);
        connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished),
                this, &ScratchpadJob::processFinished);
        connect(m_process, &KProcess::errorOccurred, this, &ScratchpadJob::processError);
    } else {
        qCCritical(PLUGIN_SCRATCHPAD) << "Empty command in scratch job.";
        deleteLater();
    }
}

void ScratchpadJob::start()
{
    const auto program = m_process->program().join(QLatin1Char(' '));
    if (!program.trimmed().isEmpty()) {
        startOutput();
        outputModel()->appendLine(i18n("Running %1...", program));
        m_process->start();
    }
}

bool ScratchpadJob::doKill()
{
    qCDebug(PLUGIN_SCRATCHPAD) << "killing process";
    m_process->kill();
    return true;
}

void ScratchpadJob::processFinished(int exitCode, QProcess::ExitStatus)
{
    qCDebug(PLUGIN_SCRATCHPAD) << "finished process";
    m_lineMaker->flushBuffers();
    outputModel()->appendLine(i18n("Process finished with exit code %1.", exitCode));
    emitResult();
}

void ScratchpadJob::processError(QProcess::ProcessError error)
{
    qCDebug(PLUGIN_SCRATCHPAD) << "process encountered error" << error;
    outputModel()->appendLine(i18n("Failed to run scratch: %1",
                                   QLatin1String(QMetaEnum::fromType<QProcess::ProcessError>().valueToKey(error))));
    emitResult();
}

KDevelop::OutputModel* ScratchpadJob::outputModel() const
{
    return static_cast<KDevelop::OutputModel*>(model());
}
