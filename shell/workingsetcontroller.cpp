/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "workingsetcontroller.h"

#include <QTimer>
#include <QVBoxLayout>

#include "mainwindow.h"
#include "partdocument.h"
#include "uicontroller.h"

#include <interfaces/iuicontroller.h>
#include <interfaces/isession.h>

#include <sublime/view.h>
#include <sublime/area.h>

#include <util/activetooltip.h>
#include <kdebug.h>

#include "workingsets/workingset.h"
#include "workingsets/workingsettooltipwidget.h"
#include "workingsets/workingsetwidget.h"
#include "workingsets/closedworkingsetswidget.h"
#include "core.h"

using namespace KDevelop;

const int toolTipTimeout = 2000;

WorkingSetController::WorkingSetController()
    : m_emptyWorkingSet(0), m_changingWorkingSet(false)
{
    m_hideToolTipTimer = new QTimer(this);
    m_hideToolTipTimer->setInterval(toolTipTimeout);
    m_hideToolTipTimer->setSingleShot(true);
}

void WorkingSetController::initialize()
{
    //Load all working-sets
    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
    foreach(const QString& set, setConfig.groupList()) {
        // do not load working set if the id contains an '|', because it then belongs to an area.
        // this is functionally equivalent to the if ( ! config->icon ) stuff which was there before.
        if ( set.contains('|') ) {
            continue;
        }
        getWorkingSet(set);
    }

    m_emptyWorkingSet = new WorkingSet("empty");

    if(!(Core::self()->setupFlags() & Core::NoUi)) {
        setupActions();
    }
}

void WorkingSetController::cleanup()
{
    foreach(Sublime::MainWindow* window, Core::self()->uiControllerInternal()->mainWindows()) {
        foreach (Sublime::Area *area, window->areas()) {
            if (!area->workingSet().isEmpty()) {
                Q_ASSERT(m_workingSets.contains(area->workingSet()));
                m_workingSets[area->workingSet()]->saveFromArea(area, area->rootIndex());
            }
        }
    }

    foreach(WorkingSet* set, m_workingSets) {
        kDebug() << "set" << set->id() << "persistent" << set->isPersistent() << "has areas:" << set->hasConnectedAreas() << "files" << set->fileList();
        if(!set->isPersistent() && !set->hasConnectedAreas()) {
            kDebug() << "deleting";
            set->deleteSet(true, true);
        }
        delete set;
    }

    m_workingSets.clear();

    delete m_emptyWorkingSet;
    m_emptyWorkingSet = 0;
}

const QString WorkingSetController::makeSetId(const QString& prefix) const
{
    QString newId;
    const uint maxRetries = 10;
    for(uint retry = 2; retry <= maxRetries; retry++) {
        newId = QString("%1_%2").arg(prefix).arg(qrand() % 10000000);
        WorkingSetIconParameters params(newId);
        foreach(WorkingSet* set, m_workingSets) {
            if(set->isEmpty()) {
                continue;
            }
            // The last retry will always generate a valid set
            if(retry != maxRetries && WorkingSetIconParameters(set->id()).similarity(params) >= retry*8) {
                newId = QString();
                break;
            }
        }
        if(! newId.isEmpty()) {
            break;
        }
    }
    return newId;
}

WorkingSet* WorkingSetController::newWorkingSet(const QString& prefix)
{

    return getWorkingSet(makeSetId(prefix));
}

WorkingSet* WorkingSetController::getWorkingSet(const QString& id)
{
    if(id.isEmpty())
        return m_emptyWorkingSet;
    
    if(!m_workingSets.contains(id)) {
        WorkingSet* set = new WorkingSet(id);
        connect(set, SIGNAL(aboutToRemove(WorkingSet*)),
                this, SIGNAL(aboutToRemoveWorkingSet(WorkingSet*)));
        m_workingSets[id] = set;
        emit workingSetAdded(set);
    }

    return m_workingSets[id];
}

QWidget* WorkingSetController::createSetManagerWidget(MainWindow* parent, Sublime::Area* fixedArea) {
    if (fixedArea) {
        return new WorkingSetWidget(fixedArea, parent);
    } else {
        return new ClosedWorkingSetsWidget(parent);
    }
}

void WorkingSetController::setupActions()
{
/*
    KActionCollection * ac =
        Core::self()->uiControllerInternal()->defaultMainWindow()->actionCollection();

    QAction* action;

    action = ac->addAction ( "view_next_window" );
    action->setText( i18n( "Next Document" ) );
    action->setIcon( QIcon::fromTheme("go-next") );
    action->setShortcut( Qt::ALT + Qt::SHIFT + Qt::Key_Right );
    action->setWhatsThis( i18n( "Switch the focus to the next open document." ) );
    action->setStatusTip( i18n( "Switch the focus to the next open document." ) );
    connect( action, SIGNAL(triggered()), this, SLOT(nextDocument()) );

    action = ac->addAction ( "view_previous_window" );
    action->setText( i18n( "Previous Document" ) );
    action->setIcon( QIcon::fromTheme("go-previous") );
    action->setShortcut( Qt::ALT + Qt::SHIFT + Qt::Key_Left );
    action->setWhatsThis( i18n( "Switch the focus to the previous open document." ) );
    action->setStatusTip( i18n( "Switch the focus to the previous open document." ) );
    connect( action, SIGNAL(triggered()), this, SLOT(previousDocument()) );
*/
}

