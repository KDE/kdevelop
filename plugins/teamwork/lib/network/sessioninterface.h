/***************************************************************************
copyright            : (C) 2007 by David Nolden
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

#ifndef SESSIONINTERFACE_H
#define SESSIONINTERFACE_H

#include <string>
#include "networkfwd.h"
#include "safesharedptr.h"

namespace Teamwork {
  class SessionInterface : public /*Weak*/SafeShared {
  public:
    virtual void startSession();

    virtual ~SessionInterface();
    /**After this function was called, the thread is going to exit soon.
    once isRunning() returns false, it can be deleted(better call join()).*/
    virtual void stopRunning() = 0;

    virtual void stopRunningNow() = 0;

    /** Use this to send a message to the other side.
     *
     * This is thread-safe.
     * */
    virtual bool send( MessageInterface* msg ) = 0;

    /** Use this to send a message to the other side, that is correctly marked as
     *  a reply to the message given by @param replyTo. This is just a convenience-function.
     *
     * @param msg The message to be sent. It will be changed so its info().replyTo() etc. will be filled correctly
     * @param replyTo The message to reply to. It does not need to be locked.
     *
     * This is thread-safe.
     * */
    virtual bool sendReply( MessageInterface* msg, MessageInterface* replyTo );

    ///Returns true if no errors occurred and the session was not ended.  Must be thread-safe.
    virtual bool isOk() = 0;

    ///Returns whether the thread is running. Must be thread-safe.
    virtual bool isRunning() = 0;

    virtual std::string sessionName();

    virtual void setSessionName( const std::string& name );

    virtual std::string peerDesc();

    virtual std::string localDesc();

    enum SessionType {
      Direct,
      Forwarded
    };

    ///this function is thread-safe
    virtual SessionType sessionType();

    enum SessionDirection {
      Outgoing,
      Incoming
    };

    ///this function is thread-safe
    virtual SessionDirection sessionDirection() = 0;

    ///This function is thread-safe.  The user is not necessarily filled.
    UserPointer safeUser() const;

    ///This must never be called again once it has been set on a session, to make it thread-safe.
    void setUser( const UserPointer& user );

  private:
    std::string sessionName_;
    UserPointer user_;
    //UserPointer user_;
    ///override standard-functions
    SessionInterface& operator= ( const SessionInterface& rhs );
    SessionInterface( const SessionInterface& rhs );

  protected:
    SessionInterface();

    /**This can be overridden to possibly handle messages directly in this object.
    The default-implementation just sends the messages to the handler which created this session.*/
    virtual bool handleMessage( MessagePointer msg ) throw();

    ///This can be overridden to do some work on a regular basis. Returns whether more time is needed.
    virtual bool think() = 0;

    ///This can be overridden to do some cleanup-work within the session-thread. By default it tries to lock the associated user, and call setSession(this) so the user has a chance to notice the difference.
    virtual void final();
  };
}
#endif
