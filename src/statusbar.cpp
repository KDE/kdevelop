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
#include <qlabel.h>
#include <qfontmetrics.h>


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

	// stretcher
	addWidget(new QWidget(this), 1);

	_status = new QLabel(this);
	_status->setMinimumWidth(_status->fontMetrics().width("OVR, ro"));
	_status->setAlignment(QWidget::AlignCenter);
	addWidget(_status, 0, true);

	_cursorPosition = new QLabel(this);
	_cursorPosition->setMinimumWidth(_cursorPosition->fontMetrics().width("Line: xxxxx, Col: xxx"));
	addWidget(_cursorPosition, 0, true);
	
    _modified = new QLabel(this);
    _modified->setFixedWidth(_modified->fontMetrics().width("*"));
    addWidget(_modified, 0, true);

    widget = 0; 

	setEditorStatusVisible(false);
}


StatusBar::~StatusBar()
{}


void StatusBar::setEditorStatusVisible(bool visible)
{
  // Note: I tried to hide/show the widgets here, but that
  // causes flicker, so I just set them to be empty.
		
  if (!visible)
	{
	  _status->setText("");
	  _modified->setText("");
	  _cursorPosition->setText("");
	}
}


void StatusBar::setStatus(const QString &str)
{
  _status->setText(str);
}


void StatusBar::setCursorPosition(int line, int col)
{
  _cursorPosition->setText(QString("Line: %1, Col: %2").arg(line).arg(col));
}


void StatusBar::setModified(bool isModified)
{
  if (isModified)
	_modified->setText("*");
  else
	_modified->setText("");
}


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
	// Note: I commented this out, as it completely breaks the
	// functionality of the toolbar. It seems like it isn't used
	// anywhere anyway. (mhk)
	
/*    if ( e->type() == QEvent::ChildInserted) {
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
*/
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
