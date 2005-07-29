/***************************************************************************
 *   Copyright (C) 2005 by Jens Herden                                     *
 *   jens@kdewebdev.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include "toolbarguibuilder.h"

#include <ktoolbar.h>

ToolbarGUIBuilder::ToolbarGUIBuilder(QWidget *parent, QWidget *widget)
  : KXMLGUIBuilder(widget), KToolBar(parent, "ToolbarGUIbuilder"), m_parent(parent)
{
//   setHidden(true);
  setFrameStyle(0);
}


ToolbarGUIBuilder::~ToolbarGUIBuilder()
{
}

QWidget * ToolbarGUIBuilder::createContainer(QWidget *parent, int index, const QDomElement &element, int &id)
{
  if (element.tagName().lower() == "toolbar")
  {
    reparent(m_parent, QPoint(0, 0), true);
    return this;
  } else
  {
    return KXMLGUIBuilder::createContainer(parent, index, element, id);
  }
}

void ToolbarGUIBuilder::removeContainer(QWidget *container, QWidget *parent, QDomElement &element, int id)
{
  if (container == this)
  {
    // i need to reparent to prevent a crash
    reparent(0, QPoint(0, 0));
  }
  else
    KXMLGUIBuilder::removeContainer(container, parent, element, id);
}

