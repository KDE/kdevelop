//
// C++ Interface: bookmarks_config
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef BOOKMARKS_CONFIG_H
#define BOOKMARKS_CONFIG_H

/**
@author KDevelop Authors
*/
class BookmarksConfig
{

public:
	BookmarksConfig();
    ~BookmarksConfig();
	
    bool toolTip();
    unsigned int codeline();
    unsigned int context();
    void setCodeline( unsigned int );
    void readConfig();
    void setContext( unsigned int );
    void setToolTip( bool );
    void writeConfig();
	
private:
	bool _tooltip;
	unsigned int _codeline;	
	unsigned int _context;

};

#endif
