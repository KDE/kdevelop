/***************************************************************************
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "setrepository.h"
#include <list>
#include <QString>
#include <iostream>
#include <limits>
#include <itemrepository.h>
#include <QMutex>
#include <QStack>
#include <QVarLengthArray>

//#define DEBUG

#ifdef DEBUG
#define ifDebug(X) X
#else
#define ifDebug(x)
#undef Q_ASSERT
#define Q_ASSERT(x)
#endif

//When this is enabled, all created nodes are split at the position computed by splitPositionForRange for their range, even
//in nodes created during set_interface, set_union, etc.
//The benefit is that more nodes are shared, the structure is more consistent, and thus several algorithms will terminate faster.
//The disadvantage is that it takes about 30% more memory, because additional nodes need to be inserted.
#define ENFORCE_SPLIT_POSITIONS

namespace Utils {

/**
 * To achieve a maximum re-usage of nodes, we make sure that sub-nodes of a node always split at specific boundaries.
 * For each range we can compute a position where that range should be split into its child-nodes.
 * When creating a new node with 2 sub-nodes, we re-create those child-nodes if their boundaries don't represent those split-positions.
 * 
 * We pick the split-positions deterministically, they are in order of priority:
 * ((1<<31)*n, n = [0,...]
 * ((1<<30)*n, n = [0,...]
 * ((1<<29)*n, n = [0,...]
 * ((1<<...)*n, n = [0,...]
 * ...
 * */


typedef BasicSetRepository::Index Index;

//Constructs a hash that is only dependent on the items in the range.
//The sum of the hashes of multiple separate ranges equal the hash of the complete range. Example:
//hashFromRange(3, 5) + hashFromRange(5, 7) == hashFromRange(3, 7)
const uint primes[] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103};
const uint primeCount = 26;

uint hashFromRange(uint start, uint end) {
  uint ret = 0;
  
  for(uint a = start, inverted = 0xffffffff - start; a < end;) {
    ret += a * primes[a % primeCount];
    
    //If the numbers are very small, we will get clashes. So also use the inverted value.
    ret += inverted * primes[inverted % (primeCount-1)]; //use primeCount-1 to prevent symmetry
    
    ++a;
    --inverted;
  }
  return ret;
}

///@param count should be the count of entries, not the count of actual "ranges"
uint hashFromRanges(const uint* ranges, uint count) {
  Q_ASSERT(count % 2 == 0);
  uint ret = 0;
  for(uint a = 0; a < count; a += 2) {
    ret += hashFromRange(ranges[a], ranges[a+1]);
  }
  return ret;
}

///The returned split position shall be the end of the first sub-range, and the start of the second
///@param splitBit should be initialized with 31, unless you know better. The value can then be used on while computing child split positions.
uint splitPositionForRange(uint start, uint end, uchar& splitBit) {

  if(end-start == 1)
    return 0;
  
  while(true) {
    uint position = ((end-1) >> splitBit) << splitBit; //Round to the split-position in this interval that is smaller than end
    if(position > start && position < end)
      return position;
    Q_ASSERT(splitBit != 0);
    --splitBit;
  }
  
  return 0;
}

uint splitPositionForRange(uint start, uint end) {
  uchar splitBit = 31;
  return splitPositionForRange(start, end, splitBit);
}

//Node used for computing the temporary split-tree
struct SplitTreeNode {
  uint start, end;
  uint hash;
  bool hasSlaves; //If this is true, the next item in the array is the first child node.
  uint rightChildOffset; //If hasSlaves is true, this contains the offset in the array to the right child node(from this node)
};

//Returns the hash-value for all children
uint splitTreeForRanges(QVector<SplitTreeNode>& target, uint** ranges, uint* rangesSizes, uint rangeListCounts, uchar splitBit = 31) {
  
  uint start = ranges[0][0];
  uint end = ranges[rangeListCounts-1][rangesSizes[rangeListCounts-1]-1];
  uint split = splitPositionForRange(start, end, splitBit);
  
  uint position = target.size();

  target.append(SplitTreeNode());
  
  SplitTreeNode* n(&target.back());
  n->start = start;
  n->end = end;
  n->hasSlaves = false;
  n->hash = 0;
  
  if(!split) {
    Q_ASSERT(rangeListCounts == 1);
    Q_ASSERT(rangesSizes[0] == 2);
    Q_ASSERT(end - start == 1); //We have ended up like this
    n->hash = hashFromRange(n->start, n->end);
    Q_ASSERT(!n->hasSlaves || n->rightChildOffset);
    return n->hash;
  }else{
    //Find the position where split splits all the ranges, 
    for(uint rangeList = 0; rangeList < rangeListCounts; ++rangeList) {
      for(uint range = 0; range < rangesSizes[rangeList]; range+= 2) {
        uint thisRangeStart = ranges[rangeList][range];
        uint thisRangeEnd = ranges[rangeList][range+1];
        
        if(split < thisRangeStart || (split >= thisRangeStart && split < thisRangeEnd)) {
          //Either split directly before [rangeList, range], or within this range
          n->hasSlaves = true;
          if(split <= thisRangeStart) {
            Q_ASSERT(rangeList != 0 || range != 0);
            //Split before the range-list
            
            uint currentNodeNumber = target.size()-1;
            
            //Manipulate the parameters a bit, so they represent the left part of the split
            uint oldRangeSize = rangesSizes[rangeList];
            rangesSizes[rangeList] = range;
            
            uint hash = splitTreeForRanges(target, ranges, rangesSizes, rangeList+1, splitBit);
            
            n = &target[position]; //The adress may have changed
            
            //Now compute the right part of the split
            
            n->rightChildOffset = target.size() - currentNodeNumber;
            
            uint* oldRanges = ranges[rangeList];
            ranges[rangeList] += range;
            rangesSizes[rangeList] = oldRangeSize - range;
            hash += splitTreeForRanges(target, ranges+rangeList, rangesSizes+rangeList, rangeListCounts - rangeList, splitBit);
            
            n = &target[position]; //The adress may have changed
            
            n->hash = hash;
            
            ranges[rangeList] = oldRanges;
            rangesSizes[rangeList] = oldRangeSize;
            Q_ASSERT(n->rightChildOffset);
            return n->hash;
          }else{
            //Split in the middle of the range list
            Q_ASSERT(0); ///@todo eventually support real ranges
          }
        }
      }
    }
  }
  
  Q_ASSERT(0);
  Q_ASSERT(!n->hasSlaves || n->rightChildOffset);
  Q_ASSERT(n->hash);
  Q_ASSERT(n->hasSlaves);
  Q_ASSERT(n->rightChildOffset);
  return n->hash;
}

