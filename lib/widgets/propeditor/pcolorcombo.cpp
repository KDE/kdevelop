/***************************************************************************
 *   Copyright (C) 2002-2004 by Alexander Dymo                             *
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
#include "pcolorcombo.h"

#include <qlayout.h>
#include <qpainter.h>

#include <kcolorcombo.h>

PColorCombo::PColorCombo(const QString &propertyName, QWidget *parent, const char *name)
    :PropertyWidget(propertyName, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new KColorCombo(this);
    l->addWidget(m_edit);

    connect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
}

QVariant PColorCombo::value() const
{
    return QVariant(m_edit->color());
}

void PColorCombo::setValue(const QVariant &value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
    m_edit->setColor(value.toColor());
    connect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
    if (emitChange)
        emit propertyChanged(propertyName(), value);
}

void PColorCombo::updateProperty(int /*val*/)
{
    emit propertyChanged(propertyName(), value());
}

void PColorCombo::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    p->setBrush(value.toColor());
    p->setPen(Qt::NoPen);
    p->drawRect(r);
}

#ifndef PURE_QT
#include "pcolorcombo.moc"
#endif
