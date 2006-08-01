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

#include <QLayout>
#include <QLineEdit>
#include <qpainter.h>
#include <QTimer>
#include <qfontmetrics.h>
#include <QLabel>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kdeversion.h>
#include <ktexteditor/view.h>

#include "statusbar.h"
#include "kdevdocumentcontroller.h"

#include "qdebug.h"

KDevStatusBar::KDevStatusBar(QWidget *parent )
    : KStatusBar(parent), _activePart(0), _view(0)
{
    QWidget * w = new QWidget( this );
    addWidget( w, 1, true );
    w->hide();

    _status = new QLabel( this );
    _status->setMinimumWidth(_status->fontMetrics().width("Line: XXXXX Col: XXX  OVR NORM * "));
    _status->setAlignment(Qt::AlignCenter);
    addWidget(_status, 0, true);

    //FIXME PORT
//     connect(DocumentController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
//         this, SLOT(activePartChanged(KParts::Part*)));
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
            connect( _view, SIGNAL( viewModeChanged(KTextEditor::View*) ), SLOT( statusChanged() ) );
            _status->show();

            connect(_view, SIGNAL(cursorPositionChanged(KTextEditor::View*, const KTextEditor::Cursor&)), SLOT(statusChanged()));
            statusChanged();
        }
        else
        {
            // we can't produce any status data, hide the status box
            _status->hide();
        }
    }
}

void KDevStatusBar::statusChanged()
{
    if (_view) {
        KTextEditor::Cursor cursor = _view->cursorPosition();
        _status->setText(i18n(" Line: %1 Col: %2 ", cursor.line() + 1, cursor.column()) + _view->viewMode());
    }
}

void KDevStatusBar::addWidget ( QWidget *widget, int stretch, bool permanent)
{
    if ( permanent )
        KStatusBar::addPermanentWidget(widget,stretch);
    else
        KStatusBar::addWidget(widget,stretch);

    if(widget->sizeHint().height() + 4 > height())
        setFixedHeight(widget->sizeHint().height() + 4);
}

#include "statusbar.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
