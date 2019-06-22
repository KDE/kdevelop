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

#include "workingsets/workingset.h"
#include "workingsets/workingsettooltipwidget.h"
#include "workingsets/workingsetwidget.h"
#include "workingsets/closedworkingsetswidget.h"
#include "core.h"
#include "debug.h"

using namespace KDevelop;

const int toolTipTimeout = 2000;

WorkingSetController::WorkingSetController()
{
    m_hideToolTipTimer = new QTimer(this);
    m_hideToolTipTimer->setInterval(toolTipTimeout);
    m_hideToolTipTimer->setSingleShot(true);
}

void WorkingSetController::initialize()
{
    //Load all working-sets
    KConfigGroup setConfig(Core::self()->activeSession()->config(), "Working File Sets");
    const auto sets = setConfig.groupList();
    for (const QString& set : sets) {
        // do not load working set if the id contains an '|', because it then belongs to an area.
        // this is functionally equivalent to the if ( ! config->icon ) stuff which was there before.
        if (set.contains(QLatin1Char('|'))) {
            continue;
        }
        workingSet(set);
    }

    m_emptyWorkingSet = new WorkingSet(QStringLiteral("empty"));

    if(!(Core::self()->setupFlags() & Core::NoUi)) {
        setupActions();
    }
}

void WorkingSetController::cleanup()
{
    const auto windows = Core::self()->uiControllerInternal()->mainWindows();
    for (Sublime::MainWindow* window : windows) {
        const auto areas = window->areas();
        for (Sublime::Area* area : areas) {
            if (!area->workingSet().isEmpty()) {
                Q_ASSERT(m_workingSets.contains(area->workingSet()));
                m_workingSets[area->workingSet()]->saveFromArea(area, area->rootIndex());
            }
        }
    }

    const auto oldWorkingSet = m_workingSets;
    for (WorkingSet* set : oldWorkingSet) {
        qCDebug(SHELL) << "set" << set->id() << "persistent" << set->isPersistent() << "has areas:" << set->hasConnectedAreas() << "files" << set->fileList();
        if(!set->isPersistent() && !set->hasConnectedAreas()) {
            qCDebug(SHELL) << "deleting";
            set->deleteSet(true, true);
        }
        delete set;
    }

    m_workingSets.clear();

    delete m_emptyWorkingSet;
    m_emptyWorkingSet = nullptr;
}

