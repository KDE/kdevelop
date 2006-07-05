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
#include "mainwindow.h"
#include "area.h"

namespace Ideal {

struct ButtonBarPrivate {
    QString titleForPlace();

    Ideal::Place place;
    MainWindow *mainWindow;

    ButtonContainer *container;
    QMap<ToolViewWidget*, Button*> viewButtons;
    QMap<Button*, ToolViewWidget*> buttonViews;
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

ButtonBar::ButtonBar(Ideal::Place place, MainWindow *parent)
    :QToolBar(parent)
{
    ButtonBarPrivate *d = new ButtonBarPrivate;
    d->place = place;
    d->mainWindow = parent;

    setWindowTitle(d->titleForPlace());

    d->container = new ButtonContainer(place, Settings::buttonMode(), this);
    addWidget(d->container);
    setMovable(false);
    //set the QToolBar's layout margin to 0 to please Mr. Fitt ;)
    findChild<QBoxLayout*>()->setMargin(0);
    if (d->container->isEmpty())
        hide();
}

void ButtonBar::addToolViewButton(ToolViewWidget *view)
{
    if (!isVisible())
        show();
    Button *button = new Button(d->container, view->widget()->windowTitle(),
        view->widget()->windowIcon(), view->widget()->toolTip());
    d->container->addButton(button/*, false*/);
    d->viewButtons[view] = button;
    d->buttonViews[button] = view;

    kDebug() << view->isVisible() << endl;
    button->setChecked(true);

    connect(button, SIGNAL(clicked()), this, SLOT(setToolViewWidgetVisibility()));
    connect(view, SIGNAL(visibilityChanged(bool)), button, SLOT(setChecked(bool)));
}

void ButtonBar::showToolViewButton(ToolViewWidget *view)
{
    d->viewButtons[view]->show();
}

void ButtonBar::hideToolViewButton(ToolViewWidget *view)
{
    d->viewButtons[view]->hide();
}

void ButtonBar::removeToolViewButton(ToolViewWidget *view)
{
    Button *button = d->viewButtons[view];
    d->container->removeButton(button);
    d->viewButtons.remove(view);
    d->buttonViews.remove(button);
    if (d->container->isEmpty())
        hide();
}

void ButtonBar::setToolViewWidgetVisibility()
{
    Button *button = qobject_cast<Ideal::Button*>(sender());
    if (!button)
        return;
    ToolViewWidget *view = d->buttonViews[button];
    if (!view)
        return;
    d->mainWindow->area()->selectToolView(view);
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

// MainWindow *ButtonBar::mainWindow()
// {
//     return d->mainWindow;
// }

}

#include "buttonbar.moc"
