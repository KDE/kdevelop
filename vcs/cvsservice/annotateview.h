/***************************************************************************
 *   Copyright (C) 2005 by Robert Gruber                                   *
 *   rgruber@users.sourceforge.net                                         *
 *                                                                         *
 *   This file has been taken from cervisia an adapted to fit my needs:    *
 *   Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>        *
 *   Copyright (c) 2003-2005 André Wöbbeking <Woebbeking@web.de>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ANNOTATEVIEW_H
#define ANNOTATEVIEW_H


#include <klistview.h>
#include <qtooltip.h>

class QDateTime;
class AnnotatePage;

/**
 * This is the main widget of each page.
 * It shows the user the output of cvs annotate.
 * The user can click any line of this view in order
 * to get a new page which shows the annotate output
 * of the clicked revision.
 */
class AnnotateView : public KListView, public QToolTip
{
    Q_OBJECT

public:

    explicit AnnotateView(AnnotatePage *parent, const char *name=0 );

    void addLine(QString rev, QString author, QDateTime date, QString content, 
            QString comment, bool odd);

    virtual QSize sizeHint() const;
    void maybeTip( const QPoint & p );

private:
    AnnotatePage * m_page;

public slots:
    void itemClicked(Q3ListViewItem *item);
};


#endif
