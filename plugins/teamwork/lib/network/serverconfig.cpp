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
#include "serverconfig.h"
#include "boost/archive/polymorphic_xml_oarchive.hpp"
#include "boost/archive/polymorphic_xml_iarchive.hpp"
#include "boost/archive/polymorphic_text_oarchive.hpp"
#include "boost/archive/polymorphic_text_iarchive.hpp"
#include "boost/serialization/list.hpp"
#include <boost/serialization/set.hpp>
#include <fstream>
#include "defines.h"
#include <iostream>

namespace Teamwork {

ServerConfiguration::ServerConfiguration() : port( STANDARDPORT ), bind( "0.0.0.0" ) {
}

bool loadServerConfiguration( ServerConfiguration& conf ) {
  std::ifstream file( "teamwork.config", ios_base::in );
  if( !file.good() ) {
    cout << "could not open config-file" << endl;
    return false;
  }
  /*boost::archive::polymorphic_text_iarchive*/
  InArchive arch( file );
  try {
  } catch( ... ) {
    cout << "error while reading the config-file" << endl;
    return false;
  }
  arch & conf;
  return true;
}

template<class Archive>
void ServerConfiguration::serialize( Archive& arch, const unsigned int /*version*/ ) {
  arch & serverName;
  arch & serverPassword;
  arch & port;
  arch & bind;
  arch & registeredUsers;
}


bool saveServerConfiguration( ServerConfiguration& conf ) {
  std::ofstream file( "teamwork.config", ios_base::out );
  if( !file.good() ) {
    cout << "could not open config-file" << endl;
    return false;
  }
  /*boost::archive::polymorphic_text_oarchive*/
  OutArchive arch( file );
  try {
  } catch( ... ) {
    cout << "error while loading the config-file" << endl;
    return false;
  }
  arch & conf;
  return true;
}

INSTANTIATE_SERIALIZATION_FUNCTIONS( ServerConfiguration );

}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
