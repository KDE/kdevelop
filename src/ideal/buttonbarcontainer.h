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
#include <QMap>
#include <QToolBar>

#include "idealdefs.h"

namespace Ideal {

class ToolView;
class ButtonBar;
class Button;

class ButtonBarContainer: public QToolBar {
public:
    ButtonBarContainer(Ideal::Place place, ButtonMode mode, QWidget *parent = 0);

    void setButtonMode(ButtonMode mode);
    void addToolViewButton(ToolView *view);
    void showToolViewButton(ToolView *view);
    void hideToolViewButton(ToolView *view);
    void removeToolViewButton(ToolView *view);

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

    virtual void setVisible(bool visible);

private:
    QString titleForPlace();

    Ideal::Place m_place;
    ButtonMode m_mode;

    ButtonBar *m_bar;
    QMap<ToolView*, Button*> m_viewButtons;

};

}
