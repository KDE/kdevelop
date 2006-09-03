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
#include "settings.h"

namespace Ideal {

struct ButtonBarPrivate {
    ButtonBarPrivate(ButtonBar *_bar, Ideal::Place _place)
        :bar(_bar), place(_place)
    {
        container = bar->createButtonContainer(place);
    }

    ButtonBar *bar;
    QString titleForPlace();

    Ideal::Place place;
    ButtonContainer *container;
};

QString ButtonBarPrivate::titleForPlace()
{
    switch (place) {
        case Ideal::Left: return i18n("Left Button Bar");
        case Ideal::Right: return i18n("Right Button Bar");
        case Ideal::Bottom: return i18n("Bottom Button Bar");
        case Ideal::Top: return i18n("Top Button Bar");
    }
    return "";
}


//class ButtonBar

ButtonBar::ButtonBar(Ideal::Place place, QWidget *parent)
    :QToolBar(parent)
{
    d = new ButtonBarPrivate(this, place);
    setWindowTitle(d->titleForPlace());
    addWidget(d->container);
    setMovable(false);
    //set the QToolBar's layout margin to 0 to please Mr. Fitt ;)
    findChild<QBoxLayout*>()->setMargin(0);
    if (d->container->isEmpty())
        hide();
}

ButtonBar::~ButtonBar()
{
    delete d;
}

void ButtonBar::addToolViewButton(Button *button)
{
//     if (!isVisible())
//         show();
    d->container->addButton(button/*, false*/);
    button->hide();
}

void ButtonBar::removeToolViewButton(Button *button)
{
    d->container->removeButton(button);
    if (d->container->isVisuallyEmpty())
        hide();
}

Qt::ToolBarArea ButtonBar::toolBarPlace()
{
    return toolBarPlace(d->place);
}

Qt::ToolBarArea ButtonBar::toolBarPlace(Ideal::Place place)
{
    Qt::ToolBarArea dockArea = Qt::LeftToolBarArea;
    if (place == Ideal::Right) dockArea = Qt::RightToolBarArea;
    else if (place == Ideal::Bottom) dockArea = Qt::BottomToolBarArea;
    else if (place == Ideal::Top) dockArea = Qt::TopToolBarArea;
    return dockArea;
}

ButtonContainer *ButtonBar::createButtonContainer(Ideal::Place place)
{
    return new ButtonContainer(place, Settings::buttonMode(), this);
}

bool ButtonBar::isEmpty() const
{
    return d->container->isVisuallyEmpty();
}

}

#include "buttonbar.moc"
