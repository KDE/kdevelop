/* This file is part of the KDE project
   Copyright (C) 2002 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "pspinbox.h"
#include "propertyeditor.h"

PSpinBox::PSpinBox ( const PropertyEditor *editor, const QString pname, const QVariant value, QWidget * parent, const char * name ):
    QSpinBox(parent, name)
{
    setValue(value, false);
    setPName(pname);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateProperty(int)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

PSpinBox::PSpinBox ( const PropertyEditor *editor, const QString pname, const QVariant value, int minValue, int maxValue, int step, QWidget * parent, const char * name ):
    QSpinBox(minValue, maxValue, step, parent, name)
{
    setPName(pname);
    setValue(value, false);
    connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateProperty(int)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PSpinBox::value() const
{
    return QVariant(cleanText().toInt());
}

void PSpinBox::setValue(const QVariant value, bool emitChange)
{
    QSpinBox::setValue(value.toInt());
    if (emitChange)
        emit propertyChanged(pname(), value);
}

void PSpinBox::updateProperty(int val)
{
    emit propertyChanged(pname(), QVariant(val));
}

#ifndef PURE_QT
#include "pspinbox.moc"
#endif
