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


QString htmlColorElement(int element) {
    QString ret = QString("%1").arg(element, 2, 16, QChar('0'));
    return ret;
}

QString htmlColor(QColor color) {
    return '#' + htmlColorElement(color.red()) + htmlColorElement(color.green()) + htmlColorElement(color.blue());
}

WorkingSetToolButton::WorkingSetToolButton(QWidget* parent, WorkingSet* set, MainWindow* mainWindow)
    : QToolButton(parent), m_set(set), m_mainWindow(mainWindow), m_toolTipEnabled(true)
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

    QColor activeBgColor = palette().color(QPalette::Active, QPalette::Highlight);
    QColor normalBgColor = palette().color(QPalette::Active, QPalette::Base);
    QColor useColor;
    if(m_mainWindow && m_mainWindow->area() && m_mainWindow->area()->workingSet() == set->id()) {
        useColor = KColorUtils::mix(normalBgColor, activeBgColor, 0.6);
        setIcon(set->activeIcon());
    }else{
        useColor = KColorUtils::mix(normalBgColor, activeBgColor, 0.2);
        setIcon(set->inactiveIcon());
    }

    QString sheet = QString("QToolButton { background : %1}").arg(htmlColor(useColor));
    setStyleSheet(sheet);
}

void WorkingSetToolButton::contextMenuEvent(QContextMenuEvent* ev)
{
    Q_ASSERT(m_set);

    QToolButton::contextMenuEvent(ev);

    QMenu menu;
    Sublime::MainWindow* mainWindow = dynamic_cast<Sublime::MainWindow*>(Core::self()->uiController()->activeMainWindow());
    Q_ASSERT(mainWindow);
    if(m_set->id() == mainWindow->area()->workingSet()) {
        menu.addAction(i18n("Close Working Set (Left Click)"), this, SLOT(closeSet()));
        menu.addAction(i18n("Duplicate Working Set"), this, SLOT(duplicateSet()));
    }else{
        menu.addAction(i18n("Load Working Set (Left Click)"), this, SLOT(loadSet()));
//         menu.addAction(i18n("Merge Working Set"), this, SLOT(mergeSet()));
//         menu.addSeparator();
//         menu.addAction(i18n("Intersect Working Set"), this, SLOT(intersectSet()));
//         menu.addAction(i18n("Subtract Working Set"), this, SLOT(subtractSet()));
    }
    menu.actions()[0]->setIcon(KIcon(m_set->iconName()));

    if(!m_set->hasConnectedAreas()) {
        menu.addSeparator();
        menu.addAction(i18n("Delete Working Set"), m_set, SLOT(deleteSet()));
    }
    menu.exec(ev->globalPos());

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
        Q_ASSERT(m_set);

        e->accept();
        static WorkingSetToolButton* oldTooltipButton;

        WorkingSetController* controller = Core::self()->workingSetControllerInternal();

        if(controller->tooltip() && oldTooltipButton == this)
            return true;

        oldTooltipButton = this;

        controller->showToolTip(m_set, QCursor::pos() + QPoint(10, 20));

        QRect extended(parentWidget()->mapToGlobal(geometry().topLeft()),
                       parentWidget()->mapToGlobal(geometry().bottomRight()));
        controller->tooltip()->addExtendRect(extended);

        return true;
    }
    return QToolButton::event(e);
}

void WorkingSetToolButton::buttonTriggered()
{
    Q_ASSERT(m_set);

    //Only close the working-set if the file was saved before
    if(!Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(mainWindow(), KDevelop::IDocument::Default, true))
        return;

    if(mainWindow()->area()->workingSet() == m_set->id()) {
        closeSet(false);
    }else{
        m_set->setPersistent(true);
        mainWindow()->area()->setWorkingSet(m_set->id());
    }
}

#include "workingsettoolbutton.moc"
