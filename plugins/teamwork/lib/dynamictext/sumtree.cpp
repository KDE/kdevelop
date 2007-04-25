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

#include <string>
#include <sstream>
#include <algorithm>
#include "sumtree.h"
#include "verify.h"
#include <limits.h>

///Must be less then SPLIT_NODES_THRESHOLD and more than JOIN_NODES_THRESHOLD
#define PREFERRED_NODES 4

#define SPLIT_NODES_THRESHOLD 8

///Must be less than PREFERRED_NODES
#define JOIN_NODES_THRESHOLD 2

namespace SumTree {

void mult( ostream& os, int count, const char* c ) {
  for ( int a = 0; a < count; a++ )
    os << c;
}

Node::Node( bool isLeaf, Node* parent ) : parent_( parent ), nodeCount_( 0 ), sum_( 0 ), indexCount_( 0 ), isLeaf_( isLeaf ) {}

Node* Node::makeOwn( Node* parent ) {
  //Node * myParent = parent_ get();
  if ( _Shared_count() == 1 ) {
    parent_ = parent;
    return this;
  }

  return ( new Node( *this ) ) ->setParent( parent );
}

Node* Node::setParent( Node* parent ) {
  //DYN_VERIFY_SAME( parent, 0 ); //reactive if WeakSharedPtr is used again

  parent_ = parent;
  return this;
}

int Map::sum( int index ) const {
  return tree_->sum( index );
}

int Map::sum() const {
  return tree_->sum();
}

bool Map::hasIndex( int index ) const {
  return tree_->hasIndex( index );
}

void Map::indexAndSum( int sum, IndexAndSum& targ ) const {
  targ.index = 0;
  targ.sum = 0;
  return tree_->indexAndSum( sum, targ );
}

int Map::index( int sum ) const {
  return tree_->index( sum );
}

Map::Map( const SumVector& summands ) {
  tree_ = new Node( false, 0 );
  int cnt = build( summands.begin(), summands.end(), tree_ );
  DYN_VERIFY_SAME( (int)cnt, (int)summands.size() );
}

Map::~Map() {}

Map::Map( const Map& rhs ) : tree_ ( new Node( *rhs.tree_ ) ) {}

Map& Map::operator = ( const Map& rhs ) {
  tree_ = new Node( *rhs.tree_ );
  return *this;
}

void Node::addNode( Node* node ) {
  nodes_.push_back( node );
  ++nodeCount_;
}

bool Node::hasIndex( int indicesNeeded ) const {
  ListType::const_iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  for ( ; it != nodes_.end() && indicesNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }
  
  if ( it != nodes_.end() ) {
    ///Ask the next one for the summ
    return ( *it ) ->hasIndex( indicesNeeded );
  }
  
  if( !isRange() )
    if( indicesNeeded == 0 ) return true;
  else
    if( indicesNeeded < indexCount_ ) return true;
  
  return false;
}

int Node::indexValue( int indicesNeeded ) const {
  ListType::const_iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  for ( ; it != nodes_.end() && indicesNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }

  if ( it != nodes_.end() ) {
    ///Ask the next one for the summ
    return ( *it ) ->indexValue( indicesNeeded );
  }

  if( !isRange() ) {
    DYN_VERIFY_SAME( indicesNeeded, 0 );
    return sum_;
  } else {
    DYN_VERIFY_SMALLER( indicesNeeded, indexCount_ );
    return sum_ / indexCount_;
  }
}

int Node::setIndexValue( int indicesNeeded, int value ) {
  ListType::iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  for ( ; it != nodes_.end() && indicesNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }

  if ( it != nodes_.end() ) {
    ///Ask the next one for the summ
    ( *it ) = ( *it ) ->makeOwn( this );
    return ( *it ) ->setIndexValue( indicesNeeded, value );
  }
  
  if( !isRange() ) {
    DYN_VERIFY_SAME( indicesNeeded, 0 );
    DYN_VERIFY( isLeaf_ );
    
    int ret = sum_;
    update( value - ret, 0 );
    return ret;
  } else {
    DYN_VERIFY_SMALLER( indicesNeeded, indexCount_ );
    ///It is a Range, split it up into two new ranges and one normal value-node which we can change.
    int eachValue = sum_ / indexCount_;
    if( eachValue == value ) return value; ///No need to split up
    Node * n = new Node( true, this );
    update( -eachValue, -1 );
    splitRange( indicesNeeded, n );
    n->update( value, 1 );
    updateStructure();
    return eachValue;
  }
}

int Node::changeIndexValue( int indicesNeeded, int value ) {
  ListType::iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  ListType::iterator end = nodes_.end();
  
  for ( ; it != end && indicesNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }

  if ( it != end ) {
    NodePointer& n( *it );
    ///Ask the next one for the summ
    n = n->makeOwn( this );
    return n->changeIndexValue( indicesNeeded, value );
  }

  if( !isRange() ) {
    DYN_VERIFY_SAME( indicesNeeded, 0 );
    DYN_VERIFY( isLeaf_ );
    
    update( value, 0 );
    return sum_;
  } else {
    DYN_VERIFY_SMALLER( indicesNeeded, indexCount_ );
    ///It is a Range, split it into two new ranges and one normal value-node which we can change.
    int eachValue = sum_ / indexCount_;
    Node * n = new Node( true, this );
    update( -eachValue, -1 );
    splitRange( indicesNeeded, n );
    n->update( value + eachValue, 1 );
    updateStructure();
    return value + eachValue;
  }
}

void Node::updateStructure() {
  if ( isLeaf_ )
    return ;
  DYN_VERIFY_SAME( (int)nodes_.size(), (int)nodeCount_ );

  int dif = nodeCount_ - PREFERRED_NODES;
  if ( dif > 0 ) {
    ///split sub-nodes into a usable count of parts
    int splitAway = dif + 1;
    ///Algorithm: find the "splitAway" nodes with the lowest index-count-sum, and export them to a container-node

    int cnt = 0; ///The count of nodes that are summed to "indices"
    int indices = 0; ///Their count of indices
    ListType::iterator first = nodes_.begin(); ///The last node of the group that sums to "indices"

    ListType::iterator best = nodes_.end();
    ListType::iterator bestLast;
    int bestIndices = INT_MAX;

    ListType::iterator it;
    for ( it = nodes_.begin(); it != nodes_.end(); ++it ) {
      if ( cnt == splitAway ) {
        if ( indices < bestIndices ) {
          best = first;
          bestLast = it;
          bestIndices = indices;
        }
        indices -= ( *first ) ->indexCount();
        ++first;
      } else {
        cnt++;
      }
      indices += ( *it ) ->indexCount();
    }
    if ( indices < bestIndices ) {
      best = first;
      bestLast = it;
      bestIndices = indices;
    }

    ///now create an own group for (best -> bestLast)
    if ( best != nodes_.end() ) {
      NodePointer branch = new Node( false, this );
      branch->nodes_.insert( branch->nodes_.begin(), best, bestLast );
      DYN_VERIFY_SAME( (int)branch->nodes_.size(), (int)cnt );
      branch->nodeCount_ = cnt;

      it = best;
      while ( it != bestLast ) {
        //(*best)->setParent( branch );
        branch->sum_ += ( *it ) ->sum_;
        branch->indexCount_ += ( *it ) ->indexCount_;
        ++it;
      }

      nodeCount_ += 1;
      nodes_.insert( best, branch );
      DYN_VERIFY_SAME( (int)nodeCount_, (int)nodes_.size() );

      nodeCount_ -= cnt;
      nodes_.erase( best, bestLast );
      DYN_VERIFY_SAME( (int)nodeCount_, (int)nodes_.size() );

    } else {
      DYN_VERIFY( 0 );
    }

  } else if ( nodeCount_ <= JOIN_NODES_THRESHOLD ) {
    if ( parent_ ) {
      ///Remove empty non-leaf-nodes
      if ( nodeCount_ == 0 ) {
        if( indexCount_ == 0 ) {
          parent_->removeChild( this );
        } else {
          ///It is a range, leave it alone.
        }
      } else {
        ///Move all own children to the parent
        for ( ListType::iterator it = parent_->nodes_.begin(); it != parent_->nodes_.end(); ++it ) {
          if ( it->data() == this ) {
            parent_->nodes_.insert( it, nodes_.begin(), nodes_.end() );
            parent_->nodeCount_ += nodeCount_;
            NodePointer n = this; ///To prevent too early deletion
            parent_->removeChild( this );
            parent_->updateStructure();
            nodeCount_ = 0;
            break;
          }
        }
      }
      ///Move child-nodes up to the parent
    } else {}
  }
}

void Node::insertIndex( int indicesNeeded, int value ) {
  ListType::iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  for ( ; it != nodes_.end() && indicesNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }

  if ( it == nodes_.end() || ( *it ) ->isLeaf() ) {

    bool range = isRange();
    
    if ( indicesNeeded <= 1 || range ) {
      Node * n = new Node( true, this );
      if( range ) {
        splitRange( indicesNeeded, n );
      } else {
        ///We have reached the end, insert the index
        nodes_.insert( it, NodePointer( n ) );
        ++nodeCount_;
      }
      n->update( value, 1 );
    } else {
      ///There are still indices missing, they could be filled with zeroes by using insertRange
      DYN_VERIFY( 0 );
    }
  } else {
    ( *it ) = ( *it ) ->makeOwn( this );
    ( *it ) ->insertIndex( indicesNeeded, value );
  }
  updateStructure();
}

void Node::splitRange( int indicesNeeded, Node* insertNode ) {
  DYN_VERIFY_SMALLERSAME( indicesNeeded, indexCount_ );
  int eachValue = sum_ / indexCount_;
  DYN_VERIFY_SAME( eachValue*indexCount_, sum_ );
  int r1Size = indicesNeeded;
  int r2Size = indexCount_ - indicesNeeded;
  
  update( -sum_, -indexCount_ ); /*This should be done more efficiently with only one single update*/
  DYN_VERIFY_SAME( sum_, 0 );
  DYN_VERIFY_SAME( indexCount_, 0 );
          ///This makes this range-node a normal node.
  
  Node * n;
//         Add the first split-range
  if( r1Size ) {
    n = new Node( false, this );
    nodes_.insert( nodes_.end(), NodePointer( n ) );
    ++nodeCount_;
    n->update( eachValue * r1Size, r1Size );
  }
  
  nodes_.insert( nodes_.end(), NodePointer( insertNode ) );
  ++nodeCount_;
  
        ///Add the second split-range
  if( r2Size ) {
    n = new Node( false, this );
    nodes_.insert( nodes_.end(), NodePointer( n ) );
    ++nodeCount_;
    n->update( eachValue * r2Size, r2Size );
  }
  
  DYN_VERIFY( !isRange() );
}

void Node::insertRange( int indicesNeeded, int size, int value ) {
  ListType::iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  for ( ; it != nodes_.end() && indicesNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }
  
  if ( it == nodes_.end() || ( *it ) ->isLeaf() ) {
    bool range = isRange();
    if ( indicesNeeded <= 1 || range ) {
      Node * n = new Node( false, this );

      if( range ) {
        /**This is a range, and needs to be made a normal inner node with a split up slave-range. */
        DYN_VERIFY( it == nodes_.end() );
        splitRange( indicesNeeded, n );
      } else {
        /**We have reached the end, insert the range*/
        nodes_.insert( it, NodePointer( n ) );
        ++nodeCount_;
      }
      
      n->update( value*size, size );
    } else {
      ///There are still indices missing, they could be filled with zeroes
      DYN_VERIFY( 0 );
    }
  } else {
    ( *it ) = ( *it ) ->makeOwn( this );
    ( *it ) ->insertRange( indicesNeeded, size, value );
  }
  updateStructure();
}

void Node::removeChild( Node* n ) {
  for ( ListType::iterator it = nodes_.begin(); it != nodes_.end(); ++it ) {
    if ( ( *it ).data() == n ) {
      nodes_.erase( it );
      --nodeCount_;
      return ;
    }
  }
}

int Node::removeIndex( int indicesNeeded ) {
  ListType::iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  for ( ; it != nodes_.end() && indicesNeeded >= 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }

  if( it != nodes_.end() && indicesNeeded == 0 && ( *it ) ->isLeaf() ) {
    ///Remove the leaf-node
    int ret = ( *it ) ->sum();
    nodes_.erase( it );
    --nodeCount_;
    update( -ret, -1 );

    updateStructure();
    return ret;
  } else if( isRange() ) {
    DYN_VERIFY_SMALLER( indicesNeeded, indexCount_ );
    int ret = sum_ / indexCount_;
    update( -ret, -1 );
    updateStructure();
    return ret;
  } else {
    DYN_VERIFY( it != nodes_.end() );
    ( *it ) = ( *it ) ->makeOwn( this );
    return ( *it ) ->removeIndex( indicesNeeded );
  }
}

int Map::setIndexValue( int index, int value ) {
  return tree_->setIndexValue( index, value );
}

int Map::changeIndexValue( int index, int diff ) {
  return tree_->changeIndexValue( index, diff );
}

int Map::indexValue( int indicesNeeded ) const {
  return tree_->indexValue( indicesNeeded );
}

void Map::insertRange( int position, int size, int value ) {
  tree_->insertRange( position, size, value );
}

///Inserts an index at position "index" with the value "value"
void Map::insertIndex( int index, int value ) {
  tree_->insertIndex( index, value );
}

int Map::removeIndex( int index ) {
  return tree_->removeIndex( index );
}

int Map::indexCount() const {
  return tree_->indexCount();
}

int Node::sum( int index ) const {
  if ( index == 0 )
    return 0;
  int summ = 0;
  int indicesNeeded = index;

  ListType::const_iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  ListType::const_iterator end = nodes_.end();
  for ( ; it != end && indicesNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->indexCount();
    if ( itCount <= indicesNeeded ) {
      summ += ( *it ) ->sum();
      indicesNeeded -= itCount;
    } else {
      break;
    }
  }

  if( indicesNeeded > 0 ) {
    if ( it != end ) {
      ///Ask the next one for the summ
      summ += ( *it ) ->sum( indicesNeeded );
    } else if( isRange() ) {
      int eachValue = sum_ / indexCount_;
      int cnt = indicesNeeded;
      if( cnt > indexCount_ ) cnt = indexCount_;
      summ += cnt * eachValue;
    }
  }

  return summ;
}
/*
int Node::index( int sum ) const {
  ListType::const_iterator end = nodes_.end();
  int index = 0;
  for( ListType::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it ) {
    int cnt = (*it)->sum();
    if( cnt <= sum ) {
      index -= cnt;
    } else {
      return index + (*it)->index( sum );
    }
  }
  return index;
  if( isLeaf_ && sum == 0  ) {
    return 0;
  } else {
    cout << nodes_.size() << endl;
 
    DYN_VERIFY( isLeaf_ && sum == 0 );
  }
}*/

int Node::index( int sum ) const {
  if ( isLeaf_ )
    return 0;
  int ind = 0;
  int sumNeeded = sum;

  ListType::const_iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as index(they are completely in)
  ListType::const_iterator end = nodes_.end();
  for ( ; it != end && sumNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->sum();
    if ( itCount <= sumNeeded ) {
      ind += ( *it ) ->indexCount();
      sumNeeded -= itCount;
    } else {
      break;
    }
  }
  if ( it != end ) {
    if ( sumNeeded > 0 ) {
      ///Ask the next one for the summ
      ind += ( *it ) ->index( sumNeeded );
    } else {
      ///Jump over all indices that have a size of zero, because those
      ///might steal the map from the index behind(it should not be possible to map to them).
      while ( it != end && ( *it ) ->sum() == 0 ) {
        ind += ( *it ) ->indexCount();

        ++it;
      }
    }
  } else if( sumNeeded > 0 && isRange() ) {
    int eachValue = sum_ / indexCount_;
    ind += ( sumNeeded / eachValue );
  }

  return ind;
}

void Node::indexAndSum( int sum, IndexAndSum& targ ) const {
  if ( isLeaf_ )
    return;
  int sumNeeded = sum;

  ListType::const_iterator it = nodes_.begin(); ///Add all nodes whose upper bound is lower/same as targ.indexex(they are completely in)
  ListType::const_iterator end = nodes_.end();
  for ( ; it != end && sumNeeded > 0 ; ++it ) {
    int itCount = ( *it ) ->sum();
    if ( itCount <= sumNeeded ) {
      targ.index += ( *it ) ->indexCount();
      targ.sum += itCount;
      sumNeeded -= itCount;
    } else {
      break;
    }
  }
  if ( it != end ) {
    if ( sumNeeded > 0 ) {
      ///Ask the next one for the summ
      ( *it ) ->indexAndSum( sumNeeded, targ );
    } else {
      ///Jump over all targ.indexices that have a size of zero, because those
      ///might steal the map from the targ.indexex behtarg.index(it should not be possible to map to them).
      while ( it != end && ( *it ) ->sum() == 0 ) {
        targ.index += ( *it ) ->indexCount();

        ++it;
      }
    }
  } else if( sumNeeded > 0 && isRange() ) {
    int eachValue = sum_ / indexCount_;
    int cnt = ( sumNeeded / eachValue );
    targ.index += cnt;
    targ.sum += cnt * eachValue;
  }
}

int Map::build( const SumVector::const_iterator& begin, const SumVector::const_iterator& end, Node* parent ) {
  if ( begin == end )
    return 0;

  SumVector::const_iterator it = begin;
  ++it;
  if ( it == end ) {
    ///It is the only item
    NodePointer ret = new Node( true, parent );

    ret->update( *begin, 1 );
    parent->addNode( ret );
    return 1;
  }
  it = begin;
  int count = 0;
  for ( SumVector::const_iterator i = begin; i != end; ++i )
    ++count;

  int threshold = count / PREFERRED_NODES;
  if ( threshold == 0 )
    threshold = 1;

  SumVector::const_iterator currentItem = begin;
  it = begin;
  count = 0;
  int totalCount = 0;
  NodePointer branch = new Node( false, parent );
  parent->addNode( branch );

  while ( 1 ) {
    if ( it == end || ( count >= threshold && it != currentItem ) ) {
      totalCount += build( currentItem, it, branch );
      currentItem = it;
      count = 0;

      if ( it == end )
        break;
    }
    ++count;

    ++it;
  }
  return totalCount;
}
/*
Map Map::rebuild() {
  std::vector<int> vecs;
  tree_->fillValues( vecs );
  return Map( vecs );
}*/

std::string Node::dump( int indexOffset, int depth ) const {
  std::ostringstream os;
  mult( os, depth, "  " );
  os << "depth: " << depth << ( isLeaf_ ? " leaf " : " helper " ) << "sum: " << sum_ << " ";

  if ( !isLeaf_ ) {
    os << "index-count " << indexCount_ << " (nodes: " << nodes_.size() << ")\n";
    for ( ListType::const_iterator it = nodes_.begin(); it != nodes_.end(); ++it ) {
      os << ( *it ) ->dump( indexOffset, depth + 1 );
      indexOffset += ( *it ) ->indexCount();
    }
  } else {
    os << "index: " << indexOffset << "\n";
  }

  return os.str();
}

std::string Map::dump() const {
  return tree_->dump( 0, 0 );
}


}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

