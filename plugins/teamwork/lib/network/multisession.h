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

#ifndef MULTISESSION_H
#define MULTISESSION_H

#include <stdio.h>
#include <stdlib.h>
#include "serialization.h"
#include "messagetypeset.h"
#include "safesharedptr.h"
#include "basicserver.h"
#include "basicsession.h"
#include <string>
#include <list>
#include <map>
#include <set>
#include "sharedptr.h"
#include "teamworkmessages.h"
#include "teamworkservermessages.h"
#include "user.h"
#include "messagesendhelper.h"
//#include "forwardsession.h"
#include "serverconfig.h"


namespace  Teamwork {
class ServerInformation;
/** MultiSession extends BasicTCPSession with the ability to manage virtual sub-sessions to users reachable through a forwarding server.
 *  It is created by Teamwork::Server and Teamwork::Client
* */

class MultiSession : public BasicTCPSession {
    bool incoming_;
    typedef std::map<UserPointer, ForwardSessionPointer, UserPointer::ValueSmallerCompare> ForwardSessionMap;
    ForwardSessionMap forwardSessions_;
    std::list<ForwardSessionPointer> garbageSessions_;

  protected:

    void deleteForwardSessions();

    virtual void final();

  public:
    MultiSession( ost::TCPSocket &server, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger );

    MultiSession( const ServerInformation& server, HandlerPointer handler, MessageTypeSet& messages, const LoggerPointer& logger, const string& namePrefix = "" );

    bool operator < ( const MultiSession& rhs ) const {
      return this < ( &rhs );
    }

    ///true if the session is an incoming one(connected to the local server), else returns false
    bool incoming() {
      return incoming_;
    }

    virtual bool think();

    virtual ~MultiSession();

    void registerForwardSession( const UserPointer& peer, const ForwardSessionPointer& sess );

    virtual bool handleMessage( MessagePointer msg ) throw();

    ForwardSessionPointer getForwardSession( const UserPointer& user ) {
      ForwardSessionMap::const_iterator it = forwardSessions_.find( user );
      if ( it != forwardSessions_.end() )
        return ( *it ).second;
      else
        return 0;
    }

    list<UserPointer> getUserList() {
      list<UserPointer> ret;
      for ( ForwardSessionMap::const_iterator it = forwardSessions_.begin(); it != forwardSessions_.end(); ++it ) {
        ret.push_back( ( *it ).first );
      }
      return ret;
    }
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
