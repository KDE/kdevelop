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

#define NOCATCH

#include "network/serialization.h"
#include "dynamictext.h"
#include "verify.h"

#ifndef NOCATCH
typedef DynamicTextError CatchedDynamicTextError;
#else
typedef DynamicTextErrorDummy CatchedDynamicTextError;
#endif


CROSSMAP_KEY_EXTRACTOR( WeakReplacementPointer, WeakReplacementPointer, 0, value )
CROSSMAP_KEY_EXTRACTOR( WeakReplacementPointer, VectorTimestamp, 0, value->vectorStamp() )
CROSSMAP_KEY_EXTRACTOR( WeakReplacementPointer, ReplacementId, 0, ReplacementId( value->primaryIndex(), value->primaryStamp() ) )

namespace __gnu_cxx {
template <>
struct hash<ReplacementId> {
  size_t operator() ( const ReplacementId& x ) {
    return x.primaryIndex * 17 + x.stamp;
  }
};
}

DynamicTextError::DynamicTextError( const String& fun, int l, const String& f, const String& exp, const String& realExp ) : function( fun ), line( l ), file( f ), expression( exp ), realExpression( realExp ) {
  cout << what() << endl;
}
String DynamicTextError::what() const {
  std::ostringstream os;
  os << "DynamicTextError in " << function << ", " << file << ":" << line;
  if ( !expression.empty() )
    os << ", expression \"" << expression << "\" failed";
  if ( !realExpression.empty() )
    os << ", values: " << realExpression;
  return os.str();
}

OffsetMap Replacement::offset( const OffsetMap& ofs ) {
  return OffsetMap( ofs( m_replacement.m_position ), int( m_replacement.m_newText.length() ) - int( m_replacement.m_oldText.length() ) );
}

void Replacement::setEnabled( bool e ) {
  m_enabled = e;
}


template <class TextType>
bool Replacement::apply( TextType& text, const OffsetMap& outerOffset, OffsetMap& contextOffset ) {
  if ( m_enabled ) {
    int pos = outerOffset( contextOffset( m_replacement.m_position ) );
    DYN_VERIFY_SMALLERSAME( 0, pos );
    DYN_VERIFY_SMALLERSAME( (int)pos, (int)text.length() );
    DYN_VERIFY_SAME( text.substr( pos, m_replacement.m_oldText.length() ), m_replacement.m_oldText );

    text.replace( pos, m_replacement.m_oldText.length(), m_replacement.m_newText );
  } else {
    contextOffset %= ~offset( outerOffset );
  }

  return true;
}

template <class TextType>
bool Replacement::unApply( TextType& text, const OffsetMap& outerOffset, OffsetMap& contextOffset ) {
  if ( m_enabled ) {
    int pos = outerOffset( contextOffset( m_replacement.m_position ) );
    DYN_VERIFY_SMALLERSAME( 0, pos );
    DYN_VERIFY_SMALLERSAME( (int)pos, (int)text.length() );
    DYN_VERIFY_SAME( text.substr( pos, m_replacement.m_newText.length() ), m_replacement.m_newText );

    text.replace( pos, m_replacement.m_newText.length(), m_replacement.m_oldText );
  } else {
    contextOffset %= offset( outerOffset );
  }

  return true;
}

void Replacement::setNext( ReplacementPointer next ) {
  m_next = next;
  /*  if ( m_next ) {
  m_next->m_prev = next;
  DYN_VERIFY( !next->m_next );
  next->m_next = m_next;
  }
  m_next = next;
  m_next->m_prev = this;*/
}

void Replacement::setPrev( ReplacementPointer prev ) {
  m_prev = prev;
  /*  if ( m_prev ) {
  m_prev->m_next = prev;
  }
  m_prev = prev;
  m_prev->m_next = this;*/
}

bool Replacement::operator < ( const Replacement& rhs ) const throw( DynamicTextError ) {
  DYN_VERIFY_SAME( rhs.primaryIndex(), primaryIndex() );
  return primaryStamp() < rhs.primaryStamp();
}

bool Replacement::operator <= ( const Replacement& rhs ) const throw( DynamicTextError ) {
  DYN_VERIFY_SAME( rhs.primaryIndex(), primaryIndex() );
  return primaryStamp() <= rhs.primaryStamp();
}

uint Replacement::primaryIndex() const {
  return m_state.primaryIndex();
}

const VectorTimestamp& Replacement::vectorStamp() const {
  return m_state;
}

