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
#ifndef WEAKPOINTER_H
#define WEAKPOINTER_H

#include "safesharedptr.h"

class WeakSafeShared;
class WeakShared;

#ifndef WEAKSAFESHAREDPTR_HAVE_DEFAULT
#define WEAKSAFESHAREDPTR_HAVE_DEFAULT
template<class Type, class Serialization = NormalSerialization>
class WeakSafeSharedPtr;
#endif


class WeakReference : public SafeShared {
    WeakSafeShared* pointer_;
    friend class WeakSafeShared;

    void invalidate() {
      pointer_ = 0;
    };
  public:
    WeakReference( WeakSafeShared* pointer ) : pointer_( pointer ) {}
    WeakSafeShared* pointer() {
      return pointer_;
    }
};

class WeakSafeShared : protected SafeShared {
    SafeSharedPtr<WeakReference> ref_;

    virtual void prepareDeletion() {
      LockedSharedPtr<WeakReference> l = ref_.lock();
      if ( l )
        l->invalidate();
    }

  public:
  WeakSafeShared() : ref_( new WeakReference( this ) ) {}

    inline WeakSafeShared &operator=( const WeakSafeShared & ) {
      return * this;
    }

    const SafeSharedPtr<WeakReference>& _WeakSafeShared_getRef() const {
      return ref_;
    }

    using SafeShared::lockCountUp;

    using SafeShared::tryLockCountUp;

    using SafeShared::lockCountDown;

    using SafeShared::_Shared_count;

    using SafeShared::_Shared_ref;

    using SafeShared::_Shared_unref;
};

/** This header gives an additional type of pointer with a higher level. It's functionality is similar to that of QPointer.
    Holding a pointer of type WeakSafeShared, does not increase the reference-count of the pointed object. If the object is deleted,
    the pointer is set to zero. This comes with additional cost. The pointed object must be based on WeakSafeShared, and for each creation of
    that type one additional allocation of an internal SafeShared-object(especially a mutex) is done.

    WeakSafeSharedPtr itself does not consume more memory than SafeSharedPtr, but locking-operations are more costly, because two locks are done instead of one.
 */

template <class Item, class Serialization>
class WeakSafeSharedPtr {
    SafeSharedPtr<WeakReference> ref_;
  public:
    WeakSafeSharedPtr() {}

    template <class Input>
    WeakSafeSharedPtr( const Input& ptr ) {
      set
        ( ptr );
    }

    SafeSharedPtr<Item, Serialization> get
      () const {
      typename SafeSharedPtr<WeakReference>::Locked l = ref_;
      if ( !l )
        return 0;
      return SafeSharedPtr<Item, Serialization>( dynamic_cast<Item*>( l->pointer() ) );
    }

    template <class Item2, class Serialization2>
    void set
      ( const SafeSharedPtr<Item2, Serialization2>& item ) {
      typename SafeSharedPtr<Item2, Serialization2>::Locked l = item;
      if ( !l ) {
        ref_ = 0;
        return ;
      } else {
        ref_ = static_cast<Item*>( l.data() ) ->_WeakSafeShared_getRef(); ///The item must be compatible
      }
    }

    void set
      ( const Item* item ) {
      if ( !item )
        ref_ = 0;
      else
        ref_ = item->_WeakSafeShared_getRef();
    }

    WeakSafeSharedPtr<Item, Serialization> operator = ( const Item* item ) {
      set
        ( item );
      return *this;
    }

    template <class Item2, class Serialization2>
    WeakSafeSharedPtr<Item, Serialization> operator = ( const SafeSharedPtr<Item2, Serialization2> item ) {
      set
        ( item );
      return *this;
    }

    ///Using this is same as locking a SafeSharedPtr, it uses the default-timeout.
    operator LockedSharedPtr<Item> () const {
      Item * p;
      {
        LockedSharedPtr<WeakReference> l = ref_;
        if ( !l )
          return 0;
        p = reinterpret_cast<Item*>( l->pointer() );
      }
      return LockedSharedPtr<Item>( SharedPtr<Item>( p ), SafeSharedLockTimeout );
    }

    template <class Serialization2>
    inline bool operator == ( const SafeSharedPtr<Item, Serialization2>& rhs ) const {
      if ( !rhs && !ref_ )
        return true;
      typename SafeSharedPtr<WeakReference>::Locked l = ref_;
      if ( !l )
        return false;

      return l->pointer() == ( WeakSafeShared* ) rhs.unsafe();
    }

