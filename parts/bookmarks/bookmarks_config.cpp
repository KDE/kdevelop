//
// C++ Implementation: bookmarks_config
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <kconfig.h>
#include <kapplication.h>

#include "bookmarks_config.h"

BookmarksConfig::BookmarksConfig()
{
	// @todo make the codeline values more intuitive
	_context = 3;
	_codeline = 1; // 0 - Never, 1 - Only if Comment, 2 - Always
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
	KConfig *config = kapp->config();   
	config->setGroup("Bookmarks");
	
	config->writeEntry("Context", _context );
	config->writeEntry("Codeline", _codeline );
	config->writeEntry("ToolTip", _tooltip );

	config->sync();
}


/*!
    \fn BookmarksConfig::readConfig()
 */
void BookmarksConfig::readConfig()
{
	// @todo implement lunacy safeguards
	
	KConfig *config = kapp->config();
	config->setGroup("Bookmarks");
	
	_context = config->readPropertyEntry( "Context", 3 ).toInt();
	_codeline = config->readPropertyEntry( "Codeline", 1 ).toInt();
	_tooltip = config->readBoolEntry( "ToolTip", true );
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
unsigned int BookmarksConfig::codeline()
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
    \fn BookmarksConfig::setCodeline( unsigned int )
 */
void BookmarksConfig::setCodeline( unsigned int codeline )
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

