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

#ifndef KDEVTEAMWORK_CLIENT_H
#define KDEVTEAMWORK_CLIENT_H


#include "teamworkfwd.h"
#include "network/safesharedptr.h"
#include "network/teamworkclient.h"
#include "kdevteamwork_messages.h"
#include <list>


#include <QTimer>

///@TODO: Dispatch the messages in a more flexible way: Allow any client to register with an arbitrary message-type that he wants to get(including all sub-messages). DynamicMessageDispatcher can do this. Use that, and make the way messages are connected more flexible.

Q_DECLARE_METATYPE( Teamwork::ServerInformation )
Q_DECLARE_METATYPE( Teamwork::UserPointer )
Q_DECLARE_METATYPE( ConnectionRequestPointer )
Q_DECLARE_METATYPE( SafeSharedPtr<KDevTeamworkTextMessage> )
Q_DECLARE_METATYPE( std::list<UserPointer> )
Q_DECLARE_METATYPE( SafeSharedPtr<KDevSystemMessage> )
Q_DECLARE_METATYPE( CollaborationMessagePointer )

///most of the functions in this class are called from within another thread
class KDevTeamworkClient : public QObject, public Teamwork::Client
{
	public:
		KDevTeamworkClient( KDevTeamwork* teamwork );

		void invalidateTeamwork() {
			disconnectAllServers();
			closeAllIncomingSessions();
			stopRunning();
			m_teamwork = 0;
		}

		int receiveMessage( CollaborationMessage* msg );

		int receiveMessage( PatchesManagerMessage* msg );

		int receiveMessage( KDevTeamworkTextMessage* msg );

		int receiveMessage( KDevSystemMessage* msg );

		int receiveMessage( ConnectionRequest* msg );

		int receiveMessage( MessageInterface* /*msg*/ ) {
			return 0;
		}

	signals:
		///All connections made to these signals must be queued, since these signals are emitted from within another thread
		void guiUserConnected( Teamwork::UserPointer );
		void guiUserDisconnected( Teamwork::UserPointer );
		void guiServerConnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation );
		void guiServerDisconnected( Teamwork::ClientSessionDesc, Teamwork::ServerInformation );
		void guiUserList( std::list<UserPointer> );

		void signalDispatchMessage( PatchesManagerMessagePointer );
		void signalDispatchMessage( CollaborationMessagePointer );

	protected:
		virtual UserPointer createUser( IdentificationMessage* msg );
		virtual UserPointer createUser( const User* user );

    ///this is called whenever a new user successfully logged into the server
		virtual void userConnected( const Teamwork::UserPointer& user );

    ///called whenever a user leaves the server
		virtual void userDisconnected( const Teamwork::UserPointer& user );

    ///this is called whenever a connection is successfully established to a server
		virtual void connectedToServer( const Teamwork::ClientSessionDesc& session, const Teamwork::ServerInformation& server );

    ///this is called whenever a server the client disconnects from a connected server in any way
		virtual void disconnectedFromServer( const Teamwork::ClientSessionDesc& session, const Teamwork::ServerInformation& server );

		virtual void gotUserList( const std::list<UserPointer>& users );

    void processMessage( MessageInterface* msg ) throw();

	private:
		
    //AllKDevTeamworkMessages, KDevTeamworkMessages
		Q_OBJECT
		KDevTeamwork* m_teamwork;

};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
