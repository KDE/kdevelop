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

#ifndef POINTER_H
#define POINTER_H

//Needed for AtomicCounter
#include <cc++/thread.h>

#include "sharedptr.h"
#include "mutexinterface.h"
#include "boost/serialization/nvp.hpp"

#define MILLI_TO_NANO 1000000
#define SEC_TO_NANO 1000000000

using namespace std;

#ifndef SAFESHAREDPTR_HAVE_DEFAULT
#define SAFESHAREDPTR_HAVE_DEFAULT
template<class Type, class Serialization = NormalSerialization>
class SafeSharedPtr;
#endif

/** Derive your classes from SafeShared if you want to use thread-safe reference-counting.
 *  It can either be used with SafeSharedPtr to also control access to the function, or with SharedPtr
 *  to only do thread-safe reference-counting.
 * */
class SafeShared : public MutexInterfaceImpl {
  public:
    /**
    * Standard constructor.  This will initialize the reference count
    * on this object to 0.
    */
    inline SafeShared() : count( 0 ) { }

    /**
    * Copy constructor.  This will @em not actually copy the objects
    * but it will initialize the reference count on this object to 0.
    */
    inline SafeShared( const SafeShared & ) : MutexInterfaceImpl(), count( 0 ) { }

    /**
    * Overloaded assignment operator.
    */
    inline SafeShared &operator=( const SafeShared & ) {
      return * this;
    }

    /**
    * Increases the reference count by one.
    */
    void _Shared_ref() const {
      ++count;
    }

    /**
    * Releases a reference (decreases the reference count by one).  If
    * the count goes to 0, this object will delete itself.
    */
    void _Shared_unref() const {
      if ( !--count ) {
        const_cast<SafeShared*>( this ) ->prepareDeletion();
        if ( !count )
          delete this;
      }
    }

    /**
    * Return the current number of references held.
    *
    * @return Number of references
    */
    inline int _Shared_count() const {
      return count;
    }

  protected:
    virtual ~SafeShared() { }
  private:
    mutable ost::AtomicCounter count;
    ///This little additional effort is necessary to support weak pointers.
    virtual void prepareDeletion() {}
};


/**A shared pointer that is locked. The mutex will not be freed until this object is deleted.
  As long as this object exists, the data pointed to can be used exclusively. The data may also be 0 (invalid)! */
#ifndef USE_NEW_LOCKED
template <class Item>
class LockedSharedPtr : protected SharedPtr<Item> {
  private:
    void release() {
      if ( *this )
        this->data() ->lockCountDown();
      *( ( SharedPtr<Item>* ) this ) = 0;
    }

    void inc() {
      if ( *this )
        this->data() ->lockCountUp();
    }

    ///tries to increase the lock-count with a given timeout in nanoseconds, if ti fails sets itself to zero.
    void inc( int timeout ) {
      if ( *this )
        if ( !this->data() ->tryLockCountUp( timeout ) )
          SharedPtr<Item>::operator=( 0 );
    }

  public:
    inline LockedSharedPtr() : SharedPtr<Item>() {
      inc();
    }

    template <class Serialization>
    inline LockedSharedPtr( const SharedPtr<Item, Serialization>& item ) : SharedPtr<Item>( item ) {
      inc();
    }

    inline LockedSharedPtr( Item* item ) : SharedPtr<Item>( item ) {
      inc();
    }

    ///timeout in nanoseconds
    template <class Serialization>
    inline LockedSharedPtr( const SharedPtr<Item, Serialization>& item, int timeout ) : SharedPtr<Item>( item ) {
      inc( timeout );
    }

    inline LockedSharedPtr( const LockedSharedPtr<Item>& rhs ) : SharedPtr<Item>( rhs ) {
      inc();
    }

    template <class Target>
    inline LockedSharedPtr<Target> cast() {
      return LockedSharedPtr<Target>( dynamic_cast<Target*>( this->data() ) );
    }

    template <class Target>
    inline Target* freeCast() {
      return dynamic_cast<Target*>( this->data() );
    }


    LockedSharedPtr<Item>& operator = ( const LockedSharedPtr<Item>& rhs ) {
      if ( rhs.data() == this->data() )
        return * this;
      release();
      SharedPtr<Item>::operator=( rhs );
      inc();
      return *this;
    }

