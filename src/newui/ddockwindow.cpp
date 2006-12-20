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
#include "ddockwindow.h"

#include <qtoolbutton.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qwidgetstack.h>
#include <qimage.h>
#include <qapplication.h>
#include <qpopupmenu.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "buttonbar.h"
#include "button.h"
#include "dmainwindow.h"

DDockWindow::DDockWindow(DMainWindow *parent, Position position)
    :QDockWindow(QDockWindow::InDock, parent), m_position(position), m_visible(false),
    m_mainWindow(parent), m_doNotCloseActiveWidget(false), m_toggledButton(0), m_lastContextMenuButton(0)
{
    setMovingEnabled(false);
    setResizeEnabled(true);

    Ideal::Place place = Ideal::Left;
    switch (position) {
        case DDockWindow::Bottom:
            m_name = "BottomToolWindow";
            place = Ideal::Bottom;
            m_internalLayout = new QVBoxLayout(boxLayout(), 0);
            m_internalLayout->setDirection(QBoxLayout::BottomToTop);
            break;
        case DDockWindow::Left:
            m_name = "LeftToolWindow";
            place = Ideal::Left;
            m_internalLayout = new QHBoxLayout(boxLayout(), 0);
            m_internalLayout->setDirection(QBoxLayout::LeftToRight);
            break;
        case DDockWindow::Right:
            m_name = "RightToolWindow";
            place = Ideal::Right;
            m_internalLayout = new QHBoxLayout(boxLayout(), 0);
            m_internalLayout->setDirection(QBoxLayout::RightToLeft);
            break;
    }

    KConfig *config = kapp->config();
    config->setGroup("UI");
    int mode = config->readNumEntry("MDIStyle", 3);
    Ideal::ButtonMode buttonMode = Ideal::Text;
    if (mode == 0)
        buttonMode = Ideal::Icons;
    else if (mode == 1)
        buttonMode = Ideal::Text;
    else if (mode == 3)
        buttonMode = Ideal::IconsAndText;

    m_bar = new Ideal::ButtonBar(place, buttonMode, this);
    m_internalLayout->addWidget(m_bar);

    m_widgetStack = new QWidgetStack(this);
    m_internalLayout->addWidget(m_widgetStack);

    m_moveToDockLeft = new KAction( i18n("Move to left dock"), 0, this, SLOT(moveToDockLeft()), this );
    m_moveToDockRight = new KAction( i18n("Move to right dock"), 0, this, SLOT(moveToDockRight()), this );
    m_moveToDockBottom = new KAction( i18n("Move to bottom dock"), 0, this, SLOT(moveToDockBottom()), this );

    setVisible(m_visible);

    loadSettings();
}

DDockWindow::~DDockWindow()
{
//done in DMainWindow now
//    saveSettings();
}

void DDockWindow::setVisible(bool v)
{
    //write dock width to the config file
    KConfig *config = kapp->config();
    QString group = QString("%1").arg(m_name);
    config->setGroup(group);

    if (m_visible)
        config->writeEntry("ViewWidth", m_position == DDockWindow::Bottom ? height() : width() );
    setResizeEnabled(v);
    setVerticallyStretchable(true);
    setHorizontallyStretchable(true);
    v ? m_widgetStack->show() : m_widgetStack->hide();
    m_visible = v;

    m_internalLayout->invalidate();
    if (!m_visible)
    {
        if (m_position == DDockWindow::Bottom)
            setFixedExtentHeight(m_internalLayout->sizeHint().height());
        else
            setFixedExtentWidth(m_internalLayout->sizeHint().width());
        emit hidden();
    }
    else
    {
        //restore widget size from the config
        int size = 0;
        if (m_position == DDockWindow::Bottom)
        {
            size = config->readNumEntry("ViewWidth", m_internalLayout->sizeHint().height());
            setFixedExtentHeight(size);
        }
        else
        {
            size = config->readNumEntry("ViewWidth", m_internalLayout->sizeHint().width());
            setFixedExtentWidth(size);
        }
    }
}

void DDockWindow::loadSettings()
{
}

void DDockWindow::saveSettings()
{
    KConfig *config = kapp->config();
    QString group = QString("%1").arg(m_name);
    int invisibleWidth = 0;
    config->setGroup(group);
    if (config->hasKey("ViewWidth"))
        invisibleWidth = config->readNumEntry("ViewWidth");
    config->deleteEntry("ViewWidth");
    config->deleteEntry("ViewLastWidget");
    if (m_toggledButton && m_visible)
    {
        config->writeEntry("ViewWidth", m_position == DDockWindow::Bottom ? height() : width());
        config->writeEntry("ViewLastWidget", m_toggledButton->realTextWithoutAccel());
    }
    else if (invisibleWidth != 0)
        config->writeEntry("ViewWidth", invisibleWidth);
}

QWidget *DDockWindow::currentWidget() const
{
    return m_widgetStack->visibleWidget();
}

void DDockWindow::addWidget(const QString &title, QWidget *widget, bool skipActivation)
{
    kdDebug(9000) << k_funcinfo << endl;
    QPixmap *pm = const_cast<QPixmap*>(widget->icon());
    Ideal::Button *button;
    if (pm != 0)
    {
        //force 16pt for now
        if (pm->height() > 16)
        {
            QImage img = pm->convertToImage();
            img = img.smoothScale(16, 16);
            pm->convertFromImage(img);
        }
        button = new Ideal::Button(m_bar, title, *pm);
    }
    else
        button = new Ideal::Button(m_bar, title);
    m_widgets[button] = widget;
    m_buttons[widget] = button;
    m_bar->addButton(button);

    m_widgetStack->addWidget(widget);
    connect(button, SIGNAL(clicked()), this, SLOT(selectWidget()));
    connect(button, SIGNAL(contextMenu(QPopupMenu*)), this, SLOT(contextMenu(QPopupMenu*)) );

    if (!skipActivation)
    {
        //if the widget was selected last time the dock is deleted
        //we need to show it
        KConfig *config = kapp->config();
        QString group = QString("%1").arg(m_name);
        config->setGroup(group);
        if (config->readEntry("ViewLastWidget") == title)
        {
            kdDebug(9000) << k_funcinfo << " : activating last widget " << title << endl;
            button->setOn(true);
            selectWidget(button);
        }
    }
}

