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
    int serial;
    QMap<int, IRun> running;
    QMap<QString, IRunProvider*> providers;
    IRunController::State state;
};

RunController::RunController(QObject *parent)
    : IRunController(parent)
    , d(new RunControllerPrivate)
{
    d->serial = 0;
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

        connect(plugin, SIGNAL(finished(int)), this, SLOT(slotFinished(int)));
    }
}

void RunController::pluginUnloaded(KDevelop::IPlugin * plugin)
{
    QMutableMapIterator<QString, IRunProvider*> it = d->providers;
    while (it.hasNext())
        if (it.next().value() == qobject_cast<IRunProvider*>(plugin))
            it.remove();
}

int RunController::run(const IRun & run)
{
    if (d->providers.contains(run.instrumentor())) {
        int newSerial = d->serial++;
        if (d->providers[run.instrumentor()]->run(run, newSerial)) {
            d->running.insert(newSerial, run);
            setState(Running);
            return newSerial;
        }
    }

    return -1;
}

void RunController::abort(int serial)
{
    if (d->running.contains(serial)) {
        const IRun& run = d->running[serial];
        d->providers[run.instrumentor()]->abort(serial);
        d->running.remove(serial);

        if (d->running.isEmpty())
            setState(Idle);
    }
}

void RunController::abortAll()
{
    foreach (int serial, d->running.keys())
        abort(serial);
}

void RunController::slotFinished(int serial)
{
    if (d->running.contains(serial)) {
        d->running.remove(serial);

        if (d->running.isEmpty())
            setState(Idle);
    }
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

void KDevelop::RunController::setState(State state)
{
    if (d->state != state) {
        d->state = state;
        emit runStateChanged(d->state);
    }
}

#include "runcontroller.moc"
