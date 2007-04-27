/* This file is part of the KDE libraries
Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
Copyright (c) 2006 David Nolden <david.nolden.kdevelop@art-master.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License version 2 as published by the Free Software Foundation.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#ifndef SharedPTR_H
#define SharedPTR_H

#include <boost/serialization/split_member.hpp>

/**
 * Warning: Not thread-safe: use SafeShared for thread-safe reference-counting.
 * Reference counting for shared objects.  If you derive your object
 * from this class, then you may use it in conjunction with
 * SharedPtr to control the lifetime of your object.
 *
 * Specifically, all classes that derive from Shared have an internal
 * counter keeping track of how many other objects have a reference to
 * their object.  If used with SharedPtr, then your object will
 * not be deleted until all references to the object have been
 * released.
 *
 * You should probably not ever use any of the methods in this class
 * directly -- let the SharedPtr take care of that.  Just derive
 * your class from Shared and forget about it.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class Shared {
  public:
    /**
     * Standard constructor.  This will initialize the reference count
     * on this object to 0.
     */
    Shared() : count( 0 ) { }

    /**
     * Copy constructor.  This will @em not actually copy the objects
     * but it will initialize the reference count on this object to 0.
     */
    Shared( const Shared & ) : count( 0 ) { }

    /**
     * Overloaded assignment operator.
     */
    Shared &operator=( const Shared & ) {
      return * this;
    }

    /**
     * Increases the reference count by one.
     */
    void _Shared_ref() const {
      count++;
    }

    /**
     * Releases a reference (decreases the reference count by one).  If
     * the count goes to 0, this object will delete itself.
     */
    void _Shared_unref() const {
      if ( !--count )
        delete this;
    }

    /**
     * Return the current number of references held.
     *
     * @return Number of references
     */
    int _Shared_count() const {
      return count;
    }

  protected:
    virtual ~Shared() { }
  private:
    mutable int count;
};

///NormaSerialization serializes the object in a primitive way by just calling the objects serialization-method.

struct NormalSerialization {
  template <class Type, class Archive, class Target>
  static void load( Target& t, Archive& arch, int /*version*/ ) {
    bool b = false;
    arch >> b;
    if ( b ) {
      t = new Type();
      t->serialize( arch, 0 );
    } else {
      t = 0;
    }
  }

  template <class Type, class Archive, class Target>
  static void save( Target& t, Archive& arch, int /*version*/ ) {
    if ( t ) {
      bool b = true;
      arch << b;
      const_cast<Type&>( *t ).serialize( arch, 0 );
    } else {
      bool b = false;
      arch << b;
    }
  }
};


template <class Type, class Container>
struct extractItemUnsafe {
  static Type* extract( const Container& cont ) {
    return const_cast<Type*>( cont.unsafe() );
  }
};


/**BoostSerialization relies on boost's pointer-serialization(Which includes tracking, versioning, etc.).
 * Objects serialized this way must be registered to the boost serialization-system.
*/
struct BoostSerialization {
  template <class Type, class Archive, class Target>
  static void load( Target& t, Archive& arch, int /*version*/ ) {
    bool b = false;
    arch >> b; //boost::serialization::make_nvp("valid",b);
    if ( b ) {
      Type * pnt = 0;
      arch & pnt;
      t = pnt;
    } else {
      t = 0;
    }
  }

  template <class Type, class Archive, class Target>
  static void save( Target& t, Archive& arch, int /*version*/ ) {
    if ( t ) {
      bool b = true;
      arch << b; //boost::serialization::make_nvp("valid",b);
      const Type* pnt = extractItemUnsafe<Type, Target>::extract( t );
      arch & pnt;
    } else {
      bool b = false;
      arch << b; //boost::serialization::make_nvp("valid",b);
    }
  }
};

struct BoostSerializationNormal {
  template <class Type, class Archive, class Target>
  static void load( Target& t, Archive& arch, int /*version*/ ) {
    bool b = false;
    arch >> b; //boost::serialization::make_nvp("valid",b);
    if ( b ) {
      Type * pnt = 0;
      arch & pnt;
      t = pnt;
    } else {
      t = 0;
    }
  }

  template <class Type, class Archive, class Target>
  static void save( Target& t, Archive& arch, int /*version*/ ) {
    if ( t ) {
      bool b = true;
      arch << b; //boost::serialization::make_nvp("valid",b);
      const Type* pnt = t.data();
      arch & pnt;
    } else {
      bool b = false;
      arch << b; //boost::serialization::make_nvp("valid",b);
    }
  }
};
/**
 * Can be used to control the lifetime of an object that has derived
 * Shared. As long a someone holds a SharedPtr on some Shared
 * object it won't become deleted but is deleted once its reference
 * count is 0.  This struct emulates C++ pointers virtually perfectly.
 * So just use it like a simple C++ pointer.
 *
 * Shared and SharedPtr are preferred over QShared / QSharedPtr
 * since they are more safe.
 *
 * WARNING: Please note that this class template provides an implicit
 * conversion to T*. Do *not* change this pointer or the pointee (don't
 * call delete on it, for instance) behind SharedPtr's back.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */

