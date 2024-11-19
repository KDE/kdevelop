/*
    SPDX-FileCopyrightText: David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "workingsettoolbutton.h"

#include <QContextMenuEvent>

#include <sublime/mainwindow.h>
#include <sublime/area.h>

#include "core.h"
#include "mainwindow.h"
#include "workingset.h"
#include "workingsetcontroller.h"
#include "workingsethelpers.h"
#include "documentcontroller.h"

#include <interfaces/idocumentcontroller.h>
#include <util/activetooltip.h>

using namespace KDevelop;

namespace {
bool saveAllDocumentsForMainWindow()
{
    return Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(
        mainWindow(), IDocumentController::SaveSelectionMode::LetUserSelect, true);
}
}

WorkingSetToolButton::WorkingSetToolButton(QWidget* parent, WorkingSet* set)
    : QToolButton(parent), m_set(set), m_toolTipEnabled(true)
{
    setFocusPolicy(Qt::NoFocus);
    setWorkingSet(set);
    setAutoRaise(true);

    connect(this, &WorkingSetToolButton::clicked, this, &WorkingSetToolButton::buttonTriggered);
}

WorkingSet* WorkingSetToolButton::workingSet() const
{
    return m_set;
}

void WorkingSetToolButton::setWorkingSet(WorkingSet* set)
{
    m_set = set;

    setIcon(set ? set->icon() : QIcon());
}

void WorkingSetToolButton::contextMenuEvent(QContextMenuEvent* ev)
{
    showTooltip(ev->globalPos());

    ev->accept();
}

void WorkingSetToolButton::intersectSet()
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);

    filterViews(Core::self()->workingSetControllerInternal()->workingSet(mainWindow()->area()->workingSet())->fileSet() & m_set->fileSet());
}

void WorkingSetToolButton::subtractSet()
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);

    filterViews(Core::self()->workingSetControllerInternal()->workingSet(mainWindow()->area()->workingSet())->fileSet() - m_set->fileSet());
}

void WorkingSetToolButton::mergeSet()
{
    Q_ASSERT(m_set);

    const QSet<QString> loadFiles = m_set->fileSet() - Core::self()->workingSetControllerInternal()->workingSet(mainWindow()->area()->workingSet())->fileSet();
    for (const QString& file : loadFiles) {
        Core::self()->documentController()->openDocument(QUrl::fromUserInput(file));
    }
}

void WorkingSetToolButton::duplicateSet()
{
    Q_ASSERT(m_set);

    if (!saveAllDocumentsForMainWindow())
        return;
    WorkingSet* set = Core::self()->workingSetControllerInternal()->newWorkingSet(QStringLiteral("clone"));
    set->setPersistent(true);
    set->saveFromArea(mainWindow()->area());
    mainWindow()->area()->setWorkingSet(set->id());
}

void WorkingSetToolButton::loadSet()
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);

    if (!saveAllDocumentsForMainWindow())
        return;
    mainWindow()->area()->setWorkingSet(QString(m_set->id()));
}

void WorkingSetToolButton::closeSet()
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);
    m_set->saveFromArea(mainWindow()->area());

    if (!saveAllDocumentsForMainWindow())
        return;
    mainWindow()->area()->setWorkingSet(QString());
}

bool WorkingSetToolButton::event(QEvent* e)
{
    if(m_toolTipEnabled && e->type() == QEvent::ToolTip) {
        auto* helpEvent = static_cast<QHelpEvent*>(e);
        showTooltip(helpEvent->globalPos());
        e->accept();
        return true;
    }

    return QToolButton::event(e);
}

void WorkingSetToolButton::showTooltip(const QPoint& globalPos)
{
    Q_ASSERT(m_set);
    static WorkingSetToolButton* oldTooltipButton;

    WorkingSetController* controller = Core::self()->workingSetControllerInternal();

    if(controller->tooltip() && oldTooltipButton == this)
        return;

    oldTooltipButton = this;

    controller->showToolTip(m_set, globalPos + QPoint(10, 20));

    QRect extended(parentWidget()->mapToGlobal(geometry().topLeft()),
                    parentWidget()->mapToGlobal(geometry().bottomRight()));
    controller->tooltip()->setHandleRect(extended);
}

void WorkingSetToolButton::buttonTriggered()
{
    Q_ASSERT(m_set);

    if(mainWindow()->area()->workingSet() == m_set->id()) {
        showTooltip(QCursor::pos());
    }else{
        //Only close the working-set if the file was saved before
        if (!saveAllDocumentsForMainWindow())
            return;
        m_set->setPersistent(true);
        mainWindow()->area()->setWorkingSet(m_set->id());
    }
}

#include "moc_workingsettoolbutton.cpp"
