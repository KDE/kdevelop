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

#ifndef GLOBALDEFS_H
#define GLOBALDEFS_H

#include <qcolor.h>
#include <qapplication.h>

#define BOXLAYOUT_DEFAULT_MARGIN 11
#define BOXLAYOUT_DEFAULT_SPACING 6

#ifndef NO_STATIC_COLORS
static QColor *backColor1 = 0;
static QColor *backColor2 = 0;
static QColor *selectedBack = 0;

static void init_colors()
{
    if ( backColor1 )
	return;

#if 0 // a calculated alternative for backColor1
    QColorGroup myCg = qApp->palette().active();
    int h1, s1, v1;
    int h2, s2, v2;
    myCg.color( QColorGroup::Base ).hsv( &h1, &s1, &v1 );
    myCg.color( QColorGroup::Background ).hsv( &h2, &s2, &v2 );
    QColor c( h1, s1, ( v1 + v2 ) / 2, QColor::Hsv );
#endif

    backColor1 = new QColor(  250, 248, 235 );
    backColor2 = new QColor( 255, 255, 255 );
    selectedBack = new QColor( 230, 230, 230 );
}

#endif

#endif
