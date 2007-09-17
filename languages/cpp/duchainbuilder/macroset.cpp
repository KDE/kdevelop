/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "macroset.h"
#include <QDataStream>

#include "hashedstring.h"

using namespace Cpp;
using namespace KDevelop;

void MacroSet::read( QDataStream& stream )  {
  Q_ASSERT(0); ///@todo reimplement
  Q_UNUSED(stream)
/*    m_idHashValid = false;
    m_valueHashValid = false;
    int cnt;
    stream >> cnt;
    m_usedMacros.clear();
    rpp::pp_macro m;
    for( int a = 0; a < cnt; a++ ) {
        m.read( stream );
        m_usedMacros.insert( m );
    }*/
}

void MacroSet::write( QDataStream& stream ) const {
  Q_ASSERT(0); ///@todo reimplement
  Q_UNUSED(stream)
/*    stream << int( m_usedMacros.size() );
    for( Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it ) {
        (*it).write( stream );
    }*/
}

void MacroSet::addMacro( const rpp::pp_macro& macro ) {
  std::pair<Macros::const_iterator, bool> r = m_usedMacros.insert( macro );
  if( !r.second ) {
    //Make sure the macro added later will be used
    m_usedMacros.erase( r.first );
    m_usedMacros.insert( macro );
  }

  m_idHashValid = m_valueHashValid = false;
}

void MacroSet::merge( const MacroSet& macros ) {
  Macros m = macros.m_usedMacros; //Swap is needed so the merged macros take precedence
  m.insert( m_usedMacros.begin(), m_usedMacros.end() );
  m_usedMacros = m;
  m_idHashValid = m_valueHashValid = false;
}


size_t MacroSet::idHash() const {
    if( !m_idHashValid ) computeHash();
    return m_idHash;
}

size_t MacroSet::valueHash() const {
    if( !m_valueHashValid ) computeHash();
    return m_valueHash;
}

int MacroSet::size() const {
  return m_usedMacros.size();
}

void MacroSet::computeHash() const {
    m_idHash = 0;
    m_valueHash = 0;
    int mult = 1;
    for( Macros::const_iterator it = m_usedMacros.begin(); it != m_usedMacros.end(); ++it ) {
        mult *= 31;
        m_idHash += (*it).idHash();
        m_valueHash += (*it).valueHash();
    }
}

bool MacroSet::hasMacro( const QString& name ) const {
    Macros::const_iterator it = m_usedMacros.find( rpp::pp_macro( name ) );
    if( it != m_usedMacros.end() ) {
        return true;
    } else {
        return false;
    }
}

bool MacroSet::hasMacro( const HashedString& name ) const {
    Macros::const_iterator it = m_usedMacros.find( rpp::pp_macro( name.str() ) );
    if( it != m_usedMacros.end() ) {
        return true;
    } else {
        return false;
    }
}

rpp::pp_macro MacroSet::macro( const HashedString& name ) const {
    Macros::const_iterator it = m_usedMacros.find( rpp::pp_macro( name ) );

    if( it != m_usedMacros.end() ) {
        return *it;
    } else {
        return rpp::pp_macro();
    }
}

