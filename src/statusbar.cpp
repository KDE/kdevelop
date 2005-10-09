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
#include <qfontmetrics.h>
#include <qlabel.h>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kdeversion.h>
#include <ktexteditor/view.h>

#include "statusbar.h"
#include "partcontroller.h"

#include "qdebug.h"

KDevStatusBar::KDevStatusBar(QWidget *parent, const char *name)
    : KStatusBar(parent), _activePart(0), _view(0)
{
	QWidget * w = new QWidget( this );
	addWidget( w, 1, true );
	w->hide();

	_status = new QLabel( this );
	_status->setMinimumWidth(_status->fontMetrics().width("Line: XXXXX Col: XXX  OVR NORM * "));
	_status->setAlignment(Qt::AlignCenter);
	addWidget(_status, 0, true);

	connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
		this, SLOT(activePartChanged(KParts::Part*)));
}


KDevStatusBar::~KDevStatusBar()
{}

void KDevStatusBar::activePartChanged(KParts::Part *part)
{
    if ( _activePart && _activePart->widget() )
        disconnect( _activePart->widget(), 0, this, 0 );

    _activePart = part;

    if (part)
    {
        _view = qobject_cast<KTextEditor::View *>(part->widget());
        if (_view) {
            connect( _view, SIGNAL( viewStatusMsg( const QString & ) ), // harryF: ### TODO
                    this, SLOT( setStatus( const QString & ) ) );
            _status->show();

            connect(_view, SIGNAL(cursorPositionChanged(View*)), this, SLOT(cursorPositionChanged()));
            cursorPositionChanged();
        }
        else
        {
            // we can't produce any status data, hide the status box
            _status->hide();
        }
    }
}

void KDevStatusBar::cursorPositionChanged()
{
    if (_view) {
        KTextEditor::Cursor cursor = _view->cursorPosition();
        setCursorPosition(cursor.line(), cursor.column());
    }
}

void KDevStatusBar::setStatus(const QString &str)
{
	_status->setText(str);
}


void KDevStatusBar::setCursorPosition(int line, int col)
{
	_status->setText(i18n(" Line: %1 Col: %2 ").arg(line+1).arg(col));
}

void KDevStatusBar::addWidget ( QWidget *widget, int stretch, bool permanent)
{
	KStatusBar::addWidget(widget,stretch,permanent);

	if(widget->sizeHint().height() + 4 > height())
		setFixedHeight(widget->sizeHint().height() + 4);
}

#include "statusbar.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
