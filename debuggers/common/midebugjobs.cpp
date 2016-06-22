/*
* Common Code for Debugger Support
*
* Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
* Copyright 2007 Hamish Rodda <rodda@kde.org>
* Copyright 2009 Andreas Pakulat <apaku@gmx.de>
* Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "midebugjobs.h"

#include "debuglog.h"
#include "dialogs/selectcoredialog.h"
#include "midebugsession.h"
#include "midebuggerplugin.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iuicontroller.h>
#include <outputview/outputmodel.h>
#include <util/environmentgrouplist.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KParts/MainWindow>

#include <QFileInfo>

using namespace KDevMI;
using namespace KDevelop;

MIDebugJob::MIDebugJob(MIDebuggerPlugin* p, ILaunchConfiguration* launchcfg,
                   IExecutePlugin* execute, QObject* parent)
    : KDevelop::OutputJob(parent)
    , m_launchcfg(launchcfg)
    , m_execute(execute)
{
    setCapabilities(Killable);

    m_session = p->createSession();
    connect(m_session, &MIDebugSession::inferiorStdoutLines, this, &MIDebugJob::stderrReceived);
    connect(m_session, &MIDebugSession::inferiorStderrLines, this, &MIDebugJob::stdoutReceived);
    connect(m_session, &MIDebugSession::finished, this, &MIDebugJob::done);

    if (launchcfg->project()) {
        setObjectName(i18nc("ProjectName: run configuration name", "%1: %2",
                            launchcfg->project()->name(), launchcfg->name()));
    } else {
        setObjectName(launchcfg->name());
    }
}

void MIDebugJob::start()
{
    Q_ASSERT(m_execute);

    QString err;

    // check if the config is valid
    QString executable = m_execute->executable(m_launchcfg, err).toLocalFile();
    if (!err.isEmpty()) {
        setError(-1);
        setErrorText(err);
        emitResult();
        return;
    }

    if (!QFileInfo(executable).isExecutable()) {
        setError(-1);
        setErrorText(i18n("'%1' is not an executable", executable));
        emitResult();
        return;
    }

    QStringList arguments = m_execute->arguments(m_launchcfg, err);
    if (!err.isEmpty()) {
        setError(-1);
        setErrorText(err);
        emitResult();
        return;
    }

    setStandardToolView(IOutputView::DebugView);
    setBehaviours(IOutputView::Behaviours(IOutputView::AllowUserClose) | KDevelop::IOutputView::AutoScroll);

    auto model = new KDevelop::OutputModel;
    model->setFilteringStrategy(OutputModel::NativeAppErrorFilter);
    setModel(model);
    setTitle(m_launchcfg->name());

    KConfigGroup grp = m_launchcfg->config();
    QString startWith = grp.readEntry(startWithEntry, QString("ApplicationOutput"));
    if (startWith == "GdbConsole") {
        setVerbosity(Silent);
    } else if (startWith == "FrameStack") {
        setVerbosity(Silent);
    } else {
        setVerbosity(Verbose);
    }

    startOutput();

    if (!m_session->startDebugging(m_launchcfg, m_execute)) {
        done();
    }
}

bool MIDebugJob::doKill()
{
    m_session->stopDebugger();
    return true;
}

void MIDebugJob::stderrReceived(const QStringList& l)
{
    if (OutputModel* m = model()) {
        m->appendLines(l);
    }
}

void MIDebugJob::stdoutReceived(const QStringList& l)
{
    if (OutputModel* m = model()) {
        m->appendLines(l);
    }
}

OutputModel* MIDebugJob::model()
{
    return qobject_cast<OutputModel*>(OutputJob::model());
}

void MIDebugJob::done()
{
    emitResult();
}

MIExamineCoreJob::MIExamineCoreJob(MIDebuggerPlugin *plugin, QObject *parent)
    : KJob(parent)
{
    setCapabilities(Killable);

    m_session = plugin->createSession();
    connect(m_session, &MIDebugSession::finished, this, &MIExamineCoreJob::done);

    setObjectName(i18n("Debug core file"));
}

void MIExamineCoreJob::start()
{
    SelectCoreDialog dlg(ICore::self()->uiController()->activeMainWindow());
    if (dlg.exec() == QDialog::Rejected) {
        done();
        return;
    }

    if (!m_session->examineCoreFile(dlg.binary(), dlg.core())) {
        done();
    }
}

bool MIExamineCoreJob::doKill()
{
    m_session->stopDebugger();
    return true;
}

void MIExamineCoreJob::done()
{
    emitResult();
}

MIAttachProcessJob::MIAttachProcessJob(MIDebuggerPlugin *plugin, int pid, QObject *parent)
    : KJob(parent)
    , m_pid(pid)
{
    setCapabilities(Killable);

    m_session = plugin->createSession();
    connect(m_session, &MIDebugSession::finished, this, &MIAttachProcessJob::done);

    setObjectName(i18n("Debug process %1", pid));
}

void MIAttachProcessJob::start()
{
    if (!m_session->attachToProcess(m_pid)) {
        done();
    }
}

bool MIAttachProcessJob::doKill()
{
    m_session->stopDebugger();
    return true;
}

void MIAttachProcessJob::done()
{
    emitResult();
}
