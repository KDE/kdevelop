/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#ifndef PIXMAPCHOOSER_H
#define PIXMAPCHOOSER_H

#include <q3filedialog.h>
#include <q3scrollview.h>
#include <qpixmap.h>
#include <q3url.h>
//Added by qt3to4:
#include <Q3StrList>

class FormWindow;

class PixmapView : public Q3ScrollView,
		   public Q3FilePreview
{
    Q_OBJECT

public:
    PixmapView( QWidget *parent );
    void setPixmap( const QPixmap &pix );
    void drawContents( QPainter *p, int, int, int, int );
    void previewUrl( const Q3Url &u );

private:
    QPixmap pixmap;

};

class ImageIconProvider : public Q3FileIconProvider
{
    Q_OBJECT

public:
    ImageIconProvider( QWidget *parent = 0, const char *name = 0 );
    ~ImageIconProvider();

    const QPixmap *pixmap( const QFileInfo &fi );

private:
    Q3StrList fmts;
    QPixmap imagepm;

};

QPixmap qChoosePixmap( QWidget *parent, FormWindow *fw = 0, const QPixmap &old = QPixmap(),  QString *fn = 0 );
QStringList qChoosePixmaps( QWidget *parent );

#endif
