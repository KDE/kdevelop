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
#include "pcombobox.h"

#include <qcombobox.h>
#include <qlayout.h>

PComboBox::PComboBox(MultiProperty *property, const QMap<QString, QVariant> &list, QWidget *parent, const char *name)
    :PropertyWidget(property, parent, name), m_valueList(list)
{
    init(false);
}

PComboBox::PComboBox(MultiProperty *property, const QMap<QString, QVariant> &list, bool rw, QWidget *parent, const char *name)
    :PropertyWidget(property, parent, name), m_valueList(list)
{
    init(rw);
}

void PComboBox::init(bool rw)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QComboBox(rw, this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);
    
    fillBox();
    
    connect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
}

void PComboBox::fillBox()
{
    for (QMap<QString, QVariant>::const_iterator it = m_valueList.begin(); it != m_valueList.end(); it++)
    {
        m_edit->insertItem(it.key());
    }
}

QVariant PComboBox::value() const
{
    QMap<QString, QVariant>::const_iterator it = m_valueList.find(m_edit->currentText());
    if (it == m_valueList.end())
        return QVariant("");
    return QVariant(it.data());
}

void PComboBox::setValue(const QVariant &value, bool emitChange)
{
#if QT_VERSION >= 0x030100
    if (!value.isNull())
#else
    if (value.canCast(QVariant::String))
#endif
    {
        disconnect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
        m_edit->setCurrentText(findDescription(value));
        connect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
        if (emitChange)
            emit propertyChanged(m_property, value);
    }
}

void PComboBox::updateProperty(int /*val*/)
{
    emit propertyChanged(m_property, value());
}

QString PComboBox::findDescription(const QVariant &value)
{
    for (QMap<QString, QVariant>::const_iterator it = m_valueList.begin(); it != m_valueList.end(); ++ it)
    {
        if (it.data() == value)
            return it.key();
    }
    return "";
}

void PComboBox::setValueList(const QMap<QString, QVariant> &valueList)
{
    m_valueList = valueList;
    m_edit->clear();
    fillBox();
}

#ifndef PURE_QT
#include "pcombobox.moc"
#endif
