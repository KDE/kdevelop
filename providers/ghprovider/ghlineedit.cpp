/* This file is part of KDevelop
 *
 * Copyright (C) 2012-2013 Miquel Sabaté <mikisabate@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <QKeyEvent>
#include <QtCore/QTimer>
#include <ghlineedit.h>


namespace gh
{

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &LineEdit::timeOut);
}

LineEdit::~LineEdit()
{
    /* There's nothing to do here! */
}

void LineEdit::keyPressEvent(QKeyEvent *e)
{
    m_timer->stop();
    if (e->key() == Qt::Key_Return) {
        e->accept();
        emit returnPressed();
        return;
    }
    m_timer->start(500);
    QLineEdit::keyPressEvent(e);
}

void LineEdit::timeOut()
{
    m_timer->stop();
    if (!text().isEmpty())
        emit returnPressed();
}

} // End of namespace gh
