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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "pyesnobutton.h"

#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>

#ifndef PURE_QT
#include <klocale.h>
#else
#include "compat_tools.h"
#endif

namespace PropertyLib{

PYesNoButton::PYesNoButton(MultiProperty* property, QWidget* parent, const char* name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QPushButton(this);
    m_edit->setToggleButton(true);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);

    connect(m_edit, SIGNAL(toggled(bool)), this, SLOT(updateProperty(bool)));
}

QVariant PYesNoButton::value() const
{
    return QVariant(m_edit->isOn());
}

void PYesNoButton::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
    PropertyWidget::drawViewer(p, cg, r, value.toBool() ? i18n("Yes") : i18n("No"));
}

void PYesNoButton::setValue(const QVariant& value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(toggled(bool)), this, SLOT(updateProperty(bool)));
    m_edit->setDown(value.toBool());
    value.toBool() ? m_edit->setText(i18n("Yes")) : m_edit->setText(i18n("No"));
    connect(m_edit, SIGNAL(toggled(bool)), this, SLOT(updateProperty(bool)));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PYesNoButton::updateProperty(bool toggled)
{
    toggled ? m_edit->setText(i18n("Yes")) : m_edit->setText(i18n("No"));
    emit propertyChanged(m_property, value());
}

}

#ifndef PURE_QT
#include "pyesnobutton.moc"
#endif
