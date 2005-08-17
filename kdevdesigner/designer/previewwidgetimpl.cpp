/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qobject.h"
//Added by qt3to4:
#include <QEvent>
#include <QCloseEvent>
#include "previewwidgetimpl.h"

PreviewWidget::PreviewWidget( QWidget *parent, const char *name )
    : PreviewWidgetBase( parent, name )
{
    // install event filter on child widgets
    QObjectList *l = queryList("QWidget");
    QObjectListIt it(*l);
    QObject * obj;
    while ((obj = it.current()) != 0) {
        ++it;
        obj->installEventFilter(this);
	((QWidget*)obj)->setFocusPolicy(NoFocus);
    }
}

void PreviewWidget::closeEvent(QCloseEvent *e)
{
    e->ignore();
}

bool PreviewWidget::eventFilter(QObject *, QEvent *e)
{
    switch ( e->type() ) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Enter:
    case QEvent::Leave:
	return TRUE; // ignore;
    default:
	break;
    }
    return FALSE;
}
