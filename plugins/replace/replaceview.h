/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __REPLACEVIEW_H__
#define __REPLACEVIEW_H__

#include <k3listview.h>

#include <QString>
#include <QRegExp>
//Added by qt3to4:
#include <QTextStream>

class QTextStream;
class QWidget;
class ReplaceItem;

class ReplaceView : public K3ListView
{
    Q_OBJECT

signals:
    void editDocument( const QString &, int );

public:
    ReplaceView( QWidget *);
    ReplaceItem * firstChild() const;
    void setReplacementData( QRegExp const &, QString const & );
    void showReplacementsForFile( QTextStream &, QString const & );
    void makeReplacementsForFile( QTextStream & istream, QTextStream & ostream, ReplaceItem const * fileitem );

private slots:
    void slotMousePressed(int, Q3ListViewItem *, const QPoint &, int);
    void slotClicked( Q3ListViewItem * );

private:
    QRegExp _regexp;
    QString _replacement;
    ReplaceItem * _latestfile;

friend class ReplaceItem;

};




#endif
