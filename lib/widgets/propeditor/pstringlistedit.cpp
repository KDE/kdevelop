/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

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
#include <klocale.h>

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdialog.h>

#include <keditlistbox.h>

#include "pstringlistedit.h"
#include "propertyeditor.h"

PStringListEdit::PStringListEdit( const PropertyEditor * editor, const QString pname, const QVariant value, QWidget * parent, const char * name )
 : QWidget(parent, name)
{
    l = new QHBoxLayout(this);

    edit = new QLineEdit(this);
    edit->setReadOnly(true);
    l->addWidget(edit);
    pbSelect = new QPushButton("...", this);
    l->addWidget(pbSelect);

    connect(pbSelect, SIGNAL(clicked()), this, SLOT(showEditor()));
    setValue(value, false);
    setPName(pname);
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PStringListEdit::value() const
{
    return QVariant(m_list);
}

void PStringListEdit::setValue(const QVariant value, bool emitChange)
{
    m_list = value.toStringList();
    if (emitChange)
        emit propertyChanged(pname(), value);
}

void PStringListEdit::showEditor( )
{
    QDialog* dia = new QDialog(this, "stringlist_dialog", true);
    QVBoxLayout *dv = new QVBoxLayout(dia, 2);

    KEditListBox *select = new KEditListBox(dia, "select_char");
    dv->addWidget(select);

    QHBoxLayout *dh = new QHBoxLayout(dv, 6);
    QPushButton *pbOk = new QPushButton(i18n("Ok"), dia);
    QPushButton *pbCancel = new QPushButton(i18n("Cancel"), dia);
    QSpacerItem *si = new QSpacerItem(30, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(pbOk, SIGNAL(clicked()), dia, SLOT(accept()));
    connect(pbCancel, SIGNAL(clicked()), dia, SLOT(reject()));

    dh->addItem(si);
    dh->addWidget(pbOk);
    dh->addWidget(pbCancel);

    select->insertStringList(m_list);
    
    if (dia->exec() == QDialog::Accepted)
    {
        m_list = select->items();
    }
    delete dia;

    emit propertyChanged(pname(), m_list);
}

#include "pstringlistedit.moc"
