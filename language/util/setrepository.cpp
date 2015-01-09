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
#include "util/debug.h"
#include <list>
#include <QtCore/QString>
#include <util/kdevvarlengtharray.h>
#include <iostream>
#include <limits>
#include <serialization/itemrepository.h>
#include <QtCore/QMutex>
#include <QtCore/QStack>
#include <algorithm>

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
#define CHECK_SPLIT_POSITION(node) Q_ASSERT(!(node).leftNode || (getLeftNode(&node)->end() <= splitPositionForRange((node).start, (node).end) && getRightNode(&node)->start() >= splitPositionForRange((node).start, (node).end)))
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

class SetNodeDataRequest;

    #define getLeftNode(node) repository.itemFromIndex(node->leftNode())
    #define getRightNode(node) repository.itemFromIndex(node->rightNode())
    #define nodeFromIndex(index) repository.itemFromIndex(index)
struct SetRepositoryAlgorithms {
  SetRepositoryAlgorithms(SetDataRepository& _repository, BasicSetRepository* _setRepository) : repository(_repository), setRepository(_setRepository) {
  }

  ///Expensive
  Index count(const SetNodeData* node) const;

  void localCheck(const SetNodeData* node);

  void check(uint node);

  void check(const SetNodeData* node);

  QString shortLabel(const SetNodeData& node) const;

  uint set_union(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit = 31);
  uint createSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left = 0, const SetNodeData* right = 0);
  uint computeSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left, const SetNodeData* right, uchar splitBit);
  uint set_intersect(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit = 31);
  bool set_contains(const SetNodeData* node, Index index);
  uint set_subtract(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit = 31);

  //Required both nodes to be split correctly
  bool set_equals(const SetNodeData* lhs, const SetNodeData* rhs);

  QString dumpDotGraph(uint node) const;

    ///Finds or inserts the given ranges into the repository, and returns the set-index that represents them
    uint setForIndices(std::vector<uint>::const_iterator begin, std::vector<uint>::const_iterator end, uchar splitBit = 31) {
        Q_ASSERT(begin != end);

        uint startIndex = *begin;
        uint endIndex = *(end-1)+1;

        if(endIndex == startIndex+1) {
            SetNodeData data(startIndex, endIndex);

            return repository.index( SetNodeDataRequest(&data, repository, setRepository) );
        }

        uint split = splitPositionForRange(startIndex, endIndex, splitBit);
        Q_ASSERT(split);

        std::vector<uint>::const_iterator splitIterator = std::lower_bound(begin, end, split);
        Q_ASSERT(*splitIterator >= split);
        Q_ASSERT(splitIterator > begin);
        Q_ASSERT(*(splitIterator-1) < split);

        return createSetFromNodes(setForIndices(begin, splitIterator, splitBit), setForIndices(splitIterator, end, splitBit));
    }


private:
  QString dumpDotGraphInternal(uint node, bool master=false) const;

  SetDataRepository& repository;
  BasicSetRepository* setRepository;
};

void SetNodeDataRequest::destroy(SetNodeData* data, KDevelop::AbstractItemRepository& _repository) {
    SetDataRepository& repository(static_cast<SetDataRepository&>(_repository));

    if(repository.setRepository->delayedDeletion()) {

        if(data->leftNode()){
            SetDataRepositoryBase::MyDynamicItem left = repository.dynamicItemFromIndex(data->leftNode());
            SetDataRepositoryBase::MyDynamicItem right = repository.dynamicItemFromIndex(data->rightNode());
            Q_ASSERT(left->m_refCount > 0);
            --left->m_refCount;
            Q_ASSERT(right->m_refCount > 0);
            --right->m_refCount;
        }else {
            //Deleting a leaf
            Q_ASSERT(data->end() - data->start() == 1);
            repository.setRepository->itemRemovedFromSets(data->start());
        }
    }
}