Timestamp Replacement::primaryStamp() const {
  return m_state.primaryStamp();
}

ReplacementPointer Replacement::next() const {
  return m_next;
}

ReplacementPointer Replacement::prev() const {
  return m_prev;
}

bool Replacement::enabled() const {
  return m_enabled;
}

#ifdef USE_LINKS
void Replacement::updateLink( uint index, ReplacementPointer link ) {
  if ( m_links.size() <= index )
    m_links.resize( index + 1 );
  if ( !m_links[ index ] || m_links[ index ] ->primaryStamp() > link->primaryStamp() )
    m_links[ index ] = link;
}
#endif

const DynamicText::Text& DynamicText::text() const {
  return m_text;
}

const String& DynamicText::initialText() const {
  return m_initialText;
}

bool DynamicText::insert( const VectorTimestamp& stamp, const SimpleReplacement& replacement ) {
  DYN_VERIFY( !m_allReplacements.values( stamp ) ); ///Make sure a same stamp has not been inserted already
  DYN_VERIFY( !stamp.isZero() );

  VectorTimestamp currentState = m_state;
  VectorTimestamp s = stamp;
  s.decrease();

  if ( s.size() > m_state.size() ) {
    m_state.resize( s.size() );
    m_unApplied.resize( s.size() );
    m_applied.resize( s.size() );
  }

  {
    ///Make sure that all wished pre-stamps are available
    for ( uint a = 0; a < stamp.size(); a++ ) {
      if ( stamp[ a ] != 0 ) {
        Timestamp max = highestStamp( a );
        if ( a == stamp.primaryIndex() ) {
          DYN_VERIFY_SAME( s[ a ], max ); ///Make sure no higher entries exist
        } else {
          DYN_VERIFY_SMALLERSAME( s[ a ], max ); ///Make sure that no non-existent states are referenced
        }

      }
    }
  }

  try {
    DYN_VERIFY( changeStateInternal( s ) );

    insertInternal( stamp.primaryIndex(), replacement );

    DYN_VERIFY( changeStateInternal( currentState ) );
  } catch ( const CatchedDynamicTextError & err ) {
    if ( m_state != currentState )
      notifyStateChanged();
    throw;
  }

  return true;
}

ReplacementPointer DynamicText::first( uint index ) const {
  if ( m_applied.size() > index && m_applied[ index ].first )
    return m_applied[ index ].first;

  if ( m_unApplied.size() > index && m_unApplied[ index ].first )
    return m_unApplied[ index ].first;

  return 0;
}

ReplacementPointer DynamicText::firstUnapplied( uint index ) const {
  if ( m_unApplied.size() > index && m_unApplied[ index ].first )
    return m_unApplied[ index ].first;

  return 0;
}

ReplacementPointer DynamicText::last( uint index ) const {
  if ( m_unApplied.size() > index && m_unApplied[ index ].last )
    return m_unApplied[ index ].last;

  if ( m_applied.size() > index && m_applied[ index ].last )
    return m_applied[ index ].last;
  return 0;
}

ReplacementPointer DynamicText::lastApplied( uint index ) const {
  if ( m_applied.size() > index && m_applied[ index ].last )
    return m_applied[ index ].last;
  return 0;
}

Timestamp DynamicText::highestStamp( uint a ) const {
  Timestamp max = 0;

  if ( m_unApplied.size() > a && m_unApplied[ a ].last )
    max = m_unApplied[ a ].last->primaryStamp();
  else if ( m_applied.size() > a && m_applied[ a ].last )
    max = m_applied[ a ].last->primaryStamp();

  if ( m_initialState[ a ] > max )
    max = m_initialState[ a ];

  return max;
}

VectorTimestamp DynamicText::insert( uint index, const SimpleReplacement& replacement ) {
  VectorTimestamp state = m_state;
  VectorTimestamp ret;
  try {
    ret = insertInternal( index, replacement );
  } catch ( const CatchedDynamicTextError & err ) {
    if ( state != m_state )
      notifyStateChanged();
    throw;
  }

  if ( state != m_state )
    notifyStateChanged();
  return ret;
}

