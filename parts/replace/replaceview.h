/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                                 *
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

class QWidget;
class ReplaceItem;

class ReplaceView : public KListView
{
public:
    ReplaceView( QWidget *);

    ReplaceItem * firstChild() const;

    void makeReplacements(QString const & pattern, QString const & replacement );
    void showReplacements( QStringList const & files, QString const & pattern, QString const & replacement );
};




#endif