    inline ~LockedSharedPtr() {
      release();
    }

    using SharedPtr<Item>::operator->;
    using SharedPtr<Item>::operator*;

    inline bool operator ! () const {
      return !( bool ) * ( ( SharedPtr<Item>* ) this );
    }

    inline operator bool () const {
      return ( bool ) * ( ( SharedPtr<Item>* ) this );
    }

    inline operator const Item* () const {
      return data();
    }

    inline operator Item* () {
      return data();
    }

    template <class Item2>
    bool valueSmaller( const LockedSharedPtr<Item2>& rhs ) const {
      if ( this->data() ) {
        if ( rhs.data() ) {
          return ( this->data() ) ->operator<( *( rhs.data() ) );
        } else {
          return true;
        }
      } else {
        if ( rhs.data() ) {
          return true;
        } else {
          return false;
        }
      }
    }

    struct ValueSmallerCompare {
      inline bool operator() ( const LockedSharedPtr<Item>& lhs, const LockedSharedPtr<Item>& rhs ) const {
        return lhs.valueSmaller( rhs );
      }
    };

    using SharedPtr<Item>::data;
};
#else
template <class Item>
class LockedSharedPtr : protected SharedPtr<Item> {
  private:
    void release() {
      if ( *this )
        this->data() ->lockCountDown();
      *( ( SharedPtr<Item>* ) this ) = 0;
    }

    void inc() {
      if ( *this )
        this->data() ->lockCountUp();
    }

    ///tries to increase the lock-count with a given timeout in nanoseconds, if ti fails sets itself to zero.
    void inc( int timeout ) {
      if ( *this )
        if ( !this->data() ->tryLockCountUp( timeout ) )
          SharedPtr<Item, Serialization>::operator=( 0 );
    }

    using SharedPtr<Item, Serialization>::serialize;

  public:
    inline LockedSharedPtr() : SharedPtr<Item, Serialization>() {
      inc();
    };

    template <class Serialization2>
    inline LockedSharedPtr( const SharedPtr<Item, Serialization2>& item ) : SharedPtr<Item, Serialization>( const_cast<Item*>( item.data() ) ) {
      inc();
    };

    inline LockedSharedPtr( Item* item ) : SharedPtr<Item, Serialization>( item ) {
      inc();
    };

    ///timeout in nanoseconds
    template <class Serialization2>
    inline LockedSharedPtr( const SharedPtr<Item, Serialization2>& item, int timeout ) : SharedPtr<Item, Serialization>( const_cast<Item*>( item.data() ) ) {
      inc( timeout );
    };

    template <class Serialization2>
    inline LockedSharedPtr( const LockedSharedPtr<Item, Serialization2>& rhs ) : SharedPtr<Item, Serialization>( const_cast<Item*>( rhs.data() ) ) {
      inc();
    }

    template <class Target>
    inline LockedSharedPtr<Target> cast() {
      return LockedSharedPtr<Target>( dynamic_cast<Target*>( this->data() ) );
    }

    template <class Target>
    inline Target* freeCast() {
      return dynamic_cast<Target*>( this->data() );
    }


    LockedSharedPtr<Item, Serialization>& operator = ( const LockedSharedPtr<Item, Serialization>& rhs ) {
      release();
      SharedPtr<Item, Serialization>::operator=( rhs );
      inc();
      return *this;
    }

    inline ~LockedSharedPtr() {
      release();
    }

    using SharedPtr<Item, Serialization>::operator->;
    using SharedPtr<Item, Serialization>::operator*;

    inline bool operator ! () const {
      return !( bool ) * ( ( SharedPtr<Item, Serialization>* ) this );
    }

    inline operator bool () const {
      return ( bool ) * ( ( SharedPtr<Item, Serialization>* ) this );
    }

    inline operator const Item* () const {
      return data();
    }

    inline operator Item* () {
      return data();
    }

