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

#include "pcombobox.h"
#include "propertyeditor.h"

PComboBox::PComboBox ( const PropertyEditor *editor, const QString pname, const QVariant value, QMap<QString, QString> *v_corresp, QWidget * parent, const char * name):
    QComboBox(parent, name), corresp(v_corresp)
{
    init(editor, pname, value);
}

PComboBox::PComboBox ( const PropertyEditor *editor, const QString pname, const QVariant value, QMap<QString, QString> *v_corresp,  bool rw, QWidget * parent, const char * name):
        QComboBox(rw, parent, name), corresp(v_corresp)
{
    init(editor, pname, value);
}

void PComboBox::init( const PropertyEditor *editor, const QString & pname, const QVariant & value )
{
    fillBox();
    setValue(value, false);
    setPName(pname);
    connect(this, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
    connect(this, SIGNAL(propertyChanged(QString, QString)), editor, SLOT(emitPropertyChange(QString, QString)));
}

void PComboBox::fillBox()
{
    for (QMap<QString, QString>::const_iterator it = corresp->begin(); it != corresp->end(); it++)
    {
        insertItem(it.key());
        r_corresp[it.data()] = it.key();
    }
}

QVariant PComboBox::value() const
{
    QMap<QString, QString>::const_iterator it = corresp->find(currentText());
    if (it==corresp->end()) return QVariant("");
    return QVariant(it.data());
}

void PComboBox::setValue(const QVariant value, bool emitChange)
{
#if QT_VERSION >= 0x030100
    if (!value.isNull())
#else
    if (value.canCast(QVariant::String))
#endif
    {
        setCurrentText(r_corresp[value.toString()]);
        if (emitChange)
            emit propertyChanged(pname(), value);
    }
}

void PComboBox::updateProperty(int val)
{
    emit propertyChanged(pname(), value());
}

#ifndef PURE_QT
#include "pcombobox.moc"
#endif
