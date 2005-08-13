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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "dtabwidget.h"

#include <qtoolbutton.h>
#include <qtabbar.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <kapplication.h>

DTabWidget::DTabWidget(QWidget *parent, const char *name)
    :KTabWidget(parent, name), m_closeButton(0)
{
    setFocusPolicy(NoFocus);
    setMargin(0);

    loadSettings();
    
    if (!m_tabBarShown)
        tabBar()->hide();
    else {
        m_closeButton = new QToolButton(this);
        m_closeButton->setIconSet(SmallIcon("tab_remove"));
        m_closeButton->adjustSize();
        m_closeButton->hide();
        setCornerWidget(m_closeButton, TopRight);
        
        if (m_closeOnHover)
            setHoverCloseButton(true);

        setTabReorderingEnabled(true);
    }
    
    connect(this, SIGNAL(currentChanged(QWidget*)), this, SLOT(setFocus(QWidget*)));
//    connect(this, SIGNAL(currentChanged(QWidget*)), this, SLOT(updateHistory(QWidget*)));
}

void DTabWidget::loadSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("UI");
//    m_tabBarShown = config->readBoolEntry("TabBarShown", true);
    m_tabBarShown = ! config->readNumEntry("TabWidgetVisibility", 0);
    m_closeOnHover = config->readBoolEntry("CloseOnHover", false);
    m_closeButtonShown = config->readBoolEntry("ShowCloseTabsButton", true);
    //we do not delay hover close buttons - that looks and feels ugly
    setHoverCloseButtonDelayed(false);
}

void DTabWidget::saveSettings()
{
}

QToolButton *DTabWidget::closeButton() const
{
    return m_closeButton;
}

void DTabWidget::setFocus(QWidget *w)
{
    if (w)
        w->setFocus();
}

void DTabWidget::insertTab(QWidget *child, const QString &label, int index)
{
    if (m_closeButton && m_closeButtonShown)
        m_closeButton->show();
    KTabWidget::insertTab(child, label, index);
    if (index != -1) tabBar()->repaint();
}

void DTabWidget::insertTab(QWidget *child, const QIconSet &iconset, 
    const QString &label, int index)
{
    if (m_closeButton && m_closeButtonShown)
        m_closeButton->show();
    KTabWidget::insertTab(child, iconset, label, index);
    if (index != -1) tabBar()->repaint();
}

/*void DTabWidget::updateHistory(QWidget *w)
{
    if (m_history.top() != w)
        m_history.push(w);
}*/

#include "dtabwidget.moc"
