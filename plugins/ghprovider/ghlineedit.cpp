/*
    SPDX-FileCopyrightText: 2012-2013 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ghlineedit.h"

#include <QKeyEvent>
#include <QTimer>


namespace gh
{

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(500);
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
    m_timer->start();
    QLineEdit::keyPressEvent(e);
}

void LineEdit::timeOut()
{
    m_timer->stop();
    if (!text().isEmpty())
        emit returnPressed();
}

} // End of namespace gh