    template <class Item2, class Serialization2>
    bool valueSmaller( const LockedSharedPtr<Item2, Serialization2>& rhs ) const {
      if ( this->data() ) {
        if ( rhs.data() ) {
          return ( this->data() ) ->operator<( *( rhs.data() ) );
        } else {
          return true;
        }
      } else {
        if ( rhs.data() ) {
          return true;
        } else {
          return false;
        }
      }
    }


    struct ValueSmallerCompare {
      inline bool operator() ( const LockedSharedPtr<Item, Serialization>& lhs, const LockedSharedPtr<Item, Serialization>& rhs ) const {
        return lhs.valueSmaller( rhs );
      }
    };

    using SharedPtr<Item, Serialization>::data;
};
#endif

///five seconds of default-timeout
const int SafeSharedLockTimeout = SEC_TO_NANO * 5;

template <class Item, class Serialization>
class SafeSharedPtr : protected SharedPtr<Item, Serialization> {
  public:
    typedef SharedPtr<Item, Serialization> Precursor;
    
    typedef LockedSharedPtr<Item> Locked;
    
    template <class Serialization2>
    inline SafeSharedPtr( const SharedPtr<Item, Serialization2>& i ) : SharedPtr<Item, Serialization>( i ) {}

    inline SafeSharedPtr() : SharedPtr<Item, Serialization>() {
    };
    
    inline SafeSharedPtr( Item* i ) : SharedPtr<Item, Serialization>( i ) {}

    inline SafeSharedPtr( const SafeSharedPtr<Item, Serialization>& rhs ) : SharedPtr<Item, Serialization>( rhs.unsafe() ) {}

    template <class Item2, class Serialization2>
    inline SafeSharedPtr( const SafeSharedPtr<Item2, Serialization2>& rhs ) : SharedPtr<Item, Serialization>( rhs.unsafe() ) {}

    template <class Serialization2>
    inline SafeSharedPtr( SafeSharedPtr<Item, Serialization2>& rhs ) : SharedPtr<Item, Serialization>( rhs.unsafe() ) {}
#ifndef USE_NEW_LOCKED

    inline SafeSharedPtr( LockedSharedPtr<Item>& rhs ) : SharedPtr<Item, Serialization>( rhs.data() ) {}
#else
    /*template<class Serialization2>
    inline SafeSharedPtr( LockedSharedPtr<Item, Serialization2>& rhs ) : SharedPtr<Item, Serialization>( rhs.data() ) {
    }*/
#endif

    ///This function returns the locked pointer. This type of locking never returns zero, except the value of this pointer is zero..
    inline Locked lock ()
      const throw() {
      return Locked( *( Precursor* ) this );
    }

    ///The locking may fail, the result may be zero. @param timeout the locking-timeout in nanoseconds
    inline Locked tryLock( int timeout = 0 ) const throw() {
      return Locked( *( Precursor* ) this, timeout );
    }
    
    template <class Type>
    inline operator SafeSharedPtr<Type, Serialization>() {
      return SafeSharedPtr<Type, Serialization>( static_cast<Type*>( this->data() ) );
    }

    /// tries to lock with the default-timeout defined by SafeSharedLockTimeout
    inline operator Locked() const {
      //if( SafeSharedLockTimeout ) {
      return Locked( *( Precursor* ) this, SafeSharedLockTimeout );
      /*} else {
        return Locked(  *(Precursor*)this );
        }*/
    }
    
    template <class TargetType>
    inline SafeSharedPtr<TargetType, Serialization> cast() const {
      return SafeSharedPtr<TargetType, Serialization>( dynamic_cast<TargetType*>( const_cast<Item*>( this->data() ) ) );
    }

    ///This omits locking the mutex, and must only be used to call functions that are thread-safe
    inline Item* unsafe() const {
      return const_cast<Item*>( this->data() );
    }


    inline SafeSharedPtr<Item, Serialization>& operator = ( const SafeSharedPtr<Item, Serialization>& rhs ) {
      SharedPtr<Item, Serialization>::operator=( const_cast<Item*>( rhs.unsafe() ) );
      return *this;
    };

    template <class Item2, class Serialization2>
    inline SafeSharedPtr<Item, Serialization>& operator = ( const SafeSharedPtr<Item2, Serialization2>& rhs ) {
      SharedPtr<Item, Serialization>::operator=( const_cast<Item2*>( rhs.unsafe() ) );
      return *this;
    }

