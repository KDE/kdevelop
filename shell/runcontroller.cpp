/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "runcontroller.h"

#include "irunprovider.h"

#include "core.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "mainwindow.h"

#include <KAction>
#include <KActionCollection>

using namespace KDevelop;

class RunController::RunControllerPrivate
{
public:
    QMap<int, IRun> running;
    QMap<QString, IRunProvider*> providers;
    IRunController::State state;
};

RunController::RunController(QObject *parent)
    : IRunController(parent)
    , d(new RunControllerPrivate)
{
    d->state = Idle;
    connect(Core::self()->pluginController(), SIGNAL(pluginLoaded(IPlugin*)), SLOT(pluginLoaded(IPlugin*)));
    connect(Core::self()->pluginController(), SIGNAL(pluginUnloaded(IPlugin*)), SLOT(pluginUnloaded(IPlugin*)));

    setupActions();
}

void RunController::pluginLoaded(IPlugin* plugin)
{
    if (IRunProvider* provider = qobject_cast<IRunProvider*>(plugin)) {
        foreach (const QString& instrumentor, provider->instrumentorsProvided())
            d->providers.insert(instrumentor, provider);

        connect(plugin, SIGNAL(finished(const IRun&)), this, SLOT(slotFinished(const IRun&)));
    }
}

void RunController::pluginUnloaded(KDevelop::IPlugin * plugin)
{
    QMutableMapIterator<QString, IRunProvider*> it = d->providers;
    while (it.hasNext())
        if (it.next().value() == qobject_cast<IRunProvider*>(plugin))
            it.remove();
}

bool RunController::run(const IRun & run)
{
    if (d->providers.contains(run.instrumentor())) {
        if (d->providers[run.instrumentor()]->run(run)) {
            d->running.insert(run.serial(), run);
            d->state = Running;
            emit runStateChanged(d->state);
            return true;
        }
    }

    // TODO provide feedback
    return false;
}

void RunController::abort(const IRun & run)
{
    if (d->running.contains(run.serial())) {
        d->providers[run.instrumentor()]->abort(run);
        d->running.remove(run.serial());
    }
}

void RunController::abortAll()
{
    foreach (const IRun& run, d->running)
        abort(run);
}

void RunController::slotFinished(const IRun & run)
{
    if (d->running.contains(run.serial())) {
        d->running.remove(run.serial());

        if (d->running.isEmpty()) {
            d->state = Idle;
            emit runStateChanged(d->state);
        }
    }
    // TODO provide feedback
}

RunController::~ RunController()
{
    delete d;
}

void RunController::setupActions()
{
    KAction *action;
    KActionCollection* ac = Core::self()->uiControllerInternal()->defaultMainWindow()->actionCollection();

    action = new KAction( KIcon("exec"), i18n("Execute Program"), this);
    action->setShortcut(Qt::SHIFT + Qt::Key_F9);
    action->setToolTip(i18n("Execute program"));
    action->setWhatsThis(i18n("<b>Execute program</b><p>Executes the currently active target or the main program specified in project settings, <b>Run Options</b> tab."));
    ac->addAction("run_execute", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(slotExecute()));
}

#include "runcontroller.moc"