SetNodeDataRequest::SetNodeDataRequest(const SetNodeData* _data, SetDataRepository& _repository, BasicSetRepository* _setRepository) : data(*_data), m_hash(_data->hash()), repository(_repository), setRepository(_setRepository), m_created(false) {
    ifDebug( SetRepositoryAlgorithms alg(repository); alg.check(_data) );
}

SetNodeDataRequest::~SetNodeDataRequest() {
    //Eventually increase the reference-count of direct children
    if(m_created) {
        if(data.leftNode())
        ++repository.dynamicItemFromIndex(data.leftNode())->m_refCount;
        if(data.rightNode())
        ++repository.dynamicItemFromIndex(data.rightNode())->m_refCount;
    }
}

//Should create an item where the information of the requested item is permanently stored. The pointer
//@param item equals an allocated range with the size of itemSize().
void SetNodeDataRequest::createItem(SetNodeData* item) const {
    Q_ASSERT((data.rightNode() && data.leftNode()) || (!data.rightNode() && !data.leftNode()));

    m_created = true;

    *item = data;

    Q_ASSERT((item->rightNode() && item->leftNode()) || (!item->rightNode() && !item->leftNode()));

    #ifdef DEBUG
    //Make sure we split at the correct split position
    if(item->hasSlaves()) {
        uint split = splitPositionForRange(data.start, data.end);
        const SetNodeData* left = repository.itemFromIndex(item->leftNode());
        const SetNodeData* right = repository.itemFromIndex(item->rightNode());
        Q_ASSERT(split >= left->end() && split <= right->start());
    }
    #endif
    if(!data.leftNode() && setRepository) {
        for(uint a = item->start(); a < item->end(); ++a)
            setRepository->itemAddedToSets(a);
    }
}

bool SetNodeDataRequest::equals(const SetNodeData* item) const {
    Q_ASSERT((item->rightNode() && item->leftNode()) || (!item->rightNode() && !item->leftNode()));
    //Just compare child nodes, since data must be correctly split, this is perfectly ok
    //Since this happens in very tight loops, we don't call an additional function here, but just do the check.
    return item->leftNode() == data.leftNode() && item->rightNode() == data.rightNode() && item->start() == data.start() && item->end() == data.end();
}

class BasicSetRepository::Private {
public:

  Private(QString _name) : name(_name) {
  }
  ~Private() {
  }

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
  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);
  return alg.count(m_repository->dataRepository.itemFromIndex(m_tree));
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

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);
  return alg.dumpDotGraph(m_tree);
}

Index SetRepositoryAlgorithms::count(const SetNodeData* node) const {
  if(node->leftNode() && node->rightNode())
    return count(getLeftNode(node)) + count(getRightNode(node));
  else
    return node->end() - node->start();
}

void SetRepositoryAlgorithms::localCheck(const SetNodeData* ifDebug(node) ) {
//   Q_ASSERT(node->start() > 0);
  Q_ASSERT(node->start() < node->end());
  Q_ASSERT((node->leftNode() && node->rightNode()) || (!node->leftNode() && !node->rightNode()));
  Q_ASSERT(!node->leftNode() || (getLeftNode(node())->start() == node->start() && getRightNode(node)->end() == node->end()));
  Q_ASSERT(!node->leftNode() || (getLeftNode(node())->end() <= getRightNode(node)->start()));
}

void SetRepositoryAlgorithms::check(uint node) {
  if(!node)
    return;

  check(nodeFromIndex(node));
}

void SetRepositoryAlgorithms::check(const SetNodeData* node) {
  localCheck(node);
  if(node->leftNode())
    check(getLeftNode(node));
  if(node->rightNode())
    check(getRightNode(node));
//  CHECK_SPLIT_POSITION(*node); Re-enable this
}

QString SetRepositoryAlgorithms::shortLabel(const SetNodeData& node) const {
  return QStringLiteral("n%1_%2").arg(node.start()).arg(node.end());
}

