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

#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H
#include <string>
#include<boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

typedef std::string String;

#include "crossmap.h"
#include "pointer.h"
#include "weakpointer.h"
#include "vectortimestamp.h"
#include "offsetutils.h"

#include "dynamictexthelpers.h"
#include "flexibletext.h"
using namespace Tree;

struct OffsetRequest {
  VectorTimestamp from;
  VectorTimestamp to;
  OffsetRequest( const VectorTimestamp& f, const VectorTimestamp& t ) : from(f), to(t) {
  }
  bool operator == ( const OffsetRequest& rhs ) const {
    return from == rhs.from && to == rhs.to;
  }
};

namespace __gnu_cxx {
template<>
struct hash<OffsetRequest> {
  size_t operator () ( const OffsetRequest& req ) const {
    return req.from.hash() + 3*req.to.hash();
  }
};
}

/** DynamicText is a class that manages a dynamic document that is
 * created by an arbitrary count of parallel sources by timestamped replacements.
 *
 * It allows to create any state that is possible without conflicts(allows for example
 * to undo all changes done by one user, or even only specific changes by disabling them
 * one for one.)
 *
 * currently, every function may throw DynamicTextError on problems
 * */
class DynamicText : public SafeShared {

  public:
    typedef std::string String;

    typedef SumTree::FlexibleText<String> Text;

		/**
		 * @param initialState the state which will be used as zero-state.
		 * */
		DynamicText( const VectorTimestamp& initialState = VectorTimestamp(), const String& initialText = String() );

    /**Does the replacement precisely, considering the timestamp. Jumps back to the current state after inserting.
    * @param stamp The timestamp in whose context the replacement has to be done
    * @param m_realReplaceents Stores all the real done replacements. not used. */
    bool insert( const VectorTimestamp& stamp, const SimpleReplacement& replacement );

		/**Inserts the replacement in the current context, using the given index, and returning the resulting identifying timestamp.
		 * */
		VectorTimestamp insert( uint index, const SimpleReplacement& replacement );

    /**The current state the text is in
     * */
    VectorTimestamp state() const;

    /**The highest state possible
     * */
    VectorTimestamp tailState() const;

		VectorTimestamp initialState() const;

		const Text& text() const;

    const String& initialText() const;
		/**
		 * @param state The state which the text should have after the change. The default-parameter changes to the current state.
		 * If force is enabled, conflicts will be ignored instead of stopping.(The resulting text may be damaged)
		 */
		bool changeState( const VectorTimestamp& state = VectorTimestamp(), bool force = false );

    /**
     * Removes everything above the current state.
     * */
    void cut();

    /**rewinds to a position that is smaller/same than state in every index
		 * @param state the state of the document will be smaller than that state
		 * */
    bool rewind( const VectorTimestamp& state );
    ///The given state must be bigger/same than the current state in every stamp
    bool advance( const VectorTimestamp& state );

		void dump() const;

		/** @return the first replacement with the given primary index, or zero */
		ReplacementPointer first( uint index ) const;
		
		/** @return the first replacement with the given primary index, that is currently not applied  to the text(its primary stamp is higher than the current state's stamp on that index), or zero */
		ReplacementPointer firstUnapplied( uint index ) const;

		/** @return the last replacement with the given primary index, or zero */
		ReplacementPointer last( uint index ) const;

		/** @return the last replacement with the given primary index, that is currently applied  to the text(its primary stamp is same as the current state's stamp on that index), or zero */
		ReplacementPointer lastApplied( uint index ) const;
		
		/**
		 * @return the highest stamp with that index
		 * */
		Timestamp highestStamp( uint index ) const;
		
		/**Returns the internal replacement-objects that can be used for example to disable
		 * a single replacement. This is dangerous.
		 * @param primaryIndex index(user?)
		 * @param stamp timestamp for that index
		 * */
		ReplacementPointer replacement( uint primaryIndex, Timestamp stamp );

		template<class Archive>
		void load( Archive& arch,  const uint version ) {
			standardSerialize( arch, version );
			m_allReplacements.clear();
			for( uint a = 0; a < m_applied.size(); a++ ) {
				ReplacementPointer p = m_applied[0].first;
				if( p == 0 ) p = m_unApplied[0].first;
				while( p != 0 ) {
					hashReplacement( p );
					p = p->next();
				}
			}
		}
		
		template<class Archive>
		void save( Archive& arch,  const uint version ) const {
			const_cast<DynamicText*>( this ) ->standardSerialize( arch, version );
		}

		BOOST_SERIALIZATION_SPLIT_MEMBER();

	private:
		template<class Archive>
		void standardSerialize( Archive& arch, const uint ) {
			arch & m_text;
			arch & m_state;
			arch & m_initialState;
			arch & m_currentOffset;
			arch & m_dummy;
			arch & m_applied;
			arch & m_unApplied;
      arch & m_initialText;
		}
    
    VectorTimestamp insertInternal( uint index, const SimpleReplacement& replacement );

    bool changeStateInternal( const VectorTimestamp& state = VectorTimestamp(), bool force = false );

    bool rewindInternal( const VectorTimestamp& state );
    ///The given state must be bigger/same than the current state in every stamp
    bool advanceInternal( const VectorTimestamp& state );

    ///The following are notification-functions that can be overriden by derived classes
    virtual void notifyInserted( const ReplacementPointer& rep );
    virtual void notifyStateChanged();
        
		///@todo check which keys are really necessary
    BIND_LIST_3( ReplacementKeys, WeakReplacementPointer, VectorTimestamp, ReplacementId );
    typedef Utils::CrossMap< WeakReplacementPointer, ReplacementKeys > ReplacementSet;
    ReplacementSet m_allReplacements; 
    friend class Advancer;

    ///This contains the chains from the beginning until(including) the last item that is applied.
    std::vector<ReplacementChain> m_applied;
    ///This contains the chains that were temporarily unapplied
    std::vector<ReplacementChain> m_unApplied;

    typedef __gnu_cxx::hash_map< OffsetRequest, OffsetMap > OffsetCache;
    OffsetCache m_offsetCache;

    Text m_text;
    String m_initialText;
    VectorTimestamp m_state;
		VectorTimestamp m_initialState;
    OffsetMap m_currentOffset; ///This offset-map represents all replacements that should have been applied(according to m_state), but are not.

    void hashReplacement( const ReplacementPointer& rep );
    void unHashReplacement( const ReplacementPointer& rep );

		///Returns the backwards-offset from the current position to the searched one(the searched must be legal, and smaller than current state). If position is set, offsets that do not affect the given position in the from-space are not included.
		OffsetMap offset( VectorTimestamp from, VectorTimestamp to, int position = -1 );
    
    ReplacementPointer m_dummy;

				///Changes the state to the newest one available
		bool toTail();
};

typedef SharedPtr<DynamicText, BoostSerialization> DynamicTextPointer;
#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
