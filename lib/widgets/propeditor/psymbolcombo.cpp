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
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>

#ifndef PURE_QT
#include <kcharselect.h>
#include <qdialog.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#endif

#include "psymbolcombo.h"
#include "propertyeditor.h"

PSymbolCombo::PSymbolCombo(const PropertyEditor *editor, const QString pname, const QVariant value, QWidget *parent, const char *name):
    QWidget(parent, name)
{
    l = new QHBoxLayout(this);

    edit = new QLineEdit(this);
    edit->setMaxLength(1);
    l->addWidget(edit);
    pbSelect = new QPushButton("...", this);
    l->addWidget(pbSelect);

#ifdef PURE_QT
    pbSelect->hide();
#endif

    connect(pbSelect, SIGNAL(clicked()), this, SLOT(selectChar()));
    setValue(value);
    setPName(pname);
    connect(edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PSymbolCombo::value() const
{
    if (!(edit->text().isNull()))
        return QVariant(QString("%1").arg(edit->text().at(0).unicode()));
    else
        return QVariant(0);
}

void PSymbolCombo::setValue(const QVariant value, bool emitChange)
{
#if QT_VERSION >= 0x030100
    if (!(value.isNull()))
#else
    if (value.canCast(QVariant::Int))

#endif
    {
        edit->setText(QChar(value.toInt()));
        if (emitChange)
            emit propertyChanged(pname(), value);
    }
}

void PSymbolCombo::selectChar()
{
#ifndef PURE_QT
    QDialog* dia = new QDialog(this, "select_dialog", true);
    QVBoxLayout *dv = new QVBoxLayout(dia, 2);

    KCharSelect *select = new KCharSelect(dia, "select_char");
    dv->addWidget(select);

    QHBoxLayout *dh = new QHBoxLayout(dv, 6);
    KPushButton *pbOk = new KPushButton(KStdGuiItem::ok(), dia);
    KPushButton *pbCancel = new KPushButton(KStdGuiItem::cancel(), dia);
    QSpacerItem *si = new QSpacerItem(30, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(pbOk, SIGNAL(clicked()), dia, SLOT(accept()));
    connect(pbCancel, SIGNAL(clicked()), dia, SLOT(reject()));

    dh->addItem(si);
    dh->addWidget(pbOk);
    dh->addWidget(pbCancel);

    if (!(edit->text().isNull()))
        select->setChar(edit->text().at(0));

    if (dia->exec() == QDialog::Accepted)
    {
        edit->setText(select->chr());
    }
    delete dia;
#endif
}

void PSymbolCombo::updateProperty(const QString& val)
{
    emit propertyChanged(pname(), QVariant(val));
}

#ifndef PURE_QT
#include "psymbolcombo.moc"
#endif
