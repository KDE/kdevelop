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
#include "pfontcombo.h"

#ifndef PURE_QT
#include <kfontcombo.h>
#else
#include <qcombobox.h>
#endif

#include <qlayout.h>

#ifdef PURE_QT
#include <qfontdatabase.h>
#endif

namespace PropertyLib{

PFontCombo::PFontCombo(MultiProperty *property, QWidget *parent, const char *name)
    :PropertyWidget(property, parent, name)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new KFontCombo(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);
    
    /*adymo: KFontCombo seems to have a bug: when it is not editable, the signals
    activated(int) and textChanged(const QString &) are not emitted*/
#ifdef PURE_QT
    QFontDatabase fonts;
    m_edit->insertStringList(fonts.families());
    connect(m_edit, SIGNAL(activated(const QString &)), this, SLOT(updateProperty(const QString&)));
#else
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
#endif
}

QVariant PFontCombo::value() const
{
#ifndef PURE_QT
    return QVariant(m_edit->currentFont());
#else
    return QVariant(m_edit->currentText());
#endif
}

void PFontCombo::setValue(const QVariant &value, bool emitChange)
{
    disconnect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
#ifndef PURE_QT
    m_edit->setCurrentFont(value.toString());
#else
    m_edit->setCurrentText(value.toString());
#endif
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PFontCombo::updateProperty(const QString &val)
{
    emit propertyChanged(m_property, QVariant(val));
}

}

#ifndef PURE_QT
#include "pfontcombo.moc"
#endif
