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

#include "plineedit.h"
#include "propertyeditor.h"

PLineEdit::PLineEdit ( const PropertyEditor *editor, const QString pname, const QVariant value, QWidget * parent, const char * name ):
    QLineEdit(parent, name)
{
    setValue(value, false);
    setPName(pname);
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PLineEdit::value() const
{
    return QVariant(text());
}

void PLineEdit::setValue(const QVariant value, bool emitChange)
{
    setText(value.toString());
    if (emitChange)
        emit propertyChanged(pname(), value);
}

void PLineEdit::updateProperty(const QString& val)
{
    emit propertyChanged(pname(), QVariant(val));
}

#ifndef PURE_QT
#include "plineedit.moc"
#endif
