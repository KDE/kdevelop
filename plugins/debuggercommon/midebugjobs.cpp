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

MIDebugJob::MIDebugJob(MIDebuggerPlugin* p, ILaunchConfiguration* launchcfg,
                   IExecutePlugin* execute, QObject* parent)
    : KDevelop::OutputJob(parent)
    , m_launchcfg(launchcfg)
    , m_execute(execute)
{
    setCapabilities(Killable);

    m_session = p->createSession();
    connect(m_session, &MIDebugSession::inferiorStdoutLines, this, &MIDebugJob::stdoutReceived);
    connect(m_session, &MIDebugSession::inferiorStderrLines, this, &MIDebugJob::stderrReceived);

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
