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
#include "toolview.h"

namespace Ideal {

ToolView::ToolView(const QString &title, QWidget *parent)
    :QDockWidget(title, parent)
{
    init();
}

ToolView::ToolView(QWidget *parent)
    :QDockWidget(parent)
{
    init();
}

void ToolView::init()
{
    setFeatures(AllDockWidgetFeatures);
}

void ToolView::setPlace(Ideal::Place place)
{
    m_place = place;
}

Qt::DockWidgetArea ToolView::dockPlace() const
{
    return dockPlace(m_place);
}

Qt::DockWidgetArea ToolView::dockPlace(Ideal::Place place)
{
    Qt::DockWidgetArea dockArea = Qt::LeftDockWidgetArea;
    if (place == Ideal::Right) dockArea = Qt::RightDockWidgetArea;
    else if (place == Ideal::Bottom) dockArea = Qt::BottomDockWidgetArea;
    else if (place == Ideal::Top) dockArea = Qt::TopDockWidgetArea;
    return dockArea;
}

void ToolView::setVisible(bool v)
{
    QDockWidget::setVisible(v);
    emit visibilityChanged(v);
}

}

#include "toolview.moc"