VectorTimestamp DynamicText::insertInternal( uint index, const SimpleReplacement& replacement ) {
  VectorTimestamp stamp( index, m_state );
  DYN_VERIFY( !m_allReplacements.values( stamp ) ); ///Make sure a same stamp has not been inserted already
  DYN_VERIFY( !stamp.isZero() );

  VectorTimestamp s = stamp;
  s.decrease();

  DYN_VERIFY_SAME( s, m_state );

  if ( stamp.size() > m_state.size() || stamp.size() > m_unApplied.size() ) {
    m_state.resize( stamp.size() );
    m_unApplied.resize( stamp.size() );
    m_applied.resize( stamp.size() );
  }

  DYN_VERIFY_SAME( m_unApplied[ index ].first, 0 /*Make sure there isn't already a replacement with that primary index*/ );

  ReplacementPointer rep = new Replacement( stamp, replacement );

  DYN_VERIFY( rep->apply( m_text, OffsetMap(), m_currentOffset ) );

  if ( s.primaryStamp() != 0 ) {
    ReplacementPointer prev = this->replacement( s.primaryIndex(), s.primaryStamp() );
    if ( prev ) {  ///Since the collaboration can start in any state, we don't have to insist on this
      DYN_VERIFY_SAME( prev, m_applied[ s.primaryIndex() ].last );
      prev->setNext( rep );
      rep->setPrev( prev );
    }
  }
  if ( m_applied.size() <= s.primaryIndex() )
    m_applied.resize( s.primaryIndex() + 1 );
  m_applied[ s.primaryIndex() ].last = rep;
  if ( m_applied[ s.primaryIndex() ].first == 0 )
    m_applied[ s.primaryIndex() ].first = rep;

#ifdef USE_LINKS
  ///Set the m_links-member of all previous-stamped items
  for ( int a = 0; a < m_applied.size(); a++ ) {
    if ( a != stamp.primaryIndex() ) {
      if ( s[ a ] != 0 ) {
        ReplacementPointer p = this->replacement( a, s[ a ] );
        DYN_VERIFY( p );

        p->updateLink( stamp.primaryIndex(), rep );
      }
    }
  }
#endif

  m_state.setPrimaryIndex( stamp.primaryIndex() );
  m_state.increase();


  DYN_VERIFY_SAME( stamp, m_state );

  hashReplacement( rep );

  notifyInserted( rep );

  return stamp;
}

void DynamicText::dump() const {}

VectorTimestamp DynamicText::state() const {
  return m_state;
}

VectorTimestamp DynamicText::initialState() const {
  return m_initialState;
}

struct SafetyCounter {
  uint m_cnt;
  uint m_maxCnt;
public:

  SafetyCounter( uint max = 10000 ) : m_cnt( 0 ), m_maxCnt( max ) {}

  operator bool() {
    ++m_cnt;
    return m_cnt < m_maxCnt;
  }
};

VectorTimestamp DynamicText::tailState() const {
  VectorTimestamp target( m_state );
  for ( uint a = 0; a < m_unApplied.size(); a++ ) {
    if ( m_unApplied[ a ].last )
      target.maximize( m_unApplied[ a ].last->vectorStamp() );
  }

  return target;
}

///Changes the state to the newest one available
bool DynamicText::toTail() {
  VectorTimestamp target( m_state );
  for ( uint a = 0; a < m_unApplied.size(); a++ ) {
    if ( m_unApplied[ a ].last )
      target.maximize( m_unApplied[ a ].last->vectorStamp() );
  }

  if ( !target.isZero() )
    changeStateInternal( target );
  return true;
}

