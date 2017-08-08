/*
    Copyright David Nolden  <david.nolden.kdevelop@art-master.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "workingsetfilelabel.h"

#include <QMouseEvent>

using namespace KDevelop;

WorkingSetFileLabel::WorkingSetFileLabel()
    : m_isActive( false )
{
}

void WorkingSetFileLabel::setIsActiveFile(bool active)
{
    if(active)
    {
        ///@todo Use a nicer-looking "blended" highlighting for the active item, like in the area-tabs
        setAutoFillBackground(true);
        setBackgroundRole(QPalette::Highlight);
        setForegroundRole(QPalette::HighlightedText);
    }else{
        setAutoFillBackground(false);
        setBackgroundRole(QPalette::Window);
        setForegroundRole(QPalette::WindowText);
    }
    m_isActive = active;
}

void WorkingSetFileLabel::mouseReleaseEvent(QMouseEvent* ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        ev->accept();
        emit clicked();
        return;
    }

    QLabel::mouseReleaseEvent(ev);
}

bool WorkingSetFileLabel::isActive() const
{
  return m_isActive;
}

void WorkingSetFileLabel::emitClicked()
{
  emit clicked();
}

