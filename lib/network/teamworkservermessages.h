#ifndef TEAMWORKSERVERMESSAGES_H
#define TEAMWORKSERVERMESSAGES_H

#include "teamworkmessages.h"
#include "user.h"
#include <list>
#include <string>

namespace Teamwork {
using namespace std;

struct UserListMessage : public TeamworkMessage {
  DECLARE_MESSAGE( UserListMessage, TeamworkMessage, 2 );
  list<User> users;

  UserListMessage( const MessageInfo& info, list<UserPointer> inUsers, const UserPointer& targetUser );

  UserListMessage( InArchive& arch, const MessageInfo& info );

  template <class Arch>
  void serial( Arch& arch ) {
    arch & users;
  }

  virtual void serialize( OutArchive& arch );
};

typedef UserListMessage
TeamworkServerMessages;
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
