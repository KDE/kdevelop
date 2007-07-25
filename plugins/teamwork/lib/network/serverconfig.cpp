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

#include "serverconfig.h"
#include <boost/serialization/extended_type_info_typeid.hpp>
#include "serialization.h"
#include <boost/archive/polymorphic_xml_oarchive.hpp>
#include <boost/archive/polymorphic_xml_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <fstream>
#include "defines.h"
#include <iostream>

#ifdef USE_POLYMORPHIC_ARCHIVE
// #include <boost/archive/polymorphic_xml_iarchive.hpp>
// #include <boost/archive/polymorphic_xml_oarchive.hpp>
// typedef boost::archive::polymorphic_xml_iarchive InternalIArchive;
// typedef boost::archive::polymorphic_xml_oarchive InternalOArchive;

typedef boost::archive::polymorphic_text_iarchive InternalIArchive;
typedef boost::archive::polymorphic_text_oarchive InternalOArchive;
#else
typedef Teamwork::InArchive InternalIArchive;
typedef Teamwork::OutArchive InternalOArchive;
#endif

namespace Teamwork {

ServerConfiguration::ServerConfiguration() : port( STANDARDPORT ), bind( "0.0.0.0" ) {
}

bool loadServerConfiguration( ServerConfiguration& conf ) {
  std::ifstream file( "teamwork.config", ios_base::in );
  if( !file.good() ) {
    cout << "could not open config-file" << endl;
    return false;
  }
  InternalIArchive arch( file );
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
  InternalOArchive arch( file );
  try {
  } catch( ... ) {
    cout << "error while loading the config-file" << endl;
    return false;
  }
  arch & conf;
  return true;
}

INSTANTIATE_SERIALIZATION_FUNCTIONS( ServerConfiguration )

}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
