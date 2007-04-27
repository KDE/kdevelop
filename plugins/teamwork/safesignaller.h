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

/*
This header implements a very simple signalling-system that uses the SafeSharedPtr or WeakSafeSharedPtr-facilities to be thread-safe.
*/

#ifndef SAFESIGNALLER_H
#define SAFESIGNALLER_H

#include <QObject>
#include <ext/hash_set>
#include "network/safesharedptr.h"
#include "network/weaksafesharedptr.h"

struct SignalProxyBase {
  virtual void signal() = 0;
  virtual ~SignalProxyBase() {
  };
};

template<class Type>
struct hashCall {
  size_t operator() ( const Type& rhs ) const {
    return (size_t)rhs.unsafe();
  }
};

///An object of this type represents a signal that multiple receiver-objects can be bound to. To get a function similar to qt's auto-disconnect, WeakSharedPtr<...> may be used as Target-type. If SafeSharedPtr is used, connected objects will be kept alive until the SafeSignal is destroyed or the object disconnected.
template<class TargetType, class Signal>
class SafeSignal {
  SafeSignal& operator = ( const SafeSignal& rhs ) {
    return *this;
  }
  SafeSignal( const SafeSignal& rhs ) {
  }

public:
  SafeSignal() {
  }

  void connect( const TargetType& targ ) {
    if( isConnected( targ ) ) return;
    m_connected.insert( targ );
  }

  void disconnect( const TargetType& targ ) {
    typename ConnectionMap::iterator it = m_connected.find( targ );
    if( it != m_connected.end() )
      m_connected.erase( it );
  }

  bool isConnected( const TargetType& targ ) {
    return m_connected.find( targ ) != m_connected.end();
  }

  void operator() ( const Signal& signal = Signal() ) {
    for( typename ConnectionMap::iterator it = m_connected.begin(); it != m_connected.end(); ++it ) {
      typename TargetType::Locked l = *it;
      if( l )
        l->dispatchSignal( signal );
    }
  }

private:
  SignalProxyBase* m_proxy;
  typedef __gnu_cxx::hash_set< TargetType, hashCall<TargetType> > ConnectionMap;
  ConnectionMap m_connected;
};

template<class TargetType, class Signal>
struct SignalProxy : public SignalProxyBase {
  SafeSignal<TargetType, Signal> s;
  Signal sign;
  SignalProxy( const TargetType& target, const Signal& sig ) : sign( sig ) {
    s.connect( target );
  }
  virtual void signal() {
    s( sign );
  }
};

///This is a very simple safe-signaller object that can be used to easily bind safe signals into QActions etc. The signal-type may be any copyable object. The given object will be copied, and sent once the signal is emitted.
class QSafeSignaller : public QObject {
  Q_OBJECT;
  QSafeSignaller& operator = ( const QSafeSignaller& rhs );
  QSafeSignaller( const QSafeSignaller& rhs );
public:
  template<class Target, class Signal>
  QSafeSignaller( const Target& target, const Signal& signal, QObject* parent ) : QObject( parent ) {
    m_proxy = new SignalProxy<Target, Signal>( target, signal );
  }
  ~QSafeSignaller();
public slots:
  void signal();
private:
  SignalProxyBase* m_proxy;
};


#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
