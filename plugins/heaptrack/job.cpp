/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "job.h"

#include "debug.h"
#include "globalsettings.h"
#include "utils.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <util/environmentprofilelist.h>
#include <util/path.h>

#include <KLocalizedString>

#include <QFileInfo>
#include <QRegularExpression>

namespace Heaptrack
{

Job::Job(KDevelop::ILaunchConfiguration* launchConfig, IExecutePlugin* executePlugin)
    : m_pid(-1)
{
    Q_ASSERT(launchConfig);
    Q_ASSERT(executePlugin);

    QString envProfile = executePlugin->environmentProfileName(launchConfig);
    if (envProfile.isEmpty()) {
        envProfile = KDevelop::EnvironmentProfileList(KSharedConfig::openConfig()).defaultProfileName();
    }
    setEnvironmentProfile(envProfile);

    QString errorString;

    m_analyzedExecutable = executePlugin->executable(launchConfig, errorString).toLocalFile();
    if (!errorString.isEmpty()) {
        setError(-1);
        setErrorText(errorString);
    }

    QStringList analyzedExecutableArguments = executePlugin->arguments(launchConfig, errorString);
    if (!errorString.isEmpty()) {
        setError(-1);
        setErrorText(errorString);
    }

    QUrl workDir = executePlugin->workingDirectory(launchConfig);
    if (workDir.isEmpty() || !workDir.isValid()) {
        workDir = QUrl::fromLocalFile(QFileInfo(m_analyzedExecutable).absolutePath());
    }
    setWorkingDirectory(workDir);

    *this << KDevelop::Path(GlobalSettings::heaptrackExecutable()).toLocalFile();
    *this << m_analyzedExecutable;
    *this << analyzedExecutableArguments;

    setup();
}

Job::Job(long int pid)
    : m_pid(pid)
{
    *this << KDevelop::Path(GlobalSettings::heaptrackExecutable()).toLocalFile();
    *this << QStringLiteral("-p");
    *this << QString::number(m_pid);

    setup();
}

void Job::setup()
{
    setProperties(DisplayStdout);
    setProperties(DisplayStderr);
    setProperties(PostProcessOutput);

    setCapabilities(Killable);
    setStandardToolView(KDevelop::IOutputView::TestView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);

    KDevelop::ICore::self()->uiController()->registerStatus(this);
    connect(this, &Job::finished, this, [this]() {
        emit hideProgress(this);
    });
}

Job::~Job()
{
}

QString Job::statusName() const
{
    QString target = m_pid < 0 ? QFileInfo(m_analyzedExecutable).fileName()
                               : QStringLiteral("PID: %1").arg(m_pid);
    return i18n("Heaptrack Analysis (%1)", target);
}

QString Job::resultsFile() const
{
    return m_resultsFile;
}

void Job::start()
{
    emit showProgress(this, 0, 0, 0);
    OutputExecuteJob::start();
}

void Job::postProcessStdout(const QStringList& lines)
{
    static const auto resultRegex =
        QRegularExpression(QStringLiteral("heaptrack output will be written to \\\"(.+)\\\""));

    if (m_resultsFile.isEmpty()) {
        QRegularExpressionMatch match;
        for (const QString & line : lines) {
            match = resultRegex.match(line);
            if (match.hasMatch()) {
                m_resultsFile = match.captured(1);
                break;
            }
        }
    }

    OutputExecuteJob::postProcessStdout(lines);
}

}

#include "moc_job.cpp"
