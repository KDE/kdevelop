/***************************************************************************
 *   Copyright (C) 2003-2004 by Alexander Dymo                             *
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
#include "pstringlistedit.h"

#include <qlineedit.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qpainter.h>

#include <klocale.h>
#include <kpushbutton.h>
#include <keditlistbox.h>
#include <kstdguiitem.h>

namespace PropertyLib{

PStringListEdit::PStringListEdit(MultiProperty *property, QWidget *parent, const char *name)
     :PropertyWidget(property, parent, name)
{
    l = new QHBoxLayout(this);

    edit = new QLineEdit(this);
    edit->setReadOnly(true);
    edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(edit);
    pbSelect = new QPushButton("...", this);
    pbSelect->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    l->addWidget(pbSelect);

    connect(pbSelect, SIGNAL(clicked()), this, SLOT(showEditor()));
}

QVariant PStringListEdit::value() const
{
    return QVariant(m_list);
}

void PStringListEdit::setValue(const QVariant &value, bool emitChange)
{
    m_list = value.toStringList();
    edit->setText(value.toStringList().join(", "));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PStringListEdit::showEditor()
{
    QDialog* dia = new QDialog(this, "stringlist_dialog", true);
    QVBoxLayout *dv = new QVBoxLayout(dia, 2);

    KEditListBox *select = new KEditListBox(dia, "select_char");
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

    select->insertStringList(m_list);

    if (dia->exec() == QDialog::Accepted)
    {
        m_list = select->items();
        edit->setText(select->items().join(", "));
    }
    delete dia;

    emit propertyChanged(m_property, m_list);
}

void PStringListEdit::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, value.toStringList().join(", "));
}

}

#ifndef PURE_QT
#include "pstringlistedit.moc"
#endif