QString SetRepositoryAlgorithms::dumpDotGraphInternal(uint nodeIndex, bool master) const {
  if(!nodeIndex)
    return QStringLiteral("empty node");

  const SetNodeData& node(*repository.itemFromIndex(nodeIndex));

  QString color = "blue";
  if(master)
    color = "red";

  QString label = QStringLiteral("%1 -> %2").arg(node.start()).arg(node.end());
  if(!node.contiguous())
    label += ", with gaps";

  QString ret = QStringLiteral("%1[label=\"%2\", color=\"%3\"];\n").arg(shortLabel(node)).arg(label).arg(color);

  if(node.leftNode()) {
    const SetNodeData& left(*repository.itemFromIndex(node.leftNode()));
    const SetNodeData& right(*repository.itemFromIndex(node.rightNode()));
    Q_ASSERT(node.rightNode());
    ret += QStringLiteral("%1 -> %2;\n").arg(shortLabel(node)).arg(shortLabel(left));
    ret += QStringLiteral("%1 -> %2;\n").arg(shortLabel(node)).arg(shortLabel(right));
    ret += dumpDotGraphInternal(node.leftNode());
    ret += dumpDotGraphInternal(node.rightNode());
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

class Set::Iterator::IteratorPrivate {
public:

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

  KDevVarLengthArray<const SetNodeData*, nodeStackAlloc> nodeStackData;
  const SetNodeData** nodeStack;
  int nodeStackSize;
  Index currentIndex;
  BasicSetRepository* repository;

  /**
   * Pushes the noed on top of the stack, changes currentIndex, and goes as deep as necessary for iteration.
   * */
  void startAtNode(const SetNodeData* node) {
    Q_ASSERT(node->start() != node->end());
    currentIndex = node->start();

    do {
      nodeStack[nodeStackSize++] = node;

      if(nodeStackSize >= nodeStackAlloc)
          resizeNodeStack();

      if(node->contiguous())
        break; //We need no finer granularity, because the range is contiguous
      node = Set::Iterator::getDataRepository(repository).itemFromIndex(node->leftNode());
    } while(node);
    Q_ASSERT(currentIndex >= nodeStack[0]->start());
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

  if(d->repository->m_mutex)
    d->repository->m_mutex->lock();

  ++d->currentIndex;

  //const SetNodeData** currentNode = &d->nodeStack[d->nodeStackSize - 1];
  if(d->currentIndex >= d->nodeStack[d->nodeStackSize - 1]->end()) {
    //Advance to the next node
    while(d->nodeStackSize && d->currentIndex >= d->nodeStack[d->nodeStackSize - 1]->end()) {
      --d->nodeStackSize;
    }

    if(!d->nodeStackSize) {
      //ready
    }else{
      //++d->nodeStackSize;
      //We were iterating the left slave of the node, now continue with the right.
      ifDebug( const SetNodeData& left = *d->repository->dataRepository.itemFromIndex(d->nodeStack[d->nodeStackSize - 1]->leftNode()); Q_ASSERT(left.end == d->currentIndex); )

      const SetNodeData& right = *d->repository->dataRepository.itemFromIndex(d->nodeStack[d->nodeStackSize - 1]->rightNode());

      d->startAtNode(&right);
    }
  }

  Q_ASSERT(d->nodeStackSize == 0 || d->currentIndex < d->nodeStack[0]->end());

  if(d->repository->m_mutex)
    d->repository->m_mutex->unlock();

  return *this;
}

BasicSetRepository::Index Set::Iterator::operator*() const {
  return d->currentIndex;
}

Set::Iterator Set::iterator() const {
  if(!m_tree || !m_repository)
    return Iterator();

  QMutexLocker lock(m_repository->m_mutex);

  Iterator ret;
  ret.d->repository = m_repository;

  if(m_tree)
    ret.d->startAtNode(m_repository->dataRepository.itemFromIndex(m_tree));
  return ret;
}

//Creates a set item with the given children., they must be valid, and they must be split around their split-position.
uint SetRepositoryAlgorithms::createSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left, const SetNodeData* right) {

    if(!left)
      left = nodeFromIndex(leftNode);
    if(!right)
      right = nodeFromIndex(rightNode);

    Q_ASSERT(left->end() <= right->start());

    SetNodeData set(left->start(), right->end(), leftNode, rightNode);

    Q_ASSERT(set.start() < set.end());

    uint ret = repository.index(SetNodeDataRequest(&set, repository, setRepository));
    Q_ASSERT(set.leftNode() >= 0x10000);
    Q_ASSERT(set.rightNode() >= 0x10000);
    Q_ASSERT(ret == repository.findIndex(SetNodeDataRequest(&set, repository, setRepository)));
    ifDebug( check(ret) );
    return ret;
}

//Constructs a set node from the given two sub-nodes. Those must be valid, they must not intersect, and they must have a correct split-hierarchy.
//The do not need to be split around their computed split-position.
uint SetRepositoryAlgorithms::computeSetFromNodes(uint leftNode, uint rightNode, const SetNodeData* left, const SetNodeData* right, uchar splitBit)
{
  Q_ASSERT(left->end() <= right->start());
  uint splitPosition = splitPositionForRange(left->start(), right->end(), splitBit);

  Q_ASSERT(splitPosition);

  if(splitPosition < left->end()) {
    //The split-position intersects the left node
    uint leftLeftNode = left->leftNode();
    uint leftRightNode = left->rightNode();

    const SetNodeData* leftLeft = this->getLeftNode(left);
    const SetNodeData* leftRight = this->getRightNode(left);

    Q_ASSERT(splitPosition >= leftLeft->end() && splitPosition <= leftRight->start());

    //Create a new set from leftLeft, and from leftRight + right. That set will have the correct split-position.
    uint newRightNode = computeSetFromNodes(leftRightNode, rightNode, leftRight, right, splitBit);

    return createSetFromNodes(leftLeftNode, newRightNode, leftLeft);

  }else if(splitPosition > right->start()) {
    //The split-position intersects the right node
    uint rightLeftNode = right->leftNode();
    uint rightRightNode = right->rightNode();

    const SetNodeData* rightLeft = this->getLeftNode(right);
    const SetNodeData* rightRight = this->getRightNode(right);

    Q_ASSERT(splitPosition >= rightLeft->end() && splitPosition <= rightRight->start());

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

  uint firstStart = first->start(), secondEnd = second->end();

  if(firstStart >= secondEnd)
    return computeSetFromNodes(secondNode, firstNode, second, first, splitBit);

  uint firstEnd = first->end(), secondStart = second->start();

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

    uint firstLeftNode = first->leftNode();
    uint firstRightNode = first->rightNode();
    uint secondLeftNode = second->leftNode();
    uint secondRightNode = second->rightNode();

    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);

    Q_ASSERT(splitPosition >= firstLeft->end() && splitPosition <= firstRight->start());
    Q_ASSERT(splitPosition >= secondLeft->end() && splitPosition <= secondRight->start());

    return createSetFromNodes( set_union(firstLeftNode, secondLeftNode, firstLeft, secondLeft, splitBit), set_union(firstRightNode, secondRightNode, firstRight, secondRight, splitBit) );

  }else if(splitPosition > firstStart && splitPosition < firstEnd) {

    uint firstLeftNode = first->leftNode();
    uint firstRightNode = first->rightNode();

    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);

    Q_ASSERT(splitPosition >= firstLeft->end() && splitPosition <= firstRight->start());

    //splitPosition does not intersect second. That means that second is completely on one side of it.
    //So we only need to union that side of first with second.

    if(secondEnd <= splitPosition) {
      return createSetFromNodes( set_union(firstLeftNode, secondNode, firstLeft, second, splitBit), firstRightNode, 0, firstRight );
    }else{
      Q_ASSERT(secondStart >= splitPosition);
      return createSetFromNodes( firstLeftNode, set_union(firstRightNode, secondNode, firstRight, second, splitBit), firstLeft );
    }

  }else if(splitPosition > secondStart && splitPosition < secondEnd) {

    uint secondLeftNode = second->leftNode();
    uint secondRightNode = second->rightNode();

    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);

    Q_ASSERT(splitPosition >= secondLeft->end() && splitPosition <= secondRight->start());

    if(firstEnd <= splitPosition) {
      return createSetFromNodes( set_union(secondLeftNode, firstNode, secondLeft, first, splitBit), secondRightNode, 0, secondRight );
    }else{
      Q_ASSERT(firstStart >= splitPosition);
      return createSetFromNodes( secondLeftNode, set_union(secondRightNode, firstNode, secondRight, first, splitBit), secondLeft );
    }

  }else{
    //We would have stopped earlier of first and second don't intersect
    ifDebug( uint test = repository.findIndex(SetNodeDataRequest(first, repository, setRepository)); qCDebug(LANGUAGE) << "found index:" << test; )
    Q_ASSERT(0);
    return 0;
  }
}

