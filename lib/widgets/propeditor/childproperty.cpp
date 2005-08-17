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
#include "childproperty.h"

#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsizepolicy.h>

#include "multiproperty.h"

namespace PropertyLib{

ChildProperty::ChildProperty(MultiProperty *parent, int type, ChildPropertyType childType, const QString &name,
    const QString &description, const QVariant &value, bool save, bool readOnly)
    :Property(type, name, description, value, save, readOnly), m_parent(parent), m_childType(childType)
{
}

ChildProperty::ChildProperty(MultiProperty *parent, const QString & name, ChildPropertyType childType,
    const QMap<QString, QVariant> &v_valueList, const QString &description,
    const QVariant &value, bool save, bool readOnly)
    :Property(name, v_valueList, description, value, save, readOnly), m_parent(parent), m_childType(childType)
{
}

void ChildProperty::setValue(const QVariant &value, bool // rememberOldValue
                             )
{
    qWarning("ChildProperty::setValue");
    if (!m_parent->valid())
        return;
    switch (m_parent->type())
    {
        case Size:
        {
            qWarning("ChildProperty::setValue for QSize");
            QSize v = m_parent->value().toSize();
            if (m_childType == Size_Height)
                v.setHeight(value.toInt());
            else if (m_childType == Size_Width)
                v.setWidth(value.toInt());
            m_parent->setValue(v);
            break;
        }
        case Point:
        {
            qWarning("ChildProperty::setValue for QPoint");
            QPoint v = m_parent->value().toPoint();
            if (m_childType == Point_X)
                v.setX(value.toInt());
            else if (m_childType == Point_Y)
                v.setY(value.toInt());
            m_parent->setValue(v);
            break;
        }
        case Rect:
        {
            qWarning("ChildProperty::setValue for QRect");
            QRect v = m_parent->value().toRect();
            if (m_childType == Rect_X)
                v.setX(value.toInt());
            else if (m_childType == Rect_Y)
                v.setY(value.toInt());
            else if (m_childType == Rect_Width)
                v.setWidth(value.toInt());
            else if (m_childType == Rect_Height)
                v.setHeight(value.toInt());
            m_parent->setValue(v);
            break;
        }
        case SizePolicy:
        {
            qWarning("ChildProperty::setValue for QSizePolicy");
            QSizePolicy v = qvariant_cast<QSizePolicy>(m_parent->value());
            if (m_childType == SizePolicy_HorData)
                v.setHorData(QSizePolicy::SizeType(value.toInt()));
            else if (m_childType == SizePolicy_VerData)
                v.setVerData(QSizePolicy::SizeType(value.toInt()));
            else if (m_childType == SizePolicy_HorStretch)
                v.setHorStretch(value.toInt());
            else if (m_childType == SizePolicy_VerStretch)
                v.setVerStretch(value.toInt());
            m_parent->setValue(v);
            break;
        }
    }
}

QVariant ChildProperty::value( ) const
{
    if (!m_parent->valid())
        return QVariant();
    switch (m_parent->type())
    {
        case Size:
            if (m_childType == Size_Height)
                return m_parent->value().toSize().height();
            else if (m_childType == Size_Width)
                return m_parent->value().toSize().width();
        case Point:
            if (m_childType == Point_X)
                return m_parent->value().toPoint().x();
            else if (m_childType == Point_Y)
                return m_parent->value().toPoint().y();
        case Rect:
            if (m_childType == Rect_X)
                return m_parent->value().toRect().x();
            else if (m_childType == Rect_Y)
                return m_parent->value().toRect().y();
            else if (m_childType == Rect_Width)
                return m_parent->value().toRect().width();
            else if (m_childType == Rect_Height)
                return m_parent->value().toRect().height();
        case SizePolicy:
            if (m_childType == SizePolicy_HorData)
                return qvariant_cast<QSizePolicy>(m_parent->value()).horData();
            else if (m_childType == SizePolicy_VerData)
                return qvariant_cast<QSizePolicy>(m_parent->value()).verData();
            else if (m_childType == SizePolicy_HorStretch)
                return qvariant_cast<QSizePolicy>(m_parent->value()).horStretch();
            else if (m_childType == SizePolicy_VerStretch)
                return qvariant_cast<QSizePolicy>(m_parent->value()).verStretch();
    }
    return QVariant();
}

}
