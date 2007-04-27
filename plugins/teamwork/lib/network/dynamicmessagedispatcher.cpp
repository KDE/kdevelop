/***************************************************************************
copyright            : (C) 2007 by David Nolden
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


#include "dynamicmessagedispatcher.h"
#include <map>
#include <list>


namespace Teamwork {

DynamicMessageDispatcher::DynamicMessageDispatcher( MessageTypeSet& messageTypes ) : m_messageTypes(messageTypes) {
  
}
  
DynamicMessageDispatcher::~DynamicMessageDispatcher() {
  for( DelivererList::iterator it = m_deliverers.begin(); it != m_deliverers.end(); ++it )
    delete *it;
}

void DynamicMessageDispatcher::addDeliverer( MessageDelivererBase* d ) {
  m_deliverers.push_back( d );
}

int DynamicMessageDispatcher::dispatch( const MessagePointer& msg ) {
  return operator() ( msg );
}


///@todo optimize this, a search-tree should be used, similar to the static tree.
int DynamicMessageDispatcher::operator ()( const MessagePointer& msg ) {

  //Find the best matching targets:
    typedef std::map<int, std::list<MessageDelivererBase*> > TargetMap;
    TargetMap targets; //sort the targets by the match-depth
    for( DelivererList::iterator it = m_deliverers.begin(); it != m_deliverers.end(); ++it ) {

      //Find all targets that match
      //Remove already deleted targets
      while( it != m_deliverers.end() && !(*it)->isOk() ) {
        delete *it;
        it = m_deliverers.erase( it );
      }
      if( it == m_deliverers.end() )
        break;
      
      //If we have a match, put it into the list
      if( msg.unsafe()->type().startsWith( (*it)->type() ) ) {
        int orderPosition = -(*it)->type().length(); //minus so biggest become first
        TargetMap::iterator targetIt = targets.find( orderPosition );
        if( targetIt == targets.end() )
          targetIt = targets.insert( std::make_pair( orderPosition, std::list<MessageDelivererBase*>() ) ).first;

        (*targetIt).second.push_front( *it );
      }
    }

    
    for( TargetMap::const_iterator it = targets.begin(); it != targets.end(); ++it ) {
      for( std::list<MessageDelivererBase*>::const_iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2 ) {
        int ret = (*it2)->tryDeliverMessage( msg );
        if( ret != 0 ) {
          return ret; //Message successfully delivered
        }
      }
    }

    return 0; //Message could not be delivered, or all fitting targets returned 0
  }

};

