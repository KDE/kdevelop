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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "dmainwindow.h"

#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>

#include <qtoolbutton.h>

#include "dtabwidget.h"
#include "docksplitter.h"

DMainWindow::DMainWindow(QWidget *parent, const char *name)
    :KParts::MainWindow(parent, name), m_currentWidget(0)
{
    loadSettings();
    createToolWindows();
    m_central = new Ideal::DockSplitter(Qt::Horizontal, this);
    m_activeTabWidget = createTab();
    m_central->addDock(0, 0, m_activeTabWidget);
    setCentralWidget(m_central);
}

void DMainWindow::loadSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("UI");
    m_openTabAfterCurrent = config->readBoolEntry("OpenNewTabAfterCurrent", true);
    m_showIconsOnTabs = config->readBoolEntry("ShowTabIcons", false);
}

DMainWindow::~DMainWindow()
{
/*    for (QValueList<QWidget*>::iterator it = m_widgets.begin(); it != m_widgets.end(); ++it)
        removeWidget(*it);*/
}

DDockWindow *DMainWindow::toolWindow(DDockWindow::Position position) const
{
    switch (position) {
        case DDockWindow::Bottom: return m_bottomDock;
        case DDockWindow::Left: return m_leftDock;
        case DDockWindow::Right: return m_rightDock;
    }
    return 0;
}

void DMainWindow::createToolWindows()
{
    m_bottomDock = new DDockWindow(this, DDockWindow::Bottom);
    moveDockWindow(m_bottomDock, Qt::DockBottom);
    m_leftDock = new DDockWindow(this, DDockWindow::Left);
    moveDockWindow(m_leftDock, Qt::DockLeft);
    m_rightDock = new DDockWindow(this, DDockWindow::Right);
    moveDockWindow(m_rightDock, Qt::DockRight);
}

void DMainWindow::addWidget(QWidget *widget, const QString &title)
{
    invalidateActiveTabWidget();
    addWidget(m_activeTabWidget, widget, title);
}

void DMainWindow::addWidget(DTabWidget *tab, QWidget *widget, const QString &title)
{
    int idx = -1;
    if (m_openTabAfterCurrent && (tab->count() > 0))
        idx = tab->currentPageIndex() + 1;
    if (m_showIconsOnTabs)
    {
        const QPixmap *pixmap = widget->icon();
        const QIconSet &icons = (pixmap && (pixmap->size().height() <= 16)) ? *(pixmap) : SmallIcon("kdevelop");
        tab->insertTab(widget, icons, title, idx);
    }
    else
    {
        kdDebug() << "tab exists with number of widgets " << tab->count() << endl;
        tab->insertTab(widget, title, idx);
    }
    m_widgets.append(widget);
    m_widgetTabs[widget] = tab;
    widget->installEventFilter(this);
    tab->showPage(widget);
}

void DMainWindow::removeWidget(QWidget *widget)
{
    if (!m_widgets.contains(widget))
        return; //not a widget in main window
    
    if (m_widgetTabs.contains(widget))
    {
        DTabWidget *tab = m_widgetTabs[widget];
        if (tab->indexOf(widget) >= 0)
        {
            tab->removePage(widget);
            if (tab->count() == 0)
            {
                tab->closeButton()->hide();
                //@fixme only secondary tabwidgets should be removed
/*                QPair<uint, uint> idx = m_central->indexOf(tab);
                m_central->removeDock(idx.first, idx.second, true);*/
            }
        }
    }
    
    m_widgets.remove(widget);
    m_widgetTabs.remove(widget);
}

DTabWidget *DMainWindow::splitHorizontal() 
{
    m_activeTabWidget = createTab();
    m_central->addDock(m_central->numRows(), 0, m_activeTabWidget);
    return m_activeTabWidget;
}

DTabWidget *DMainWindow::splitVertical() 
{
    invalidateActiveTabWidget();
    int row = m_central->indexOf(m_activeTabWidget).first;
    m_activeTabWidget = createTab();
    m_central->addDock(row, m_central->numCols(row), m_activeTabWidget);
    return m_activeTabWidget;
}

void DMainWindow::invalidateActiveTabWidget()
{
    QWidget *focused = m_central->focusWidget();
    if (focused == 0)
        return;
    if (!m_widgets.contains(focused))
        return;
    if (m_widgetTabs.contains(focused))
    {
        DTabWidget *tab = m_widgetTabs[focused];
        if (tab->indexOf(focused) >= 0)
            m_activeTabWidget = tab;
    }
}

DTabWidget *DMainWindow::createTab()
{
    DTabWidget *tab = new DTabWidget(m_central);
    m_tabs.append(tab);
    if (tab->closeButton())
        connect(tab->closeButton(), SIGNAL(clicked()), this, SLOT(closeTab()));
    connect(tab, SIGNAL(contextMenu(QWidget*,const QPoint &)), 
        this, SLOT(tabContext(QWidget*,const QPoint &)));
    return tab;
}

bool DMainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    QWidget *w = (QWidget*)obj;
    if (!m_widgets.contains(w))
        return KParts::MainWindow::eventFilter(obj, ev);
    
    if ((m_currentWidget != w) && (ev->type() == QEvent::FocusIn))
    {
        m_currentWidget = w;
        emit widgetChanged(w);
    }
    else if (ev->type() == QEvent::IconChange)
    {
        if (m_widgetTabs.contains(w))
        {
            DTabWidget *tab = m_widgetTabs[w];
            tab->setTabIconSet(w, w->icon() ? (*(w->icon())) : QPixmap());
        }
    }
    else if (ev->type() == QEvent::CaptionChange)
    {
        kdDebug() << "caption change" << endl;
    }

    return KParts::MainWindow::eventFilter(obj, ev);
}

void DMainWindow::closeTab()
{
    //nothing to do here, should be reimplemented
}

void DMainWindow::tabContext(QWidget *, const QPoint &)
{
    //nothing to do here, should be reimplemented
}

#include "dmainwindow.moc"