#ifdef OLDOFFSET
OffsetMap DynamicText::offset( VectorTimestamp from, VectorTimestamp to ) {
  if ( from == to )
    return OffsetMap();

  OffsetRequest request( from, to );

  OffsetCache::iterator it = m_offsetCache.find( request );

  if ( it != m_offsetCache.end() ) {
    return ( *it ).second;
  }

  bool backwards = true;
  if ( from.smallerOrSame( to ) ) {
    backwards = false;
    VectorTimestamp t = from;
    from = to;
    to = t;
  }
  DYN_VERIFY( to.smallerOrSame( from ) );

  static int depth = 0;
  depth++;

  DYN_VERIFY_SMALLER( depth, 5000 );

  if ( from == to )
    return OffsetMap();

  bool applied = true;

  uint sz = m_applied.size();

  DYN_VERIFY_SMALLERSAME( to.size(), sz );
  DYN_VERIFY_SMALLERSAME( from.size(), sz );

  std::vector<ReplacementPointer> chains( sz );
  for ( uint a = 0; a < sz; a++ ) {
    chains[ a ] = m_applied[ a ].last;
    if ( !chains[ a ] )
      chains[ a ] = m_unApplied[ a ].first;
    if ( chains[ a ] )
      DYN_VERIFY_SAME( chains[ a ] ->primaryIndex(), a );
    //if( backwards ) {
    while ( chains[ a ] && chains[ a ] ->primaryStamp() > from[ a ] )
      chains[ a ] = chains[ a ] ->prev();
    //} else {
    while ( chains[ a ] && chains[ a ] ->primaryStamp() < from[ a ] )
      chains[ a ] = chains[ a ] ->next();
    //}
    if ( chains[ a ] )
      DYN_VERIFY_SAME( chains[ a ] ->primaryStamp(), from[ a ] );
  }

  OffsetMap offset;
  VectorTimestamp currentState = from;

  SafetyCounter s( 10000 );

  bool waited = false;

  while ( applied || waited ) {
    DYN_VERIFY( s );

    bool force = false;
    if ( waited && !applied ) { ///If there is an inheritance-chain, break it by just applying the first one.
      force = true;
      waited = false;
    }
    applied = false;
    waited = false;

    //Apply all replacements that can be applied to the current state, and need to be applied.
    for ( uint a = 0; a < chains.size(); a++ ) {
      ReplacementPointer& r( chains[ a ] );
      if ( !r )
        continue;
      DYN_VERIFY( r );
      const VectorTimestamp& s( r->vectorStamp() );

      if ( s.primaryStamp() > to[ s.primaryIndex() ] ) {

        if ( !force ) {
          ///Find out if there's another one that can be unapplied, and that should be unapplied before this one. If there is one, wait.
          bool wait = false;
          for ( uint b = 0; b < chains.size(); b++ ) {
            if ( b == a )
              continue;

            ReplacementPointer& r2( chains[ b ] );
            if ( !r2 )
              continue;
            const VectorTimestamp& s2( r2->vectorStamp() );

            if ( s2.primaryStamp() > to[ s2.primaryIndex() ] && currentState[ s2.primaryIndex() ] >= s2.primaryStamp() ) {
              ///The other replacement has to be unapplied
              if ( s[ b ] < s2.primaryStamp() || s2[ a ] >= s.primaryStamp() ) {
                ///The other replacement should be unapplied before this one
                wait = true;
                break;
              }
            }
          }

          if ( wait ) {
            waited = true;
            continue;
          }
        } else {
          force = false;
        }

        OffsetMap selfOffset;

        /*VectorTimestamp prev = currentState;
        prev.setPrimaryIndex( a );
        if ( prev.primaryStamp() != 0 ) {
          prev.decrease();
          VectorTimestamp selfPrev = s;
          DYN_VERIFY( selfPrev.primaryStamp() != 0 );
          selfPrev.decrease();
          DYN_VERIFY( selfPrev.primaryStamp() >= 0 );

          selfOffset = this->offset( prev, selfPrev );
        }*/

        if ( backwards ) {
          offset = offset % ( chains[ a ] ->offset( selfOffset ) );
        } else {
          offset = offset % ( ~chains[ a ] ->offset( selfOffset ) );
        }
        /*if ( backwards ) {
          offset = ( chains[ a ] ->offset( selfOffset ) ) % offset;
        } else {
          offset = ( ~chains[ a ] ->offset( selfOffset ) ) % offset;
        }*/

        r = r->prev();

        currentState.setPrimaryIndex( s.primaryIndex() );
        currentState.decrease();
        DYN_VERIFY_SMALLERSAME( to[ s.primaryIndex() ], currentState.primaryStamp() );

        applied = true;
      }
    }
  }

  depth--;

  DYN_VERIFY_SAME( currentState, to );
  m_offsetCache.insert( OffsetCache::value_type( request, offset ) );

  return offset;
}
#else

