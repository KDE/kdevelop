/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

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
#include "pcheckbox.h"
#include "propertyeditor.h"

PCheckBox::PCheckBox( const PropertyEditor * editor, const QString pname, const QVariant value, QWidget * parent, const char * name )
    :QCheckBox(parent, name)
{
    setValue(value, false);
    setPName(pname);
    connect(this, SIGNAL(toggled(bool)), this, SLOT(updateProperty(bool)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PCheckBox::value() const
{
    return QVariant(isChecked());
}

void PCheckBox::setValue(const QVariant value, bool emitChange)
{
    setChecked(value.toBool());
    if (emitChange)
        emit propertyChanged(pname(), value);
}

void PCheckBox::updateProperty( bool val )
{
    emit propertyChanged(pname(), QVariant(val));
}

#include "pcheckbox.moc"
