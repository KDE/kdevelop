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

#include <klistview.h>

#include <qstring.h>
#include <qregexp.h>

class QTextStream;
class QWidget;
class ReplaceItem;

class ReplaceView : public KListView
{
public:
    ReplaceView( QWidget *);
    ReplaceItem * firstChild() const;
    void setReplacementData( QRegExp const &, QString const & );
    void showReplacementsForFile( QTextStream &, QString const & );
    void makeReplacementsForFile( QTextStream & istream, QTextStream & ostream, ReplaceItem const * fileitem );

private:
    QRegExp _regexp;
    QString _replacement;
    ReplaceItem * _latestfile;

};




#endif
