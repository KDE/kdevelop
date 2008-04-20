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

#include "core.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "mainwindow.h"
#include "ioutputview.h"

#include <QApplication>
#include <QStandardItemModel>
#include <QItemDelegate>

#include <KAction>
#include <KActionCollection>
#include <KMessageBox>
#include <KLocale>
#include <KDebug>

using namespace KDevelop;

class RunController::RunControllerPrivate
{
public:
    int serial;
    QMap<int, IRunProvider*> running;
    QMap<int, QPair<int, QStandardItemModel*> > outputModels;
    IRunController::State state;
};

RunController::RunController(QObject *parent)
    : IRunController(parent)
    , d(new RunControllerPrivate)
{
    // TODO: need to implement compile only if needed before execute
    // TODO: need to implement abort all running programs when project closed

    d->serial = 0;
    d->state = Idle;

    setupActions();
}

int RunController::execute(const IRun & run)
{
    if (IRunProvider* provider = findProvider(run.instrumentor())) {
        int newSerial = d->serial++;
        if (provider->execute(run, newSerial)) {
            d->running.insert(newSerial, provider);

            createModel(newSerial, run);

            setState(Running);
            return newSerial;
        }
    }

    KMessageBox::error(qApp->activeWindow(), i18n("Execution failed: no plugin found for requested instrumentor \"%1\"", run.instrumentor()), i18n("Execution Error"));
    
    return -1;
}

void RunController::abort(int serial)
{
    if (d->running.contains(serial)) {
        d->running[serial]->abort(serial);
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

    action = new KAction( KIcon("system-run"), i18n("Execute Program"), this);
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

void RunController::slotExecute()
{
    execute(defaultRun());
}

IRun KDevelop::RunController::defaultRun() const
{
    KConfigGroup group(KGlobal::config(), "Run Options" );

    IRun run;
    run.setExecutable(group.readEntry( "Executable", "" ));
    run.setWorkingDirectory(group.readEntry( "Working Directory", "" ));
    run.setArguments(QStringList() << group.readEntry( "Arguments", QString() ));
    run.setInstrumentor("default");

    if (group.readEntry("Compile Before Execution", false))
        if (group.readEntry("Install Before Execution", false))
            if (group.readEntry("Super User Install", false))
                ;// TODO: sudo make install
            else
                ;// TODO: make install
        else
            ;// TODO: make

    if (group.readEntry("Start In Terminal", false))
        ;// TODO: start in terminal rather than output view

    return run;
}

void KDevelop::RunController::slotOutput(int serial, const QString& line, KDevelop::IRunProvider::OutputTypes type)
{
    if (!d->outputModels.contains(serial)) {
        kWarning() << "No output model available for input";
        return;
    }

    QStandardItemModel* model = d->outputModels[serial].second;

    QStandardItem* item = new QStandardItem(line);

    switch (type) {
        case IRunProvider::StandardError:
            item->setForeground(Qt::red);
            break;
        case IRunProvider::RunProvider:
            item->setForeground(Qt::blue);
            break;
        default:
            break;
    }

    model->appendRow(item);
}

void KDevelop::RunController::createModel(int serial, const IRun& run)
{
    IPlugin* i = Core::self()->pluginController()->pluginForExtension("org.kdevelop.IOutputView");
    if( i )
    {
        KDevelop::IOutputView* view = i->extension<KDevelop::IOutputView>();
        if( view )
        {
            int tvid = view->registerToolView(i18n("Run") );
            int id = view->registerOutputInToolView( tvid, run.executable().path(), KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );

            QStandardItemModel* model = new QStandardItemModel(this);
            d->outputModels.insert(serial, qMakePair(id, model));
            connect(i, SIGNAL(outputRemoved(int, int)), this, SLOT(outputViewRemoved(int, int)));

            view->setModel(id, model);
            view->setDelegate(id, new QItemDelegate(this));
        }
    }
}

void KDevelop::RunController::outputViewRemoved(int /*toolviewId*/, int id)
{
    QMutableMapIterator<int, QPair<int, QStandardItemModel*> > it = d->outputModels;
    while (it.hasNext()) {
        if (it.next().value().first == id) {
            delete it.value().second;
            it.remove();
            return;
        }
    }
}

IRunProvider * KDevelop::RunController::findProvider(const QString & instrumentor)
{
    foreach (IPlugin* i, Core::self()->pluginController()->allPluginsForExtension("org.kdevelop.IRunProvider", QStringList())) {
        if (KDevelop::IRunProvider* provider = i->extension<KDevelop::IRunProvider>()) {
            if (provider->instrumentorsProvided().contains(instrumentor)) {
                i->disconnect(this);
                connect(i, SIGNAL(finished(int)), this, SLOT(slotFinished(int)));
                connect(i, SIGNAL(output(int, const QString&, KDevelop::IRunProvider::OutputTypes)), this, SLOT(slotOutput(int, const QString&, KDevelop::IRunProvider::OutputTypes)));
                return provider;
            }
        }
    }

    return 0;
}

#include "runcontroller.moc"
