/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "midebugjobs.h"

#include "debuglog.h"
#include "dialogs/selectcoredialog.h"
#include "midebugsession.h"
#include "midebuggerplugin.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iproject.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iuicontroller.h>
#include <outputview/outputmodel.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KParts/MainWindow>

#include <QFileInfo>
#include <QPointer>

using namespace KDevMI;
using namespace KDevelop;

template<class JobBase>
MIDebugJobBase<JobBase>::MIDebugJobBase(MIDebuggerPlugin* plugin, QObject* parent)
    : JobBase(parent)
{
    Q_ASSERT(plugin);

    JobBase::setCapabilities(KJob::Killable);

    m_session = plugin->createSession();
    QObject::connect(m_session, &MIDebugSession::finished, this, &MIDebugJobBase::done);

    qCDebug(DEBUGGERCOMMON) << "created debug job" << this << "with" << m_session;
}

template<class JobBase>
MIDebugJobBase<JobBase>::~MIDebugJobBase()
{
    // Don't print m_session unconditionally, because it can be already destroyed if this job is finished.
    qCDebug(DEBUGGERCOMMON) << "destroying debug job" << this;
    if (!JobBase::isFinished()) {
        qCDebug(DEBUGGERCOMMON) << "debug job destroyed before it finished, stopping debugger of" << m_session;
        m_session->stopDebugger();
    }
}

template<typename JobBase>
void MIDebugJobBase<JobBase>::done()
{
    qCDebug(DEBUGGERCOMMON) << "finishing debug job" << this << "with" << m_session;
    JobBase::emitResult();
}

template<typename JobBase>
bool MIDebugJobBase<JobBase>::doKill()
{
    qCDebug(DEBUGGERCOMMON) << "killing debug job" << this << "and stopping debugger of" << m_session;
    m_session->stopDebugger();
    return true;
}

MIDebugJob::MIDebugJob(MIDebuggerPlugin* p, ILaunchConfiguration* launchcfg,
                   IExecutePlugin* execute, QObject* parent)
    : MIDebugJobBase(p, parent)
    , m_launchcfg(launchcfg)
    , m_execute(execute)
{
    connect(m_session, &MIDebugSession::inferiorStdoutLines, this, &MIDebugJob::stdoutReceived);
    connect(m_session, &MIDebugSession::inferiorStderrLines, this, &MIDebugJob::stderrReceived);

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
    QString startWith = grp.readEntry(Config::StartWithEntry, QStringLiteral("ApplicationOutput"));
    if (startWith == QLatin1String("ApplicationOutput")) {
        setVerbosity(Verbose);
    } else {
        setVerbosity(Silent);
    }

    startOutput();

    if (!m_session->startDebugging(m_launchcfg, m_execute)) {
        done();
    }
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

MIExamineCoreJob::MIExamineCoreJob(MIDebuggerPlugin *plugin, QObject *parent)
    : MIDebugJobBase(plugin, parent)
{
    setObjectName(i18n("Debug core file"));
}

void MIExamineCoreJob::start()
{
    QPointer<SelectCoreDialog> dlg = new SelectCoreDialog(ICore::self()->uiController()->activeMainWindow());
    if (dlg->exec() == QDialog::Rejected) {
        done();
        delete dlg;
        return;
    }

    if (!m_session->examineCoreFile(dlg->executableFile(), dlg->core())) {
        done();
    }
    delete dlg;
}

MIAttachProcessJob::MIAttachProcessJob(MIDebuggerPlugin *plugin, int pid, QObject *parent)
    : MIDebugJobBase(plugin, parent)
    , m_pid(pid)
{
    setObjectName(i18n("Debug process %1", pid));
}

void MIAttachProcessJob::start()
{
    if (!m_session->attachToProcess(m_pid)) {
        done();
    }
}
