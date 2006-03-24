/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "ddockwindow.h"

#include <QStackedWidget>
#include <QBoxLayout>
#include <QToolBar>
#include <QResizeEvent>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kaction.h>
#include <kmainwindow.h>

DDockWidget::DDockWidget(Qt::DockWidgetArea area, KMainWindow* mainWindow)
    : QDockWidget(mainWindow)
    , m_selectedAction(0)
    , m_expanded(false)
    , m_area(area)
{
    setFeatures(NoDockWidgetFeatures);

    QWidget* container = new QWidget(this);
    setWidget(container);

    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, container);
    layout->setMargin(0);
    layout->setSpacing(0);
    switch (area) {
        default:
        case Qt::TopDockWidgetArea:
            setObjectName("TopToolWindow");
            layout->setDirection(QBoxLayout::BottomToTop);
            break;
        case Qt::LeftDockWidgetArea:
            setObjectName("LeftToolWindow");
            layout->setDirection(QBoxLayout::RightToLeft);
            break;
        case Qt::RightDockWidgetArea:
            setObjectName("RightToolWindow");
            layout->setDirection(QBoxLayout::LeftToRight);
            break;
        case Qt::BottomDockWidgetArea:
            setObjectName("BottomToolWindow");
            layout->setDirection(QBoxLayout::TopToBottom);
            break;
    }

    m_widgetStack = new QStackedWidget(container);
    QSizePolicy wsp = m_widgetStack->sizePolicy();

    m_selectionBar = new QToolBar(container);
    QSizePolicy ssp = m_selectionBar->sizePolicy();

    switch (area) {
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            m_selectionBar->setOrientation(Qt::Vertical);
            wsp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
            ssp.setHorizontalPolicy(QSizePolicy::Fixed);
            break;

        default:
            wsp.setVerticalPolicy(QSizePolicy::MinimumExpanding);
            ssp.setVerticalPolicy(QSizePolicy::Fixed);
            break;
    }

    m_widgetStack->setSizePolicy(wsp);
    m_selectionBar->setSizePolicy(ssp);

    /*KConfig *config = KGlobal::config();
    config->setGroup("UI");
    int mode = config->readEntry("MDIStyle", 3);
    m_selectionBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    if (mode == 0)
        m_selectionBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    else if (mode == 1)
        m_selectionBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    else if (mode == 3)
        m_selectionBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);*/

    m_selectionBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    layout->addWidget(m_widgetStack);
    layout->addWidget(m_selectionBar);

    m_selectActionGroup = new QActionGroup(this);
    m_selectActionGroup->setExclusive(false);

    mainWindow->addDockWidget(area, this);

    setExpanded(m_expanded);

    loadSettings();

    connect(m_selectActionGroup, SIGNAL(triggered(QAction*)), SLOT(selectWidget(QAction*)));
}

DDockWidget::~DDockWidget()
{
    saveSettings();
}

void DDockWidget::setExpanded(bool v)
{
    //write dock width to the config file
    KConfig *config = KGlobal::config();
    QString group = QString("%1").arg(objectName());
    config->setGroup(group);

    bool isBottom = (m_area == Qt::BottomDockWidgetArea);

    if (m_expanded)
        config->writeEntry("ViewWidth", isBottom ? height() : width() );

    m_widgetStack->setVisible(v);
    m_expanded = v;

    if (!m_expanded)
    {
        widget()->setMinimumSize(m_selectionBar->minimumSize());
        if (isBottom)
            resize(width(), layout()->minimumSize().height());
        else
            resize(layout()->minimumSize().width(), height());
    }
    else
    {
        //restore widget size from the config
        int size = 0;
        if (isBottom)
        {
            size = config->readEntry("ViewWidth", layout()->minimumSize().height());
            resize(width(), size);
        }
        else
        {
            size = config->readEntry("ViewWidth", layout()->minimumSize().width());
            resize(size, height());
        }
    }
}

void DDockWidget::loadSettings()
{
}