bool SetRepositoryAlgorithms::set_equals(const SetNodeData* lhs, const SetNodeData* rhs)
{
  if(lhs->leftNode() != rhs->leftNode() || lhs->rightNode() != rhs->rightNode())
    return false;
  else
    return true;
}

uint SetRepositoryAlgorithms::set_intersect(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit)
{
  if(firstNode == secondNode)
    return firstNode;

  if(first->start() >= second->end())
    return 0;

  if(second->start() >= first->end())
    return 0;

  //The ranges of first and second do intersect
  uint firstStart = first->start(), firstEnd = first->end(), secondStart = second->start(), secondEnd = second->end();

  uint newStart = firstStart < secondStart ? firstStart : secondStart;
  uint newEnd = firstEnd > secondEnd ? firstEnd : secondEnd;

  //Compute the split-position for the resulting merged node
  uint splitPosition = splitPositionForRange(newStart, newEnd, splitBit);

  //Since the ranges overlap, we can be sure that either first or second contain splitPosition.
  //The node that contains it, will also be split by it.


  if(splitPosition > firstStart && splitPosition < firstEnd && splitPosition > secondStart && splitPosition < secondEnd) {
    //The split-position intersect with both first and second. Continue the intersection on both sides

    uint firstLeftNode = first->leftNode();
    uint firstRightNode = first->rightNode();

    uint secondLeftNode = second->leftNode();
    uint secondRightNode = second->rightNode();

    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);

    Q_ASSERT(splitPosition >= firstLeft->end() && splitPosition <= firstRight->start());
    Q_ASSERT(splitPosition >= secondLeft->end() && splitPosition <= secondRight->start());

    uint newLeftNode = set_intersect(firstLeftNode, secondLeftNode, firstLeft, secondLeft, splitBit);
    uint newRightNode = set_intersect(firstRightNode, secondRightNode, firstRight, secondRight, splitBit);

    if(newLeftNode && newRightNode)
      return createSetFromNodes( newLeftNode, newRightNode );
    else if(newLeftNode)
      return newLeftNode;
    else
      return newRightNode;

  }else if(splitPosition > firstStart && splitPosition < firstEnd) {

    uint firstLeftNode = first->leftNode();
    uint firstRightNode = first->rightNode();

    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);

    Q_ASSERT(splitPosition >= firstLeft->end() && splitPosition <= firstRight->start());

    //splitPosition does not intersect second. That means that second is completely on one side of it.
    //So we can completely ignore the other side of first.

    if(secondEnd <= splitPosition) {
      return set_intersect(firstLeftNode, secondNode, firstLeft, second, splitBit);
    }else{
      Q_ASSERT(secondStart >= splitPosition);
      return set_intersect(firstRightNode, secondNode, firstRight, second, splitBit);
    }
  }else if(splitPosition > secondStart && splitPosition < secondEnd) {

    uint secondLeftNode = second->leftNode();
    uint secondRightNode = second->rightNode();

    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);

    Q_ASSERT(splitPosition >= secondLeft->end() && splitPosition <= secondRight->start());

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
    if(node->start() > index || node->end() <= index)
      return false;

    if(node->contiguous())
      return true;

    const SetNodeData* leftNode = nodeFromIndex(node->leftNode());

    if(index < leftNode->end())
      node = leftNode;
    else {
      const SetNodeData* rightNode = nodeFromIndex(node->rightNode());
      node = rightNode;
    }
  }

  return false;
}

