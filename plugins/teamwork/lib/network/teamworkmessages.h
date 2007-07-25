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

#ifndef TEAMWORKMESSAGES_H
#define TEAMWORKMESSAGES_H

#include "networkfwd.h"
#include "user.h"
#include "messageimpl.h"
#include "binder.h"

namespace Teamwork {

/*
vector<char> messageToData( MessageInterface* msg ) {
  vector<char> vec;
  serializeToBuffer( vec, *msg );
  return vec;
}*/

/** Abstract message for all messages that have to do with the teamwork-stuff */

class TeamworkMessage : public RawMessage {
    DECLARE_MESSAGE( TeamworkMessage, RawMessage, 1 );
  public:
    TeamworkMessage( const MessageConstructionInfo& messageTypes );
    TeamworkMessage( InArchive& arch, const MessageInfo& info );
};

class IdentificationMessage : public TeamworkMessage {
    DECLARE_MESSAGE( IdentificationMessage, TeamworkMessage, 1 );
    User user_;

    template <class Arch>
    void serial( Arch& arch ) {
      arch & user_;
    }

  public:

    IdentificationMessage( const MessageConstructionInfo& messageTypes, const User& user );

    IdentificationMessage( InArchive& arch, const MessageInfo& info );

    operator User();

    virtual void serialize( OutArchive& arch );
};

typedef TextMessage MaintenanceMessage;
typedef Chain< MaintenanceMessage, TextMessage, 15> MaintenanceMessageChain;


class ForwardMessage : public RawMessage {
    DECLARE_MESSAGE( ForwardMessage, RawMessage, 2 );
    User source_;
    User target_;
  public:
    ForwardMessage( const MessageConstructionInfo& messageTypes, MessageInterface* messageToForward, const User& source, const User& targ );

    ForwardMessage( InArchive& arch, const MessageInfo& info );

    template <class Arch>
    void serial( Arch& arch ) {
      arch & source_;
      arch & target_;
    }

    virtual void serialize( OutArchive& arch );

    User& source();

    User& target();

    MessagePointer deserializeContained( MessageTypeSet& messageTypes );

    ///Through this variable, messages should state that they should be stored on the server until the target-user is online if it cannot be reached
    virtual bool storeOnServer();
};


typedef Binder< TeamworkMessage >
::Append< IdentificationMessage >::Result
//::Append< MaintenanceMessageChain >::Result
::Append< ForwardMessage >::Result
TeamworkMessages;
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
