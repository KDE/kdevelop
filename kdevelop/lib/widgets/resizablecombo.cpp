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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "resizablecombo.h"

#include "kcomboview.h"

#include <qevent.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qwhatsthis.h>

#include <klocale.h>

static const char * resize_xpm[] = {
"9 18 2 1",
" 	c None",
".	c #000000",
"    .    ",
"         ",
"    .    ",
"         ",
"    .    ",
"         ",
"  . . .  ",
" ..   .. ",
".........",
" ..   .. ",
"  . . .  ",
"         ",
"    .    ",
"         ",
"    .    ",
"         ",
"    .    ",
"         "};

ResizableCombo::ResizableCombo(KComboView *view, QWidget *parent, const char *name):
    QWidget(parent, name), m_sizer(0), m_combo(view)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    view->reparent(this, QPoint(0,0));
    l->addWidget(view);

    m_sizer = new MyPushButton(this);
    m_sizer->setPixmap(QPixmap(resize_xpm));
    QWhatsThis::add(m_sizer, i18n("Drag this to resize the combobox."));
    l->addWidget(m_sizer);
}

void MyPushButton::mousePressEvent( QMouseEvent * e )
{
    m_resizing = true;
    m_pressedPos = e->globalPos();
    m_width = m_combo->m_combo->width();
    QPushButton::mousePressEvent(e);
}

void MyPushButton::mouseReleaseEvent( QMouseEvent * e )
{
    m_resizing = false;
    QPushButton::mouseReleaseEvent(e);
}

void MyPushButton::mouseMoveEvent( QMouseEvent * e )
{
    if (m_resizing)
        m_combo->m_combo->setMinimumWidth(m_width + e->globalPos().x() - pressedPos().x());

    QPushButton::mouseMoveEvent(e);
}

MyPushButton::MyPushButton( ResizableCombo * parent, const char * name )
    :QPushButton(parent, name), m_resizing(false), m_combo(parent)
{
    setFocusPolicy(NoFocus);
    setFlat(true);
    m_width = m_combo->m_combo->width();
}


#include "resizablecombo.moc"