    inline operator bool () const {
      return ( bool ) * ( ( SharedPtr<Item, Serialization>* ) this );
    }

    inline bool operator !() const {
      return !( ( bool ) * this );
    }

    template <class Archive>
    void load( Archive& arch, uint /*version*/ ) {
      *this = 0;
      Serialization::template load<Item>
      ( *this, arch, 0 );
    }

    template <class Archive>
    void save( Archive& arch, uint /*version*/ ) const {
      Locked l = lock ()
                   ;
      Serialization::template save<Item>
      ( *this, arch, 0 );
    }
    
    template <class Serialization2>
    inline bool operator == ( const SafeSharedPtr<Item, Serialization2>& rhs ) const {
      return this->data() == rhs.data();
    }

    template <class Serialization2>
    inline bool operator < ( const SafeSharedPtr<Item, Serialization2>& rhs ) const {
      return this->data() < rhs.data();
    }
    
    /**This uses non-timed locks, so there is a deadlock-danger. */
    template <class Item2, class Serialization2>
    bool valueSmaller( const SafeSharedPtr<Item2, Serialization2>& rhs ) const {
      if ( this->data() ) {
        if ( rhs.data() ) {
          Locked l = lock ()
                       ;
          typename SafeSharedPtr<Item2, Serialization2>::Locked lr = rhs.lock();
          return ( this->data() ) ->operator<( *( rhs.data() ) );
        } else {
          return true;
        }
      } else {
        if ( rhs.data() ) {
          return true;
        } else {
          return false;
        }
      }
    }
    template <class Item2, class Serialization2>
    bool valueSame( const SafeSharedPtr<Item2, Serialization2>& rhs ) const {
      bool a = !this->data(), b = !rhs.data();

      if ( a && b )
        return true;
      else if ( a || b )
        return false;

      if ( this->data() ) {
        if ( rhs.data() ) {
          Locked l = lock ()
                       ;
          typename SafeSharedPtr<Item2, Serialization2>::Locked lr = rhs.lock();
          return ( this->data() ) ->operator==( *( rhs.data() ) );
        } else {
          return true;
        }
      }
      return false;
    }
    /*
    template <class OT>
    bool operator < ( const SafeSharedPtr<OT>& rhs ) const {
      if( this->data() ) {
        if( rhs.data() ) {
          Locked l = lock();
          typename SafeSharedPtr<OT>::Locked lr = rhs.lock();
          return (this->data())->operator<(*(rhs.data()) );
        } else {
          return true;
        }
      } else {
        if( rhs.data() ) {
          return true;
        } else {
          return false;
        }
      }
    }

    template <class OT>
    bool operator < ( const SharedPtr<OT>& rhs ) const {
      if( this->data() ) {
        if( rhs.data() ) {
          Locked l = lock();
          return (this->data())->operator<(*(rhs.data()) );
        } else {
          return true;
        }
      } else {
        if( rhs.data() ) {
          return true;
        } else {
          return false;
        }
      }
    }

    template <class OT>
    bool operator < ( const OT& rhs ) const {
      if( this->data() ) {
        Locked l = lock();
        return (this->data())->operator<(rhs);
      } else {
        return true;
      }
    }*/

    struct ValueSmallerCompare {
      inline bool operator() ( const SafeSharedPtr<Item, Serialization>& lhs, const SafeSharedPtr<Item, Serialization>& rhs ) const {
        return lhs.valueSmaller( rhs );
      }
    };

    struct ValueIdentCompare {
      inline bool operator() ( const SafeSharedPtr<Item, Serialization>& lhs, const SafeSharedPtr<Item, Serialization>& rhs ) const {
        return lhs.valueSame( rhs );
      }
    };
    

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

namespace boost {
namespace serialization {
template <class T, class B >
struct implementation_level< SafeSharedPtr<T, B> > {
  typedef mpl::integral_c_tag tag;
  typedef mpl::int_< boost::serialization::object_serializable > type;
  BOOST_STATIC_CONSTANT(
    int,
    value = implementation_level::type::value
  );
};
}
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
