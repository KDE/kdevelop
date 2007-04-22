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

#include "serverconfig.h"
#include "boost/archive/polymorphic_xml_oarchive.hpp"
#include "boost/archive/polymorphic_xml_iarchive.hpp"
#include "boost/archive/polymorphic_text_oarchive.hpp"
#include "boost/archive/polymorphic_text_iarchive.hpp"
#include "boost/serialization/list.hpp"
#include <fstream>
#include "defines.h"

namespace Teamwork {

ServerConfiguration::ServerConfiguration() : port( STANDARDPORT ), bind( "127.0.0.1" ) {
}

bool loadServerConfiguration( ServerConfiguration& conf ) {
  std::ifstream file( "teamwork.config", ios_base::in );
  if( !file.good() ) {
    cout << "could not open config-file";
    return false;
  }
  boost::archive::polymorphic_text_iarchive arch( file );
  try {
  } catch( ... ) {
    cout << "error while reading the config-file" << endl;
    return false;
  }
  arch & conf;
  return true;
}

bool saveServerConfiguration( ServerConfiguration& conf ) {
  std::ofstream file( "teamwork.config", ios_base::out );
  if( !file.good() ) {
    cout << "could not open config-file";
    return false;
  }
  boost::archive::polymorphic_text_oarchive arch( file );
  try {
  } catch( ... ) {
    cout << "error while loading the config-file" << endl;
    return false;
  }
  arch & conf;
  return true;
}

}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
