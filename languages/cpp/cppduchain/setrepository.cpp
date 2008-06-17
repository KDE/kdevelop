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
#include <QVarLengthArray>
#include <iostream>
#include <limits>
#include <language/duchain/repositories/itemrepository.h>
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

#ifndef DEBUG
#define CHECK_SPLIT_POSITION(Node)
#else
#define CHECK_SPLIT_POSITION(node) Q_ASSERT(!(node).leftNode || (this->leftNode(&node)->end <= splitPositionForRange((node).start, (node).end) && this->rightNode(&node)->start >= splitPositionForRange((node).start, (node).end)))
#endif

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
///In the end, it will contain the bit used to split the range. It will also contain zero if no split-position exists(length 1)
uint splitPositionForRange(uint start, uint end, uchar& splitBit) {

  if(end-start == 1) {
    splitBit = 0;
    return 0;
  }
  
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
  
  inline bool contiguous() const {
    return !leftNode;
  }
  
  inline bool hasSlaves() const {
    return (bool)leftNode;
  }
  
  //Must always be called when an attribute was changed!
  void updateHash(const SetNodeData* left, const SetNodeData* right) {
    hash = 0;
    if(left && right) {
      hash = left->hash + right->hash;
    } else
      hash = hashFromRange(start, end);
  }
};

class SetNodeDataRequest;
typedef KDevelop::ItemRepository<SetNodeData, SetNodeDataRequest, false> SetDataRepository;

struct SetRepositoryAlgorithms {
  SetRepositoryAlgorithms(SetDataRepository& _repository) : repository(_repository) {
  }
  
  inline const SetNodeData* leftNode(const SetNodeData* node) const {
    return repository.itemFromIndex(node->leftNode);
  }
  
  inline const SetNodeData* rightNode(const SetNodeData* node) const {
    return repository.itemFromIndex(node->rightNode);
  }
  
  inline const SetNodeData* nodeFromIndex(uint index) const {
    return repository.itemFromIndex(index);
  }
  
  ///Expensive
  Index count(const SetNodeData* node) const;
  
  void localCheck(const SetNodeData* node);
  
  void check(uint node);
  
  void check(const SetNodeData* node);

  QString shortLabel(const SetNodeData& node) const;

  uint applyBounds(uint node, Index lowerBound, Index upperBound);
  uint set_union(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit = 31);
  uint createSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left = 0, const SetNodeData* right = 0);
  uint computeSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left, const SetNodeData* right, uchar splitBit);
  uint set_intersect(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit = 31);
  bool set_contains(const SetNodeData* node, Index index);
  uint set_subtract(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit = 31);
  
  //Required both nodes to be split correctly
  bool set_equals(const SetNodeData* lhs, const SetNodeData* rhs);
  bool set_equals(const SplitTreeNode* lhs, const SetNodeData* rhs);
  
  QString dumpDotGraph(uint node) const;
private:
  QString dumpDotGraphInternal(uint node, bool master=false) const;
  
  SetDataRepository& repository;
};

struct SetNodeDataRequest {

  enum {
    AverageSize = sizeof(SetNodeData)
  };
  
  //This constructor creates a request that finds or creates a node that equals the given node
  //The hash must be up to dat, and the node must be split correctly around its splitPosition
  inline SetNodeDataRequest(const SetNodeData* _data, SetDataRepository& _repository) : data(_data), splitNode(0), m_hash(_data->hash), repository(_repository) {
    ifDebug( SetRepositoryAlgorithms alg(repository); alg.check(_data) );
  }
  
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
  }
  
  //Should return whether the here requested item equals the given item
  inline bool equals(const SetNodeData* item) const {
    Q_ASSERT(item->hash);
    Q_ASSERT((item->rightNode && item->leftNode) || (!item->rightNode && !item->leftNode));
    if(data) {
      //Just compare child nodes, since data must be correctly split, this is perfectly ok
      //Since this happens in very tight loops, we don't call an additional function here, but just do the check.
      if(item->leftNode) {
        if(!data->leftNode)
          return false;
        else
          return item->leftNode == data->leftNode && item->rightNode == data->rightNode;
      }else{
        if(data->leftNode)
          return false;
        else
          return data->start == item->start && data->end == item->end;
      }
    } else {
      SetRepositoryAlgorithms alg(repository);
      return alg.set_equals(splitNode, item);
    }
  }
  
  const SetNodeData* data;
  
  const SplitTreeNode* splitNode;
  uint m_hash;
  mutable SetDataRepository& repository;
};

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

