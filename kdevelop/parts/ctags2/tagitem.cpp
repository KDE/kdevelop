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

#include "tagitem.h"

TagItem::TagItem( QListView * lv, QString const & tag, QString const & type, QString const & file, QString const & pattern )
	: QListViewItem( lv ), tag(tag), type(type), file(file), pattern(pattern)
{}

TagItem::~TagItem()
{
}



