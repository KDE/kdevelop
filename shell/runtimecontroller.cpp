/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "runtimecontroller.h"
#include <QProcess>
#include <QDebug>
#include <QComboBox>
#include <KActionCollection>
#include <KLocalizedString>
#include <KProcess>
#include <util/path.h>
#include "core.h"
#include "uicontroller.h"
#include "mainwindow.h"

using namespace KDevelop;

class IdentityRuntime : public IRuntime
{
    QString name() const override { return i18n("Host System"); }

    void startProcess(KProcess *process) override {
        connect(process, &QProcess::errorOccurred, this, [process](QProcess::ProcessError error) {
            qWarning() << "error!!!" << error << process->program();
        });
        process->start();
    }
    void startProcess(QProcess *process) override {
        connect(process, &QProcess::errorOccurred, this, [process](QProcess::ProcessError error) {
            qWarning() << "error!!!" << error << process->program();
        });
        process->start();
    }
    KDevelop::Path pathInHost(const KDevelop::Path & runtimePath) override { return runtimePath; }
    KDevelop::Path pathInRuntime(const KDevelop::Path & localPath) override { return localPath; }
    void setEnabled(bool /*enabled*/) override {}
};

KDevelop::RuntimeController::RuntimeController(KDevelop::Core* core)
    : m_runtimesMenu(new QMenu())
{
    addRuntimes({new IdentityRuntime});
    setCurrentRuntime(m_runtimes.constFirst());

    // TODO not multi-window friendly, FIXME
    KActionCollection* ac = core->uiControllerInternal()->defaultMainWindow()->actionCollection();

    auto action = new QAction(this);
    action->setStatusTip(i18n("Allows to select a runtime"));
    action->setMenu(m_runtimesMenu.data());
    action->setIcon(QIcon::fromTheme(QStringLiteral("file-library-symbolic")));
    auto updateActionText = [action](IRuntime* currentRuntime){
        action->setText(i18n("Runtime: %1", currentRuntime->name()));
    };
    connect(this, &RuntimeController::currentRuntimeChanged, action, updateActionText);
    updateActionText(m_currentRuntime);

    ac->addAction(QStringLiteral("switch_runtimes"), action);
}

KDevelop::RuntimeController::~RuntimeController()
{
    m_currentRuntime = nullptr;
    qDeleteAll(m_runtimes);
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
    qDebug() << "setting runtime..." << runtime->name() << "was" << m_currentRuntime;
    m_currentRuntime = runtime;
    Q_EMIT currentRuntimeChanged(runtime);

    m_currentRuntime->setEnabled(false);
}

void KDevelop::RuntimeController::addRuntimes(const QVector<KDevelop::IRuntime *>& runtimes)
{
    m_runtimes << runtimes;

    for(auto runtime : runtimes) {
        QAction* runtimeAction = new QAction(runtime->name(), m_runtimesMenu.data());
        runtimeAction->setCheckable(true);
        connect(runtimeAction, &QAction::triggered, runtime, [this, runtime]() {
            setCurrentRuntime(runtime);
        });
        connect(this, &RuntimeController::currentRuntimeChanged, runtimeAction, [runtimeAction, runtime](IRuntime* currentRuntime) {
            runtimeAction->setChecked(runtime == currentRuntime);
        });

        m_runtimesMenu->addAction(runtimeAction);
    }
}