ActiveToolTip* WorkingSetController::tooltip() const
{
    return m_tooltip;
}

void WorkingSetController::showToolTip(WorkingSet* set, const QPoint& pos)
{
    delete m_tooltip;

    KDevelop::MainWindow* window = static_cast<KDevelop::MainWindow*>(Core::self()->uiControllerInternal()->activeMainWindow());

    m_tooltip = new KDevelop::ActiveToolTip(window, pos);
    QVBoxLayout* layout = new QVBoxLayout(m_tooltip);
    layout->setMargin(0);
    WorkingSetToolTipWidget* widget = new WorkingSetToolTipWidget(m_tooltip, set, window);
    layout->addWidget(widget);
    m_tooltip->resize( m_tooltip->sizeHint() );

    connect(widget, SIGNAL(shouldClose()), m_tooltip, SLOT(close()));

    ActiveToolTip::showToolTip(m_tooltip);
}

void WorkingSetController::showGlobalToolTip()
{
    KDevelop::MainWindow* window = static_cast<KDevelop::MainWindow*>(Core::self()->uiControllerInternal()->activeMainWindow());

    showToolTip(getWorkingSet(window->area()->workingSet()),
                              window->mapToGlobal(window->geometry().topRight()));

    connect(m_hideToolTipTimer, SIGNAL(timeout()),  m_tooltip, SLOT(deleteLater()));
    m_hideToolTipTimer->start();
    connect(m_tooltip, SIGNAL(mouseIn()), m_hideToolTipTimer, SLOT(stop()));
    connect(m_tooltip, SIGNAL(mouseOut()), m_hideToolTipTimer, SLOT(start()));
}

void WorkingSetController::nextDocument()
{
    if(!m_tooltip)
        showGlobalToolTip();

    m_hideToolTipTimer->stop();
    m_hideToolTipTimer->start(toolTipTimeout);

    if(m_tooltip)
    {
        WorkingSetToolTipWidget* widget = m_tooltip->findChild<WorkingSetToolTipWidget*>();
        Q_ASSERT(widget);
        widget->nextDocument();
    }
}

void WorkingSetController::previousDocument()
{
    if(!m_tooltip)
        showGlobalToolTip();

    m_hideToolTipTimer->stop();
    m_hideToolTipTimer->start(toolTipTimeout);

    if(m_tooltip)
    {
        WorkingSetToolTipWidget* widget = m_tooltip->findChild<WorkingSetToolTipWidget*>();
        Q_ASSERT(widget);
        widget->previousDocument();
    }
}

void WorkingSetController::initializeController( UiController* controller )
{
  connect( controller, SIGNAL(areaCreated(Sublime::Area*)), this, SLOT(areaCreated(Sublime::Area*)) );
}

QList< WorkingSet* > WorkingSetController::allWorkingSets() const
{
  return m_workingSets.values();
}

void WorkingSetController::areaCreated( Sublime::Area* area )
{
    if (!area->workingSet().isEmpty()) {
        WorkingSet* set = getWorkingSet( area->workingSet() );
        set->connectArea( area );
    }

    connect(area, SIGNAL(changingWorkingSet(Sublime::Area*,QString,QString)),
            this, SLOT(changingWorkingSet(Sublime::Area*,QString,QString)));
    connect(area, SIGNAL(changedWorkingSet(Sublime::Area*,QString,QString)),
            this, SLOT(changedWorkingSet(Sublime::Area*,QString,QString)));
    connect(area, SIGNAL(viewAdded(Sublime::AreaIndex*,Sublime::View*)),
            this, SLOT(viewAdded(Sublime::AreaIndex*,Sublime::View*)));
}

void WorkingSetController::changingWorkingSet(Sublime::Area* area, const QString& from, const QString& to)
{
    kDebug() << "changing working-set from" << from << "to" << to << "area" << area;
    if (from == to)
        return;

    if (!from.isEmpty()) {
        WorkingSet* oldSet = getWorkingSet(from);
        oldSet->disconnectArea(area);
        if (!oldSet->id().isEmpty()) {
            oldSet->saveFromArea(area, area->rootIndex());
        }
    }
}

void WorkingSetController::changedWorkingSet(Sublime::Area* area, const QString& from, const QString& to)
{
    kDebug() << "changed working-set from" << from << "to" << to << "area" << area;
    if (from == to || m_changingWorkingSet)
        return;
    
    if (!to.isEmpty()) {
        WorkingSet* newSet = getWorkingSet(to);
        newSet->connectArea(area);
        newSet->loadToArea(area, area->rootIndex());
    }else{
        // Clear silently, any user-interaction should have happened before
        area->clearViews(true);
    }

    emit workingSetSwitched();
}

void WorkingSetController::viewAdded( Sublime::AreaIndex* , Sublime::View* )
{
    Sublime::Area* area = qobject_cast< Sublime::Area* >(sender());
    Q_ASSERT(area);

    if (area->workingSet().isEmpty()) {
        //Spawn a new working-set
        m_changingWorkingSet = true;
        WorkingSet* set = Core::self()->workingSetControllerInternal()->newWorkingSet(area->objectName());
        kDebug() << "Spawned new working-set" << set->id() << "because a view was added";
        set->connectArea(area);
        set->saveFromArea(area, area->rootIndex());
        area->setWorkingSet(set->id());
        m_changingWorkingSet = false;
    }
}


#include "workingsetcontroller.moc"