OffsetMap DynamicText::offset( VectorTimestamp from, VectorTimestamp to, int position ) {
  if ( from == to )
    return OffsetMap();

  OffsetRequest request( from, to );

//   OffsetCache::iterator it = m_offsetCache.find( request );
//
//   if ( it != m_offsetCache.end() ) {
//     return ( *it ).second;
//   }

  uint sz = m_applied.size();

  DYN_VERIFY_SMALLERSAME( to.size(), sz );
  DYN_VERIFY_SMALLERSAME( from.size(), sz );

  std::vector<ReplacementPointer> chains( sz );
  std::vector<ReplacementPointer> next( sz ); ///Contains the next replacement into the direction from -> to
  for ( uint a = 0; a < sz; a++ ) {
    chains[ a ] = replacement( a, from[ a ] );
    if ( chains[ a ] ) {
      DYN_VERIFY_SAME( chains[ a ] ->primaryStamp(), from[ a ] );
    }
    if ( to[ a ] < from[ a ] )
      next[ a ] = replacement( a, from[ a ] - 1 );
    else if ( to[ a ] > from[ a ] )
      next[ a ] = replacement( a, from[ a ] + 1 );
  }

  //VectorTimestamp currentState = from;

  SafetyCounter s( 10000 );

  bool waited = false;
  bool first = true;

  while ( waited || first ) {
    DYN_VERIFY( s );
    first = false;

    bool force = false;
    if ( waited ) { ///If there is an inheritance-chain, break it by just applying the first one.
      force = true;
      waited = false;
    }

    for ( uint a = 0; a < chains.size(); a++ ) {
      if ( from[ a ] != to[ a ] ) {
        bool unapply = from[ a ] > to[ a ];
        if ( !force ) {
          ///Find out if there's another one that can be unapplied, and that should be unapplied before this one. If there is one, wait.
          bool wait = false;
          for ( uint b = 0; b < chains.size(); b++ ) {
            if ( b == a )
              continue;

            if ( !chains[ b ] )
              continue;

            const VectorTimestamp& s2( chains[ b ] ->vectorStamp() );
            if ( from[ b ] >= s2[ b ] && s2[ b ] > to[ b ] ) {
              DYN_VERIFY( chains[ a ] );
              ///The other replacement has to be unapplied
              if ( unapply ) {
                ///Should the replacement b be unapplied before a is unapplied?
                if ( chains[ a ] ->vectorStamp() [ b ] < s2[ b ] || s2[ a ] >= from[ a ] ) {
                  ///a expects b not to be applied || b expects a to be applied
                  wait = true;
                  break;
                }
              } else {
                ///Should the replacement b be unapplied before a is applied?
                DYN_VERIFY( next[ a ] );
                if ( next[ a ] ->vectorStamp() [ b ] < s2[ b ] || s2[ a ] < from[ a ] + 1 ) {
                  ///next a expects b not to be applied || b expects next a not to be applied
                  wait = true;
                  break;
                }
              }
            }

            if ( from[ b ] < s2[ b ] && s2[ b ] <= to[ b ] ) {
              DYN_VERIFY( next[ b ] );

              if ( unapply ) {
                ///Should the replacement b be applied before a is unapplied?
                if ( chains[ a ] ->vectorStamp() [ b ] >= next[ b ] ->primaryStamp() || next[ b ] ->vectorStamp() [ a ] >= from[ a ] ) {
                  ///a expects next b to be applied || next b expects a to be applied
                  wait = true;
                  break;
                }
              } else {
                ///Should the replacement b be applied before a is applied?
                DYN_VERIFY( next[ a ] );
                if ( next[ a ] ->vectorStamp() [ b ] > next[ b ] ->primaryStamp() || next[ b ] ->vectorStamp() [ a ] < from[ a ] + 1 ) {
                  ///next a expects next b to be applied || next b expects next a not to be applied
                  wait = true;
                  break;
                }
              }
            }
          }

          if ( wait ) {
            waited = true;
            continue;
          }
        } else {
          force = false;
        }

        VectorTimestamp preNextStamp;
        VectorTimestamp nextStamp;
        VectorTimestamp intermediate;
        OffsetMap ret;

        if ( to[ a ] < from[ a ] ) {
          VectorTimestamp n = from;
          n.setPrimaryIndex( a );
          n.decrease();

          VectorTimestamp prev = chains[a]->vectorStamp();
          prev.decrease();

          ///The position-checking has to be done to avoid endless recursion
          OffsetMap innerOffset = ~offset( prev, n, chains[a]->replacement().m_position );
          ret = offset( n, to );
          if( position != -1 && position < innerOffset( chains[a]->replacement().m_position ) )
              return ret;
          ret = chains[a]->offset( innerOffset ) % ret;
        } else {
          VectorTimestamp n = from;
          n.setPrimaryIndex( a );
          n.increase();
          VectorTimestamp prev = next[a]->vectorStamp();
          prev.decrease();
          OffsetMap innerOffset = ~offset( prev, from, next[a]->replacement().m_position );
          ret = offset( n, to );
          if( position != -1 && position < innerOffset( next[a]->replacement().m_position ) )
            return ret;
          ret = ~next[a]->offset( innerOffset ) % ret;
        }

        /// Now: ret = offset( preNextStamp, nextStamp )

          /**chains[a]->offset() = of( v( chains[a].state ) -> chains[a].state )
           * with v(chains[a].state) = pre-state of chains[a].state (stamp at position primaryStamp decrease by one)
           *
           * Simple case: of( from -> to )(x) = of( next -> to )( ( of( from -> next )(x) )
           *
           * Problem: What if the current state does not exactly match the state created by the item to remove?
           * of( chains[a].prevStamp -> chains[a].stamp ) = chains[a]->offset()
           * of[ from -> to ](x) = of[chains[a].stamp -> to ]( of[ chains[a].prevStamp -> chains[a].stamp ] ( of[ from -> chains[a].prevStamp ](x) ) )
           **/

        m_offsetCache.insert( OffsetCache::value_type( request, ret ) );
        return ret;
      }
    }
  }

  DYN_VERIFY( 0 );
}
#endif


