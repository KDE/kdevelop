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

#include "pfontcombo.h"
#include "propertyeditor.h"

#ifdef PURE_QT
#include <qfontdatabase.h>
#endif

PFontCombo::PFontCombo (const PropertyEditor *editor, const QString pname, const QVariant value, QWidget *parent, const char *name):
#ifndef PURE_QT
    KFontCombo(parent, name)
#else
    QComboBox(parent, name)
#endif
{
    //AD: KFontCombo seems to have a bug: when it is not editable, the signals
    //activated(int) and textChanged(const QString &) are not emitted
//    setEditable(false);
#ifdef PURE_QT
    QFontDatabase fonts;
    insertStringList( fonts.families() );
#endif

    setPName(pname);
    setValue(value, false);
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

PFontCombo::PFontCombo (const PropertyEditor *editor, const QString pname, const QVariant value, const QStringList &fonts, QWidget *parent, const char *name):
#ifndef PURE_QT
    KFontCombo(fonts, parent, name)
#else
    QComboBox(parent, name)
#endif
{
    //AD: KFontCombo seems to have a bug: when it is not editable, the signals
    //activated(int) and textChanged(const QString &) are not emitted
//    setEditable(false);
#ifdef PURE_QT
    insertStringList( fonts );
#endif

    setValue(value, false);
    setPName(pname);
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PFontCombo::value() const
{
#ifndef PURE_QT
    return QVariant(currentFont());
#else
    return QVariant(currentText());
#endif
}

void PFontCombo::setValue(const QVariant value, bool emitChange)
{
#ifndef PURE_QT
    setCurrentFont(value.toString());
#else
    setCurrentText(value.toString());
#endif
    if (emitChange)
        emit propertyChanged(pname(), value);
}

void PFontCombo::updateProperty(const QString &val)
{
    emit propertyChanged(pname(), QVariant(val));
}

#ifndef PURE_QT
#include "pfontcombo.moc"
#endif
