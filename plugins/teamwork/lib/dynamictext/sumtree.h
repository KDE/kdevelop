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

#ifndef SUMTREE_H
#define SUMTREE_H

#include <list>
#include <vector>

#include "network/safesharedptr.h"
#include "network/weaksafesharedptr.h"

#include "verify.h"

namespace SumTree {
class Node;
class EndIndexCompare;
typedef SharedPtr<Node> NodePointer;
typedef WeakSharedPtr<Node> WeakNodePointer;


///A pair that contains an index and the sum of all values before that index
struct IndexAndSum {
  int index;
  int sum;
};

///Everything may throw DynamicTextError. Every pointer to Nodes, except those from parent-node to slave-node, must be weak-pointers.(because the ref-count is important for ownership-stuff)
class Node : public Shared {
  typedef std::list<NodePointer> ListType;
  public:
    Node( bool isLeaf, Node* parent );

    ///If this node is already owned by parent, returns this. Else creates a copy owned by parent.
    inline Node* makeOwn( Node* parent );
    Node* setParent( Node* parent );

    ///Can be used to add/subtract a value from the sum(should only be directly called on end-items without children, and the index-range is only expanded, never shrinked.)
    inline void update( const int sumDif, const int indexCountDif ) {
      sum_ += sumDif;
      indexCount_ += indexCountDif;

      //      DYN_VERIFY_SMALLER( sum_, 1000000 );
      Node* parent = parent_/*.get()*/;
      if( parent && (sumDif != 0 || indexCountDif != 0 ) )
        parent_->update( sumDif, indexCountDif );
    }

    void addNode( Node* node );

    ///The sum is counted until that index, not including it.
    int sum( int index ) const;

    int index( int sum ) const;

    int sum() const {
      return sum_;
    }
    int indexCount() const {
      return indexCount_;
    }

    void indexAndSum( int sum, IndexAndSum& targ ) const;

    std::string dump( int offset,int depth ) const;

    template<class Container>
    void fillValues( Container& c ) {
      if( isLeaf_ ) {
        c.push_back( sum_ );
      }else {
        for( ListType::iterator it = nodes_.begin(); it != nodes_.end(); ++it )
          (*it)->fillValues( c );
      }
    }

    bool hasIndex( int index ) const;

    int indexValue( int index ) const;

    int setIndexValue( int index, int value );

    int changeIndexValue( int index, int diff );

    void insertIndex( int index, int value  );

    void insertRange( int position, int size, int value );

    int removeIndex( int index );

  bool isLeaf() const {
    return isLeaf_;
  }

    bool isRange() const {
      return !isLeaf_ && nodeCount_ == 0 && indexCount_ != 0;
    }

    void updateStructure();

  private:
    void splitRange( int splitPosition, Node* insertNode );

    void removeChild( Node* n );

    friend class EndIndexCompare;
    /*WeakNodePointer*/Node* parent_; ///if this is zero, the node is available for taking, else it must be copied.
    ListType nodes_;
    int nodeCount_; ///Cache for nodes_.size()
    int sum_;  ///Sum over all sub-elements
    int indexCount_; ///The count of indices this node and all subnodes cover
    bool isLeaf_;
};

///This object can be copied around and changed at will, no other instances of it will be affected, while still staying efficient(Because of implicit node-sharing)
class Map : public WeakShared {
  public:
    typedef std::vector< int > SumVector; ///Pairs of (index, sum-value). Muss dicht sein.
    ///preferredOrder says how many slave-nodes one node should ideally have. Higher values -> faster manipulation, lower values -> faster lookup
    Map( const SumVector& summands = SumVector() );
    Map( const Map& rhs );
    ~Map();

    Map& operator = ( const Map& rhs );

    ///Returns the sum over all indices
    int sum() const;
    ///Returns the values of all indices up to(not including) index summed together
    int sum( int index ) const;

    ///Returns the index that the given sum falls into. Warning: indices that have a value of zero cannot be mapped back with this function.
    int index( int sum ) const;

    void indexAndSum( int sum, IndexAndSum& targ ) const;

    bool hasIndex( int index ) const;
  //    Map rebuild();

    ///Complexity: O(1)
    int indexCount()const ;

    ///Returns the value of that specified index
    int indexValue( int index ) const;

    ///Returns the previous value of that index. The value must not be zero, because that would not allow consistent mapping back(exception is thrown)
    int setIndexValue( int index, int value );

    ///Returns the new value of that index. If the value becomes zero, the index cannot be mapped back through index(..) anymore.
    int changeIndexValue( int index, int diff );

    ///Inserts an index at position "index" with the value "value"
    void insertIndex( int index, int value = 0 ) ;

      /**Efficiently inserts a range of indices with
     * same value(may be zero) into the tree.
     * @param position Starting-position of the range
     * @param size Count of indices the range will occupy
     * @param value The value each index of the range will get(the whole range will have the sum size*range)
     * $
     */
  void insertRange( int position, int size, int eachValue = 0 );

    ///Removes one index at position index, returns the personal sum that index had associated
    int removeIndex( int index );

    std::string dump() const;

  private:
    int build( const SumVector::const_iterator& begin, const SumVector::const_iterator& end, Node* parent );
    NodePointer tree_;
};

}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
