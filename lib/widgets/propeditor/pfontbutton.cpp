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
#include "pfontbutton.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>

#include <kfontrequester.h>

#ifndef PURE_QT
#include <klocale.h>
#else
#include "compat_tools.h"
#endif

namespace PropertyLib{

PFontButton::PFontButton(MultiProperty* property, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new KFontRequester(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
#ifndef PURE_QT
    m_edit->button()->setText(i18n("..."));
#endif
    l->addWidget(m_edit);

    connect(m_edit, SIGNAL(fontSelected(const QFont& )), this, SLOT(updateProperty(const QFont& )));
}

QVariant PFontButton::value() const
{
    return QVariant(m_edit->font());
}

void PFontButton::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    QFontInfo fi(value.toFont());
    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, 
        fi.family() + (fi.bold() ? i18n(" Bold") : QString("")) +
        (fi.italic() ? i18n(" Italic") : QString("")) +
        " " + QString("%1").arg(fi.pointSize()) );
}

void PFontButton::setValue(const QVariant& value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(fontSelected(const QFont&)), this, SLOT(updateProperty(const QFont&)));
    m_edit->setFont(value.toFont());
    connect(m_edit, SIGNAL(fontSelected(const QFont& )), this, SLOT(updateProperty(const QFont& )));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PFontButton::updateProperty(const QFont &font)
{
    emit propertyChanged(m_property, value());
}

}

#ifndef PURE_QT
#include "pfontbutton.moc"
#endif
