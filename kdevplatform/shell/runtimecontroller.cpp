/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "runtimecontroller.h"
#include <QActionGroup>
#include <QProcess>
#include <QComboBox>
#include <KActionCollection>
#include <KLocalizedString>
#include <KProcess>
#include <KShell>
#include <util/path.h>
#include "core.h"
#include "uicontroller.h"
#include "mainwindow.h"
#include "debug.h"

using namespace KDevelop;

class IdentityRuntime : public IRuntime
{
    Q_OBJECT
public:
    QString name() const override { return i18n("Host System"); }

    void startProcess(KProcess *process) const override {
        catchErrors(*process);
        process->start();
    }
    void startProcess(QProcess *process) const override {
        catchErrors(*process);
        process->start();
    }
    KDevelop::Path pathInHost(const KDevelop::Path & runtimePath) const override { return runtimePath; }
    KDevelop::Path pathInRuntime(const KDevelop::Path & localPath) const override { return localPath; }
    QString findExecutable(const QString& executableName) const override
    {
        return QStandardPaths::findExecutable(executableName);
    }
    void setEnabled(bool /*enabled*/) override {}
    QByteArray getenv(const QByteArray & varname) const override { return qgetenv(varname.constData()); }
    KDevelop::Path buildPath() const override { return {}; }

private:
    static void catchErrors(const QProcess& process)
    {
        connect(&process, &QProcess::errorOccurred, [&process](QProcess::ProcessError error) {
            qCWarning(SHELL).noquote().nospace()
                << "process finished with error: " << error << " \"" << process.errorString()
                << "\", the command line: \"" << KShell::quoteArg(process.program()) << ' '
                << KShell::joinArgs(process.arguments()) << '"';
        });
    }
};

KDevelop::RuntimeController::RuntimeController(KDevelop::Core* core)
    : m_core(core)
    , m_group(new QActionGroup(this))
{
    const bool haveUI = (core->setupFlags() != Core::NoUi);
    if (haveUI) {
        m_runtimesMenu.reset(new QMenu());
    }

    addRuntimes(new IdentityRuntime);
    setCurrentRuntime(m_runtimes.first());

    if (haveUI) {
        setupActions();
    }
}

KDevelop::RuntimeController::~RuntimeController()
{
    m_currentRuntime->setEnabled(false);
    m_currentRuntime = nullptr;
}

void RuntimeController::setupActions()
{
    // TODO not multi-window friendly, FIXME
    KActionCollection* ac = m_core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    auto action = new QAction(this);
    action->setToolTip(i18n("Allows to select a runtime"));
    action->setMenu(m_runtimesMenu.data());
    action->setIcon(QIcon::fromTheme(QStringLiteral("file-library-symbolic")));
    auto updateActionText = [action](IRuntime* currentRuntime){
        action->setText(i18n("Runtime: %1", currentRuntime->name()));
    };
    connect(this, &RuntimeController::currentRuntimeChanged, action, updateActionText);
    updateActionText(m_currentRuntime);

    ac->addAction(QStringLiteral("switch_runtimes"), action);
}

void KDevelop::RuntimeController::initialize()
{
}

KDevelop::IRuntime * KDevelop::RuntimeController::currentRuntime() const
{
    Q_ASSERT(m_currentRuntime);
    return m_currentRuntime;
}

QVector<KDevelop::IRuntime *> KDevelop::RuntimeController::availableRuntimes() const
{
    return m_runtimes;
}

void KDevelop::RuntimeController::setCurrentRuntime(KDevelop::IRuntime* runtime)
{
    if (m_currentRuntime == runtime)
        return;

    Q_ASSERT(m_runtimes.contains(runtime));

    if (m_currentRuntime) {
        m_currentRuntime->setEnabled(false);
    }
    qCDebug(SHELL) << "setting runtime..." << runtime->name() << "was" << m_currentRuntime;
    m_currentRuntime = runtime;
    m_currentRuntime->setEnabled(true);
    Q_EMIT currentRuntimeChanged(runtime);
}

void KDevelop::RuntimeController::addRuntimes(KDevelop::IRuntime * runtime)
{
    if (!runtime->parent())
        runtime->setParent(this);

    if (m_core->setupFlags() != Core::NoUi) {
        auto* runtimeAction = new QAction(runtime->name(), m_runtimesMenu.data());
        m_group->addAction(runtimeAction);
        runtimeAction->setCheckable(true);
        connect(runtimeAction, &QAction::triggered, runtime, [this, runtime]() {
            setCurrentRuntime(runtime);
        });
        connect(this, &RuntimeController::currentRuntimeChanged, runtimeAction, [runtimeAction, runtime](IRuntime* currentRuntime) {
            runtimeAction->setChecked(runtime == currentRuntime);
        });

        connect(runtime, &QObject::destroyed, this, [this, runtimeAction](QObject* obj) {
            Q_ASSERT(m_currentRuntime != obj);
            m_runtimes.removeAll(qobject_cast<KDevelop::IRuntime *>(obj));
            delete runtimeAction;
        });
        m_runtimesMenu->addAction(runtimeAction);
    } else {
        connect(runtime, &QObject::destroyed, this, [this](QObject* obj) {
            Q_ASSERT(m_currentRuntime != obj);
            m_runtimes.removeAll(qobject_cast<KDevelop::IRuntime *>(obj));
        });
    }

    m_runtimes << runtime;
}

#include "runtimecontroller.moc"
#include "moc_runtimecontroller.cpp"
