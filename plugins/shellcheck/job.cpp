/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "job.h"
#include "parameters.h"
#include "outputparser.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>
#include <shell/problem.h>

#include "shellcheckdebug.h"

#include <KLocalizedString>


namespace shellcheck 
{

Job::Job(const Parameters& params, QObject* parent)
    : KDevelop::OutputExecuteJob(parent)
    , m_parser(new OutputParser)
{
    setJobName(i18n("ShellCheck checking (%1)", params.shellFileToCheck));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll);

    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput);

    *this << params.commandLine();
    qCDebug(PLUGIN_SHELLCHECK) << "checking path" << params.shellFileToCheck;
    qWarning(PLUGIN_SHELLCHECK) << "JOb is called with thise parameters " << params.commandLine();
}

Job::~Job()
{
    doKill();
}

void Job::postProcessStdout(const QStringList& lines)
{
    //qWarning(PLUGIN_SHELLCHECK) << "Job gave us these lines " << lines;
    m_parser->add(lines);

    KDevelop::OutputExecuteJob::postProcessStdout(lines);
}

void Job::postProcessStderr(const QStringList& lines)
{
    qWarning(PLUGIN_SHELLCHECK) << "Job gave us these (stderr) lines " << lines;
    KDevelop::OutputExecuteJob::postProcessStderr(lines);
}

void Job::start()
{
    m_parser->clear();
    
    qCDebug(PLUGIN_SHELLCHECK) << "executing:" << commandLine().join(QLatin1Char(' '));
    KDevelop::OutputExecuteJob::start();
}

void Job::childProcessError(QProcess::ProcessError e)
{
    QString messageText;

    switch (e) {
    case QProcess::FailedToStart:
        messageText = i18n("Failed to start shellcheck from \"%1\".", commandLine()[0]);
        break;

    case QProcess::Crashed:
        if (status() != KDevelop::OutputExecuteJob::JobStatus::JobCanceled) {
            messageText = i18n("Shellcheck crashed.");
        }
        break;

    case QProcess::Timedout:
        messageText = i18n("Shellcheck process timed out.");
        break;

    case QProcess::WriteError:
        messageText = i18n("Write to shellcheck process failed.");
        break;

    case QProcess::ReadError:
        messageText = i18n("Read from shellcheck process failed.");
        break;

    case QProcess::UnknownError:
        // current shellcheck errors will be displayed in the output view
        // don't notify the user
        break;
    }

     if (!messageText.isEmpty()) {
         auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
         KDevelop::ICore::self()->uiController()->postMessage(message);
     }

    KDevelop::OutputExecuteJob::childProcessError(e);
}

void Job::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus) 
{
    qCDebug(PLUGIN_SHELLCHECK) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;
    
    // Parse all the gathered output here. 
    m_problems = m_parser->parse();
    if(!m_problems.isEmpty()) {
        qCWarning(PLUGIN_SHELLCHECK) << "!!!!Process Finished, Num Problems detected: " << m_problems.size();
        emit problemsDetected(m_problems);
    } else {
        qCWarning(PLUGIN_SHELLCHECK) << "!!!!Process Finished - No Problems??";
    }
    
    // ShellCheck uses exit codes 0 to 4
    // 0: All files successfully scanned with no issues.
    // 1: All files successfully scanned with some issues.
    // The latter is in our book a success, so in case we get a 1, we transform it to 0, so we get the right output view raised.
    // Higher exitcodes indicate actual errors, so we keep those as is.
    if(exitCode == 1)
    {
        exitCode = 0;
    }
    KDevelop::OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}

}