uint SetRepositoryAlgorithms::set_subtract(uint firstNode, uint secondNode, const SetNodeData* first, const SetNodeData* second, uchar splitBit)
{
  if(firstNode == secondNode)
    return 0;

  if(first->start() >= second->end() || second->start() >= first->end())
    return firstNode;

  //The ranges of first and second do intersect
  uint firstStart = first->start(), firstEnd = first->end(), secondStart = second->start(), secondEnd = second->end();

  uint newStart = firstStart < secondStart ? firstStart : secondStart;
  uint newEnd = firstEnd > secondEnd ? firstEnd : secondEnd;

  //Compute the split-position for the resulting merged node
  uint splitPosition = splitPositionForRange(newStart, newEnd, splitBit);

  //Since the ranges overlap, we can be sure that either first or second contain splitPosition.
  //The node that contains it, will also be split by it.

  if(splitPosition > firstStart && splitPosition < firstEnd && splitPosition > secondStart && splitPosition < secondEnd) {
    //The split-position intersect with both first and second. Continue the subtract on both sides of the split-position, and merge it.

    uint firstLeftNode = first->leftNode();
    uint firstRightNode = first->rightNode();

    uint secondLeftNode = second->leftNode();
    uint secondRightNode = second->rightNode();

    const SetNodeData* firstLeft = repository.itemFromIndex(firstLeftNode);
    const SetNodeData* firstRight = repository.itemFromIndex(firstRightNode);
    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);


    Q_ASSERT(splitPosition >= firstLeft->end() && splitPosition <= firstRight->start());
    Q_ASSERT(splitPosition >= secondLeft->end() && splitPosition <= secondRight->start());

    uint newLeftNode = set_subtract(firstLeftNode, secondLeftNode, firstLeft, secondLeft, splitBit);
    uint newRightNode = set_subtract(firstRightNode, secondRightNode, firstRight, secondRight, splitBit);

    if(newLeftNode && newRightNode)
      return createSetFromNodes(newLeftNode, newRightNode);
    else if(newLeftNode)
      return newLeftNode;
    else
      return newRightNode;

  }else if(splitPosition > firstStart && splitPosition < firstEnd) {

//    Q_ASSERT(splitPosition >= firstLeft->end() && splitPosition <= firstRight->start());

    uint firstLeftNode = first->leftNode();
    uint firstRightNode = first->rightNode();

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

    uint secondLeftNode = second->leftNode();
    uint secondRightNode = second->rightNode();

    const SetNodeData* secondLeft = repository.itemFromIndex(secondLeftNode);
    const SetNodeData* secondRight = repository.itemFromIndex(secondRightNode);

    Q_ASSERT(splitPosition >= secondLeft->end() && splitPosition <= secondRight->start());

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

Set BasicSetRepository::createSetFromIndices(const std::vector<Index>& indices) {

  QMutexLocker lock(m_mutex);

  if(indices.empty())
    return Set();

  SetRepositoryAlgorithms alg(dataRepository, this);

  return Set(alg.setForIndices(indices.begin(), indices.end()), this);
}

Set BasicSetRepository::createSet(Index i) {
    QMutexLocker lock(m_mutex);
    SetNodeData data(i, i+1);

    return Set(dataRepository.index( SetNodeDataRequest(&data, dataRepository, this) ), this);
}

Set BasicSetRepository::createSet(const std::set<Index>& indices) {

  if(indices.empty())
      return Set();

  QMutexLocker lock(m_mutex);

  std::vector<Index> indicesVector;
  indicesVector.reserve(indices.size());

  for( std::set<Index>::const_iterator it = indices.begin(); it != indices.end(); ++it )
    indicesVector.push_back(*it);

  return createSetFromIndices(indicesVector);
}

BasicSetRepository::BasicSetRepository(QString name, KDevelop::ItemRepositoryRegistry* registry, bool delayedDeletion) : d(new Private(name)), dataRepository(this, name, registry), m_mutex(0), m_delayedDeletion(delayedDeletion) {
    m_mutex = dataRepository.mutex();
}

struct StatisticsVisitor {
  StatisticsVisitor(const SetDataRepository& _rep) : nodeCount(0), badSplitNodeCount(0), zeroRefCountNodes(0), rep(_rep) {
  }
  bool operator() (const SetNodeData* item) {
    if(item->m_refCount == 0)
        ++zeroRefCountNodes;
    ++nodeCount;
    uint split = splitPositionForRange(item->start(), item->end());
    if(item->hasSlaves())
      if(split < rep.itemFromIndex(item->leftNode())->end() || split > rep.itemFromIndex(item->rightNode())->start())
        ++badSplitNodeCount;
    return true;
  }
  uint nodeCount;
  uint badSplitNodeCount;
  uint zeroRefCountNodes;
  const SetDataRepository& rep;
};

void BasicSetRepository::printStatistics() const {

  StatisticsVisitor stats(dataRepository);
  dataRepository.visitAllItems<StatisticsVisitor>(stats);
  qCDebug(LANGUAGE) << "count of nodes:" << stats.nodeCount << "count of nodes with bad split:" << stats.badSplitNodeCount << "count of nodes with zero reference-count:" << stats.zeroRefCountNodes;
}

BasicSetRepository::~BasicSetRepository() {

  delete d;
}

void BasicSetRepository::itemRemovedFromSets(uint /*index*/) {
}

void BasicSetRepository::itemAddedToSets(uint /*index*/) {
}

////////////Set convenience functions//////////////////

bool Set::contains(Index index) const
{
  if(!m_tree || !m_repository)
    return false;

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);
  return alg.set_contains(m_repository->dataRepository.itemFromIndex(m_tree), index);
}