QString printSplitTree(SplitTreeNode* tree, QString prefix = "") {
    QString ret = prefix + QString("%1 -> %2\n").arg(tree->start).arg(tree->end);
    if(tree->hasSlaves) {
        Q_ASSERT(tree->rightChildOffset);
        ret += printSplitTree(tree+1, prefix+" ");
        ret += printSplitTree(tree+tree->rightChildOffset, prefix+" ");
    }
    return ret;
}

struct SetNodeData {
  //Rule: start <= end
  Index start, end; //This set-node bounds all indices starting at start until end, not including end.

  //Child nodes
  //Rule: left->start == start, right->end == end
  //Rule: (left != 0 && right != 0) || (left == 0 && right == 0)
  //Ptr left, right;
  uint leftNode, rightNode;
  uint hash;
  
  SetNodeData() : start(1), end(1), leftNode(0), rightNode(0), hash(0) {
  }
  
  bool contiguous() const {
    return !leftNode;
  }
  
  bool hasSlaves() const {
    return leftNode && rightNode;
  }
  
  //Must always be called when an attribute was changed!
  void updateHash(const SetNodeData* left, const SetNodeData* right) {
    hash = 0;
    if(left && right) {
      hash = left->hash + right->hash;
    } else
      hash = hashFromRange(start, end);
  }
  
  //Only respects the local range, does not really test whether the item is contained in the set
  inline bool boundingRangeContains(Index i) {
    return i >= start && i < end;
  }
  
  //Does not respect gaps
  Index boundingRangeSize() const {
    return end - start;
  }
};

class SetNodeDataRequest;
typedef KDevelop::ItemRepository<SetNodeData, SetNodeDataRequest, false> SetDataRepository;

struct SetNodeDataRequest {

  enum {
    AverageSize = sizeof(SetNodeData)
  };
  
  //This constructor creates a request that finds or creates a node that equals the given node
  //The hash must be up to date
  SetNodeDataRequest(const SetNodeData* _data, SetDataRepository& _repository);
  
  SetNodeDataRequest(const SplitTreeNode* _splitNode, SetDataRepository& _repository) : data(0), splitNode(_splitNode), m_hash(_splitNode->hash), repository(_repository) {
  }

  typedef unsigned int HashType;
  
  //Should return the hash-value associated with this request(For example the hash of a string)
  HashType hash() const {
    return m_hash;
  }
  
