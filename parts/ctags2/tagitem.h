/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TAGITEM_H
#define TAGITEM_H

#include <q3listview.h>

class TagItem : public Q3ListViewItem
{
public:
    TagItem(Q3ListView * lv, QString const & tag, QString const & type, QString const & file, QString const & pattern );
    ~TagItem();

	QString tag;
	QString type;
	QString file;
	QString pattern;
};

#endif
