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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>

#ifndef PURE_QT
#include <qdialog.h>

#include <kcharselect.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#endif

#include "psymbolcombo.h"

namespace PropertyLib{

PSymbolCombo::PSymbolCombo(MultiProperty *property, QWidget *parent, const char *name)
    :PropertyWidget(property, parent, name)
{
    l = new QHBoxLayout(this);

    m_edit = new QLineEdit(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_edit->setMaxLength(1);
    l->addWidget(m_edit);
    m_select = new QPushButton("...", this);
    m_select->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
    l->addWidget(m_select);

#ifdef PURE_QT
    m_select->hide();
#endif

    connect(m_select, SIGNAL(clicked()), this, SLOT(selectChar()));
    connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
}

QVariant PSymbolCombo::value() const
{
    if (!(m_edit->text().isNull()))
        return QVariant(QString("%1").arg(m_edit->text().at(0).unicode()));
    else
        return QVariant(0);
}

void PSymbolCombo::setValue(const QVariant &value, bool emitChange)
{
#if QT_VERSION >= 0x030100
    if (!(value.isNull()))
#else
    if (value.canCast(QVariant::Int))
#endif
    {
        disconnect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
        m_edit->setText(QChar(value.toInt()));
        connect(m_edit, SIGNAL(textChanged(const QString&)), this, SLOT(updateProperty(const QString&)));
        if (emitChange)
            emit propertyChanged(m_property, value);
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

    if (!(m_edit->text().isNull()))
        select->setChar(m_edit->text().at(0));

    if (dia->exec() == QDialog::Accepted)
    {
        m_edit->setText(select->chr());
    }
    delete dia;
#endif
}

void PSymbolCombo::updateProperty(const QString& val)
{
    emit propertyChanged(m_property, QVariant(QString("%1").arg(val.at(0).unicode())));
}

void PSymbolCombo::drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value)
{
    p->setBrush(cg.background());
    p->setPen(Qt::NoPen);
    p->drawRect(r);
    p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::SingleLine, QChar(value.toInt()));
}

}

#ifndef PURE_QT
#include "psymbolcombo.moc"
#endif