  //Should return the size of an item created with createItem
  size_t itemSize() const {
      return sizeof(SetNodeData);
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(SetNodeData* item) const {
    if(data) {
      Q_ASSERT((data->rightNode && data->leftNode) || (!data->rightNode && !data->leftNode));
      
      *item = *data;
#ifdef ENFORCE_SPLIT_POSITIONS
      uint split = splitPositionForRange(data->start, data->end);
      
      if(data->hasSlaves()) {
        const SetNodeData* left = repository.itemFromIndex(data->leftNode);
        const SetNodeData* right = repository.itemFromIndex(data->rightNode);
        Q_ASSERT(left && right);
        if(left->end <= split && right->start >= split) {
          //The split is already perfect
        }else{
          //The split-position intersects with one of the children.
          //Since we make sure that all children split at the correct positions, that child must have its split-position at "split".
          if(split < left->end) {
            Q_ASSERT(split >= left->start);
            Q_ASSERT(left->hasSlaves()); //A single node with just 1 entry cannot intersect anything, we assume that items are represented by single nodes
            //split intersects the left child-node
            const SetNodeData* leftLeft = repository.itemFromIndex(left->leftNode);
            const SetNodeData* leftRight = repository.itemFromIndex(left->rightNode);
            
            //Since we do this stuff here, we can be sure that children always have the correct split position
            Q_ASSERT(split >= leftLeft->end && split <= leftRight->start);
            
            //The left node should be leftLeft, and we have to create a new right node, that 
            //has the children leftRight and right, then we'll split around "split".
            SetNodeData newRight;
            newRight.start = leftRight->start;
            newRight.end = right->end;
          
            newRight.leftNode = left->rightNode;
            newRight.rightNode = data->rightNode;
            newRight.updateHash(leftRight, right);
            
            item->rightNode = repository.index( SetNodeDataRequest( &newRight, repository ) );
            item->leftNode = left->leftNode;
          }else{
            //split intersects the right child-node
            Q_ASSERT(split > right->start && split < right->end);
            Q_ASSERT(right->hasSlaves()); //A single node with just 1 entry cannot intersect anything, we assume that items are represented by single nodes
            
            const SetNodeData* rightLeft = repository.itemFromIndex(right->leftNode);
            const SetNodeData* rightRight = repository.itemFromIndex(right->rightNode);
            
            //Since we do this stuff here, we can be sure that children always have the correct split position
            Q_ASSERT(split >= rightLeft->end && split <= rightRight->start);
            
            //We have to create a new left node that contains left and rightLeft,
            //and the right node will be rightRight
            SetNodeData newLeft;
            newLeft.start = left->start;
            newLeft.end = rightLeft->end;
          
            newLeft.leftNode = data->leftNode;
            newLeft.rightNode = right->leftNode;
            newLeft.updateHash(left, rightLeft);
            
            item->leftNode = repository.index( SetNodeDataRequest( &newLeft, repository ) );
            item->rightNode = right->rightNode;
          }
        }
      }
#endif
      Q_ASSERT(item->hash);
    }else{
      Q_ASSERT(splitNode);
      
      item->start = splitNode->start;
      item->end = splitNode->end;
      item->hash = m_hash;
      if(splitNode->hasSlaves) {
        item->leftNode = repository.index( SetNodeDataRequest( splitNode+1, repository) );
        item->rightNode = repository.index( SetNodeDataRequest( splitNode+splitNode->rightChildOffset, repository) );
        Q_ASSERT(item->leftNode);
        Q_ASSERT(item->rightNode);
      }else{
        item->leftNode = 0;
        item->rightNode = 0;
      }
    }
    
    Q_ASSERT((item->rightNode && item->leftNode) || (!item->rightNode && !item->leftNode));
    
#ifdef ENFORCE_SPLIT_POSITIONS
#ifdef DEBUG
    //Make sure we split at the correct split position
    if(item->hasSlaves()) {
      uint split;
      if(data)
        split = splitPositionForRange(data->start, data->end);
      else
        split = splitPositionForRange(splitNode->start, splitNode->end);
      const SetNodeData* left = repository.itemFromIndex(item->leftNode);
      const SetNodeData* right = repository.itemFromIndex(item->rightNode);
      Q_ASSERT(split >= left->end && split <= right->start);
    }
#endif
#endif
  }
  
  //Should return whether the here requested item equals the given item
  bool equals(const SetNodeData* item) const;
  
  const SetNodeData* data;
  
  const SplitTreeNode* splitNode;
  uint m_hash;
  mutable SetDataRepository& repository;
};

struct SetRepositoryAlgorithms {
  SetRepositoryAlgorithms(SetDataRepository& _repository) : repository(_repository) {
  }
  
  //Returns the left slave node of the given node, or zero if there is none
  const SetNodeData* leftNode(const SetNodeData* node) const {
    if(!node->leftNode)
      return 0;
    return repository.itemFromIndex(node->leftNode);
  }
  
  const SetNodeData* rightNode(const SetNodeData* node) const {
    if(!node->rightNode)
      return 0;
    return repository.itemFromIndex(node->rightNode);
  }
  
  //Gets the node with the given index from the repository
  const SetNodeData* nodeFromIndex(uint index) {
    if(!index)
      return 0;
    return repository.itemFromIndex(index);
  }
  
  ///Expensive
  Index count(const SetNodeData* node) const {
    if(node->leftNode && node->rightNode)
      return count(leftNode(node)) + count(rightNode(node));
    else
      return node->end - node->start;
  }
  
  void localCheck(const SetNodeData* node) {
    Q_ASSERT(node->start > 0);
    Q_ASSERT(node->start < node->end);
    Q_ASSERT((node->leftNode && node->rightNode) || (!node->leftNode && !node->rightNode));
    Q_ASSERT(!node->leftNode || (leftNode(node)->start == node->start && rightNode(node)->end == node->end));
    Q_ASSERT(!node->leftNode || (leftNode(node)->end <= rightNode(node)->start));
  }
  
  void check(uint node) {
    if(!node)
      return;
    
    check(nodeFromIndex(node));
  }
  
  void check(const SetNodeData* node) {
    localCheck(node);
    if(node->leftNode)
      check(leftNode(node));
    if(node->rightNode)
      check(rightNode(node));
  }

  QString shortLabel(const SetNodeData& node) const {
    return QString("n%1_%2").arg(node.start).arg(node.end);
  }

