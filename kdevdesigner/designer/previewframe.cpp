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

#include <qvbox.h>
#include <qpainter.h>

#include "previewframe.h"

PreviewFrame::PreviewFrame( QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);

    PreviewWorkspace * w = new PreviewWorkspace( this );
    w->setEraseColor(colorGroup().dark());
    previewWidget = new PreviewWidget( w );
    previewWidget->move( 10, 10 );
}

void PreviewFrame::setPreviewPalette(QPalette pal)
{
    previewWidget->setPalette(pal);
}

void PreviewWorkspace::paintEvent( QPaintEvent* )
{
    QPainter p ( this );
    p.setPen( QPen( white ) );
    p.drawText ( 0, height() / 2,  width(), height(), AlignHCenter,
		"The moose in the noose\nate the goose who was loose." );
}

