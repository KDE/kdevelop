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

#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>

#include "bookmarks_config.h"

BookmarksConfig::BookmarksConfig()
{
	_context = 5;
	_codeline = Never; // 0 - Never, 1 - Only if Comment, 2 - Always
	_tooltip = true;
}


BookmarksConfig::~BookmarksConfig()
{
}


/*!
    \fn BookmarksConfig::writeConfig()
 */
void BookmarksConfig::writeConfig()
{
	KConfig *config = KGlobal::config();   
	config->setGroup("Bookmarks");
	
	config->writeEntry("Context", _context );
	config->writeEntry("Codeline", _codeline );
	config->writeEntry("ToolTip", _tooltip );
	config->writeEntry("Token", _token );

	config->sync();
}


/*!
    \fn BookmarksConfig::readConfig()
 */
void BookmarksConfig::readConfig()
{
	KConfig *config = KGlobal::config();
	config->setGroup("Bookmarks");
	
	_context = config->readPropertyEntry( "Context", 5 ).toInt();
	_tooltip = config->readBoolEntry( "ToolTip", true );
	_token = config->readEntry( "Token", "//" );
	unsigned int cl = config->readPropertyEntry( "Codeline", 0 ).toInt();
	
	switch( cl )
	{
		case 1:
			_codeline = Token;
		break;
		case 2: 
			_codeline = Always;
		break;
		default:
			_codeline = Never;
	}
	
	if ( _context > 15 ) _context = 15;
}


/*!
    \fn BookmarksConfig::toolTip
 */
bool BookmarksConfig::toolTip()
{
    return _tooltip;
}


/*!
    \fn BookmarksConfig::codeline
 */
BookmarksConfig::CodeLineType BookmarksConfig::codeline()
{
    return _codeline;
}


/*!
    \fn BookmarksConfig::context()
 */
unsigned int BookmarksConfig::context()
{
    return _context;
}


/*!
    \fn BookmarksConfig::setCodeline( CodeLineType )
 */
void BookmarksConfig::setCodeline( CodeLineType codeline )
{
	_codeline = codeline;
}

/*!
    \fn BookmarksConfig::setContext( unsigned int )
 */
void BookmarksConfig::setContext( unsigned int context )
{
	_context = context;
}


/*!
    \fn BookmarksConfig::setToolTip( bool )
 */
void BookmarksConfig::setToolTip( bool tooltip )
{
	_tooltip = tooltip;
}

QString BookmarksConfig::token( )
{
	return _token;
}

void BookmarksConfig::setToken( QString const & token )
{
	_token = token;
}

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
