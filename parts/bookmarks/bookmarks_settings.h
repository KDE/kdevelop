//
// C++ Interface: 
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

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

