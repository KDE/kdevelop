/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mskat.net                                                 *
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
#include "pdummywidget.h"

#include <qpainter.h>

namespace PropertyLib{

PDummyWidget::PDummyWidget(MultiProperty *property, QWidget *parent, const char *name)
    :PropertyWidget(property, parent, name)
{
}

QVariant PDummyWidget::value() const
{
    return m_value;
}

void PDummyWidget::setValue(const QVariant &value, bool emitChange)
{
    m_value = value;
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PDummyWidget::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &/*value*/)
{
    p->setBrush(cg.background());
    p->setPen(Qt::NoPen);
    p->drawRect(r);
}

}

#ifndef PURE_QT
#include "pdummywidget.moc"
#endif
