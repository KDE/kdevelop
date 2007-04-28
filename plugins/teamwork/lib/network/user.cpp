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


#include "serialization.h"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/export.hpp> //must be included after archive-headers

#include "user.h"
#include "sessioninterface.h"
#include "teamworkmessages.h"

namespace Teamwork {

void OnlineInformation::setSession( const SessionPointer& sess ) {
  session_ = sess;
}

OnlineInformation::OnlineInformation( const OnlineInformation& /*rhs*/ ) {
}

OnlineInformation& OnlineInformation::operator = ( const OnlineInformation& /*rhs*/ ) {
  return *this;
}

OnlineInformation::OnlineInformation() {}

OnlineInformation::~OnlineInformation() {
}

const SessionPointer& OnlineInformation::session() {
  return session_;
}

OnlineInformation::operator bool() {
  return session_ && session_.unsafe() ->isOk();
}

User::User( const User* user ) {
  *this = *user;
}

User::User( const UserIdentity& identity ) : name_ ( identity.name() ), rights_( UserRights ) {}

User::User( string name, string password, string description ) : name_( name ), password_( password ), description_( description ), rights_( UserRights ) {}

User::~User() {}

int User::rights() const {
  return rights_;
}

std::string User::email() const {
  return email_;
}

void User::setEmail( const std::string& em ) {
  email_ = em;
}

std::string User::rightsAsString() const {
  std::string ret;
  if( rights_ & (int)UserRights ) ret += "user, ";
  if( rights_ & (int)TrustedRights ) ret += "trusted, ";
  if( rights_ & (int)AdminRights ) ret += "admin, ";
  if(!ret.empty() ) ret = ret.substr( 0, ret.length() - 2 );
  
  return ret;
}

void User::setRights( int rights ) {
  rights_ = rights;
}

bool User::matchRight( int right ) {
  return right & rights_;
}

OnlineInformation& User::online() {
  return online_;
}

template<class Archive>
void User::serialize( Archive& arch, const uint /*version*/ ) {
  arch & boost::serialization::make_nvp( "Name", name_ );
  arch & boost::serialization::make_nvp( "password", password_ );
  arch & boost::serialization::make_nvp( "description", description_ );
  arch & boost::serialization::make_nvp( "rights", rights_ );
  arch & boost::serialization::make_nvp( "email", email_ );
}

///this function is used for authentification. The given user should be the one trying to connect.
bool User::match( const User& u ) {
  if ( rights_ & BannedRights )
    return false;
  return ( password_.compare( u.password_ ) == 0 ) && ( name_.compare( u.name_ ) == 0 );
}

bool User::matchIdentity( const User& u ) {
  return name_ == u.name_;
}

///this function strips all information that is not necessary for correct authentification/identification at the given target-user
void User::stripForTarget( const User& /*target*/ ) {
  password_ = "";
  description_ = "";
}

///strips away all information that should stay private, but leaves things like description etc.
void User::stripForPublic() {
  password_ = "";
}

///this function strips all information that is not necessary to generally identify a user(this might strip away more than stripForTarget, and includes passwords and such)
void User::stripForIdentification() {
  password_ = "";
  description_ = "";
}

UserIdentity User::identity() {
  return UserIdentity( name_ );
}


string User::description() const {
  return description_;
}

bool User::operator < ( const User& rhs ) const {
  return name_ < rhs.name_;
}

bool User::operator < ( const string& rhs ) const {
  return name_ < rhs;
}

bool User::operator < ( const UserIdentity& rhs ) const {
  return name_ < rhs.name();
}

string User::safeName() const {
  SafeSharedPtr<User>::Locked l = const_cast<User*>( this );
  if ( l ) {
    return name_;
  } else {
    return "unlockable_user";
  }
}

void User::setSession( const SessionPointer& sess ) {
  online_.setSession( sess );
}

const string& User::name() const {
  return name_;
}

const string& User::password() const {
  return password_;
}

void User::ban( bool banned ) {
  if( banned )
    rights_ |= BannedRights;
  else if( rights_ & BannedRights )
    rights_ -= BannedRights;
}

bool User::banned() const {
  return rights_ & BannedRights;
}

void User::setPassword( const string& password ) {
  password_ = password;
}

INSTANTIATE_SERIALIZATION_FUNCTIONS( User );
}

///This should be done for each class derived from User, so it can correctly be serialized.
BOOST_CLASS_EXPORT_GUID( Teamwork::User, "Teamwork::User" );

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
