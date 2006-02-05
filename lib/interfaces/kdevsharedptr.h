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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KDevSharedPTR_H
#define KDevSharedPTR_H

// #include "kdelibs_export.h"

/**
 * Reference counting for shared objects.  If you derive your object
 * from this class, then you may use it in conjunction with
 * KDevSharedPtr to control the lifetime of your object.
 *
 * Specifically, all classes that derive from KDevShared have an internal
 * counter keeping track of how many other objects have a reference to
 * their object.  If used with KDevSharedPtr, then your object will
 * not be deleted until all references to the object have been
 * released.
 *
 * You should probably not ever use any of the methods in this class
 * directly -- let the KDevSharedPtr take care of that.  Just derive
 * your class from KDevShared and forget about it.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
class /*KDECORE_EXPORT*/ KDevShared {
public:
   /**
    * Standard constructor.  This will initialize the reference count
    * on this object to 0.
    */
   KDevShared() : count(0) { }

   /**
    * Copy constructor.  This will @em not actually copy the objects
    * but it will initialize the reference count on this object to 0.
    */
   KDevShared( const KDevShared & ) : count(0) { }

   /**
    * Overloaded assignment operator.
    */
   KDevShared &operator=(const KDevShared & ) { return *this; }

   /**
    * Increases the reference count by one.
    */
   void _KDevShared_ref() const { count++; }

   /**
    * Releases a reference (decreases the reference count by one).  If
    * the count goes to 0, this object will delete itself.
    */
   void _KDevShared_unref() const { if (!--count) delete this; }

   /**
    * Return the current number of references held.
    *
    * @return Number of references
    */
   int _KDevShared_count() const { return count; }

protected:
   virtual ~KDevShared() { }
private:
   mutable int count;
};

/**
 * Can be used to control the lifetime of an object that has derived
 * KDevShared. As long a someone holds a KDevSharedPtr on some KDevShared
 * object it won't become deleted but is deleted once its reference
 * count is 0.  This struct emulates C++ pointers virtually perfectly.
 * So just use it like a simple C++ pointer.
 *
 * KDevShared and KDevSharedPtr are preferred over QShared / QSharedPtr
 * since they are more safe.
 *
 * WARNING: Please note that this class template provides an implicit
 * conversion to T*. Do *not* change this pointer or the pointee (don't
 * call delete on it, for instance) behind KDevSharedPtr's back.
 *
 * @author Waldo Bastian <bastian@kde.org>
 */
template< class T >
class KDevSharedPtr
{
public:

    typedef T Type;

/**
 * Creates a null pointer.
 */
  KDevSharedPtr()
    : ptr(0) { }
  /**
   * Creates a new pointer.
   * @param t the pointer
   */
  KDevSharedPtr( T* t )
    : ptr(t) { if ( ptr ) ptr->_KDevShared_ref(); }

  /**
   * Copies a pointer.
   * @param p the pointer to copy
   */
  KDevSharedPtr( const KDevSharedPtr& p )
    : ptr(p.ptr) { if ( ptr ) ptr->_KDevShared_ref(); }

  /**
   * Unreferences the object that this pointer points to. If it was
   * the last reference, the object will be deleted.
   */
  ~KDevSharedPtr() { if ( ptr ) ptr->_KDevShared_unref(); }

  KDevSharedPtr<T>& operator= ( const KDevSharedPtr<T>& p ) {
    if ( ptr == p.ptr ) return *this;
    if ( ptr ) ptr->_KDevShared_unref();
    ptr = p.ptr;
    if ( ptr ) ptr->_KDevShared_ref();
    return *this;
  }
  KDevSharedPtr<T>& operator= ( T* p ) {
    if ( ptr == p ) return *this;
    if ( ptr ) ptr->_KDevShared_unref();
    ptr = p;
    if ( ptr ) ptr->_KDevShared_ref();
    return *this;
  }
  bool operator== ( const KDevSharedPtr<T>& p ) const { return ( ptr == p.ptr ); }
  bool operator!= ( const KDevSharedPtr<T>& p ) const { return ( ptr != p.ptr ); }
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
  int count() const { return ptr->_KDevShared_count(); } // for debugging purposes
private:
  T* ptr;
};

#endif