Index SetRepositoryAlgorithms::count(const SetNodeData* node) const {
  if(node->leftNode && node->rightNode)
    return count(leftNode(node)) + count(rightNode(node));
  else
    return node->end - node->start;
}

void SetRepositoryAlgorithms::localCheck(const SetNodeData* ifDebug(node) ) {
  Q_ASSERT(node->start > 0);
  Q_ASSERT(node->start < node->end);
  Q_ASSERT((node->leftNode && node->rightNode) || (!node->leftNode && !node->rightNode));
  Q_ASSERT(!node->leftNode || (leftNode(node)->start == node->start && rightNode(node)->end == node->end));
  Q_ASSERT(!node->leftNode || (leftNode(node)->end <= rightNode(node)->start));
}

void SetRepositoryAlgorithms::check(uint node) {
  if(!node)
    return;
  
  check(nodeFromIndex(node));
}

void SetRepositoryAlgorithms::check(const SetNodeData* node) {
  localCheck(node);
  if(node->leftNode)
    check(leftNode(node));
  if(node->rightNode)
    check(rightNode(node));
  CHECK_SPLIT_POSITION(*node);
}

QString SetRepositoryAlgorithms::shortLabel(const SetNodeData& node) const {
  return QString("n%1_%2").arg(node.start).arg(node.end);
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

const int nodeStackAlloc = 500;

struct Set::Iterator::IteratorPrivate {
  IteratorPrivate() : nodeStackSize(0), currentIndex(0), repository(0) {
    nodeStackData.resize(nodeStackAlloc);
    nodeStack = nodeStackData.data();
  }
  
  IteratorPrivate(const IteratorPrivate& rhs) : nodeStackData(rhs.nodeStackData), nodeStackSize(rhs.nodeStackSize), currentIndex(rhs.currentIndex), repository(rhs.repository) {
    nodeStack = nodeStackData.data();
  }
  
  void resizeNodeStack() {
    nodeStackData.resize(nodeStackSize + 1);
    nodeStack = nodeStackData.data();
  }
  
  QVarLengthArray<const SetNodeData*, nodeStackAlloc> nodeStackData;
  const SetNodeData** nodeStack;
  int nodeStackSize;
  Index currentIndex;
  BasicSetRepository* repository;

  /**
   * Pushes the noed on top of the stack, changes currentIndex, and goes as deep as necessary for iteration.
   * */
  void startAtNode(const SetNodeData* node) {
    currentIndex = node->start;

    do {
      nodeStack[nodeStackSize++] = node;
      
      if(nodeStackSize >= nodeStackAlloc)
          resizeNodeStack();
      
      if(node->contiguous())
        break; //We need no finer granularity, because the range is contiguous
      node = repository->d->dataRepository.itemFromIndex(node->leftNode);
    } while(node);
    Q_ASSERT(currentIndex >= nodeStack[0]->start);
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
  return d->nodeStackSize;
}

Set::Iterator& Set::Iterator::operator++() {
  
  Q_ASSERT(d->nodeStackSize);
  
  if(d->repository->d->mutex)
    d->repository->d->mutex->lock();
  
  ++d->currentIndex;
  
  //const SetNodeData** currentNode = &d->nodeStack[d->nodeStackSize - 1];
  if(d->currentIndex >= d->nodeStack[d->nodeStackSize - 1]->end) {
    //Advance to the next node
    while(d->nodeStackSize && d->currentIndex >= d->nodeStack[d->nodeStackSize - 1]->end) {
      --d->nodeStackSize;
    }
    
    if(!d->nodeStackSize) {
      //ready
    }else{
      //++d->nodeStackSize;
      //We were iterating the left slave of the node, now continue with the right.
      ifDebug( const SetNodeData& left = *d->repository->d->dataRepository.itemFromIndex(d->nodeStack[d->nodeStackSize - 1]->leftNode); Q_ASSERT(left.end == d->currentIndex); )
      
      const SetNodeData& right = *d->repository->d->dataRepository.itemFromIndex(d->nodeStack[d->nodeStackSize - 1]->rightNode);
      
      d->startAtNode(&right);
    }
  }

  Q_ASSERT(d->nodeStack.isEmpty() || d->currentIndex < d->nodeStack[0]->end);
  
  if(d->repository->d->mutex)
    d->repository->d->mutex->unlock();
  
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

//Creates a set item with the given children., they must be valid, and they must be split around their split-position.
uint SetRepositoryAlgorithms::createSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left, const SetNodeData* right) {
    
    if(!left)
      left = nodeFromIndex(leftNode);
    if(!right)
      right = nodeFromIndex(rightNode);
  
    Q_ASSERT(left->end <= right->start);
    
    SetNodeData set;
    set.leftNode = leftNode;
    set.rightNode = rightNode;

    set.start = left->start;
    set.end = right->end;

    set.updateHash(left, right);
    
    uint ret = repository.index(SetNodeDataRequest(&set, repository));
    ifDebug( check(ret) );
    return ret;
}

//Constructs a set node from the given two sub-nodes. Those must be valid, they must not intersect, and they must have a correct split-hierarchy.
//The do not need to be split around their computed split-position.
uint SetRepositoryAlgorithms::computeSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left, const SetNodeData* right, uchar splitBit)
{
  Q_ASSERT(left->end <= right->start);
  uint splitPosition = splitPositionForRange(left->start, right->end, splitBit);
  
  Q_ASSERT(splitPosition);
  
  if(splitPosition < left->end) {
    //The split-position intersects the left node
    uint leftLeftNode = left->leftNode;
    uint leftRightNode = left->rightNode;
    
    const SetNodeData* leftLeft = this->leftNode(left);
    const SetNodeData* leftRight = this->rightNode(left);
    
    Q_ASSERT(splitPosition >= leftLeft->end && splitPosition <= leftRight->start);
    
    //Create a new set from leftLeft, and from leftRight + right. That set will have the correct split-position.
    uint newRightNode = computeSetFromNodes(leftRightNode, rightNode, leftRight, right, splitBit);
    
    return createSetFromNodes(leftLeftNode, newRightNode, leftLeft);
  
  }else if(splitPosition > right->start) {
    //The split-position intersects the right node
    uint rightLeftNode = right->leftNode;
    uint rightRightNode = right->rightNode;
    
    const SetNodeData* rightLeft = this->leftNode(right);
    const SetNodeData* rightRight = this->rightNode(right);
    
    Q_ASSERT(splitPosition >= rightLeft->end && splitPosition <= rightRight->start);
    
    //Create a new set from left + rightLeft, and from rightRight. That set will have the correct split-position.
    uint newLeftNode = computeSetFromNodes(leftNode, rightLeftNode, left, rightLeft, splitBit);
    
    return createSetFromNodes(newLeftNode, rightRightNode, 0, rightRight);
  }else{
    return createSetFromNodes(leftNode, rightNode, left, right);
  }
}

uint SetRepositoryAlgorithms::set_union(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit)
{
  if(firstNode == secondNode)
    return firstNode;
  
  uint firstStart = first->start, secondEnd = second->end;
  
  if(firstStart >= secondEnd)
    return computeSetFromNodes(secondNode, firstNode, second, first, splitBit);
        
  uint firstEnd = first->end, secondStart = second->start;
  
  if(secondStart >= firstEnd)
    return computeSetFromNodes(firstNode, secondNode, first, second, splitBit);
  
  //The ranges of first and second do intersect
  
  uint newStart = firstStart < secondStart ? firstStart : secondStart;
  uint newEnd = firstEnd > secondEnd ? firstEnd : secondEnd;

  //Compute the split-position for the resulting merged node
  uint splitPosition = splitPositionForRange(newStart, newEnd, splitBit);
  
  //Since the ranges overlap, we can be sure that either first or second contain splitPosition.
  //The node that contains it, will also be split by it.

  if(splitPosition > firstStart && splitPosition < firstEnd && splitPosition > secondStart && splitPosition < secondEnd) {
    //The split-position intersect with both first and second. Continue the union on both sides of the split-position, and merge it.
    
    uint firstLeftNode = first->leftNode;
    uint firstRightNode = first->rightNode;
    uint secondLeftNode = second->leftNode;
    uint secondRightNode = second->rightNode;
    
    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);
    
    Q_ASSERT(splitPosition >= firstLeft->end && splitPosition <= firstRight->start);
    Q_ASSERT(splitPosition >= secondLeft->end && splitPosition <= secondRight->start);
    
    return createSetFromNodes( set_union(firstLeftNode, secondLeftNode, firstLeft, secondLeft, splitBit), set_union(firstRightNode, secondRightNode, firstRight, secondRight, splitBit) );
    
  }else if(splitPosition > firstStart && splitPosition < firstEnd) {
    
    uint firstLeftNode = first->leftNode;
    uint firstRightNode = first->rightNode;
    
    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    
    Q_ASSERT(splitPosition >= firstLeft->end && splitPosition <= firstRight->start);
    
    //splitPosition does not intersect second. That means that second is completely on one side of it.
    //So we only need to union that side of first with second.
    
    if(secondEnd <= splitPosition) {
      return createSetFromNodes( set_union(firstLeftNode, secondNode, firstLeft, second, splitBit), firstRightNode, 0, firstRight );
    }else{
      Q_ASSERT(secondStart >= splitPosition);
      return createSetFromNodes( firstLeftNode, set_union(firstRightNode, secondNode, firstRight, second, splitBit), firstLeft );
    }
    
  }else if(splitPosition > secondStart && splitPosition < secondEnd) {
    
    uint secondLeftNode = second->leftNode;
    uint secondRightNode = second->rightNode;
    
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);
    
    Q_ASSERT(splitPosition >= secondLeft->end && splitPosition <= secondRight->start);
    
    if(firstEnd <= splitPosition) {
      return createSetFromNodes( set_union(secondLeftNode, firstNode, secondLeft, first, splitBit), secondRightNode, 0, secondRight );
    }else{
      Q_ASSERT(firstStart >= splitPosition);
      return createSetFromNodes( secondLeftNode, set_union(secondRightNode, firstNode, secondRight, first, splitBit), secondLeft );
    }
    
  }else{
    //We would have stopped earlier of first and second don't intersect
    Q_ASSERT(0);
    return 0;
  }
}

