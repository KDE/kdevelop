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
#include "pcolorbutton.h"

#include <qlayout.h>
#include <qpainter.h>

#ifndef PURE_QT
#include <kcolorbutton.h>
#else
#include <qpushbutton.h>
#endif
#include <qcolordialog.h>

namespace PropertyLib {

PColorButton::PColorButton(MultiProperty* property, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
#ifndef PURE_QT
    m_edit = new KColorButton(this);
    connect(m_edit, SIGNAL(changed(const QColor&)), this, SLOT(updateProperty(const QColor&)));
#else
    m_edit = new QPushButton(this);
    connect(m_edit, SIGNAL(clicked()), this, SLOT(changeColor()));
#endif

    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);
}

QVariant PColorButton::value() const
{
#ifndef PURE_QT
    return QVariant(m_edit->color());
#else
    return QVariant(m_color);
#endif
}

void PColorButton::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
/*    p->setBrush(value.toColor());
    p->setPen(Qt::NoPen);
    p->drawRect(r);*/
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    
    p->setBrush(value.toColor());
    p->setPen(Qt::SolidLine);
    QRect r2(r);
    r2.setTopLeft(r.topLeft() + QPoint(5,5));
    r2.setBottomRight(r.bottomRight() - QPoint(5,5));
    p->drawRect(r2);
}

void PColorButton::setValue(const QVariant& value, bool emitChange)
{
#ifndef PURE_QT
    disconnect(m_edit, SIGNAL(changed(const QColor&)), this, SLOT(updateProperty(const QColor&)));
    m_edit->setColor(value.toColor());
    connect(m_edit, SIGNAL(changed(const QColor&)), this, SLOT(updateProperty(const QColor&)));
#else
    m_color = value.toColor();
    m_edit->setText(m_color.name());
#endif
    if (emitChange)
        emit propertyChanged(m_property, value);

}

void PColorButton::updateProperty(const QColor &color)
{
    emit propertyChanged(m_property, value());
}

void PColorButton::changeColor()
{
#ifdef PURE_QT
   m_color = QColorDialog::getColor(m_color,this);
   updateProperty(m_color);
#endif 
}

}
#ifndef PURE_QT
#include "pcolorbutton.moc"
#endif
