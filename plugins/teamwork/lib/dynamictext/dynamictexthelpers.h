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

#define private public

#ifndef DYNAMICTEXT_HELPERS
#define DYNAMICTEXT_HELPERS

#include "offsetutils.h"
#include "verify.h"
#include "sharedptr.h"
#include "weakpointer.h"
#include "vectortimestamp.h"

///Check whether these are useful for something or should be completely removed
//#define USELINKS

class DynamicText;
class Replacement;
class Advancer;
typedef SharedPtr<Replacement, BoostSerializationNormal> ReplacementPointer;
typedef WeakSharedPtr<Replacement, BoostSerializationNormal> WeakReplacementPointer;

struct SimpleReplacement {
	uint m_position;
	std::string m_oldText;
	std::string m_newText;
	SimpleReplacement() : m_position( 0 ) {
	}
	SimpleReplacement( uint position, const std::string& newText, const std::string& oldText ) : m_position( position ), m_oldText( oldText ), m_newText( newText ) {}

	operator bool() const {
		return m_oldText.length() != 0 || m_newText.length() != 0;
	}
	template<class Archive>
	void  serialize( Archive& arch, const uint ) {
		arch & m_position & m_oldText & m_newText;
	}
};

class Replacement : public WeakShared {
	public:

		Replacement( const VectorTimestamp& state, const SimpleReplacement& repl ) : m_state( state ), m_replacement( repl ), m_enabled(true) {}
		Replacement() : m_enabled(true) {}

    ///Returns a map that simulates applying the replacement. of( x -> y ) with x pre-state of this state, and y state of this.
		OffsetMap offset( const OffsetMap& ofs = OffsetMap() );
		
		ReplacementPointer next() const;
		ReplacementPointer prev() const;

		bool enabled() const;

		///These only compare the primary stamp
		bool operator < ( const Replacement& rhs ) const throw(DynamicTextError);
		bool operator <= ( const Replacement& rhs ) const throw(DynamicTextError);

    uint primaryIndex() const;
    Timestamp primaryStamp() const;

		const VectorTimestamp& vectorStamp() const;

		template<class Archive>
		void serialize( Archive& arch, const uint ) {
			arch & m_state & m_replacement & m_next & m_prev & m_enabled;
		}

		const SimpleReplacement& replacement() const {
			return m_replacement;
		}

    ///This must only be changed in an unapplied state. Else it will lead to corruption of the document.
		void setEnabled( bool e );

	private:
		void setNext( ReplacementPointer next );
		void setPrev( ReplacementPointer prev );

    template<class TextType>
    bool apply( TextType& text, const OffsetMap& outerOffset, OffsetMap& contextOffset );
    template<class TextType>
    bool unApply( TextType& text, const OffsetMap& outerOffset, OffsetMap& contextOffset );

#ifdef USE_LINKS
    ///Checks whether "link" should be put into the "links"-map. If it should
    void updateLink( uint index, ReplacementPointer link );
#endif

		friend class DynamicText;
		VectorTimestamp m_state; ///Each Replacement has a unique primary stamp, m_next has the same plus 1, m_prev the same minus one.

		SimpleReplacement m_replacement;

		///One is weak one is not, so a chain of replacements cannot keep itself alive. Once the first is lost, all are lost.
		ReplacementPointer m_next;
		WeakReplacementPointer m_prev;

#ifdef USELINKS
    std::vector<WeakReplacementPointer> m_links; ///This maps the index-numbers of other Collaborators to their first replacement that uses the timestamp of this Replacement.
#endif

		bool m_enabled;
};

struct ReplacementChain {
	ReplacementPointer first, last;
	template<class Archive>
	void serialize( Archive& arch, const uint ) {
		arch & first & last;
	}
};

struct ReplacementId {
	uint primaryIndex;
	Timestamp stamp;
	ReplacementId( uint i = 0, Timestamp st = 0 ) : primaryIndex( i ), stamp( st ) {}
	ReplacementId( ReplacementPointer p ) : primaryIndex( p->primaryIndex() ), stamp( p->primaryStamp() ) {}
	bool operator < ( const ReplacementId& rhs ) const {
		return primaryIndex < rhs.primaryIndex || ( primaryIndex == rhs.primaryIndex && stamp < rhs.stamp );
	}
	template<class Archive>
	void serialize( Archive& arch, const uint version ) {
		arch & primaryIndex & stamp;
	}
};

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