  ///The hash must be up to date!
  uint createSet(const SetNodeData& data) {
    Q_ASSERT(data.hash);
    uint ret = repository.index(SetNodeDataRequest(&data, repository));
    ifDebug( check(ret) );
    return ret;
  }
  
  uint applyBounds(uint node, Index lowerBound, Index upperBound);
  uint set_union(uint first, uint second, Index lowerBound = 0, Index upperBound = std::numeric_limits<Index>::max());
  uint set_intersect(uint first, uint second);
  bool set_contains(const SetNodeData* node, Index index);
  uint set_subtract(uint first, uint second);
  
  bool set_equals(const SetNodeData* lhs, const SetNodeData* rhs);
  bool set_equals(const SplitTreeNode* lhs, const SetNodeData* rhs);
  
  QString dumpDotGraph(uint node) const;
private:
  QString dumpDotGraphInternal(uint node, bool master=false) const;
  
  SetDataRepository& repository;
};

SetNodeDataRequest::SetNodeDataRequest(const SetNodeData* _data, SetDataRepository& _repository) : data(_data), splitNode(0), m_hash(_data->hash), repository(_repository) {
  ///@todo make sure the children of the created set also respect the split-positions. The created node itself can not do that.
  SetRepositoryAlgorithms alg(repository);
  ifDebug(alg.check(_data));
}

bool SetNodeDataRequest::equals(const SetNodeData* item) const {
  if(item->hash != m_hash)
    return false;
  Q_ASSERT(item->hash);
  Q_ASSERT((item->rightNode && item->leftNode) || (!item->rightNode && !item->leftNode));
  SetRepositoryAlgorithms alg(repository);
  if(data)
    return alg.set_equals(data, item);
  else
    return alg.set_equals(splitNode, item);
}


struct BasicSetRepository::Private {
  Private(QString _name, bool doLocking, uint dataSize) : dataRepository(_name, dataSize), mutex(doLocking ? new QMutex(QMutex::Recursive) : 0), name(_name) {
  }
  ~Private() {
    delete mutex;
  }

  SetDataRepository dataRepository;
  QMutex* mutex;
  QString name;
  private:
};

Set::Set() : m_tree(0), m_repository(0) {
}

Set::~Set() {
}

unsigned int Set::count() const {
  if(!m_repository || !m_tree)
    return 0;
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  return alg.count(m_repository->d->dataRepository.itemFromIndex(m_tree));
}

Set::Set(uint treeNode, BasicSetRepository* repository) : m_tree(treeNode), m_repository(repository) {
}


Set::Set(const Set& rhs) {
  m_repository = rhs.m_repository;
  m_tree = rhs.m_tree;
}

Set& Set::operator=(const Set& rhs) {
  m_repository = rhs.m_repository;
  m_tree = rhs.m_tree;
  return *this;
}

QString Set::dumpDotGraph() const {
  if(!m_repository || !m_tree)
    return QString();
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  return alg.dumpDotGraph(m_tree);
}

QString SetRepositoryAlgorithms::dumpDotGraphInternal(uint nodeIndex, bool master) const {
  if(!nodeIndex)
    return QString("empty node");
  
  const SetNodeData& node(*repository.itemFromIndex(nodeIndex));
  
  QString color = "blue";
  if(master)
    color = "red";

  QString label = QString("%1 -> %2").arg(node.start).arg(node.end);
  if(!node.contiguous())
    label += ", with gaps";
  
  QString ret = QString("%1[label=\"%2\", color=\"%3\"];\n").arg(shortLabel(node)).arg(label).arg(color);

  if(node.leftNode) {
    const SetNodeData& left(*repository.itemFromIndex(node.leftNode));
    const SetNodeData& right(*repository.itemFromIndex(node.rightNode));
    Q_ASSERT(node.rightNode);
    ret += QString("%1 -> %2;\n").arg(shortLabel(node)).arg(shortLabel(left));
    ret += QString("%1 -> %2;\n").arg(shortLabel(node)).arg(shortLabel(right));
    ret += dumpDotGraphInternal(node.leftNode);
    ret += dumpDotGraphInternal(node.rightNode);
  }
  
  return ret;
}

QString SetRepositoryAlgorithms::dumpDotGraph(uint nodeIndex) const {
  QString ret = "digraph Repository {\n";
  ret += dumpDotGraphInternal(nodeIndex, true);
  ret += "}\n";
  return ret;
}

struct Set::Iterator::IteratorPrivate {
  IteratorPrivate() : currentIndex(0), repository(0) {
  }
  
  std::list<const SetNodeData*> nodeStack;
  Index currentIndex;
  BasicSetRepository* repository;

