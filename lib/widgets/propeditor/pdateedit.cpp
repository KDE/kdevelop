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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "pdateedit.h"

#include <qdatetimeedit.h>
#include <qpainter.h>
#include <qlayout.h>

namespace PropertyLib{

PDateEdit::PDateEdit(MultiProperty* property, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QDateEdit(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);
    
    connect(m_edit, SIGNAL(valueChanged(const QDate&)), this, SLOT(updateProperty(const QDate&)));
}

QVariant PDateEdit::value() const
{
     return QVariant(m_edit->date());
}

void PDateEdit::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, value.toDate().toString(Qt::LocalDate));
}

void PDateEdit::setValue(const QVariant& value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(valueChanged(const QDate&)), this, SLOT(updateProperty(const QDate&)));
    m_edit->setDate(value.toDate());
    connect(m_edit, SIGNAL(valueChanged(const QDate&)), this, SLOT(updateProperty(const QDate&)));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PDateEdit::updateProperty(const QDate &val)
{
    emit propertyChanged(m_property, QVariant(val));
}

}

#ifndef PURE_QT
#include "pdateedit.moc"
#endif
