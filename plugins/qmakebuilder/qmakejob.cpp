/* KDevelop QMake Support
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
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
    setToolTitle(i18n("QMake"));
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

void QMakeJob::slotFailed(QProcess::ProcessError error)
{
    qCDebug(KDEV_QMAKEBUILDER) << error;

    if (!m_killed) {
        setError(ConfigureError);
        // FIXME need more detail i guess
        setErrorText(i18n("Configure error"));
    }
    emitResult();
}

void QMakeJob::slotCompleted(int code)
{
    if (code != 0) {
        setError(FailedShownError);
    }
    emitResult();
}

bool QMakeJob::doKill()
{
    m_killed = true;
    m_cmd->kill();
    return true;
}
