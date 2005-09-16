/* This file is part of the KDE libraries
   Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KSharedPTR_H
#define KSharedPTR_H

// #include "kdelibs_export.h"

/**
 * Reference counting for shared objects.  If you derive your object
 * from this class, then you may use it in conjunction with
 * KSharedPtr to control the lifetime of your object.
 *
 * Specifically, all classes that derive from KShared have an internal
 * counter keeping track of how many other objects have a reference to
 * their object.  If used with KSharedPtr, then your object will
 * not be deleted until all references to the object have been
 * released.
 *
 * You should probably not ever use any of the methods in this class
 * directly -- let the KSharedPtr take care of that.  Just derive
 * your class from KShared and forget about it.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class /*KDECORE_EXPORT*/ KShared {
public:
   /**
    * Standard constructor.  This will initialize the reference count
    * on this object to 0.
    */
   KShared() : count(0) { }

   /**
    * Copy constructor.  This will @em not actually copy the objects
    * but it will initialize the reference count on this object to 0.
    */
   KShared( const KShared & ) : count(0) { }

   /**
    * Overloaded assignment operator.
    */
   KShared &operator=(const KShared & ) { return *this; }

   /**
    * Increases the reference count by one.
    */
   void _KShared_ref() const { count++; }

   /**
    * Releases a reference (decreases the reference count by one).  If
    * the count goes to 0, this object will delete itself.
    */
   void _KShared_unref() const { if (!--count) delete this; }

   /**
    * Return the current number of references held.
    *
    * @return Number of references
    */
   int _KShared_count() const { return count; }

protected:
   virtual ~KShared() { }
private:
   mutable int count;
};

/**
 * Can be used to control the lifetime of an object that has derived
 * KShared. As long a someone holds a KSharedPtr on some KShared
 * object it won't become deleted but is deleted once its reference
 * count is 0.  This struct emulates C++ pointers virtually perfectly.
 * So just use it like a simple C++ pointer.
 *
 * KShared and KSharedPtr are preferred over QShared / QSharedPtr
 * since they are more safe.
 *
 * WARNING: Please note that this class template provides an implicit
 * conversion to T*. Do *not* change this pointer or the pointee (don't
 * call delete on it, for instance) behind KSharedPtr's back.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
template< class T >
class KSharedPtr
{
public:

    typedef T Type;

/**
 * Creates a null pointer.
 */
  KSharedPtr()
    : ptr(0) { }
  /**
   * Creates a new pointer.
   * @param t the pointer
   */
  KSharedPtr( T* t )
    : ptr(t) { if ( ptr ) ptr->_KShared_ref(); }

  /**
   * Copies a pointer.
   * @param p the pointer to copy
   */
  KSharedPtr( const KSharedPtr& p )
    : ptr(p.ptr) { if ( ptr ) ptr->_KShared_ref(); }

  /**
   * Unreferences the object that this pointer points to. If it was
   * the last reference, the object will be deleted.
   */
  ~KSharedPtr() { if ( ptr ) ptr->_KShared_unref(); }

  KSharedPtr<T>& operator= ( const KSharedPtr<T>& p ) {
    if ( ptr == p.ptr ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p.ptr;
    if ( ptr ) ptr->_KShared_ref();
    return *this;
  }
  KSharedPtr<T>& operator= ( T* p ) {
    if ( ptr == p ) return *this;
    if ( ptr ) ptr->_KShared_unref();
    ptr = p;
    if ( ptr ) ptr->_KShared_ref();
    return *this;
  }
  bool operator== ( const KSharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const KSharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
  bool operator== ( const T* p ) const { return ( ptr == p ); }
  bool operator!= ( const T* p ) const { return ( ptr != p ); }
  bool operator!() const { return ( ptr == 0 ); }
  operator T*() const { return ptr; }

  /**
   * Returns the pointer.
   * @return the pointer
   */
  T* data() { return ptr; }

  /**
   * Returns the pointer.
   * @return the pointer
   */
  const T* data() const { return ptr; }

  const T& operator*() const { return *ptr; }
  T& operator*() { return *ptr; }
  const T* operator->() const { return ptr; }
  T* operator->() { return ptr; }

  /**
   * Returns the number of references.
   * @return the number of references
   */
  int count() const { return ptr->_KShared_count(); } // for debugging purposes
private:
  T* ptr;
};

#endif