bool SetRepositoryAlgorithms::set_equals(const SetNodeData* lhs, const SetNodeData* rhs)
{
  if(lhs->leftNode != rhs->leftNode || lhs->rightNode != rhs->rightNode)
    return false;
  else
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
        rightNextStack.push(rightNode(rhs));
        Q_ASSERT(rightNextStack.top());
        rhs = leftNode(rhs);
        if(!rhs)
            return false;
        }
    }
  }
  
  return true;
}

uint SetRepositoryAlgorithms::set_intersect(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit)
{
  if(firstNode == secondNode)
    return firstNode;
  
  if(first->start >= second->end)
    return 0;
        
  if(second->start >= first->end)
    return 0;
  
  //The ranges of first and second do intersect
  uint firstStart = first->start, firstEnd = first->end, secondStart = second->start, secondEnd = second->end;
  
  uint newStart = firstStart < secondStart ? firstStart : secondStart;
  uint newEnd = firstEnd > secondEnd ? firstEnd : secondEnd;

  //Compute the split-position for the resulting merged node
  uint splitPosition = splitPositionForRange(newStart, newEnd, splitBit);
  
  //Since the ranges overlap, we can be sure that either first or second contain splitPosition.
  //The node that contains it, will also be split by it.

  
  if(splitPosition > firstStart && splitPosition < firstEnd && splitPosition > secondStart && splitPosition < secondEnd) {
    //The split-position intersect with both first and second. Continue the intersection on both sides
    
    uint firstLeftNode = first->leftNode;
    uint firstRightNode = first->rightNode;
    
    uint secondLeftNode = second->leftNode;
    uint secondRightNode = second->rightNode;
    
    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);
    
    Q_ASSERT(splitPosition >= firstLeft->end && splitPosition <= firstRight->start);
    Q_ASSERT(splitPosition >= secondLeft->end && splitPosition <= secondRight->start);
    
    uint newLeftNode = set_intersect(firstLeftNode, secondLeftNode, firstLeft, secondLeft, splitBit);
    uint newRightNode = set_intersect(firstRightNode, secondRightNode, firstRight, secondRight, splitBit);
    
    if(newLeftNode && newRightNode)
      return createSetFromNodes( newLeftNode, newRightNode );
    else if(newLeftNode)
      return newLeftNode;
    else
      return newRightNode;
    
  }else if(splitPosition > firstStart && splitPosition < firstEnd) {
    
    uint firstLeftNode = first->leftNode;
    uint firstRightNode = first->rightNode;
    
    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    
    Q_ASSERT(splitPosition >= firstLeft->end && splitPosition <= firstRight->start);
    
    //splitPosition does not intersect second. That means that second is completely on one side of it.
    //So we can completely ignore the other side of first.
    
    if(secondEnd <= splitPosition) {
      return set_intersect(firstLeftNode, secondNode, firstLeft, second, splitBit);
    }else{
      Q_ASSERT(secondStart >= splitPosition);
      return set_intersect(firstRightNode, secondNode, firstRight, second, splitBit);
    }
  }else if(splitPosition > secondStart && splitPosition < secondEnd) {
    
    uint secondLeftNode = second->leftNode;
    uint secondRightNode = second->rightNode;
    
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);
    
    Q_ASSERT(splitPosition >= secondLeft->end && splitPosition <= secondRight->start);
    
    if(firstEnd <= splitPosition) {
      return set_intersect(secondLeftNode, firstNode, secondLeft, first, splitBit);
    }else{
      Q_ASSERT(firstStart >= splitPosition);
      return set_intersect(secondRightNode, firstNode, secondRight, first, splitBit);
    }
  }else{
    //We would have stopped earlier of first and second don't intersect
    Q_ASSERT(0);
    return 0;
  }
  Q_ASSERT(0);
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

