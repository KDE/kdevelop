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

#include "sessioninterface.h"
#include "user.h"
#include "messageinterface.h"

namespace Teamwork {
  SessionInterface& SessionInterface::operator= ( const SessionInterface& /*rhs*/ ) {
    return *this;
  }
  
  SessionInterface::SessionInterface( const SessionInterface& /*rhs*/ ) : SafeShared() {
  }
  
  void SessionInterface::startSession() {}

  std::string SessionInterface::sessionName() {
    return sessionName_;
  }

  bool SessionInterface::sendReply( MessageInterface* msg, MessageInterface* replyTo ) {
    msg->info().setReply( replyTo->info().uniqueId() );
    msg->info().setReplyMessage( replyTo );
    return send( msg );
  }

  void SessionInterface::setSessionName( const std::string& name ) {
    sessionName_ = name;
  }

  std::string SessionInterface::peerDesc() {
    return std::string();
  }

  std::string SessionInterface::localDesc() {
    return std::string();
  }

  SessionInterface::SessionType SessionInterface::sessionType() {
    return Direct;
  }
}
