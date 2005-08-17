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
#include "plinestyleedit.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QHBoxLayout>

namespace PropertyLib {

    const char *nopen[]={
    "48 16 1 1",
    ". c None",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    const char *solid[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".###########################################....",
    ".###########################################....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    const char *dash[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".#########..#########..#########..##########....",
    ".#########..#########..#########..##########....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    const char *dashdot[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".#########..##..#########..##..#########..##....",
    ".#########..##..#########..##..#########..##....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    const char *dashdotdot[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".#########..##..##..#########..##..##..#####....",
    ".#########..##..##..#########..##..##..#####....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};


PLineStyleEdit::PLineStyleEdit(MultiProperty* property, QWidget* parent, const char* name): PropertyWidget(property, parent)
{
    QHBoxLayout *l = new QHBoxLayout(this, 0, 0);
    m_edit = new QComboBox(this);
    m_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    l->addWidget(m_edit);

    m_edit->insertItem(QPixmap(nopen));
    m_edit->insertItem(QPixmap(solid));
    m_edit->insertItem(QPixmap(dash));
    m_edit->insertItem(QPixmap(dashdot));
    m_edit->insertItem(QPixmap(dashdotdot));    

    connect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
}

QVariant PLineStyleEdit::value() const
{
    return m_edit->currentItem();
}

void PLineStyleEdit::drawViewer(QPainter* p, const QColorGroup& cg, const QRect& r, const QVariant& value)
{
    p->setPen(Qt::NoPen);
    p->setBrush(cg.background());
    p->drawRect(r);
    
    if (!value.canCast(QVariant::Int))
    if ((value.toInt() > 5) || (value.toInt() < 0))
        return;
    
    switch (value.toInt()) {
    case 0:
        p->drawPixmap(r, QPixmap(nopen));
        break;
    case 1:
        p->drawPixmap(r, QPixmap(solid));
        break;
    case 2:
        p->drawPixmap(r, QPixmap(dash));
        break;
    case 3:
        p->drawPixmap(r, QPixmap(dashdot));
        break;
    case 4:
        p->drawPixmap(r, QPixmap(dashdot));
        break;
    case 5:
        p->drawPixmap(r, QPixmap(dashdotdot));
        break;
    }
}

void PLineStyleEdit::setValue(const QVariant& value, bool emitChange)
{
    if (!value.canCast(QVariant::Int))
        return;
    if ((value.toInt() > 5) || (value.toInt() < 0))
        return;
    disconnect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
    m_edit->setCurrentItem(value.toInt());
    connect(m_edit, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
    if (emitChange)
        emit propertyChanged(m_property, value);
}

void PLineStyleEdit::updateProperty(int val)
{
    emit propertyChanged(m_property, QVariant(val));
}

}

#ifndef PURE_QT
#include "plinestyleedit.moc"
#endif