uint SetRepositoryAlgorithms::set_subtract(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit)
{
  if(firstNode == secondNode)
    return 0;
  
  if(first->start >= second->end || second->start >= first->end)
    return firstNode;
        
  //The ranges of first and second do intersect
  uint firstStart = first->start, firstEnd = first->end, secondStart = second->start, secondEnd = second->end;
  
  uint newStart = firstStart < secondStart ? firstStart : secondStart;
  uint newEnd = firstEnd > secondEnd ? firstEnd : secondEnd;

  //Compute the split-position for the resulting merged node
  uint splitPosition = splitPositionForRange(newStart, newEnd, splitBit);
  
  //Since the ranges overlap, we can be sure that either first or second contain splitPosition.
  //The node that contains it, will also be split by it.

  if(splitPosition > firstStart && splitPosition < firstEnd && splitPosition > secondStart && splitPosition < secondEnd) {
    //The split-position intersect with both first and second. Continue the subtract on both sides of the split-position, and merge it.
    
    uint firstLeftNode = first->leftNode;
    uint firstRightNode = first->rightNode;
    
    uint secondLeftNode = second->leftNode;
    uint secondRightNode = second->rightNode;
    
    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);
    
    
    Q_ASSERT(splitPosition >= firstLeft->end && splitPosition <= firstRight->start);
    Q_ASSERT(splitPosition >= secondLeft->end && splitPosition <= secondRight->start);
    
    uint newLeftNode = set_subtract(firstLeftNode, secondLeftNode, firstLeft, secondLeft, splitBit);
    uint newRightNode = set_subtract(firstRightNode, secondRightNode, firstRight, secondRight, splitBit);
    
    if(newLeftNode && newRightNode)
      return createSetFromNodes(newLeftNode, newRightNode);
    else if(newLeftNode)
      return newLeftNode;
    else
      return newRightNode;
    
  }else if(splitPosition > firstStart && splitPosition < firstEnd) {
    
    Q_ASSERT(splitPosition >= firstLeft->end && splitPosition <= firstRight->start);
    
    uint firstLeftNode = first->leftNode;
    uint firstRightNode = first->rightNode;
    
    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    
    //splitPosition does not intersect second. That means that second is completely on one side of it.
    //So we only need to subtract that side of first with second.
    
    uint newLeftNode = firstLeftNode, newRightNode = firstRightNode;
    
    if(secondEnd <= splitPosition) {
      newLeftNode = set_subtract(firstLeftNode, secondNode, firstLeft, second, splitBit);
    }else{
      Q_ASSERT(secondStart >= splitPosition);
      newRightNode = set_subtract(firstRightNode, secondNode, firstRight, second, splitBit);
    }
    
    if(newLeftNode && newRightNode)
      return createSetFromNodes(newLeftNode, newRightNode);
    else if(newLeftNode)
      return newLeftNode;
    else
      return newRightNode;
  
    }else if(splitPosition > secondStart && splitPosition < secondEnd) {
    
    uint secondLeftNode = second->leftNode;
    uint secondRightNode = second->rightNode;
    
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);
    
    Q_ASSERT(splitPosition >= secondLeft->end && splitPosition <= secondRight->start);
    
    if(firstEnd <= splitPosition) {
      return set_subtract(firstNode, secondLeftNode, first, secondLeft, splitBit);
    }else{
      Q_ASSERT(firstStart >= splitPosition);
      return set_subtract(firstNode, secondRightNode, first, secondRight, splitBit);
    }
    
  }else{
    //We would have stopped earlier of first and second don't intersect
    Q_ASSERT(0);
    return 0;
  }
  Q_ASSERT(0);
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

