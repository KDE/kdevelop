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
#include "psizepolicyedit.h"

#include <qlineedit.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qsizepolicy.h>

namespace PropertyLib{

PSizePolicyEdit::PSizePolicyEdit(MultiProperty* property, const QMap<QString, QVariant> &spValues, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name), m_spValues(spValues)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QLineEdit(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);

    m_edit->setReadOnly(true);
}

QVariant PSizePolicyEdit::value() const
{
    return m_value;
}

void PSizePolicyEdit::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, QString("%1/%2/%3/%4").arg(findValueDescription(value.toSizePolicy().horData())).arg(findValueDescription(value.toSizePolicy().verData())).arg(value.toSizePolicy().horStretch()).arg(value.toSizePolicy().verStretch()));
}

void PSizePolicyEdit::setValue(const QVariant& value, bool emitChange)
{
    m_value = value;
    m_edit->setText(QString("%1/%2/%3/%4").arg(findValueDescription(value.toSizePolicy().horData())).arg(findValueDescription(value.toSizePolicy().verData())).arg(value.toSizePolicy().horStretch()).arg(value.toSizePolicy().verStretch()));

    if (emitChange)
        emit propertyChanged(m_property, value);
}

QString PSizePolicyEdit::findValueDescription(QVariant val) const
{
//    qWarning("PSizePolicyEdit::findValueDescription : %d", val.toInt());
    for (QMap<QString, QVariant>::const_iterator it = m_spValues.begin(); it != m_spValues.end(); ++ it)
    {
        if (it.data() == val)
            return it.key();
    }
    return "";
}

}

#ifndef PURE_QT
#include "psizepolicyedit.moc"
#endif
