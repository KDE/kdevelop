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
#include "ppointedit.h"

#include <qlineedit.h>
#include <qlayout.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QHBoxLayout>

namespace PropertyLib{

PPointEdit::PPointEdit(MultiProperty* property, QWidget* parent, const char* name): PropertyWidget(property, parent)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QLineEdit(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);

    m_edit->setReadOnly(true);
}

QVariant PPointEdit::value() const
{
    return m_value;
}

void PPointEdit::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, QString("[ %1, %2 ]").arg(value.toPoint().x()).arg(value.toPoint().y()));
}

void PPointEdit::setValue(const QVariant& value, bool emitChange)
{
    m_value = value;
    m_edit->setText(QString("[ %1, %2 ]").arg(value.toPoint().x()).arg(value.toPoint().y()));

    if (emitChange)
        emit propertyChanged(m_property, value);
}

}

#ifndef PURE_QT
#include "ppointedit.moc"
#endif
