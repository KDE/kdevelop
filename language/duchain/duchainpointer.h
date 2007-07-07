/* 
   Copyright (C) 2007 David Nolden <user@host.de>
   (where user = david.nolden.kdevelop, host = art-master)

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

#ifndef DUCHAINPOINTER_H
#define DUCHAINPOINTER_H

#include <ksharedptr.h>
#include <languageexport.h>

namespace KDevelop {

class DUContext;
class DUChainBase;
class Declaration;
class ClassFunctionDeclaration;
class Use;
class Definition;

/**
 * Whenever the du-chain is unlocked and locked again, any du-chain item may have been deleted in between.
 * For that reason, the following class should be used to make sure that no deleted objects are accessed. It contains a pointer
 * that will be reset to zero once the pointed object is deleted.
 *
 * Access to the data must still be serialized through duchain-locking. Using this comes with no additional cost.
 *
 * In practice this means:
 * Store an instance of DUChainPointer instead of a pointer to the du-chain object.
 * Then, access the eventually still existing object by calling pointer->base().
 *
 * To make it even more convenient see 
 * */
class KDEVPLATFORMLANGUAGE_EXPORT  DUChainPointerData : public KShared {
  public:
    /**
     * Will return zero once the pointed-to object was deleted
     * */
    DUChainBase* base() {
      return d;
    }
    
    /**
     * Will return zero once the pointed-to object was deleted
     * */
    const DUChainBase* base() const {
      return d;
    }

    ///Default-initialization of an invalid reference
    DUChainPointerData() : d(0) {
    }
    
    ///Should not be used from outside, but is needed sometimes to construct an invalid dummy-pointer
    DUChainPointerData( DUChainBase* base ) : d(base) {
    }
  private:
    friend class DUChainBase;
    DUChainBase* d;
    Q_DISABLE_COPY(DUChainPointerData)
};

typedef KSharedPtr<DUChainPointerData> DUChainBasePointer;

/**
 * A smart-pointer similar class that conveniently wraps around DUChainPointer without
 * too many dynamic casts.
 *
 * It can be used like a normal pointer.
 *
 * Access must be serialized through the du-chain locks
 * */
  
  template<class Type>
  class DUChainPointer {
    public:
    //Implemented so this can be used in arrays etc. avoid using this, it does an unneeded allocation
    DUChainPointer() : d(new DUChainPointerData(0)) {
    }

    DUChainPointer( Type* rhs ) {
      if( rhs )
        d = rhs->weakPointer();
      else
        d = new DUChainPointerData(0);
    }
    
    operator bool() const {
      return (bool)d->base();
    }

    Type& operator* () {
      return *static_cast<Type*>(d->base());
    }
    
    const Type& operator* () const {
      return *static_cast<const Type*>(d->base());
    }

    Type* operator->() {
      return static_cast<Type*>(d->base());
    }

    operator const Type*() const {
      return static_cast<const Type*>(d->base());
    }
    
    operator Type*() {
      return static_cast<Type*>(d->base());
    }

    const Type* operator->() const {
      return static_cast<const Type*>(d->base());
    }
    
    Type* data() {
      return static_cast<Type*>(d->base());
    }

    const Type* data() const {
      return static_cast<const Type*>(d->base());
    }

    DUChainPointer<Type>& operator= ( Type* rhs ) {
      d = rhs->weakPointer();
      return *this;
    }
    
    private:
      DUChainBasePointer d;
  };

  typedef DUChainPointer<DUContext> DUContextPointer;
  typedef DUChainPointer<Declaration> DeclarationPointer;
  typedef DUChainPointer<Use> UsePointer;
  typedef DUChainPointer<Definition> DefinitionPointer;
  typedef DUChainPointer<ClassFunctionDeclaration> FunctionDeclarationPointer;
}

#endif
