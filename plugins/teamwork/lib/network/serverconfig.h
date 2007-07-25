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
 
#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include "user.h"
#include <string>
#include <set>

namespace Teamwork {
struct ServerConfiguration {
  std::string serverName;
  std::string serverPassword; ///A password that is used for users that don't have an own password set
  
  int port;
  std::string bind;

  typedef std::set<User> UserSet;
  
  ServerConfiguration();
  
  std::set<User> registeredUsers;
  
  template<class Archive>
  void serialize( Archive& arch, const unsigned int /*version*/ );
};

bool loadServerConfiguration( ServerConfiguration& conf );
bool saveServerConfiguration( ServerConfiguration& conf );
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