struct StatisticsVisitor {
  StatisticsVisitor(const SetDataRepository& _rep) : nodeCount(0), badSplitNodeCount(0), rep(_rep) {
  }
  bool operator() (const SetNodeData* item) {
    ++nodeCount;
    uint split = splitPositionForRange(item->start, item->end);
    if(item->hasSlaves())
      if(split < rep.itemFromIndex(item->leftNode)->end || split > rep.itemFromIndex(item->rightNode)->start)
        ++badSplitNodeCount;
    return true;
  }
  uint nodeCount;
  uint badSplitNodeCount;
  const SetDataRepository& rep;
};

void BasicSetRepository::printStatistics() const {
  kDebug() << "Size of " << d->name << ":" << d->dataRepository.usedMemory();
  
/*  StatisticsVisitor stats(d->dataRepository);
  d->dataRepository.visitAllItems<StatisticsVisitor>(stats);
  kDebug() << "count of nodes:" << stats.nodeCount << "count of nodes with bad split:" << stats.badSplitNodeCount;*/
}

BasicSetRepository::~BasicSetRepository() {
  printStatistics();
  
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
  
  uint retNode = alg.set_union(m_tree, first.m_tree, m_repository->d->dataRepository.itemFromIndex(m_tree), m_repository->d->dataRepository.itemFromIndex(first.m_tree));
  
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
  
  m_tree = alg.set_union(m_tree, first.m_tree, m_repository->d->dataRepository.itemFromIndex(m_tree), m_repository->d->dataRepository.itemFromIndex(first.m_tree));
  
  ifDebug(alg.check(m_tree));
  return *this;
}

