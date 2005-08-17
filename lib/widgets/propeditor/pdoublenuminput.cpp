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
#include "pdoublenuminput.h"

#ifndef PURE_QT
#include <knuminput.h>
#else
#include "qfloatinput.h"
//Added by qt3to4:
#include <QHBoxLayout>
#endif

#include <limits.h>
#include <math.h>
#include <qlayout.h>

namespace PropertyLib{

PDoubleNumInput::PDoubleNumInput(MultiProperty *property, QWidget *parent)
    :PropertyWidget(property, parent)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
#ifndef PURE_QT
    m_edit = new KDoubleNumInput(-999999.0, 999999.0, 0.0, 0.01, 2, this);
    m_edit->setLabel(QString::null);
    connect(m_edit, SIGNAL(valueChanged(double)), this, SLOT(updateProperty(double)));
#else
    m_edit = new QFloatInput(-999999, 999999, 0.01, 2, this );
    connect(m_edit, SIGNAL(valueChanged(int)), this, SLOT(updateProperty(int)));
#endif
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
/*    m_edit->setMinValue(-999999999);
    m_edit->setMaxValue(+999999999);
    m_edit->setPrecision(2);*/
    l->addWidget(m_edit);

}

QVariant PDoubleNumInput::value() const
{
    return QVariant(m_edit->value());
}

void PDoubleNumInput::setValue(const QVariant &value, bool emitChange)
{
#ifndef PURE_QT
    disconnect(m_edit, SIGNAL(valueChanged(double)), this, SLOT(updateProperty(double)));
    m_edit->setValue(value.toDouble());
    connect(m_edit, SIGNAL(valueChanged(double)), this, SLOT(updateProperty(double)));
#else
    disconnect(m_edit, SIGNAL(valueChanged(int)), this, SLOT(updateProperty(int)));
    m_edit->setValue(int(value.toDouble()*pow(m_edit->digits(),10)));
    connect(m_edit, SIGNAL(valueChanged(int)), this, SLOT(updateProperty(int)));
#endif
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PDoubleNumInput::updateProperty(double val)
{
    emit propertyChanged(m_property, QVariant(val));
}
void PDoubleNumInput::updateProperty(int val)
{
#ifdef PURE_QT
    QString format = QString("%.%1f").arg( m_edit->digits() );
    QString strVal = QString().sprintf(format.latin1(),
                                       (val/(float)pow(m_edit->digits(),10)) );
    emit propertyChanged(m_property, QVariant(strVal));
#else
    Q_UNUSED(val);
#endif
}

}

#ifndef PURE_QT
#include "pdoublenuminput.moc"
#endif
