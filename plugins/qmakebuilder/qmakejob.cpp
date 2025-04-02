/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakejob.h"

#include <debug.h>
#include "qmakeconfig.h"

#include <interfaces/iproject.h>
#include <outputview/ioutputview.h>
#include <outputview/outputmodel.h>
#include <project/projectmodel.h>
#include <makebuilder/imakebuilder.h>
#include <util/commandexecutor.h>
#include <util/path.h>

#include <QDir>
#include <KLocalizedString>

using namespace KDevelop;

QMakeJob::QMakeJob(QObject* parent)
    : OutputExecuteJob(parent)
{
    setCapabilities(Killable);
    setFilteringStrategy(OutputModel::CompilerFilter);
    setProperties(NeedWorkingDirectory | PortableMessages | DisplayStderr | IsBuilderHint);
    setToolTitle(i18nc("@title:window", "QMake"));
    setStandardToolView(IOutputView::BuildView);
    setBehaviours(IOutputView::AllowUserClose | IOutputView::AutoScroll );
}

void QMakeJob::start()
{
    qCDebug(KDEV_QMAKEBUILDER) << "Running qmake in" << workingDirectory();

    if (!m_project) {
        setError(NoProjectError);
        setErrorText(i18n("No project specified."));
        emitResult();
        return;
    }

    // create build directory if it does not exist yet
    QDir::temp().mkpath(workingDirectory().toLocalFile());

    OutputExecuteJob::start();
}

QUrl QMakeJob::workingDirectory() const
{
    if (!m_project) {
        return QUrl();
    }

    return QMakeConfig::buildDirFromSrc(m_project, m_project->path()).toUrl();
}

QStringList QMakeJob::commandLine() const
{
    if (!m_project) {
        return {};
    }

    const QStringList args{
        QMakeConfig::qmakeExecutable(m_project),
        m_project->path().toUrl().toLocalFile(),
    };

    return args;
}

void QMakeJob::setProject(KDevelop::IProject* project)
{
    m_project = project;

    if (m_project)
        setObjectName(i18n("QMake: %1", m_project->name()));
}

bool QMakeJob::doKill()
{
    m_killed = true;
    m_cmd->kill();
    return true;
}

#include "moc_qmakejob.cpp"