bool DynamicText::rewindInternal( const VectorTimestamp& state ) {
  SafetyCounter s( 10000 );

  bool applied = true;
  bool waited = false;

  while ( applied || waited ) {
    DYN_VERIFY( s );

    bool force = false;
    if ( waited && !applied ) { ///If there is an inheritance-chain, break it by just applying the first one.
      force = true;
      waited = false;
    }
    applied = false;
    waited = false;


    for ( uint a = 0; a < m_applied.size(); a++ ) {
      if ( state[ a ] > m_state[ a ] )
        continue;
      ReplacementChain& r( m_applied[ a ] );
      if ( !r.first && !r.last )
        continue;
      DYN_VERIFY( r.last );
      const VectorTimestamp& s( r.last->vectorStamp() );

      if ( s.primaryStamp() > state[ s.primaryIndex() ] ) {

        if ( !force ) {
          ///Find out if there's another one that can be unapplied, and that should be unapplied before this one. If there is one, wait.
          bool wait = false;
          for ( uint b = 0; b < m_applied.size(); b++ ) {
            if ( b == a )
              continue;

            ReplacementChain& r2( m_applied[ b ] );
            if ( !r2.first && !r2.last )
              continue;
            const VectorTimestamp& s2( r2.last->vectorStamp() );

            if ( s2.primaryStamp() > state[ s2.primaryIndex() ] && m_state[ s2.primaryIndex() ] >= s2.primaryStamp() ) {
              ///The other replacement has to be unapplied
              if ( s[ b ] < s2.primaryStamp() || s2[ a ] >= s.primaryStamp() ) {
                ///The other replacement should be unapplied before this one
                wait = true;
                break;
              }
            }
          }

          if ( wait ) {
            waited = true;
            continue;
          }
        } else {
          force = false;
        }

        //It can be unapplied now
        /*VectorTimestamp prev( s );
        prev.decrease();*/


        VectorTimestamp prevCurrent = m_state;
        prevCurrent.setPrimaryIndex( a );
        DYN_VERIFY_NOTSAME( prevCurrent.primaryStamp(), 0 );

        //prevCurrent.decrease();

        VectorTimestamp prev( s );
        //prev.decrease();
        OffsetMap prevOffset;
        OffsetMap offset;
        //if ( !prev.isZero() )
        offset = prevOffset = this->offset( prevCurrent, prev );

        DYN_VERIFY( r.last->unApply( m_text, offset, m_currentOffset ) );

        /*OffsetMap relOffset = ~( prevOffset % (~offset) );
        m_currentOffset %= relOffset;*/

        m_state.setPrimaryIndex( s.primaryIndex() );
        m_state.decrease();

        applied = true;

        if ( m_unApplied.size() <= a )
          m_unApplied.resize( a + 1 );

        if ( m_unApplied[ a ].first ) {
          DYN_VERIFY_SAME( r.last, m_unApplied[ a ].first->prev() );
          m_unApplied[ a ].first = r.last;
        } else {
          m_unApplied[ a ].first = m_unApplied[ a ].last = r.last;
        }
        if ( r.last->prev() ) {
          r.last = r.last->prev();
        } else {
          r.first = r.last = 0;
        }
      }
    }
  }

  DYN_VERIFY( m_state.smallerOrSame( state ) );

  return true;
}
/*
// ReplacementPointer DynamicText::findBound( const VectorTimestamp& stamp ) {
// }

OffsetMap DynamicText::backwardsOffset( const VectorTimestamp& to ) {
DYN_VERIFY( to.smallerOrSame( m_state ) );
DYN_VERIFY( m_allReplacements.iterator( to ) );

bool applied = true;

int sz = m_applied.size();

DYN_VERIFY_SMALLERSAME( to.size(), sz );
DYN_VERIFY_SMALLERSAME( m_state.size(), sz );
//DYN_VERIFY( sz >= to.size() && sz >= m_state.size() );

std::vector<ReplacementPointer> chains( sz );
for ( int a = 0; a < sz; a++ ) {
  chains[ a ] = m_applied[ a ].last;
  if ( chains[ a ] )
    DYN_VERIFY( chains[ a ] ->primaryIndex() == a );
}

OffsetMap offset = m_currentOffset;
VectorTimestamp currentState = m_state;

SafetyCounter s( 10000 );

while ( applied ) {
  DYN_VERIFY( s );
  applied = false;
  for ( int a = 0; a < sz; a++ ) {
    if ( !chains[ a ] )
      continue;
    DYN_VERIFY ( to[ a ] <= currentState[ a ] );
    DYN_VERIFY( chains[ a ] );

    VectorTimestamp s = chains[ a ] ->vectorStamp();
    //s.decrease();

    if ( !( currentState == to ) && s == currentState ) {
      offset = offset % ( ~chains[ a ] ->offset() );
      applied = true;
      chains[ a ] = chains[ a ] ->prev();
      s.decrease();
      currentState = s;
    }
  }
}

DYN_VERIFY( currentState == to );
return offset;
}*/

