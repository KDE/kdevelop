/* This file is part of KDevelop
   Copyright 2011 Mathieu Lornac <mathieu.lornac@gmail.com>
   Copyright 2011 Damien Coppel <damien.coppel@gmail.com>
   Copyright 2011 Lionel Duc <lionel.data@gmail.com>
   Copyright 2011 Sebastien Rannou <mxs@sbrk.org>
   Copyright 2011 Lucas Sarie <lucas.sarie@gmail.com>
   Copyright 2006-2008 Hamish Rodda <rodda@kde.org>
   Copyright 2002 Harald Fernengel <harry@kdevelop.org>
   Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
   Copyright 2015 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <KShell>
#include <KMessageBox>
#include <QApplication>
#include <KLocalizedString>
#include <QRegularExpression>

#include "cppcheckparser.h"
#include "debug.h"
#include "job.h"

namespace cppcheck
{

Job::Job(const Parameters &params, QObject* parent)
    : KDevelop::OutputExecuteJob(parent)
{
    setJobName(i18n("Cppcheck output"));

    setCapabilities(KJob::Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AutoScroll);

    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStdout);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::DisplayStderr);
    setProperties(KDevelop::OutputExecuteJob::JobProperty::PostProcessOutput);

    *this << params.executable;

    *this << "--force";
    *this << "--xml-version=2";

    if (!params.parameters.isEmpty())
        *this << KShell::splitArgs(params.parameters);

    if (params.checkStyle)
        *this << "--enable=style";

    if (params.checkPerformance)
        *this << "--enable=performance";

    if (params.checkPortability)
        *this << "--enable=portability";

    if (params.checkInformation)
        *this << "--enable=information";

    if (params.checkUnusedFunction)
        *this << "--enable=unusedFunction";

    if (params.checkMissingInclude)
        *this << "--enable=missingInclude";

    *this << params.path;
    qCDebug(KDEV_CPPCHECK) << "checking path" << params.path;
}

Job::~Job()
{
    doKill();
}

void Job::processStdoutLines(const QStringList& lines)
{
    m_standardOutput << lines;
}

void Job::processStderrLines(const QStringList& lines)
{
    static const auto xmlStartRegex = QRegularExpression("\\s*<");

    for (const QString & line : lines) {
        // unfortunately sometime cppcheck send non-XML messages to stderr.
        // For example, if we pass '-I /missing_include_dir' to the argument list,
        // then stderr output will contains such line (tested on cppcheck 1.72):
        //
        // (information) Couldn't find path given by -I '/missing_include_dir'
        //
        // Therefore we must 'move' such messages to m_standardOutput.

        if (line.indexOf(xmlStartRegex) != -1) // the line contains XML
            m_xmlOutput << line;
        else
            m_standardOutput << line;
    }
}

void Job::postProcessStdout(const QStringList& lines)
{
    processStdoutLines(lines);

    KDevelop::OutputExecuteJob::postProcessStdout(lines);
}

void Job::postProcessStderr(const QStringList& lines)
{
    processStderrLines(lines);

    KDevelop::OutputExecuteJob::postProcessStderr(lines);
}

void Job::start()
{
    m_standardOutput.clear();
    m_xmlOutput.clear();

    qCDebug(KDEV_CPPCHECK) << "executing:" << commandLine().join(' ');

    KDevelop::OutputExecuteJob::start();
}

QVector<KDevelop::IProblem::Ptr> Job::problems() const
{
    return m_problems;
}

void Job::childProcessError(QProcess::ProcessError e)
{
    QString message;

    switch (e) {
    case QProcess::FailedToStart:
        message = i18n("Failed to start Cppcheck from \"%1\".", commandLine()[0]);
        break;

    case QProcess::Crashed:
        message = i18n("Cppcheck crashed.");
        break;

    case QProcess::Timedout:
        message = i18n("Cppcheck process timed out.");
        break;

    case QProcess::WriteError:
        message = i18n("Write to Cppcheck process failed.");
        break;

    case QProcess::ReadError:
        message = i18n("Read from Cppcheck process failed.");
        break;

    case QProcess::UnknownError:
        // current cppcheck errors will be displayed in the output view
        // don't notify the user
        break;
    }

    if (!message.isEmpty())
        KMessageBox::error(qApp->activeWindow(), message, i18n("Cppcheck Error"));

    KDevelop::OutputExecuteJob::childProcessError(e);
}

void Job::childProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(KDEV_CPPCHECK) << "Process Finished, exitCode" << exitCode << "process exit status" << exitStatus;

    if (exitCode != 0) {
        qCDebug(KDEV_CPPCHECK) << "cppcheck failed, standard output: ";
        qCDebug(KDEV_CPPCHECK) << m_standardOutput.join('\n');
        qCDebug(KDEV_CPPCHECK) << "cppcheck failed, XML output: ";
        qCDebug(KDEV_CPPCHECK) << m_xmlOutput.join('\n');
    } else {
        CppcheckParser parser;
        parser.addData(m_xmlOutput.join('\n'));
        parser.parse();
        m_problems = parser.problems();
    }

    KDevelop::OutputExecuteJob::childProcessExited(exitCode, exitStatus);
}

}
