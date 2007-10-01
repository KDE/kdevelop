/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevteamwork_user.h"
#include "network/teamworkmessages.h"
#include "kdevteamwork.h"
#include <boost/serialization/extended_type_info_typeid.hpp>
#include <boost/serialization/export.hpp>
#include "kdevteamwork_helpers.h"
#include "network/sessioninterface.h"


KDevTeamworkUser::KDevTeamworkUser( const User* user ) {
	qRegisterMetaType<KDevTeamworkUserPointer>( "KDevTeamworkUserPointer" );
	if( user ) {
		(*(User*)this) = *user;
	}
}

KDevTeamworkUser::KDevTeamworkUser( IdentificationMessage* msg ) : User((User)*msg) {
	qRegisterMetaType<KDevTeamworkUserPointer>( "KDevTeamworkUserPointer" );
}

void KDevTeamworkUser::setSession( const SessionPointer& sess ) {
	User::setSession( sess );
	emit userStateChanged( this );
}

QIcon KDevTeamworkUser::icon( KIconLoader::Group size ) {
	SessionPointer s = online().session();

	if( s && s.unsafe()->isOk() ) {
		SessionPointer::Locked l = s;
		if( l ) {
			if( l->sessionType() == SessionInterface::Direct )
				return IconCache::instance()->getIcon( "presence_online", size );
			else
				return IconCache::instance()->getIcon( "presence_away", size );
		} else {
			return IconCache::instance()->getIcon( "presence_offline", size );
		}
	} else {
		return IconCache::instance()->getIcon( "presence_offline", size );
	}
	return IconCache::instance()->getIcon( "remove" );
}

QString KDevTeamworkUser::toolTip() {
	return "";
}

Q_DECLARE_METATYPE( KDevTeamworkUserPointer )
BOOST_CLASS_EXPORT_GUID(KDevTeamworkUser, "KDevTeamworkUser")


#include "kdevteamwork_user.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
