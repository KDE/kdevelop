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
#include "kdevteamwork_helpers.h"
#include <QIcon>
#include <kiconloader.h>
#include "network/messagetypeset.h"

IconCache* IconCache::m_instance = 0;


IconCache::IconCache() {
  ///Here, the appropriate icon-names can be chosen by mapping
	m_instance = this;
	maps_[ "unknown" ] = "help";
	maps_[ "text_message_in" ] = "mail_get";
	maps_[ "text_message_out" ] = "mail_send";

	maps_[ "document_message_in" ] = "folder_inbox";
	maps_[ "document_message_out" ] = "folder_outbox";

	maps_[ "generalrequest_in" ] = "bookmark";
	maps_[ "generalrequest_out" ] = "wizard";
	maps_[ "generalrequest_in_denied" ] = "fileclose";
	maps_[ "generalrequest_in_accepted" ] = "ok";
	maps_[ "generalrequest_out_denied" ] = "fileclose";
	maps_[ "generalrequest_out_accepted" ] = "ok";
	maps_[ "generalrequest_out_failed" ] = "remove";
	maps_[ "generalrequest_in_failed" ] = "remove";

	maps_[ "collaborationrequest_in" ] = "bookmark";
	maps_[ "collaborationrequest_out" ] = "wizard";
	maps_[ "collaborationrequest_accepted" ] = "ok";
	maps_[ "collaborationrequest_denied" ] = "fileclose";
	maps_[ "collaborationrequest_in_denied" ] = "fileclose";
	maps_[ "collaborationrequest_in_accepted" ] = "ok";
	maps_[ "collaborationrequest_out_denied" ] = "fileclose";
	maps_[ "collaborationrequest_out_accepted" ] = "ok";
	maps_[ "collaborationrequest_out_failed" ] = "remove";
	maps_[ "collaborationrequest_in_failed" ] = "remove";

	maps_[ "collaboration_file" ] = "completion";
	maps_[ "collaboration_file_disabled" ] = "remove";
	maps_[ "collaboration_file_dead" ] = "fileclose";
	maps_[ "collaboration_session" ] = "switchuser";
	maps_[ "collaboration_session_finished" ] = "remove";
	maps_[ "collaboration_session_connectionlost" ] = "fileclose";

	maps_[ "filecollaborationrequest_in" ] = "bookmark";
	maps_[ "filecollaborationrequest_out" ] = "wizard";
	maps_[ "filecollaborationrequest_accepted" ] = "ok";
	maps_[ "filecollaborationrequest_denied" ] = "fileclose";
	maps_[ "filecollaborationrequest_in_denied" ] = "fileclose";
	maps_[ "filecollaborationrequest_in_accepted" ] = "ok";
	maps_[ "filecollaborationrequest_out_denied" ] = "fileclose";
	maps_[ "filecollaborationrequest_out_accepted" ] = "ok";
	maps_[ "filecollaborationrequest_out_failed" ] = "remove";
	maps_[ "filecollaborationrequest_in_failed" ] = "remove";
	maps_[ "filecollaborationrequest_in_failed" ] = "remove";
	maps_[ "fileusercollaboration_inactive" ] = "presence_unknown";

	maps_[ "patch" ] = "txt2";
	maps_[ "failed" ] = "fileclose";

	maps_[ "error" ] = "stop";
}

void SafeTeamworkEmitter::updateMessageInfo( const MessagePointer& msg ) {
  emit signalUpdateMessageInfo( msg );
}

QIcon IconCache::operator () ( const QString& name, KIconLoader::Group grp ) {
  return getIcon( name, grp );
}

QIcon IconCache::getIconStatic( QString name, KIconLoader::Group grp ) {
  return IconCache::instance() ->getIcon( name, grp );
}

QIcon IconCache::getIcon( QString name, KIconLoader::Group grp ) {
  {
    QMap<QString, QString>::iterator it = maps_.find( name );

    if ( it != maps_.end() )
      name = *it;
  }

  QPair<QString, KIconLoader::Group> pos( name, grp );
  IconMap::iterator it = m_icons.find( pos );
  if ( it != m_icons.end() ) {
    return * it;
  } else {
    m_icons[ pos ] = QIcon( KIconLoader::global() ->loadIcon( name, grp ) );
    return m_icons[ pos ];
  }
}

IconCache* IconCache::instance() {
  return m_instance;
}


#include "kdevteamwork_helpers.moc"
