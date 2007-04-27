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

#ifndef TEAMWORK_USER_H
#define TEAMWORK_USER_H

#include <boost/serialization/nvp.hpp>
#include <string>

#include "networkfwd.h"
#include "safesharedptr.h"
#include "weaksafesharedptr.h"

namespace Teamwork {
///when a user is flagged as online that means that there is a session to that user. It does not mean that that session is really functional.
class OnlineInformation {
    friend class User;
    SafeSharedPtr<SessionInterface> session_;
    //SessionPointer session_;

    void setSession( const SessionPointer& sess );

    OnlineInformation( const OnlineInformation& rhs );
    OnlineInformation& operator = ( const OnlineInformation& rhs );
  public:
    OnlineInformation();
    ~OnlineInformation();

    ///When a user is reported as online, it is guaranteed that the session is valid.
    operator bool();

    const SessionPointer& session();
};

///This represents the identity of a user, it can be used to store it persistently.
class UserIdentity {
    std::string name_;
    friend class User;

  public:
    UserIdentity( const std::string& name ) {
      name_ = name;
    }
    UserIdentity() {}

    const std::string& name() const {
      return name_;
    }

    template<class Archive>
    void serialize( Archive& arch, const uint /*version*/ ) {
      arch & boost::serialization::make_nvp( "name", name_ );
    }

    bool operator == ( const UserIdentity& rhs ) const {
      return name_ == rhs.name_;
    }

    bool operator < ( const UserIdentity& rhs ) const {
      return name_ < rhs.name_;
    }

    operator bool() const {
      return !name_.empty();
    }
    bool operator !() const {
      return name_.empty();
    }
};


class User : public WeakSafeShared {
    std::string name_;
    std::string password_;
    std::string description_;
    std::string email_;
    int rights_;
    OnlineInformation online_;
  public:
    enum Rights {
      UserRights = 1,
      TrustedRights = 2,
      AdminRights = 4,
      BannedRights = 8
  };

    User( const UserIdentity& identity );

    User( const User* user );

    User( std::string name = "", std::string password = "", std::string description = "" );

    virtual ~User();

    template<class Archive>
    void serialize( Archive& arch, const uint /*version*/ );

    bool matchRight( int right );

    ///this function is used for authentification. The given user should be the one trying to connect.
    virtual bool match( const User& u );

    virtual bool matchIdentity( const User& u );

    ///this function strips all information that is not necessary for correct authentification/identification at the given target-user
    virtual void stripForTarget( const User& /*target*/ );

    ///strips away all information that should stay private, but leaves things like description etc.
    virtual void stripForPublic();

    ///this function strips all information that is not necessary to generally identify a user(this might strip away more than stripForTarget, and removes passwords and such). It should not be called on derived classes, because the User-class is enough for simple Identification.
    void stripForIdentification();

    ///the structure can be changed through this.
    OnlineInformation& online();

    ///To set the user offline, this should be called with a zero-pointer
    virtual void setSession( const SessionPointer& sess );

    virtual std::string description() const;

    bool operator < ( const User& rhs ) const;

    bool operator < ( const std::string& rhs ) const;

    bool operator < ( const UserIdentity& rhs ) const;

    const std::string& name() const;

    int rights() const;

    void setRights( int rights );

    std::string rightsAsString() const;

    std::string email() const;

    void setEmail( const std::string& );

    ///This function is thread-safe(no locking must be done before calling it)
    std::string safeName() const;

    const std::string& password() const;

    void ban( bool banned );

    bool banned() const;

    void setPassword( const std::string& password );

    ///Creates a lightweigth-structure for identification which may be used for persistent storage.
    UserIdentity identity();
};

typedef SafeSharedPtr<User, BoostSerialization> UserPointer;
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