template < class T, class Serialization = NormalSerialization >
class SharedPtr {
  public:
    friend class NormalSerialization;
    /**
     * Creates a null pointer.
     */
    inline SharedPtr()
        : ptr( 0 ) { }
    /**
     * Creates a new pointer.
     * @param t the pointer
     */
    inline SharedPtr( T* t )
        : ptr( t ) {
      if ( ptr )
        ptr->_Shared_ref();
    }

    /**
     * Copies a pointer.
     * @param p the pointer to copy
     */
    inline SharedPtr( const SharedPtr& p )
        : ptr( p.ptr ) {
      if ( ptr )
        ptr->_Shared_ref();
    }

    /**
     * Unreferences the object that this pointer points to. If it was
     * the last reference, the object will be deleted.
     */
    inline ~SharedPtr() {
      if ( ptr )
        ptr->_Shared_unref();
    }

    SharedPtr<T, Serialization>& operator= ( const SharedPtr<T, Serialization>& p ) {
      if ( ptr == p.ptr )
        return * this;
      if ( ptr )
        ptr->_Shared_unref();
      ptr = p.ptr;
      if ( ptr )
        ptr->_Shared_ref();
      return *this;
    }

    template <class Serialization2>
    SharedPtr<T, Serialization>& operator= ( const SharedPtr<T, Serialization2>& p ) {
      if ( ptr == p.ptr )
        return * this;
      if ( ptr )
        ptr->_Shared_unref();
      ptr = p.ptr;
      if ( ptr )
        ptr->_Shared_ref();
      return *this;
    }

    SharedPtr<T, Serialization>& operator= ( T* p ) {
      if ( ptr == p )
        return * this;
      if ( ptr )
        ptr->_Shared_unref();
      ptr = p;
      if ( ptr )
        ptr->_Shared_ref();
      return *this;
    }
    template <class Serialization2>
    inline bool operator== ( const SharedPtr<T, Serialization2>& p ) const {
      return ( ptr == p.ptr );
    }
    template <class Serialization2>
    inline bool operator!= ( const SharedPtr<T, Serialization2>& p ) const {
      return ( ptr != p.ptr );
    }
    inline bool operator== ( const T* p ) const {
      return ( ptr == p );
    }
    inline bool operator!= ( const T* p ) const {
      return ( ptr != p );
    }
    inline bool operator!() const {
      return ( ptr == 0 );
    }
    inline operator T*() const {
      return ptr;
    }

    /**
     * Returns the pointer.
     * @return the pointer
     */
    inline T* data() {
      return ptr;
    }

    /**
     * Returns the pointer.
     * @return the pointer
     */
    inline const T* data() const {
      return ptr;
    }

    /*    inline const T& operator*() const {
          return * ptr;
    }*/
    inline T& operator*() {
      return * ptr;
    }
    inline T& operator*() const {
      return * const_cast<T*>( ptr );
    }
    /*inline const T* operator->() const {
      return ptr;
    }*/
    inline T* operator->() const {
      return const_cast<T*>( ptr );  ///That a reference to a pointer is constant does not mean that the pointed-to object is constant(to me)
    }

    template <class Archive>
    void load( Archive& arch, int /*version*/ ) {
      Serialization::template load<T>
      ( *this, arch, 0 );
    }

    template <class Archive>
    void save( Archive& arch, int /*version*/ ) const {
      Serialization::template save<T>
      ( *this, arch, 0 );
    }
    /*
    ///pointers without target are sorted as lowest
    template <class OT>
    bool operator < ( const SharedPtr<OT>& rhs ) const {
      if( ptr ) {
        if( rhs.ptr ) {
          return *ptr < *rhs.ptr;
        } else {
          return true;
        }
      } else {
        if( rhs.ptr ) {
          return true;
        } else {
          return false;
        }
      }
    }

    template <class OT>
    bool operator < ( const OT& rhs ) const {
      if( ptr ) {
        return *ptr < rhs;
      } else {
        return true;
      }
    }*/

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    /**
     * Returns the number of references.
     * @return the number of references
     */
    inline int count() const {
      return ptr->_Shared_count();
    } // for debugging purposes
  private:
    T* ptr;
};


template <class Type, class Serialization>
struct extractItemUnsafe<Type, SharedPtr<Type, Serialization> > {
  static Type* extract( const SharedPtr<Type, Serialization>& cont ) {
    return const_cast<Type*>( cont.data() );
  }
};

/*template<class Type, class Serialization>
Type* extractItemUnsafe( const SharedPtr< Type, Serialization >& cont ) {
return const_cast<Type*>( cont.data() );
}*/

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