bool DynamicText::advanceInternal( const VectorTimestamp& state ) {
  DYN_VERIFY( m_state.smallerOrSame( state ) );
  if ( state.isZero() )
    return true;

  uint sz = m_applied.size();
  /*
  DYN_VERIFY_SMALLERSAME( sz, state.size() );
  DYN_VERIFY_SMALLERSAME( sz, m_state.size() );*/

  if ( m_unApplied.size() < m_applied.size() )
    m_unApplied.resize( m_applied.size() );

  bool applied = true;
  bool waited = false;

  SafetyCounter s( 10000 );

  while ( applied || waited ) {
    DYN_VERIFY( s );

    bool force = false;
    if ( waited && !applied ) { ///If there is an inheritance-chain, break it by just applying the first one.
      force = true;
      waited = false;
    }
    applied = false;
    waited = false;

    //Apply all replacements that can be applied to the current state, and need to be applied.
    for ( uint a = 0; a < sz; a++ ) {
      DYN_VERIFY_SMALLERSAME ( m_state[ a ], state[ a ] );
      ReplacementChain& r( m_unApplied[ a ] );
      if ( !r.first && !r.last )
        continue;
      DYN_VERIFY( r.first );
      const VectorTimestamp& s( r.first->vectorStamp() );

      if ( s.primaryStamp() <= state[ s.primaryIndex() ] ) {

        if ( !force ) {
          ///Find out if there's another one that can be applied, and that should be applied before this one. If there is one, wait.
          bool wait = false;
          for ( uint b = 0; b < sz; b++ ) {
            if ( b == a )
              continue;

            ReplacementChain& r2( m_unApplied[ b ] );
            if ( !r2.first && !r2.last )
              continue;
            const VectorTimestamp& s2( r2.first->vectorStamp() );

            if ( s2.primaryStamp() <= state[ s2.primaryIndex() ] && m_state[ s2.primaryIndex() ] < s2.primaryStamp() ) {
              ///The other replacement has to be applied
              if ( s[ b ] >= s2.primaryStamp() || s2[ a ] < s.primaryStamp() ) {
                ///The other replacement should be applied before this one
                wait = true;
                break;
              }
            }
          }

          if ( wait ) {
            waited = true;
            continue;
          }
        } else {
          force = false;
        }

        //It can be applied now
        VectorTimestamp prev( s );
        prev.decrease();
        OffsetMap prevOffset;
        OffsetMap offset;
        if ( !prev.isZero() )
          offset = prevOffset = this->offset( m_state, prev );

        DYN_VERIFY( r.first->apply( m_text, offset, m_currentOffset ) );

        /*OffsetMap relOffset = ~( prevOffset % (~offset) );
        m_currentOffset %= relOffset;*/

        m_state.setPrimaryIndex( s.primaryIndex() );
        m_state.increase();

        applied = true;
        if ( m_applied[ a ].last ) {
          DYN_VERIFY_SAME( m_applied[ a ].last, r.first->prev() );
          m_applied[ a ].last = r.first;
        } else {
          m_applied[ a ].last = m_applied[ a ].first = r.first;
        }
        if ( r.first->next() ) {
          r.first = r.first->next();
        } else {
          r.last = r.first = 0;
        }
      }
    }
  }

  DYN_VERIFY_SAME( m_state, state );
  return m_state == state;
}