Set Set::operator &(const Set& first) const {
  if(!first.m_tree || !m_tree)
    return Set();
  
  Q_ASSERT(m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  Set ret( alg.set_intersect(m_tree, first.m_tree, m_repository->d->dataRepository.itemFromIndex(m_tree), m_repository->d->dataRepository.itemFromIndex(first.m_tree)), m_repository );
  
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
  
  m_tree = alg.set_intersect(m_tree, first.m_tree, m_repository->d->dataRepository.itemFromIndex(m_tree), m_repository->d->dataRepository.itemFromIndex(first.m_tree));
  ifDebug(alg.check(m_tree));
  return *this;
}

Set Set::operator -(const Set& rhs) const {
  if(!m_tree || !rhs.m_tree)
    return *this;

  Q_ASSERT(m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  Set ret( alg.set_subtract(m_tree, rhs.m_tree, m_repository->d->dataRepository.itemFromIndex(m_tree), m_repository->d->dataRepository.itemFromIndex(rhs.m_tree)), m_repository );
  ifDebug( alg.check(ret.m_tree) );
  return ret;
}

Set& Set::operator -=(const Set& rhs) {
  if(!m_tree || !rhs.m_tree)
    return *this;

  Q_ASSERT(m_repository);
  
  QMutexLocker lock(m_repository->d->mutex);
  
  SetRepositoryAlgorithms alg(m_repository->d->dataRepository);
  
  m_tree = alg.set_subtract(m_tree, rhs.m_tree, m_repository->d->dataRepository.itemFromIndex(m_tree), m_repository->d->dataRepository.itemFromIndex(rhs.m_tree));

  ifDebug(alg.check(m_tree));
  return *this;
}

BasicSetRepository* Set::repository() const {
  return m_repository;
}
}

