/***************************************************************************
 copyright            : (C) 2006 by David Nolden
 email                : david.nolden.kdevelop(at)art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** This file needs to be included into any file where boost-serialization takes place
 * */

#ifndef TEAMWORK_COMMON_H
#define TEAMWORK_COMMON_H

#include "networkfwd.h"

//#include <boost/serialization/extended_type_info_no_rtti.hpp>
/*
#define BOOST_SERIALIZATION_DEFAULT_TYPE_INFO(T) \
    extended_type_info_no_rtti<const T>
*/
//#include <cc++/thread.h>
#ifndef USE_TEXT_ARCHIVE
#ifdef USE_POLYMORPHIC_ARCHIVE
#include <boost/archive/polymorphic_binary_iarchive.hpp>
#include <boost/archive/polymorphic_binary_oarchive.hpp>
#else
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#endif
#else
#ifdef USE_POLYMORPHIC_ARCHIVE
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#else
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#endif
#endif
//#include <boost/serialization/base_object.hpp>
#include <boost/serialization/extended_type_info.hpp>
//#include <boost/serialization/utility.hpp>
//#include <boost/serialization/list.hpp>
//#include <boost/serialization/vector.hpp>
#include <boost/serialization/level.hpp>
#include <vector>

namespace Teamwork {

  class TeamworkError : public std::exception {
      std::string str_;
    public:
      TeamworkError( const std::string& str ) {
        str_ = str;
      }
      virtual ~TeamworkError() throw() {}

      virtual const char* what ( ) const throw () {
        return str_.c_str();
      }
  };

  ///This error may be thrown while serialization and it will only stop the message from being sent, it will not close the session.
  struct NonFatalSerializationError : public TeamworkError {
    NonFatalSerializationError( const std::string& str ) : TeamworkError( "non-fatal serialization-error: " + str ) {}
  }
  ;

  /**Normally while debugging, all messages are tested by serializing,
    *then rebuilding, serializing the rebuilt message, and comparing the
    *created streams. If this test fails, the message is not sent. in some
    *cases that behavior is not appropriate. Then this exception can be thrown
    *while reserializing or rebuilding the message, to make clear that the
    *message should be sent anyway. */
  struct CannotReserialize : public NonFatalSerializationError {
    CannotReserialize() : NonFatalSerializationError( "the message cannot be serialized after being deserialized" ) {}
  }
  ;

  class StreamError : public TeamworkError {
    public:
      StreamError( const std::string& str ) : TeamworkError( "Stream-Error: " + str ) {}
  }
  ;
}

/**Because gcc does not support the export-keyword template-functions that are not in the header need to be explicitly instantiated.
 * This is a convenience-macro to instantiate the two default serialization-functions(for InArchive and OutArchive)
 * */
#define INSTANTIATE_SERIALIZATION_FUNCTIONS(Class) \
template void Class::serialize( InArchive& arch, const uint ); \
template void Class::serialize( OutArchive& arch, const uint );

#endif
