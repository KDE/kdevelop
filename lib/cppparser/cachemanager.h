
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

#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H
#include <set>
#include <limits>

class CacheManager;


class CacheNode {
  typedef CacheManager Manager;
  public:
  CacheNode( Manager* manager );
  
  virtual ~CacheNode();

  void access() const;

  inline uint value() const {
    return m_value;
  }

  inline void setValue( const uint v ) const {
    m_value = v;
  }

  inline void addValue( const uint diff ) const {
    m_value += diff;
  }
  
  private:
    Manager* m_manager;
    mutable uint m_value; //This value stands for the priority of the node(higher is better)
};

class CacheNodeCompare {
  public:
  bool operator() ( const CacheNode* lhs, const CacheNode* rhs ) const {
    if( lhs->value() != rhs->value() )
      return lhs->value() < rhs->value();
    else
      return lhs < rhs; //To be able to identify nodes precisely
  }
};


class CacheManager {
  typedef std::set< const CacheNode*, CacheNodeCompare > SetType;
  public:
    CacheManager( int maxNodes = 1000 ) : m_currentFrame(1), m_maxNodes( maxNodes ), m_currentMax(1) {
    };

    inline int currentFrame() const {
      return m_currentFrame;
    }

    void access( const CacheNode* node );

    ///Can be used from outside to set the maximum count of nodes to keep.
    void setMaxNodes ( int maxNodes );

    void increaseFrame();

    ///Can be used on a regular basis(time-triggered) to save memory: Removes half of all triggered 
    void removeLowerHalf();

    virtual void saveMemory();

    int currentMax() const {
      return m_currentMax;
    }

    ///This triggered function should erase the given node.
    virtual void erase( const CacheNode* node ) = 0;
  private:
    void restart( uint normalizeby );
    friend class CacheNode;
    void remove( const CacheNode* node );
    void add( const CacheNode* node );
    int m_currentFrame;
    int m_maxNodes;
    int m_currentMax;
    SetType m_set;
};


#endif
