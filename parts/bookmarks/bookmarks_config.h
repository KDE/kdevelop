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

#ifndef BOOKMARKS_CONFIG_H
#define BOOKMARKS_CONFIG_H

/**
@author KDevelop Authors
*/
class BookmarksConfig
{

public:
	
	enum CodeLineType { Never = 0, Token, Always };
	
	BookmarksConfig();
    ~BookmarksConfig();
	
    bool toolTip();
    void setToolTip( bool );
	
	CodeLineType codeline();
	void setCodeline( CodeLineType );
	
    unsigned int context();
    void setContext( unsigned int );

	QString token();
	void setToken( QString const & );
	    
	void readConfig();
    void writeConfig();
	
private:
	bool _tooltip;
	CodeLineType _codeline;
	unsigned int _context;
	QString _token;

};

#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