void DDockWidget::saveSettings()
{
    KConfig *config = KGlobal::config();
    QString group = QString("%1").arg(objectName());
    int invisibleWidth = 0;
    config->setGroup(group);
    if (config->hasKey("ViewWidth"))
        invisibleWidth = config->readEntry("ViewWidth", 0);
    config->deleteEntry("ViewWidth");
    config->deleteEntry("ViewLastWidget");
    if (m_selectedAction && m_expanded)
    {
        config->writeEntry("ViewWidth", (m_area == Qt::BottomDockWidgetArea) ? height() : width());
        config->writeEntry("ViewLastWidget", m_selectedAction->text());
    }
    else if (invisibleWidth != 0)
        config->writeEntry("ViewWidth", invisibleWidth);
}

QWidget *DDockWidget::currentWidget() const
{
    return m_widgetStack->currentWidget();
}

void DDockWidget::addWidget(const QString &title, QWidget *widget)
{
    KAction* action = new KAction(title, widget->windowIcon(), 0, 0, 0, mainWindow()->actionCollection(), QString("dock_select_%1").arg(title).toLatin1().constData());
    action->setActionGroup(m_selectActionGroup);
    action->setCheckable(true);
    m_widgets[action] = widget;
    m_actions[widget] = action;
    m_selectionBar->addAction(action);

    m_widgetStack->addWidget(widget);

    //if the widget was selected last time the dock is deleted
    //we need to show it
    KConfig *config = KGlobal::config();
    QString group = QString("%1").arg(objectName());
    config->setGroup(group);
    if (config->readEntry("ViewLastWidget") == title)
    {
        kDebug() << k_funcinfo << " : activating last widget " << title << endl;
        action->setChecked(true);
        selectWidget(action);
    }
}

void DDockWidget::raiseWidget(QWidget *widget)
{
    kDebug() << k_funcinfo << endl;
    KAction* action = m_actions[widget];
    if (action && (!action->isChecked()))
        action->trigger();
}

void DDockWidget::removeWidget(QWidget *widget)
{
    kDebug() << k_funcinfo << endl;
    if (m_widgetStack->indexOf(widget) == -1)
        return; //not in dock

    bool changeVisibility = false;
    if (m_widgetStack->currentWidget() == widget)
        changeVisibility = true;

    KAction* action = m_actions.take(widget);
    m_widgets.remove(action);
    delete action;
    m_selectedAction = 0;

    m_widgetStack->removeWidget(widget);

    if (changeVisibility)
    {
        m_selectedAction = 0;
        setExpanded(false);
    }
}

void DDockWidget::selectWidget(QAction* qaction)
{
    KAction* action = qobject_cast<KAction*>(qaction);
    if (!action)
      return;

    if (m_selectedAction == action)
    {
        setExpanded(!m_expanded);
        return;
    }

    setSelectedAction(action);

    setExpanded(true);

    m_widgetStack->setCurrentWidget(m_widgets[action]);
}

void DDockWidget::hideWidget(QWidget *widget)
{
    KAction* action = m_actions[widget];
    if (action)
    {
        action->setChecked(false);
        action->setVisible(false);
    }
    widget->hide();
    if (action == m_selectedAction)
        setExpanded(false);
}

void DDockWidget::showWidget(QWidget *widget)
{
    if (KAction* action = m_actions[widget])
        action->setVisible(true);

    widget->show();
}

KMainWindow * DDockWidget::mainWindow( ) const
{
    return static_cast<KMainWindow*>(const_cast<QObject*>(parent()));
}

void DDockWidget::setSelectedAction( KAction * action )
{
    if (m_selectedAction) {
        m_selectedAction->blockSignals(true);
        m_selectedAction->setChecked(false);
        m_selectedAction->blockSignals(false);
    }

    m_selectedAction = action;

    /*if (m_selectedAction) {
        m_selectedAction->blockSignals(true);
        static_cast<QToolButton*>(m_selectionBar->widgetForAction(m_selectedAction))->setDown(true);
        m_selectedAction->blockSignals(false);
    }*/
}

void DDockWidget::resizeEvent( QResizeEvent * event )
{
  kDebug() << k_funcinfo << event->oldSize() << " new " << event->size() << endl;
  QDockWidget::resizeEvent(event);
}

#include "ddockwindow.moc"