const QString WorkingSetController::makeSetId(const QString& prefix) const
{
    QString newId;
    const uint maxRetries = 10;
    for(uint retry = 2; retry <= maxRetries; retry++) {
        newId = QStringLiteral("%1_%2").arg(prefix).arg(qrand() % 10000000);
        WorkingSetIconParameters params(newId);
        for (WorkingSet* set : m_workingSets) {
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
    return workingSet(makeSetId(prefix));
}

WorkingSet* WorkingSetController::workingSet(const QString& id)
{
    if(id.isEmpty())
        return m_emptyWorkingSet;

    auto workingSetIt = m_workingSets.find(id);
    if (workingSetIt == m_workingSets.end()) {
        auto* set = new WorkingSet(id);
        connect(set, &WorkingSet::aboutToRemove,
                this, &WorkingSetController::aboutToRemoveWorkingSet);
        workingSetIt= m_workingSets.insert(id, set);
        emit workingSetAdded(set);
    }

    return *workingSetIt;
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
}

ActiveToolTip* WorkingSetController::tooltip() const
{
    return m_tooltip;
}

void WorkingSetController::showToolTip(WorkingSet* set, const QPoint& pos)
{
    delete m_tooltip;

    auto* window = static_cast<KDevelop::MainWindow*>(Core::self()->uiControllerInternal()->activeMainWindow());

    m_tooltip = new KDevelop::ActiveToolTip(window, pos);
    auto* layout = new QVBoxLayout(m_tooltip);
    layout->setMargin(0);
    auto* widget = new WorkingSetToolTipWidget(m_tooltip, set, window);
    layout->addWidget(widget);
    m_tooltip->resize( m_tooltip->sizeHint() );

    connect(widget, &WorkingSetToolTipWidget::shouldClose, m_tooltip.data(), &ActiveToolTip::close);

    ActiveToolTip::showToolTip(m_tooltip);
}

void WorkingSetController::showGlobalToolTip()
{
    auto* window = static_cast<KDevelop::MainWindow*>(Core::self()->uiControllerInternal()->activeMainWindow());

    showToolTip(workingSet(window->area()->workingSet()),
                              window->mapToGlobal(window->geometry().topRight()));

    connect(m_hideToolTipTimer, &QTimer::timeout,  m_tooltip.data(), &ActiveToolTip::deleteLater);
    m_hideToolTipTimer->start();
    connect(m_tooltip.data(), &ActiveToolTip::mouseIn, m_hideToolTipTimer, &QTimer::stop);
    connect(m_tooltip.data(), &ActiveToolTip::mouseOut, m_hideToolTipTimer, QOverload<>::of(&QTimer::start));
}

WorkingSetToolTipWidget* WorkingSetController::workingSetToolTip()
{
    if(!m_tooltip)
        showGlobalToolTip();

    m_hideToolTipTimer->start();

    if(m_tooltip)
    {
        auto* widget = m_tooltip->findChild<WorkingSetToolTipWidget*>();
        Q_ASSERT(widget);
        return widget;
    }
    return nullptr;
}

void WorkingSetController::nextDocument()
{
    auto widget = workingSetToolTip();
    if (widget) {
        widget->nextDocument();
    }
}

void WorkingSetController::previousDocument()
{
    auto widget = workingSetToolTip();
    if (widget) {
        widget->previousDocument();
    }
}

void WorkingSetController::initializeController( UiController* controller )
{
  connect( controller, &UiController::areaCreated, this, &WorkingSetController::areaCreated );
}

QList< WorkingSet* > WorkingSetController::allWorkingSets() const
{
  return m_workingSets.values();
}

void WorkingSetController::areaCreated( Sublime::Area* area )
{
    if (!area->workingSet().isEmpty()) {
        WorkingSet* set = workingSet( area->workingSet() );
        set->connectArea( area );
    }

    connect(area, &Sublime::Area::changingWorkingSet,
            this, &WorkingSetController::changingWorkingSet);
    connect(area, &Sublime::Area::changedWorkingSet,
            this, &WorkingSetController::changedWorkingSet);
    connect(area, &Sublime::Area::viewAdded,
            this, &WorkingSetController::viewAdded);
    connect(area, &Sublime::Area::clearWorkingSet,
            this, &WorkingSetController::clearWorkingSet);
}

void WorkingSetController::changingWorkingSet(Sublime::Area* area, const QString& from, const QString& to)
{
    qCDebug(SHELL) << "changing working-set from" << from << "to" << to << "area" << area;
    if (from == to)
        return;

    if (!from.isEmpty()) {
        WorkingSet* oldSet = workingSet(from);
        oldSet->disconnectArea(area);
        if (!oldSet->id().isEmpty()) {
            oldSet->saveFromArea(area, area->rootIndex());
        }
    }
}

void WorkingSetController::changedWorkingSet(Sublime::Area* area, const QString& from, const QString& to)
{
    qCDebug(SHELL) << "changed working-set from" << from << "to" << to << "area" << area;
    if (from == to || m_changingWorkingSet)
        return;

    if (!to.isEmpty()) {
        WorkingSet* newSet = workingSet(to);
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
    auto* area = qobject_cast< Sublime::Area* >(sender());
    Q_ASSERT(area);

    if (area->workingSet().isEmpty()) {
        //Spawn a new working-set
        m_changingWorkingSet = true;
        WorkingSet* set = Core::self()->workingSetControllerInternal()->newWorkingSet(area->objectName());
        qCDebug(SHELL) << "Spawned new working-set" << set->id() << "because a view was added";
        set->connectArea(area);
        set->saveFromArea(area, area->rootIndex());
        area->setWorkingSet(set->id());
        m_changingWorkingSet = false;
    }
}

void WorkingSetController::clearWorkingSet(Sublime::Area * area)
{
    const QString workingSetId = area->workingSet();
    if (workingSetId.isEmpty()) {
        // Nothing to do - area has no working set
        return;
    }

    WorkingSet* set = workingSet(workingSetId);
    set->deleteSet(true);

    WorkingSet* newSet = workingSet(workingSetId);
    newSet->connectArea(area);
    newSet->loadToArea(area, area->rootIndex());
    Q_ASSERT(newSet->fileList().isEmpty());
}