Set Set::operator +(const Set& first) const
{
  if(!first.m_tree)
    return *this;
  else if(!m_tree || !m_repository)
    return first;

  Q_ASSERT(m_repository == first.m_repository);

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);

  uint retNode = alg.set_union(m_tree, first.m_tree, m_repository->dataRepository.itemFromIndex(m_tree), m_repository->dataRepository.itemFromIndex(first.m_tree));

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

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);

  m_tree = alg.set_union(m_tree, first.m_tree, m_repository->dataRepository.itemFromIndex(m_tree), m_repository->dataRepository.itemFromIndex(first.m_tree));

  ifDebug(alg.check(m_tree));
  return *this;
}

Set Set::operator &(const Set& first) const {
  if(!first.m_tree || !m_tree)
    return Set();

  Q_ASSERT(m_repository);

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);

  Set ret( alg.set_intersect(m_tree, first.m_tree, m_repository->dataRepository.itemFromIndex(m_tree), m_repository->dataRepository.itemFromIndex(first.m_tree)), m_repository );

  ifDebug(alg.check(ret.m_tree));

  return ret;
}

Set& Set::operator &=(const Set& first) {
  if(!first.m_tree || !m_tree) {
    m_tree = 0;
    return *this;
  }

  Q_ASSERT(m_repository);

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);

  m_tree = alg.set_intersect(m_tree, first.m_tree, m_repository->dataRepository.itemFromIndex(m_tree), m_repository->dataRepository.itemFromIndex(first.m_tree));
  ifDebug(alg.check(m_tree));
  return *this;
}

