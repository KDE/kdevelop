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
#include "purledit.h"

#include <kurlrequester.h>
#include <qlayout.h>

namespace PropertyLib{

PUrlEdit::PUrlEdit(KFile::Mode mode, MultiProperty* property, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new KURLRequester(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);
    m_edit->setMode(mode);

    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
}

QVariant PUrlEdit::value() const
{
     return QVariant(m_edit->url());
}

void PUrlEdit::setValue(const QVariant& value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    m_edit->setURL(value.toString());
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PUrlEdit::updateProperty(const QString &val)
{
    emit propertyChanged(m_property, QVariant(val));
}

}

#include "purledit.moc"