  /**
   * Pushes the noed on top of the stack, changes currentIndex, and goes as deep as necessary for iteration.
   * */
  void startAtNode(const SetNodeData* node) {
    currentIndex = node->start;

    do {
      nodeStack.push_back(node);
      if(node->contiguous())
        break; //We need no finer granularity, because the range is contiguous
      node = repository->d->dataRepository.itemFromIndex(node->leftNode);
    } while(node);
    Q_ASSERT(currentIndex >= nodeStack.front()->start);
  }
};


std::set<Index> Set::stdSet() const
{
  Set::Iterator it = iterator();
  std::set<Index> ret;
  
  while(it) {
    Q_ASSERT(ret.find(*it) == ret.end());
    ret.insert(*it);
    ++it;
  }
  
  return ret;
}

Set::Iterator::Iterator(const Iterator& rhs) : d(new IteratorPrivate(*rhs.d)) {
}

Set::Iterator& Set::Iterator::operator=(const Iterator& rhs) {
  delete d;
  d = new IteratorPrivate(*rhs.d);
  return *this;
}

Set::Iterator::Iterator() : d(new IteratorPrivate) {
}

Set::Iterator::~Iterator() {
  delete d;
}

Set::Iterator::operator bool() const {
  return !d->nodeStack.empty();
}

Set::Iterator& Set::Iterator::operator++() {
  
  Q_ASSERT(!d->nodeStack.empty());
  
  QMutexLocker lock(d->repository->d->mutex);
  
  ++d->currentIndex;
  if(d->currentIndex >= d->nodeStack.back()->end) {
    //Advance to the next node
    while(!d->nodeStack.empty() && d->currentIndex >= d->nodeStack.back()->end) {
      d->nodeStack.pop_back();
    }
    if(d->nodeStack.empty()) {
      //ready
    }else{
      //We were iterating the left slave of the node, now continue with the right.
      const SetNodeData& left = *d->repository->d->dataRepository.itemFromIndex(d->nodeStack.back()->leftNode);
      const SetNodeData& right = *d->repository->d->dataRepository.itemFromIndex(d->nodeStack.back()->rightNode);
      
      Q_ASSERT(left.end == d->currentIndex);
      d->startAtNode(&right);
    }
  }

  Q_ASSERT(d->nodeStack.empty() || d->currentIndex < d->nodeStack.front()->end);
  
  return *this;
}

BasicSetRepository::Index Set::Iterator::operator*() const {
  return d->currentIndex;
}

Set::Iterator Set::iterator() const {
  if(!m_tree || !m_repository)
    return Iterator();
  
  QMutexLocker lock(m_repository->d->mutex);
  
  Iterator ret;
  ret.d->repository = m_repository;
  
  if(m_tree)
    ret.d->startAtNode(m_repository->d->dataRepository.itemFromIndex(m_tree));
  return ret;
}

///Returns a node that has the same content as the given node, except that everything out of the given bounds is cut away
uint SetRepositoryAlgorithms::applyBounds(uint nodeIndex, Index lowerBound, Index upperBound)
{
  const SetNodeData* node = repository.itemFromIndex(nodeIndex);
  if(node->start >= lowerBound && node->end <= upperBound)
    return nodeIndex;
  if(node->start >= upperBound || node->end <= lowerBound)
    return 0;
  
  if(leftNode(node)->end <= lowerBound) //left node is completely out of the bounds, continue with the right
    return applyBounds(node->rightNode, lowerBound, upperBound);
  
  if(rightNode(node)->start >= upperBound) //right node is completely out of the bounds, continue with the left
    return applyBounds(node->leftNode, lowerBound, upperBound);

  uint leftNode = applyBounds(node->leftNode, lowerBound, upperBound);
  uint rightNode = applyBounds(node->rightNode, lowerBound, upperBound);
  if(!leftNode)
    return rightNode;
  else if(!rightNode)
    return leftNode;
  
  SetNodeData set;
  set.leftNode = leftNode;
  set.rightNode = rightNode;
  
  const SetNodeData* left = nodeFromIndex(leftNode);
  const SetNodeData* right = nodeFromIndex(rightNode);

  set.start = left->start;
  set.end = right->end;

  ifDebug( check(&set) );
  Q_ASSERT(set.end <= upperBound);
  Q_ASSERT(set.start >= lowerBound);
  
  set.updateHash(left, right);
  
  return createSet(set);
}

uint SetRepositoryAlgorithms::set_union(uint firstNode, uint secondNode, Index lowerBound, Index upperBound)
{
  const SetNodeData* first = nodeFromIndex(firstNode);
  const SetNodeData* second = nodeFromIndex(secondNode);
  
  if(first == second || (first->start == second->start && first->end == second->end && first->contiguous() && second->contiguous()))
    return applyBounds(firstNode, lowerBound, upperBound);
  
  Index firstStart = first->start, secondEnd=second->end;

  if(firstStart >= upperBound)
    return applyBounds(secondNode, lowerBound, upperBound);
  
  if(firstStart >= secondEnd)
  {
    uint leftNode = applyBounds(secondNode, lowerBound, upperBound);
    uint rightNode = applyBounds(firstNode, lowerBound, upperBound);
    if(!leftNode)
      return rightNode;
    else if(!rightNode)
      return leftNode;
    
    const SetNodeData* left = nodeFromIndex(leftNode);
    const SetNodeData* right = nodeFromIndex(rightNode);
    
    SetNodeData set;
    set.leftNode = leftNode;
    set.rightNode = rightNode;

    set.start = left->start;
    set.end = right->end;

    set.updateHash(left, right);
    
    ifDebug( check(&set) );
    Q_ASSERT(set.end <= upperBound);
    Q_ASSERT(set.start >= lowerBound);
    return createSet(set);
  }
  
  Index secondStart = second->start, firstEnd = first->end;

  if(firstEnd <= lowerBound)
    return applyBounds(secondNode, lowerBound, upperBound);
  
  if(secondStart >= firstEnd)
  {
    uint leftNode = applyBounds(firstNode, lowerBound, upperBound);
    uint rightNode = applyBounds(secondNode, lowerBound, upperBound);
    
    const SetNodeData* left = nodeFromIndex(leftNode);
    const SetNodeData* right = nodeFromIndex(rightNode);
    
    if(!left)
      return rightNode;
    else if(!right)
      return leftNode;
    
    SetNodeData set;
    set.leftNode = leftNode;
    set.rightNode = rightNode;

    set.start = left->start;
    set.end = right->end;

    set.updateHash(left, right);
    
    ifDebug( check(&set) );
    Q_ASSERT(set.end <= upperBound);
    Q_ASSERT(set.start >= lowerBound);
    return createSet(set);
  }
  
  //first and second intersect.

  uint leftUnion;
  uint rightUnion;
  //Always split up the one side that is bigger, so we have a better chance that shared nodes meet each other.
  if(firstEnd - firstStart < secondEnd - secondStart)
  {
    //Merge first with second->leftNode and second->rightNode
    uint leftNode = second->leftNode, rightNode = second->rightNode;
    
    const SetNodeData* left = nodeFromIndex(leftNode);
//    const SetNodeData* right = nodeFromIndex(rightNode);
    
    leftUnion = set_union(firstNode, leftNode, lowerBound, left->end < upperBound ? left->end : upperBound);
    rightUnion = set_union(firstNode, rightNode, left->end > lowerBound ? left->end : lowerBound, upperBound);
    if(leftUnion == leftNode && rightUnion == rightNode)
      return secondNode;
  } else {
    //Merge second with first->leftNode and first->rightNode
    uint leftNode = first->leftNode, rightNode = first->rightNode;
    const SetNodeData* left = nodeFromIndex(leftNode);
//    const SetNodeData* right = nodeFromIndex(rightNode);
    
    leftUnion = set_union(secondNode, leftNode, lowerBound, left->end < upperBound ? left->end : upperBound);
    rightUnion = set_union(secondNode, rightNode, left->end > lowerBound ? left->end : lowerBound, upperBound);
    if(leftUnion == leftNode && rightUnion == rightNode)
      return firstNode;
  }

  if(leftUnion && !rightUnion)
    return leftUnion;
  else if(rightUnion && !leftUnion)
    return rightUnion;
  else if(!leftUnion && !rightUnion)
    return 0;

  SetNodeData set;
  set.leftNode = leftUnion;
  set.rightNode = rightUnion;

  const SetNodeData* left = nodeFromIndex(leftUnion);
  const SetNodeData* right = nodeFromIndex(rightUnion);
  
  set.start = left->start;
  set.end = right->end;

  ifDebug( check(&set) );
  Q_ASSERT(set.end <= upperBound);
  Q_ASSERT(set.start >= lowerBound);
  
  set.updateHash(left, right);
  
  return createSet(set);
}

bool SetRepositoryAlgorithms::set_equals(const SetNodeData* lhs, const SetNodeData* rhs)
{
  QVarLengthArray<const SetNodeData*, 200> leftNextStack, rightNextStack;

  if(lhs->start != rhs->start || lhs->end != rhs->end)
    return false;
  
  while(true) {
    
    if(lhs == rhs || (lhs->contiguous() && rhs->contiguous())) {
      
      if(lhs->start != rhs->start || lhs->end != rhs->end || lhs->hash != rhs->hash)
        return false;
      
      if(leftNextStack.isEmpty() && rightNextStack.isEmpty())
        return true; //Ideally we end here at some point
      
      if(leftNextStack.isEmpty() || rightNextStack.isEmpty())
        return false;
      int newLeftSize = leftNextStack.size()-1;
      lhs = leftNextStack[newLeftSize];
      leftNextStack.resize(newLeftSize);

      int newRightSize = rightNextStack.size()-1;
      rhs = rightNextStack[newRightSize];
      rightNextStack.resize(newRightSize);
    }
    
    if(lhs->start == rhs->start && lhs->end == rhs->end && lhs->hash != rhs->hash)
      return false;
    
    //We assume that on the bottom level there is exactly one set for each item
    if(!rhs->contiguous() || !lhs->contiguous()) {
        if(lhs->end - lhs->start > rhs->end - rhs->start) {
        leftNextStack.append(rightNode(lhs));
        lhs = leftNode(lhs);
        if(!lhs)
            return false;
        //Q_ASSERT(leftNextStack.top());
        }else{
        rightNextStack.append(rightNode(rhs));
        rhs = leftNode(rhs);
        if(!rhs)
            return false;
        
        //Q_ASSERT(rightNextStack.top());
        }
    }
  }
  
  return true;
}

bool SetRepositoryAlgorithms::set_equals(const SplitTreeNode* lhs, const SetNodeData* rhs)
{
  QStack<const SplitTreeNode*> leftNextStack;
  QStack<const SetNodeData*> rightNextStack;

  if(lhs->start != rhs->start || lhs->end != rhs->end)
    return false;
  
  while(true) {
    if(!lhs->hasSlaves && rhs->contiguous()) {
      //Here it must match, because the basic sets have size 1
      Q_ASSERT(lhs->end - lhs->start == 1 && rhs->end - rhs->start == 1); //We do this assumption in the algorithm
      
      if(lhs->start != rhs->start || lhs->end != rhs->end || lhs->hash != rhs->hash)
        return false;
      
      if(leftNextStack.isEmpty() && rightNextStack.isEmpty())
        return true; //Ideally we end here at some point
      
      if(leftNextStack.isEmpty() || rightNextStack.isEmpty())
        return false;
      
      lhs = leftNextStack.top();
      rhs = rightNextStack.top();
      leftNextStack.pop();
      rightNextStack.pop();
    }
    
    if(lhs->start == rhs->start && lhs->end == rhs->end && lhs->hash != rhs->hash)
      return false;
    
    if(lhs->hasSlaves || !rhs->contiguous()) {
        //We assume that on the bottom level there is exactly one set for each item
        if(lhs->end - lhs->start > rhs->end - rhs->start) {
        if(!lhs->hasSlaves)
            return false;
        
        leftNextStack.push(lhs+lhs->rightChildOffset);
        lhs = lhs+1;
        }else{
        rightNextStack.push(rightNode(rhs)); ///@todo herausfinden warum hier assertion wenn ranges gleich(anstatt match)
        Q_ASSERT(rightNextStack.top());
        rhs = leftNode(rhs);
        if(!rhs)
            return false;
        }
    }
  }
  
  return true;
}

uint SetRepositoryAlgorithms::set_intersect(uint firstNode, uint secondNode)
{
  const SetNodeData* first = nodeFromIndex(firstNode);
  const SetNodeData* second = nodeFromIndex(secondNode);
  
  Index firstStart = first->start, secondEnd=second->end;
  if(firstStart >= secondEnd)
    return 0;
  
  Index secondStart = second->start, firstEnd = first->end;
  if(secondStart >= firstEnd)
    return 0;
  
  if(first == second)
    return firstNode;
  
  //first and second intersect.

  uint splitLeftIntersectionNode;
  uint splitRightIntersectionNode;
  //Always split up the one side that is bigger, so we have a better chance that shared nodes meet each other.
  if(firstEnd-firstStart < secondEnd - secondStart)
  {
    //Intersect first with second->left and second->right
    uint leftNode = second->leftNode, rightNode = second->rightNode;
    splitLeftIntersectionNode = set_intersect(firstNode, leftNode);
    splitRightIntersectionNode = set_intersect(firstNode, rightNode);
    if(splitLeftIntersectionNode == leftNode && splitRightIntersectionNode == rightNode)
      return secondNode;
  } else {
    uint leftNode = first->leftNode, rightNode = first->rightNode;
    splitLeftIntersectionNode = set_intersect(secondNode, leftNode);
    splitRightIntersectionNode = set_intersect(secondNode, rightNode);
    if(splitLeftIntersectionNode == leftNode && splitRightIntersectionNode == rightNode)
      return firstNode;
  }
  
  if(splitLeftIntersectionNode && splitRightIntersectionNode)
  {
    SetNodeData set;
    set.leftNode = splitLeftIntersectionNode;
    set.rightNode = splitRightIntersectionNode;

    set.start = leftNode(&set)->start;
    set.end = rightNode(&set)->end;
    
    set.updateHash(nodeFromIndex(set.leftNode), nodeFromIndex(set.rightNode));
    ifDebug( check(&set) );
    return createSet(set);
  }else if(splitLeftIntersectionNode) {
    return splitLeftIntersectionNode;
  }else{
    return splitRightIntersectionNode;
  }
  
  return 0;
}

bool SetRepositoryAlgorithms::set_contains(const SetNodeData* node, Index index)
{
  while(true) {
    if(node->start > index || node->end <= index)
      return false;

    if(node->contiguous())
      return true;

    if(index < leftNode(node)->end)
      node = nodeFromIndex(node->leftNode);
    else
      node = nodeFromIndex(node->rightNode);
  }
  
  return false;
}

uint SetRepositoryAlgorithms::set_subtract(uint firstNode, uint secondNode)
{
  const SetNodeData* first = nodeFromIndex(firstNode);
  const SetNodeData* second = nodeFromIndex(secondNode);
  
  Index firstStart = first->start, secondEnd=second->end;
  if(firstStart >= secondEnd)
    return firstNode;
  
  Index secondStart = second->start, firstEnd = first->end;
  if(secondStart >= firstEnd)
    return firstNode;
  
  if(firstNode == secondNode)
    return 0;
  
  //first and second intersect.

  uint splitLeftSubtractionNode;
  uint splitRightSubtractionNode;
  //Always split up the one side that is bigger, so we have a better chance that shared nodes meet each other.
  if(firstEnd-firstStart < secondEnd - secondStart)
  {
    uint temp = set_subtract(firstNode, second->leftNode);
    if(!temp)
      return 0;
    return set_subtract(temp, second->rightNode);
  } else {
    uint leftNode = first->leftNode, rightNode = first->rightNode;
    
/*    const SetNodeData* left = nodeFromIndex(leftNode);
    const SetNodeData* right = nodeFromIndex(rightNode);*/
    
    splitLeftSubtractionNode = set_subtract(leftNode, secondNode);
    splitRightSubtractionNode = set_subtract(rightNode, secondNode);
    if(splitLeftSubtractionNode == leftNode && splitRightSubtractionNode == rightNode)
      return firstNode;
  }

  if(splitLeftSubtractionNode && splitRightSubtractionNode)
  {
    SetNodeData set;
    set.leftNode = splitLeftSubtractionNode;
    set.rightNode = splitRightSubtractionNode;

    set.start = leftNode(&set)->start;
    set.end = rightNode(&set)->end;

    set.updateHash(nodeFromIndex(set.leftNode), nodeFromIndex(set.rightNode));
    
    ifDebug( check(&set) );
    
    return createSet(set);
    
  }else if(splitLeftSubtractionNode) {
    return splitLeftSubtractionNode;
  }else{
    return splitRightSubtractionNode;
  }
  
  return 0;
}

Set BasicSetRepository::createSetFromRanges(const std::vector<Index>& indices) {
  QMutexLocker lock(d->mutex);
  std::vector<Index> shortRanges; //Currently we only support ranges of length 1, so we actually do not support them
  
  for(uint a = 0; a < indices.size(); a += 2) {
    for(uint b = indices[a]; b < indices[a+1]; ++b) {
      shortRanges.push_back(b);
      shortRanges.push_back(b+1);
    }
  }
  
  if(shortRanges.empty())
      return Set();
  
  uint rangeCount = shortRanges.size();
  uint* ranges = &shortRanges[0];
  
  QVector<SplitTreeNode> splitTree;
  
  splitTreeForRanges(splitTree, &ranges, &rangeCount, 1);

  //kDebug() << printSplitTree(splitTree.data());
  
  return Set(d->dataRepository.index( SetNodeDataRequest(&splitTree[0], d->dataRepository) ), this);
}

Set BasicSetRepository::createSet(Index i) {
  std::vector<Index> ranges;
  ranges.push_back(i);
  ranges.push_back(i+1);
  return createSetFromRanges(ranges);
}

Set BasicSetRepository::createSet(const std::set<Index>& indices) {

  std::vector<Index> ranges;

  for( std::set<Index>::const_iterator it = indices.begin(); it != indices.end(); ++it )
  {
    if(ranges.empty() || ranges.back() != *it)
    {
      //Create new range
      ranges.push_back(*it);
      ranges.push_back(*it);
    }
    
    ++ranges.back();
  }

  return createSetFromRanges(ranges);
}

BasicSetRepository::BasicSetRepository(QString name, bool doLocking, uint dataSize) : d(new Private(name, doLocking, dataSize)) {
}

BasicSetRepository::~BasicSetRepository() {
  kDebug() << "Size of " << d->name << ":" << d->dataRepository.usedMemory();
  delete d;
}

////////////Set convenience functions//////////////////

bool Set::contains(Index index) const
{
  if(!m_tree || !m_repository)
    return false;
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  return alg.set_contains(m_repository->d->dataRepository.itemFromIndex(m_tree), index);
}

Set Set::operator +(const Set& first) const
{
  if(!first.m_tree)
    return *this;
  else if(!m_tree || !m_repository)
    return first;
  
  Q_ASSERT(m_repository == first.m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  uint retNode = alg.set_union(m_tree, first.m_tree);
  
  ifDebug(alg.check(retNode));
  
  return Set(retNode, m_repository);
}

Set& Set::operator +=(const Set& first) {
  if(!first.m_tree)
    return *this;
  else if(!m_tree || !m_repository) {
    m_tree = first.m_tree;
    m_repository = first.m_repository;
    return *this;
  }
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  m_tree = alg.set_union(m_tree, first.m_tree);
  
  ifDebug(alg.check(m_tree));
  return *this;
}

Set Set::operator &(const Set& first) const {
  if(!first.m_tree || !m_tree)
    return Set();
  
  Q_ASSERT(m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  Set ret( alg.set_intersect(m_tree, first.m_tree), m_repository );
  
  ifDebug(alg.check(ret.m_tree));
  
  return ret;
}

Set& Set::operator &=(const Set& first) {
  if(!first.m_tree || !m_tree) {
    m_tree = 0;
    return *this;
  }
  
  Q_ASSERT(m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  m_tree = alg.set_intersect(m_tree, first.m_tree);
  ifDebug(alg.check(m_tree));
  return *this;
}

Set Set::operator -(const Set& rhs) const {
  if(!m_tree || !rhs.m_tree)
    return *this;

  Q_ASSERT(m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  Set ret( alg.set_subtract(m_tree, rhs.m_tree), m_repository );
  ifDebug( alg.check(ret.m_tree) );
  return ret;
}

Set& Set::operator -=(const Set& rhs) {
  if(!m_tree || !rhs.m_tree)
    return *this;

  Q_ASSERT(m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  m_tree = alg.set_subtract(m_tree, rhs.m_tree);

  ifDebug(alg.check(m_tree));
  return *this;
}

BasicSetRepository* Set::repository() const {
  return m_repository;
}
}