    inline bool operator == ( const LockedSharedPtr<Item>& rhs ) const {
      if ( !rhs && !ref_ )
        return true;
      LockedSharedPtr<WeakReference> l = ref_;
      if ( !l )
        return false;

      return l->pointer() == ( WeakSafeShared* ) rhs.unsafe();
    }

    template <class Serialization2>
    inline bool operator == ( const SharedPtr<Item, Serialization2>& rhs ) const {
      if ( !rhs && !ref_ )
        return true;
      LockedSharedPtr<WeakReference> l = ref_;
      if ( !l )
        return false;

      return l->pointer() == ( WeakSafeShared* ) rhs.data();
    }

    inline bool operator == ( const Item* rhs ) const {
      if ( !rhs && !ref_ )
        return true;
      LockedSharedPtr<WeakReference> l = ref_;
      if ( !l )
        return false;

      return l->pointer() == ( WeakSafeShared* ) rhs;
    }
};

class NormalWeakReference : public Shared {
    WeakShared* pointer_;
    friend class WeakShared;

    void invalidate() {
      pointer_ = 0;
    };
  public:
    NormalWeakReference( WeakShared* pointer ) : pointer_( pointer ) {}
    WeakShared* pointer() {
      return pointer_;
    }
    const WeakShared* pointer() const {
      return pointer_;
    }
};

/**  To use WeakSharedPtr on an object, that object must be based this type.
 */

class WeakShared : private Shared {
    SharedPtr<NormalWeakReference> ref_;

    virtual void prepareDeletion() {
      ref_->invalidate();
    }

  public:
    WeakShared() : ref_( new NormalWeakReference( this ) ) {}

    inline WeakShared &operator=( const WeakShared & ) {
      return * this;
    }

    const SharedPtr<NormalWeakReference>& _WeakShared_getRef() const {
      return ref_;
    }

    using Shared::_Shared_count;

    using Shared::_Shared_ref;

    using Shared::_Shared_unref;
};

template <class Item, class Serialization = NormalSerialization>
class WeakSharedPtr {
    SharedPtr<NormalWeakReference> ref_;
  public:
    WeakSharedPtr() {}

    template <class Input>
    WeakSharedPtr( const Input& ptr ) {
      set
        ( ptr );
    }

    Item* get
      () {
      if ( !ref_ )
        return 0;
      return ( Item* ) ref_->pointer();
    }

    Item* get
      () const {
        if ( !ref_ )
          return 0;
        return ( Item* ) const_cast<WeakShared*>( ref_->pointer() );
      }

    void set
      ( const Item* item ) {
      if ( !item )
        ref_ = 0;
      else
        ref_ = item->_WeakShared_getRef();
    }

    /*WeakSharedPtr<Item, Serialization> operator = ( const Item* item ) {
      set
        ( item );
      return *this;
    }*/

    WeakSharedPtr<Item, Serialization> operator = ( const WeakSharedPtr<Item, Serialization>& item ) {
      set
        ( item );
      return *this;
    }

    /*operator SharedPtr<Item>() const {
      return get();
    }*/

    operator Item*() {
      return get
               ();
    }

    operator Item*() const {
      return const_cast<Item*>( get
                                  () );
    }

    Item* operator ->() {
      return get
               ();
    }

    ///This leads to another order then sorting equivalent normal pointers
    bool operator < ( const WeakSharedPtr<Item, Serialization>& rhs ) const {
      return ref_ < rhs.ref_;
    }

    operator bool() const {
      return ( bool ) get
               ();
    }

    const Item* operator ->() const {
      return get
               ();
    }

    operator SharedPtr<Item, Serialization>() const {
      return SharedPtr<Item, Serialization>( get() );
    }

    bool operator == ( const Item* rhs ) const {
      if ( !rhs && !ref_ )
        return true;
      if ( !ref_ )
        return false;

      return ref_->pointer() == ( WeakShared* ) rhs;
    }

    bool operator == ( const WeakSharedPtr<Item, Serialization>& rhs ) const {
      return ref_ == rhs.ref_;
    }

    template <class Archive>
    void serialize( Archive& arch, int /*version*/ ) {
      SharedPtr<Item, Serialization> it( get
                                           () );
      arch & it;
      set
        ( it );
    }
};


#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
