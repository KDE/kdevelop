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

#ifndef TEAMWORK_HELPERS
#define TEAMWORK_HELPERS
#include "common.h"
#include "pointer.h"
#include <list>
#include <set>
#include <map>

class LockMutex {
	Mutex& m_;
	LockMutex& operator=( const LockMutex& rhs ) {
		UNUSED( rhs );
		return *this;
	};
	
public:
	LockMutex( Mutex& mutex ) : m_( mutex ) {
		m_.enterMutex();
	}

	LockMutex( LockMutex& rhs ) : m_( rhs.m_ ) {
		m_.enterMutex();
	};

	~LockMutex() {
		m_.leaveMutex();
	}
};

template <class Type>
class SafeList {
	std::list<Type> list_;
	mutable Mutex m_;
public:
	SafeList() {
	};
	
	void push_back( const Type& t ) {
		LockMutex l( m_ );
		list_.push_back( t );
	}
	
	void push_front( const Type& t ) {
		LockMutex l( m_ );
		list_.push_front( t );
	}
	
	void pop_back() {
		LockMutex l( m_ );
		list_.pop_back();
	}
	
	void pop_front() {
		LockMutex l( m_ );
		list_.pop_front();
	}
	
	Type& front() {
		LockMutex l( m_ );
		return list_.front();
	}
	
	Type& back() {
		LockMutex l( m_ );
		return list_.back();
	}
	
	const Type& front() const {
		LockMutex l( m_ );
		return list_.front();
	}
	
	const Type& back() const {
		LockMutex l( m_ );
		return list_.back();
	}
	
	SafeList& operator << ( const Type& item ) {
		LockMutex l( m_ );
		push_back( item );
		return *this;
	}
	
	bool empty() const {
		LockMutex l( m_ );
		return list_.empty();
	}
	
	void clear() {
		LockMutex l( m_ );
		list_.clear();
	}
	
	bool empty() {
		LockMutex l( m_ );
		return list_.empty();
	}
	
	operator bool() const {
		return !empty();
	}
	
	int size() const {
		LockMutex l( m_ );
		return list_.size();
	}
};


#include "message.h"


namespace Teamwork {
	class BasicTCPSession;
//	typedef SafeSharedPtr<BasicTCPSession> SessionPointer;

		template<class Archive>
	DispatchableMessage buildMessageFromArchive( Archive& arch, MessageTypeSet& messages, SessionPointer sess  ) {
		MessageInfo inf( arch );
		inf.setSession( sess );
		
		return messages.buildMessage( arch, inf );
	}
	
	///May throw several exceptions
	template<class ArchType>
	void serializeMessageToArchive( ArchType& arch, MessageInterface& message ) {
		message.info().serialize( arch );
		
		message.serialize( arch ); ///it must be called like this, because the serialization would add type-information we don't want
	}

	///May throw several exceptions
	void serializeMessageToBuffer( std::vector<char>& buf, MessageInterface& message );
	DispatchableMessage buildMessageFromBuffer( const std::vector<char>& buf, MessageTypeSet& messages, SessionPointer sess = 0  );
}

template <class VecType, class Vec2Type>
std::vector<VecType>& operator += ( std::vector<VecType>& lhs, const std::vector<Vec2Type>& rhs ) {
	uint osize = lhs.size();
	lhs.resize( lhs.size() + ( (rhs.size() * sizeof( Vec2Type )) / sizeof(VecType))  );
	memcpy( &lhs[osize], &rhs[0], sizeof(Vec2Type) * rhs.size() );
	return lhs;
}


std::string formatInt( int i );

std::vector<char> binaryInt( int num );

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
