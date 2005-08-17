/***************************************************************************
 *   Copyright (C) 2002-2004 by Alexander Dymo                             *
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
#include "plineedit.h"

#include <qlineedit.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QHBoxLayout>

namespace PropertyLib{

PLineEdit::PLineEdit(MultiProperty *property, QWidget *parent)
    :PropertyWidget(property, parent)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QLineEdit(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);
    
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
}

QVariant PLineEdit::value() const
{
    return QVariant(m_edit->text());
}

void PLineEdit::setValue(const QVariant &value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    m_edit->setText(value.toString());
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PLineEdit::updateProperty(const QString& val)
{
    emit propertyChanged(m_property, QVariant(val));
}

}

#ifndef PURE_QT
#include "plineedit.moc"
#endif
