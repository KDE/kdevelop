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

#include "workingsettoolbutton.h"

#include <QMenu>
#include <QContextMenuEvent>

#include <KLocalizedString>
#include <KColorUtils>

#include <sublime/mainwindow.h>
#include <sublime/area.h>

#include "core.h"
#include "mainwindow.h"
#include "workingset.h"
#include "workingsetcontroller.h"
#include "workingsethelpers.h"
#include "documentcontroller.h"

#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <util/activetooltip.h>

using namespace KDevelop;

WorkingSetToolButton::WorkingSetToolButton(QWidget* parent, WorkingSet* set)
    : QToolButton(parent), m_set(set), m_toolTipEnabled(true)
{
    setFocusPolicy(Qt::NoFocus);
    setWorkingSet(set);

    connect(this, SIGNAL(clicked(bool)), SLOT(buttonTriggered()));
}

WorkingSet* WorkingSetToolButton::workingSet() const
{
    return m_set;
}

void WorkingSetToolButton::setWorkingSet(WorkingSet* set)
{
    m_set = set;

    if (!set) {
        setIcon(QIcon());
        return;
    }

    setIcon(set->icon());
}

void WorkingSetToolButton::contextMenuEvent(QContextMenuEvent* ev)
{
    showTooltip();

    ev->accept();
}

void WorkingSetToolButton::intersectSet()
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);

    filterViews(Core::self()->workingSetControllerInternal()->getWorkingSet(mainWindow()->area()->workingSet())->fileList().toSet() & m_set->fileList().toSet());
}

void WorkingSetToolButton::subtractSet()
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);

    filterViews(Core::self()->workingSetControllerInternal()->getWorkingSet(mainWindow()->area()->workingSet())->fileList().toSet() - m_set->fileList().toSet());
}

void WorkingSetToolButton::mergeSet()
{
    Q_ASSERT(m_set);

    QSet< QString > loadFiles = m_set->fileList().toSet() - Core::self()->workingSetControllerInternal()->getWorkingSet(mainWindow()->area()->workingSet())->fileList().toSet();
    foreach(const QString& file, loadFiles)
        Core::self()->documentController()->openDocument(file);
}

void WorkingSetToolButton::duplicateSet()
{
    Q_ASSERT(m_set);

    if(!Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(mainWindow(), KDevelop::IDocument::Default, true))
        return;
    WorkingSet* set = Core::self()->workingSetControllerInternal()->newWorkingSet("clone");
    set->setPersistent(true);
    set->saveFromArea(mainWindow()->area(), mainWindow()->area()->rootIndex());
    mainWindow()->area()->setWorkingSet(set->id());
}

void WorkingSetToolButton::loadSet()
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);

    if(!Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(mainWindow(), KDevelop::IDocument::Default, true))
        return;
    mainWindow()->area()->setWorkingSet(QString(m_set->id()));
}

void WorkingSetToolButton::closeSet(bool ask)
{
    Q_ASSERT(m_set);

    m_set->setPersistent(true);
    m_set->saveFromArea(mainWindow()->area(), mainWindow()->area()->rootIndex());

    if(ask && !Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(mainWindow(), KDevelop::IDocument::Default, true))
        return;
    mainWindow()->area()->setWorkingSet(QString());
}

bool WorkingSetToolButton::event(QEvent* e)
{
    if(m_toolTipEnabled && e->type() == QEvent::ToolTip) {
        showTooltip();
        e->accept();
        return true;
    }
    
    return QToolButton::event(e);
}

void WorkingSetToolButton::showTooltip()
{
    Q_ASSERT(m_set);
    static WorkingSetToolButton* oldTooltipButton;

    WorkingSetController* controller = Core::self()->workingSetControllerInternal();

    if(controller->tooltip() && oldTooltipButton == this)
        return;

    oldTooltipButton = this;

    controller->showToolTip(m_set, QCursor::pos() + QPoint(10, 20));

    QRect extended(parentWidget()->mapToGlobal(geometry().topLeft()),
                    parentWidget()->mapToGlobal(geometry().bottomRight()));
    controller->tooltip()->addExtendRect(extended);
}

void WorkingSetToolButton::buttonTriggered()
{
    Q_ASSERT(m_set);

    if(mainWindow()->area()->workingSet() == m_set->id()) {
        showTooltip();
    }else{
        //Only close the working-set if the file was saved before
        if(!Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(mainWindow(), KDevelop::IDocument::Default, true))
            return;
        m_set->setPersistent(true);
        mainWindow()->area()->setWorkingSet(m_set->id());
    }
}

#include "workingsettoolbutton.moc"
