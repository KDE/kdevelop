/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qtimer.h>
#include <kdebug.h>

#include "statusbar.h"


StatusBar::StatusBar(QWidget *parent, const char *name)
    : QStatusBar(parent, name)
{
    setSizeGripEnabled(false);
    
    QLineEdit *le = new QLineEdit(this);
    //    le->setFont(KGlobalSettings::fixedFont());
    setFixedHeight(le->sizeHint().height());
    delete le;

    timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );
    
    widget = 0; 
}


StatusBar::~StatusBar()
{}


void StatusBar::message(const QString &str)
{
    msg = str;
    repaint();

    if (widget)
        timer->start(1500, true);
}


void StatusBar::timeout()
{
    msg = QString::null;
    repaint();
}


bool StatusBar::event(QEvent *e)
{
    if ( e->type() == QEvent::ChildInserted) {
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        if (ce->child()->isWidgetType()) {
            delete widget;
            delete layout();
            widget = static_cast<QWidget*>(ce->child());
            QBoxLayout *lay = new QVBoxLayout(this);
            lay->addWidget(widget);
            lay->activate();
            timeout();
            return true;
        }
    } else if (e->type() == QEvent::ChildRemoved) {
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        //        if (ce->child()->isWidgetType()) {
        if (ce->child() == widget) {
            widget = 0;
            return true;
        }
    }

    return QStatusBar::event(e);
}


void StatusBar::paintEvent(QPaintEvent *e)
{
    if (!msg.isNull()) {
        QPainter p(this);
        p.setPen(colorGroup().text());
        p.drawText(4, 0, width()-8, height(),
                   AlignVCenter+SingleLine, msg);
    } else {
        QStatusBar::paintEvent(e);
    }
}
#include "statusbar.moc"
