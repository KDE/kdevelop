/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include <kconfig.h>
#include <kdebug.h>

#include "profiles.h"

QStringList Profiles::getPluginsForProfile( const QString & profile )
{
	KConfig config("kdevpluginprofilerc");
	
	return getPluginsForProfile_internal( config, profile );
}

QStringList Profiles::getPluginsForProfile_internal( KConfig & config, const QString & profile )
{
	QStringList profilelist;

	config.setGroup( profile );
	QStringList inheritlist = config.readListEntry( "inherits" );
	QStringList::Iterator it = inheritlist.begin();
	while ( it != inheritlist.end() )
	{
		profilelist += getPluginsForProfile_internal( config, *it );
		++it;
	}
	
	config.setGroup( profile );
	profilelist += config.readListEntry( "plugins" );
	
	return profilelist;
}

QString Profiles::getProfileForCategory( const QString & category )
{
	KConfig config("kdevpluginprofilerc");
	
	config.setGroup( "Category" );
	QString profile = config.readEntry( category );
	
	config.setGroup( "Plugin Profiles" );
	QStringList profilelist = config.readListEntry( "profiles" );
	
	if ( profile.isEmpty() || !profilelist.contains( profile ) )
	{
		kdDebug() << "Warning: No valid profile found. Selecting default profile." << endl;
		config.setGroup( "Plugin Profiles" );
		profile = config.readEntry( "default" );
	}
	
	return profile;
}
