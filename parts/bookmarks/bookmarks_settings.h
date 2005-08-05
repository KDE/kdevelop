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

#ifndef BOOKMARKS_SETTINGS_H
#define BOOKMARKS_SETTINGS_H

#include "bookmarks_settings_base.h"

class BookmarksPart;

class BookmarkSettings : public BookmarkSettingsBase
{
  Q_OBJECT

public:
  BookmarkSettings( BookmarksPart * part, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
  ~BookmarkSettings();

public slots:
	void slotAccept();
	
protected:

protected slots:

private:

	BookmarksPart * m_part;

};

#endif

