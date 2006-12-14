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

#include "cachemanager.h"
#include <kdebug.h>

void CacheNode::access() const {
  m_manager->access( this );
}

void CacheManager::remove( const CacheNode* node ) {
  m_set.erase( node );
}

void CacheManager::add( const CacheNode* node ) {
  m_set.insert( node );
}

CacheNode::CacheNode( Manager* manager ) : m_manager( manager ), m_value(manager->currentMax()) { //initialize m_value with the current maximum, so the new node has a chance even in a cache full of high-rated nodes
  m_manager->add( this  );
}
  
CacheNode::~CacheNode() {
  m_manager->remove( this );
};

void CacheManager::restart( uint normalizeby ) {
  m_currentFrame = 1;
  m_currentMax = 1;
  SetType oldSet = m_set;
  m_set = SetType();
  for( SetType::iterator it = oldSet.begin(); it != oldSet.end(); ++it ) {
    int newValue = (*it)->value() / ( normalizeby / 1000 );
    if( newValue > m_currentMax ) m_currentMax = newValue;
    (*it)->setValue( newValue ); ///This way not all information is discarded
    m_set.insert( *it );
  }
}

void CacheManager::access( const CacheNode* node ) {
  static const int limit = (std::numeric_limits<uint>::max() / 3)*2;
  m_set.erase( node );
  node->setValue( m_currentMax+1 );
  m_set.insert( node );
  if( node->value() > m_currentMax )
    m_currentMax = node->value();
  if( node->value() > limit )
    restart( node->value() );
}

void CacheManager::setMaxNodes ( int maxNodes ) {
  m_maxNodes = maxNodes;
  increaseFrame();
}

void CacheManager::increaseFrame() {
  m_currentFrame ++;
  if( m_set.size() > m_maxNodes ) {
    //kdDebug( 9007 ) << "CacheManager: Have " << m_set.size() << " nodes, maximum is " << m_maxNodes << ", erasing." << endl;
    int mustErase = m_set.size() - m_maxNodes;
    while( !m_set.empty() && mustErase != 0 ) {
      --mustErase;
      SetType::iterator it = m_set.begin();
      erase( *it );
    }
    //kdDebug( 9007 ) << "CacheManager: Have " << m_set.size() << " nodes after erasing." << endl;
  }
}

void CacheManager::removeLowerHalf() {
  int maxNodes = m_maxNodes;
  setMaxNodes( m_set.size() / 2 );
  setMaxNodes( maxNodes );
}

void CacheManager::saveMemory() {
  removeLowerHalf();
}