bool DynamicText::rewind( const VectorTimestamp& state ) {
  VectorTimestamp v = m_state;
  try {
    rewindInternal( state );
  } catch ( const CatchedDynamicTextError & err ) {
    if ( v != m_state )
      notifyStateChanged();
    throw;
  }


  if ( v != m_state )
    notifyStateChanged();
  return true;
}

///The given state must be bigger/same than the current state in every stamp
bool DynamicText::advance( const VectorTimestamp& state ) {
  VectorTimestamp v = m_state;
  try {
    advanceInternal( state );
  } catch ( const CatchedDynamicTextError & err ) {

    if ( v != m_state )
      notifyStateChanged();
    throw;
  }

  if ( v != m_state )
    notifyStateChanged();
  return true;
}

bool DynamicText::changeState( const VectorTimestamp& state, bool force ) {
  VectorTimestamp v = m_state;
  bool ret;
  try {
    ret = changeStateInternal( state, force );
  } catch ( const CatchedDynamicTextError & err ) {
    if ( v != m_state )
      notifyStateChanged();
    throw;
  }

  if ( v != m_state )
    notifyStateChanged();
  return ret;
}

bool DynamicText::changeStateInternal( const VectorTimestamp& state, bool /*force*/ ) {
  if ( state.isZero() )
    return toTail();
  if ( state == m_state )
    return true;
  ///First some verification

  for ( uint a = 0; a < m_applied.size(); a++ ) {
    if ( m_applied[ a ].last == 0 )
      continue;
    DYN_VERIFY_SAME( m_applied[ a ].last->primaryIndex(), a );
    DYN_VERIFY_SAME( m_applied[ a ].last->primaryStamp(), m_state[ a ] );
  }

  DYN_VERIFY( rewindInternal( state ) );

  DYN_VERIFY( advanceInternal( state ) );


  return true;
}

void DynamicText::cut() {
  for ( uint a = 0; a < m_applied.size(); a++ ) {
    if ( m_applied[ a ].last )
      m_applied[ a ].last->setNext( 0 );
  }
  for ( uint a = 0; a < m_unApplied.size(); a++ ) {
    m_unApplied[ a ].first = 0;
    m_unApplied[ a ].last = 0;
  }
}

DynamicText::DynamicText( const VectorTimestamp& initialState, const String& initialText ) : m_text( initialText ), m_initialText( initialText ), m_state( initialState ), m_initialState( initialState ) {
  m_dummy = new Replacement();
}

ReplacementPointer DynamicText::replacement( uint primaryIndex, Timestamp stamp ) {
  ReplacementSet::Iterator it = m_allReplacements.values( ReplacementId( primaryIndex, stamp ) );
  if ( it && *it ) {
    return ( *it ).get();
  } else {
    return 0;
  }
}

void DynamicText::hashReplacement( const ReplacementPointer& rep ) {
  DYN_VERIFY( !m_allReplacements.find( rep ) );
  m_allReplacements.insert( rep );
}

void DynamicText::unHashReplacement( const ReplacementPointer& rep ) {
  m_allReplacements.remove( rep );
}

void DynamicText::notifyInserted( const ReplacementPointer& /*rep*/ ) {}

void DynamicText::notifyStateChanged() {}

BOOST_CLASS_EXPORT_GUID( DynamicText, "DynamicText" )

template bool Replacement::unApply( DynamicText::Text& text, const OffsetMap& outerOffset, OffsetMap& contextOffset );
template bool Replacement::apply( DynamicText::Text& text, const OffsetMap& outerOffset, OffsetMap& contextOffset );

///For testing
template bool Replacement::unApply( std::string& text, const OffsetMap& outerOffset, OffsetMap& contextOffset );
template bool Replacement::apply( std::string& text, const OffsetMap& outerOffset, OffsetMap& contextOffset );

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
