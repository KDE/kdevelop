/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "buttonbar.h"

#include <klocale.h>
#include <kdebug.h>

#include "button.h"
#include "buttoncontainer.h"
#include "buttonbar.h"
#include "toolviewwidget.h"
#include "settings.h"

namespace Ideal {

ButtonBar::ButtonBar(Ideal::Place place, QWidget *parent)
    :QToolBar(parent), m_place(place)
{
    setWindowTitle(titleForPlace());

    m_bar = new ButtonContainer(place, Settings::buttonMode(), this);
    addWidget(m_bar);
    setMovable(false);
    findChild<QBoxLayout*>()->setMargin(0);
    if (m_bar->isEmpty())
        hide();
}

void ButtonBar::setButtonMode(ButtonMode mode)
{
    m_bar->setMode(mode);
}

void ButtonBar::addToolViewButton(ToolViewWidget *view)
{
    if (!isVisible())
        show();
    Button *button = new Button(m_bar, view->widget()->windowTitle(),
        view->widget()->windowIcon(), view->widget()->toolTip());
    m_bar->addButton(button/*, false*/);
    m_viewButtons[view] = button;
    m_buttonViews[button] = view;

    kDebug() << view->isVisible() << endl;
    button->setChecked(true);

    connect(button, SIGNAL(clicked()), this, SLOT(setToolViewWidgetVisibility()));
    connect(view, SIGNAL(visibilityChanged(bool)), button, SLOT(setChecked(bool)));
}

void ButtonBar::showToolViewButton(ToolViewWidget *view)
{
    m_viewButtons[view]->show();
}

void ButtonBar::hideToolViewButton(ToolViewWidget *view)
{
    m_viewButtons[view]->hide();
}

void ButtonBar::removeToolViewButton(ToolViewWidget *view)
{
    Button *button = m_viewButtons[view];
    m_bar->removeButton(button);
    m_viewButtons.remove(view);
    m_buttonViews.remove(button);
    if (m_bar->isEmpty())
        hide();
}

void ButtonBar::setVisible(bool visible)
{
/*    if (visible && m_bar->isEmpty())
        return;*/
    QToolBar::setVisible(visible);
}

QString ButtonBar::titleForPlace()
{
    switch (m_place) {
        case Ideal::Left: return i18n("Left Button Bar");
        case Ideal::Right: return i18n("Right Button Bar");
        case Ideal::Bottom: return i18n("Bottom Button Bar");
        case Ideal::Top: return i18n("Top Button Bar");
    }
    return "";
}

void ButtonBar::setToolViewWidgetVisibility()
{
    Button *button = qobject_cast<Ideal::Button*>(sender());
    if (!button)
        return;
    ToolViewWidget *view = m_buttonViews[button];
    if (!view)
        return;
    view->setVisible(!view->isVisible());
}

}

#include "buttonbar.moc"
