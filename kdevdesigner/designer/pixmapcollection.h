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

#ifndef PIXMAPCOLLECTION_H
#define PIXMAPCOLLECTION_H

#include <qstring.h>
#include <qpixmap.h>
#include <q3valuelist.h>
#include "designerappiface.h"

class Q3MimeSourceFactory;
class Project;

class PixmapCollection
{
public:
    struct Pixmap
    {
	QPixmap pix;
	QString name;
	QString absname;
	Q_DUMMY_COMPARISON_OPERATOR( Pixmap )
    };

    PixmapCollection( Project *pro );
    ~PixmapCollection();

    bool addPixmap( const Pixmap &pix, bool force = TRUE );
    void removePixmap( const QString &name );
    QPixmap pixmap( const QString &name );

    Q3ValueList<Pixmap> pixmaps() const;
    bool isEmpty() const;

    void setActive( bool b );

    void load( const QString& filename );

    DesignerPixmapCollection *iFace();

private:
    QString unifyName( const QString &n );
    void savePixmap( Pixmap &pix );

    QString imageDir() const;
    void mkdir();

private:
    Q3ValueList<Pixmap> pixList;
    Q3MimeSourceFactory *mimeSourceFactory;
    Project *project;
    DesignerPixmapCollectionImpl *iface;

};

#endif
