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

#include "pcolorcombo.h"
#include "propertyeditor.h"

PColorCombo::PColorCombo(const PropertyEditor *editor, const QString pname, const QVariant value, QWidget *parent, const char *name):
    KColorCombo(parent, name)
{
    setValue(value);
    setPName(pname);
    connect(this, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PColorCombo::value() const
{
//    return QString("%1,%2,%3").arg(color().red()).arg(color().green()).arg(color().blue());
    return QVariant(color());
}

void PColorCombo::setValue(const QVariant value, bool emitChange)
{
/*    setColor(QColor(value.section(',', 0, 0).toInt(),
        value.section(',', 1, 1).toInt(),
        value.section(',', 2, 2).toInt()));*/
    setColor(value.toColor());
    if (emitChange)
        emit propertyChanged(pname(), value);
}

void PColorCombo::updateProperty(int /*val*/)
{
    emit propertyChanged(pname(), value());
}

#ifndef PURE_QT
#include "pcolorcombo.moc"
#endif
