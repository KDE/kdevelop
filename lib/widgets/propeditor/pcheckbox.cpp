/***************************************************************************
 *   Copyright (C) 2003-2004 by Alexander Dymo                             *
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
#include "pcheckbox.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QHBoxLayout>

#ifndef PURE_QT
#include <klocale.h>
#else
#include "compat_tools.h"
#endif

namespace PropertyLib{

PCheckBox::PCheckBox(MultiProperty *property, QWidget *parent, const char *name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QCheckBox(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);
    
    connect(m_edit, SIGNAL(toggled(bool)), this, SLOT(updateProperty(bool)));
}

QVariant PCheckBox::value() const
{
    return QVariant(m_edit->isChecked());
}

void PCheckBox::setValue(const QVariant &value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(toggled(bool)), this, SLOT(updateProperty(bool)));
    m_edit->setChecked(value.toBool());
    connect(m_edit, SIGNAL(toggled(bool)), this, SLOT(updateProperty(bool)));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PCheckBox::updateProperty(bool val)
{
    emit propertyChanged(m_property, QVariant(val));
}

void PCheckBox::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    p->setBrush(cg.background());
    p->setPen(Qt::NoPen);
    p->drawRect(r);
    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, value.toBool() ? i18n("true") : i18n("false"));
}

}

#ifndef PURE_QT
#include "pcheckbox.moc"
#endif
