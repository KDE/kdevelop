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

#ifndef TEAMWORK_COMMON_H
#define TEAMWORK_COMMON_H

#define USE_BOOST_MUTEX

///This can be used to shut the use of boost-muxes on/off. The advantage of boost-mutexes is that they natively provide a timeout-function

///Still needs some work in serializeMessage and buildMessageFromBuffer to drop the text-format
//#define USE_TEXT_ARCHIVE
//#define USE_POLYMORPHIC_ARCHIVE
//#include <boost/serialization/extended_type_info_no_rtti.hpp>
/*
#define BOOST_SERIALIZATION_DEFAULT_TYPE_INFO(T) \
    extended_type_info_no_rtti<const T>
*/
#include <cc++/thread.h>
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
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/extended_type_info.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/level.hpp>
#define UNUSED(x) (void)(x)

#ifndef USE_TEXT_ARCHIVE
 #ifdef USE_POLYMORPHIC_ARCHIVE
typedef boost::archive::polymorphic_binary_oarchive OutArchive;
typedef boost::archive::polymorphic_binary_iarchive InArchive;
#else
typedef boost::archive::binary_oarchive OutArchive;
typedef boost::archive::binary_iarchive InArchive;
#endif

#else
 #ifdef USE_POLYMORPHIC_ARCHIVE
typedef boost::archive::polymorphic_text_oarchive OutArchive;
typedef boost::archive::polymorphic_text_iarchive InArchive;
#else
typedef boost::archive::text_oarchive OutArchive;
typedef boost::archive::text_iarchive InArchive;
#endif
#endif

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
    *message sound be sent anyway. */
  struct CannotReserialize : public NonFatalSerializationError {
    CannotReserialize() : NonFatalSerializationError( "the message cannot be serialized after being deserialized" ) {}
  }
  ;

  class StreamError : public TeamworkError {
    public:
      StreamError( const std::string& str ) : TeamworkError( "Stream-Error: " + str ) {}
  }
  ;
};

#ifndef USE_BOOST_MUTEX

class MutexInterfaceImpl {
    mutable Mutex m_;
  public:
    void lockCountUp() const {
      m_.enterMutex();
    }

    bool tryLockCountUp() const {
      return m_.tryEnterMutex();
    }

    ///not implemented correctly with this type of mutexes
    bool tryLockCountUp( int /*timeout*/ ) {
      lockCountUp();
      return true;
      /*
      timeout /= MILLI_TO_NANO;
      while( timeout >= 0 ) {
        if( tryLockCountUp() ) return true;
        sleep( 1 );
        --timeout;
      }
      return false; */
    }

    void lockCountDown() const {
      m_.leaveMutex();
    };

    ~MutexInterfaceImpl() {}
}
;
#else

///this ugly hack is necessary because the general concept of the boost thread-library wants
///to force us to use scoped lock-classes, but we want to do that work ourselves.
///the other option would be to change the whole design and thereby make it incompatible to
///the common-cpp mutexes.
#define private public
#include <boost/thread/recursive_mutex.hpp>
#undef private
#include <boost/thread/xtime.hpp>

class MutexInterfaceImpl {
    typedef boost::recursive_timed_mutex MutexType;
    mutable MutexType m_;
  public:
    MutexInterfaceImpl( const MutexInterfaceImpl& rhs ) {
      UNUSED( rhs );
    }

    const MutexInterfaceImpl& operator=( const MutexInterfaceImpl& rhs ) {
      UNUSED( rhs );
      return *this;
    }

    MutexInterfaceImpl() {}

    void lockCountUp() const {
      m_.do_lock();
      //l_.lock();
    }

    bool tryLockCountUp() const {
      return m_.do_trylock();
    }

    ///timeout in nanoseconds, may have a significant delay, so it shouldn't be used too much
    bool tryLockCountUp( int timeout ) const {
      boost::xtime t;
      xtime_get( &t, boost::TIME_UTC );
      t.nsec += timeout;
      return m_.do_timedlock( t );
    }

    void lockCountDown() const {
      m_.do_unlock();
    };

    ~MutexInterfaceImpl() {}
}
;

#endif

typedef unsigned int u32;
typedef std::vector<char> DataVector;

#endif