void DDockWindow::raiseWidget(QWidget *widget)
{
    kdDebug(9000) << k_funcinfo << endl;

    if ( !widget ) return;

    Ideal::Button *button = m_buttons[widget];
    if ((button != 0) && (!button->isOn()))
    {
        button->setOn(true);
        selectWidget(button);
    }
}

void DDockWindow::lowerWidget(QWidget * widget)
{
    kdDebug(9000) << k_funcinfo << endl;

    if ( !widget ) return;

    Ideal::Button *button = m_buttons[widget];
    if ((button != 0) && (button->isOn()))
    {
        button->setOn(false);
        selectWidget(button);
    }
}

void DDockWindow::removeWidget(QWidget *widget)
{
    kdDebug(9000) << k_funcinfo << endl;
    if (m_widgetStack->id(widget) == -1)
        return; //not in dock

    bool changeVisibility = false;
    if (m_widgetStack->visibleWidget() == widget)
        changeVisibility = true;

    Ideal::Button *button = m_buttons[widget];
    if (button)
        m_bar->removeButton(button);
    m_widgets.remove(button);
    m_buttons.remove(widget);
    m_widgetStack->removeWidget(widget);

    if (changeVisibility)
    {
        m_toggledButton = 0;
        setVisible(false);
    }
}

void DDockWindow::selectWidget(Ideal::Button *button)
{
    bool special = m_doNotCloseActiveWidget;
    m_doNotCloseActiveWidget = false;
    kdDebug(9000) << k_funcinfo << endl;
    if (m_toggledButton == button)
    {
        if (special && m_visible && (!isActive()))
        {
            //special processing for keyboard navigation events
            m_toggledButton->setOn(true);
            m_widgets[button]->setFocus();
        }
        else
        {
            m_widgets[button]->setFocus();
            setVisible(!m_visible);
        }
        return;
    }

    if (m_toggledButton)
        m_toggledButton->setOn(false);
    m_toggledButton = button;
    setVisible(true);
    m_widgetStack->raiseWidget(m_widgets[button]);
    m_widgets[button]->setFocus();
}

void DDockWindow::selectWidget()
{
    selectWidget((Ideal::Button*)sender());
}

void DDockWindow::hideWidget(QWidget *widget)
{
    Ideal::Button *button = m_buttons[widget];
    if (button != 0)
    {
        button->setOn(false);
        button->hide();
    }
    widget->hide();
    if (button == m_toggledButton)
        setVisible(false);
}

void DDockWindow::showWidget(QWidget *widget)
{
    Ideal::Button *button = m_buttons[widget];
    if (button != 0)
        button->show();
//     widget->show();
}

void DDockWindow::setMovingEnabled(bool)
{
    //some operations on KMainWindow cause moving to be enabled
    //but we always don't want DDockWindow instances to be movable
    QDockWindow::setMovingEnabled(false);
}

void DDockWindow::selectLastWidget()
{
    m_doNotCloseActiveWidget = true;
    if (m_toggledButton)
        m_toggledButton->animateClick();
    else if (Ideal::Button *button = m_bar->firstButton())
        button->animateClick();
}

bool DDockWindow::isActive()
{
    if (m_toggledButton)
    {
        QWidget *w = qApp->focusWidget();
        QWidget *toolWidget = m_widgets[m_toggledButton];
        if (toolWidget == w)
            return true;
        else
        {
            do {
                w = (QWidget*)w->parent();
                if (w && (w == toolWidget)) return true;
            } while (w);
        }
    }
    return false;
}

void DDockWindow::selectNextWidget()
{
    if (!m_toggledButton)
        return;
    Ideal::Button *b = m_bar->nextTo(m_toggledButton);
    if (b)
        b->animateClick();
}

void DDockWindow::selectPrevWidget()
{
    if (!m_toggledButton)
        return;
    Ideal::Button *b = m_bar->prevTo(m_toggledButton);
    if (b)
        b->animateClick();
}

void DDockWindow::contextMenu(QPopupMenu * menu)
{
    m_lastContextMenuButton = static_cast<Ideal::Button*>( const_cast<QObject*>( sender() ) );

    menu->insertSeparator();

    if ( position() != DDockWindow::Left )
        m_moveToDockLeft->plug( menu );
    if ( position()!= DDockWindow::Right )
        m_moveToDockRight->plug( menu );
    if ( position() != DDockWindow::Bottom )
        m_moveToDockBottom->plug( menu );
}

void DDockWindow::moveToDockLeft()
{
    moveToDock( DDockWindow::Left );
}

void DDockWindow::moveToDockRight()
{
    moveToDock( DDockWindow::Right );
}

void DDockWindow::moveToDockBottom()
{
    moveToDock( DDockWindow::Bottom );
}

void DDockWindow::moveToDock(DDockWindow::Position position )
{
    if (  m_widgets.contains( m_lastContextMenuButton ) )
    {
        mainWindow()->moveWidget( position, m_widgets[ m_lastContextMenuButton ], m_lastContextMenuButton->realTextWithoutAccel() );
    }
}

#include "ddockwindow.moc"
