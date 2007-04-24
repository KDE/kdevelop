/***************************************************************************
  copyright            : (C) 2006 by David Nolden
  email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevteamwork.h"
#include "kdevteamwork_part.h"
#include <QtGui/QHeaderView>

#include <kmenu.h>
#include <kdebug.h>
#include <kfile.h>
#include <klocale.h>


#include <QtCore/qdebug.h>
#include "common.h"
#include "message.h"
#include "teamworkmessages.h"
#include <kiconloader.h>
#include <QAction>
#include <QCursor>
#include <QTimer>
#include "kdevteamwork_client.h"
#include "kdevteamwork_user.h"
#include "kdevteamwork_helpers.h"
#include "defines.h"


std::string stringToAddr( const QString& txt ) {
  if ( txt.indexOf( ':' ) != -1 )
    return txt.left( txt.indexOf( ':' ) ).toUtf8().data();
  return txt.toUtf8().data();
}

int stringToPort( const QString& txt ) {
  if ( txt.indexOf( ':' ) == -1 )
    return STANDARDPORT;
  else
    return txt.mid( txt.indexOf( ':' ) + 1 ).toLong();
}


KDevTeamworkClient::KDevTeamworkClient( KDevTeamwork* teamwork ) : Client( ServerInformation( "0.0.0.0", STANDARDPORT ), teamwork->m_logger.cast<Logger>() ), m_teamwork( teamwork ), dispatcher_( *this ) {
  qRegisterMetaType<Teamwork::UserPointer>( "Teamwork::UserPointer" );
  qRegisterMetaType<Teamwork::ClientSessionDesc>( "Teamwork::ClientSessionDesc" );
  qRegisterMetaType<Teamwork::ServerInformation>( "Teamwork::ServerInformation" );
  qRegisterMetaType<SafeSharedPtr<KDevTeamworkTextMessage> >( "SafeSharedPtr<KDevTeamworkTextMessage>" );
  qRegisterMetaType<ConnectionRequestPointer >( "ConnectionRequestPointer" );
  qRegisterMetaType<std::list<UserPointer> >( "std::list<UserPointer>" );
  qRegisterMetaType<SafeSharedPtr<KDevSystemMessage> >( "SafeSharedPtr<KDevSystemMessage>" );
  qRegisterMetaType<PatchesManagerMessagePointer >( "PatchesManagerMessagePointer" );
  qRegisterMetaType<CollaborationMessagePointer >( "CollaborationMessagePointer" );

  connect( this, SIGNAL( guiUserConnected( Teamwork::UserPointer ) ), m_teamwork, SLOT( guiUserConnected( Teamwork::UserPointer ) ), Qt::QueuedConnection );
  connect( this, SIGNAL( guiUserDisconnected( Teamwork::UserPointer ) ), m_teamwork, SLOT( guiUserDisconnected( Teamwork::UserPointer ) ), Qt::QueuedConnection );
  connect( this, SIGNAL( guiServerConnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation ) ), m_teamwork, SLOT( guiServerConnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation ) ), Qt::QueuedConnection );
  connect( this, SIGNAL( guiServerDisconnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation ) ), m_teamwork, SLOT( guiServerDisconnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation ) ), Qt::QueuedConnection );
  connect( this, SIGNAL( guiUserList( std::list<UserPointer> ) ), m_teamwork, SLOT( guiUserList( std::list<UserPointer> ) ), Qt::QueuedConnection );

  //    messageTypes().registerMessageTypes<KDevTeamworkMessages>();
  setIdentity( new User( "server&client" ) );
}

int KDevTeamworkClient::dispatchMessage( KDevSystemMessage* msg ) {
  if ( m_teamwork )
    QMetaObject::invokeMethod( m_teamwork, "dispatchMessage", Qt::QueuedConnection, Q_ARG( SafeSharedPtr<KDevSystemMessage>, msg ) );
  return 1;
}

int KDevTeamworkClient::dispatchMessage( CollaborationMessage* msg ) {
  emit signalDispatchMessage( CollaborationMessagePointer( msg ) );
  return 1;
}

int KDevTeamworkClient::dispatchMessage( PatchesManagerMessage* msg ) {
  emit signalDispatchMessage( PatchesManagerMessagePointer( msg ) );
  return 1;
}

int KDevTeamworkClient::dispatchMessage( KDevTeamworkTextMessage* msg ) {
  if ( m_teamwork )
    QMetaObject::invokeMethod( m_teamwork, "handleTextMessage", Qt::QueuedConnection, Q_ARG( SafeSharedPtr<KDevTeamworkTextMessage>, msg ) );
  return 1;
}

int KDevTeamworkClient::dispatchMessage( ConnectionRequest* msg ) {
  out( Logger::Debug ) << "dispatching connection-request";
  if ( m_teamwork )
    QMetaObject::invokeMethod( m_teamwork, "connectionRequest", Qt::QueuedConnection, Q_ARG( ConnectionRequestPointer, msg ) );
  return 1;
}

void KDevTeamworkClient::gotUserList( const std::list<UserPointer>& users ) {
  emit guiUserList( users );
}

UserPointer KDevTeamworkClient::createUser( IdentificationMessage* msg ) {
  return new KDevTeamworkUser( msg );
}


UserPointer KDevTeamworkClient::createUser( const User* user ) {
  return new KDevTeamworkUser( user );
}

///this is called whenever a new user successfully logged into the server
void KDevTeamworkClient::userConnected( const Teamwork::UserPointer& user ) {
  kdDebug() << "userConnected" << endl;

  emit guiUserConnected( user );

  Client::userConnected( user );

}

///called whenever a user leaves the server
void KDevTeamworkClient::userDisconnected( const Teamwork::UserPointer& user ) {
  kdDebug() << "userDisconnected" << endl;

  emit guiUserDisconnected( user );

  Client::userDisconnected( user );
}

///this is called whenever a connection is successfully established to a server
void KDevTeamworkClient::connectedToServer( const Teamwork::ClientSessionDesc& session, const Teamwork::ServerInformation& server ) {
  kdDebug() << "connectedToServer" << endl;

  emit guiServerConnected( session, server );

  Client::connectedToServer( session, server );
}

///this is called whenever a server the client disconnects from a connected server in any way
void KDevTeamworkClient::disconnectedFromServer( const Teamwork::ClientSessionDesc& session, const Teamwork::ServerInformation& server ) {
  kdDebug() << "disconnectedFromServer" << endl;

  emit guiServerDisconnected( session, server );

  Client::disconnectedFromServer( session, server );
}

#include "kdevteamwork_client.moc"

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