Set Set::operator -(const Set& rhs) const {
  if(!m_tree || !rhs.m_tree)
    return *this;

  Q_ASSERT(m_repository);

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);

  Set ret( alg.set_subtract(m_tree, rhs.m_tree, m_repository->dataRepository.itemFromIndex(m_tree), m_repository->dataRepository.itemFromIndex(rhs.m_tree)), m_repository );
  ifDebug( alg.check(ret.m_tree) );
  return ret;
}

Set& Set::operator -=(const Set& rhs) {
  if(!m_tree || !rhs.m_tree)
    return *this;

  Q_ASSERT(m_repository);

  QMutexLocker lock(m_repository->m_mutex);

  SetRepositoryAlgorithms alg(m_repository->dataRepository, m_repository);

  m_tree = alg.set_subtract(m_tree, rhs.m_tree, m_repository->dataRepository.itemFromIndex(m_tree), m_repository->dataRepository.itemFromIndex(rhs.m_tree));

  ifDebug(alg.check(m_tree));
  return *this;
}

BasicSetRepository* Set::repository() const {
  return m_repository;
}

void Set::staticRef() {
  if(!m_tree)
    return;

    QMutexLocker lock(m_repository->m_mutex);
    SetNodeData* data = m_repository->dataRepository.dynamicItemFromIndexSimple(m_tree);
    ++data->m_refCount;
}

