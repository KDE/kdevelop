/***************************************************************************
 *   Copyright (C) 2005-2006 by Alexander Dymo                             *
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
#include "mainwindow.h"

#include <QMap>
#include <QToolBar>
#include <QSettings>
#include <QLayout>

#include "area.h"
#include "button.h"
#include "buttonbar.h"
#include "toolview.h"

namespace Ideal {

//==================== ButtonBarContainer =====================

class ButtonBarContainer: public QToolBar {
public:
    ButtonBarContainer(Ideal::Place place, ButtonMode mode, QWidget *parent = 0)
        :QToolBar(parent), m_place(place)
    {
        setWindowTitle(titleForPlace());

        m_bar = new ButtonBar(place, mode, this);
        addWidget(m_bar);
        setMovable(false);
        findChild<QBoxLayout*>()->setMargin(0);
        if (m_bar->isEmpty())
            hide();
    }

    void setButtonMode(ButtonMode mode)
    {
        m_bar->setMode(mode);
    }

    void addToolViewButton(ToolView *view)
    {
        if (!isVisible())
            show();
        Button *button = new Button(m_bar, view->widget()->windowTitle(),
            view->widget()->windowIcon(), view->widget()->toolTip());
        m_bar->addButton(button/*, false*/);
        m_viewButtons[view] = button;

    }

    void showToolViewButton(ToolView *view)
    {
        m_viewButtons[view]->show();
    }

    void hideToolViewButton(ToolView *view)
    {
        m_viewButtons[view]->hide();
    }

    void removeToolViewButton(ToolView *view)
    {
        m_bar->removeButton(m_viewButtons[view]);
        m_viewButtons.remove(view);
        if (m_bar->isEmpty())
            hide();
    }

    Qt::ToolBarArea toolBarPlace()
    {
        return toolBarPlace(m_place);
    }

    static Qt::ToolBarArea toolBarPlace(Ideal::Place place)
    {
        Qt::ToolBarArea dockArea = Qt::LeftToolBarArea;
        if (place == Ideal::Right) dockArea = Qt::RightToolBarArea;
        else if (place == Ideal::Bottom) dockArea = Qt::BottomToolBarArea;
        else if (place == Ideal::Top) dockArea = Qt::TopToolBarArea;
        return dockArea;
    }

    virtual void setVisible(bool visible)
    {
/*        if (visible && m_bar->isEmpty())
            return;*/
        QToolBar::setVisible(visible);
    }

private:

    QString titleForPlace()
    {
        switch (m_place) {
            case Ideal::Left: return "Left Button Bar";
            case Ideal::Right: return "Right Button Bar";
            case Ideal::Bottom: return "Bottom Button Bar";
            case Ideal::Top: return "Top Button Bar";
        }
        return "";
    }

    Ideal::Place m_place;
    ButtonMode m_mode;

    ButtonBar *m_bar;
    QMap<ToolView*, Button*> m_viewButtons;

};


//==================== MainWindowPrivate =====================

struct MainWindowPrivate {
    MainWindowPrivate(MainWindow *window)
        :w(window)
    {
        area = 0;
    }
    ~MainWindowPrivate()
    {
        delete area;
    }

    MainWindow *w;
    QList<ToolView*> toolViews;
    QMap<QWidget*, ToolView*> viewsForWidgets;
    QMap<Ideal::Place, ButtonBarContainer*> buttonBars;

    Area *area;
    Ideal::ButtonMode buttonMode;
};


//======================== MainWindow ========================

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
{
    d = new MainWindowPrivate(this);
    initSettings();

    initButtonBar(Ideal::Left);
    initButtonBar(Ideal::Right);
    initButtonBar(Ideal::Bottom);
    initButtonBar(Ideal::Top);
}

void MainWindow::initSettings()
{
    QSettings settings("Ideal", "Ideal");
    d->buttonMode = static_cast<Ideal::ButtonMode>(
        settings.value("ButtonMode", Ideal::Text).toInt());
}

void MainWindow::initButtonBar(Ideal::Place place)
{
    ButtonBarContainer *bar = new ButtonBarContainer(place, d->buttonMode, this);
    addToolBar(bar->toolBarPlace(), bar);
    d->buttonBars[place] = bar;
}

MainWindow::~MainWindow()
{
    delete d;
}

// ButtonBar *MainWindow::buttonBar(Ideal::Place place) const
// {
//     return d->buttonBars[place];
// }

void MainWindow::setAreaKind(int areaKind)
{
    if (d->area)
        delete d->area;
    d->area = new Area(areaKind, this);
}

void MainWindow::addToolView(QWidget *view, Ideal::Place defaultPlace, int defaultArea)
{
    ToolView *toolView = new ToolView(view->windowTitle());
    toolView->setPlace(defaultPlace);
    toolView->setAreaKind(defaultArea);
    toolView->setWidget(view);

    if (d->area)
        d->area->placeToolView(toolView);

    d->toolViews.append(toolView);
    d->viewsForWidgets[view] = toolView;

    d->buttonBars[toolView->place()]->addToolViewButton(toolView);
}

void MainWindow::removeToolView(QWidget *view)
{
    d->toolViews.removeAll(d->viewsForWidgets[view]);
}

QList<ToolView*> MainWindow::toolViews() const
{
    return d->toolViews;
}

}

#include "mainwindow.moc"