///Mutex must be locked
void Set::unrefNode(uint current) {
    SetNodeData* data = m_repository->dataRepository.dynamicItemFromIndexSimple(current);
    Q_ASSERT(data->m_refCount);
    --data->m_refCount;
    if(!m_repository->delayedDeletion()) {
        if(data->m_refCount == 0) {

            if(data->leftNode()){
                Q_ASSERT(data->rightNode());
                unrefNode(data->rightNode());
                unrefNode(data->leftNode());
            }else {
                //Deleting a leaf
                Q_ASSERT(data->end() - data->start() == 1);
                m_repository->itemRemovedFromSets(data->start());
            }

        m_repository->dataRepository.deleteItem(current);
        }
    }
}

///Decrease the static reference-count of this set by one. This set must have a reference-count > 1.
///If this set reaches the reference-count zero, it will be deleted, and all sub-nodes that also reach the reference-count zero
///will be deleted as well. @warning Either protect ALL your sets by using reference-counting, or don't use it at all.
void Set::staticUnref() {
  if(!m_tree)
    return;

    QMutexLocker lock(m_repository->m_mutex);

    unrefNode(m_tree);
}

StringSetRepository::StringSetRepository(QString name) : Utils::BasicSetRepository(name) {
}

void StringSetRepository::itemRemovedFromSets(uint index) {
    ///Call the IndexedString destructor with enabled reference-counting
    KDevelop::IndexedString string = KDevelop::IndexedString::fromIndex(index);

    KDevelop::enableDUChainReferenceCounting(&string, sizeof(KDevelop::IndexedString));
    string.~IndexedString(); //Call destructor with enabled reference-counting
    KDevelop::disableDUChainReferenceCounting(&string);
}

void StringSetRepository::itemAddedToSets(uint index) {
    ///Call the IndexedString constructor with enabled reference-counting

    KDevelop::IndexedString string = KDevelop::IndexedString::fromIndex(index);

    char data[sizeof(KDevelop::IndexedString)];

    KDevelop::enableDUChainReferenceCounting(data, sizeof(KDevelop::IndexedString));
    new (data) KDevelop::IndexedString(string); //Call constructor with enabled reference-counting
    KDevelop::disableDUChainReferenceCounting(data);
}

}

