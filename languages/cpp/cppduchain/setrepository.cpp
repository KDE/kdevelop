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
#include <limits>

//#define DEBUG

#ifdef DEBUG
#define ifDebug(X) X
#else
#define ifDebug(x)
#undef Q_ASSERT
#define Q_ASSERT(x)
#endif
/**
 * Notes:
 * - Tree tree is balanced while insertion
 * - All nodes in the repository that represent ranges contained in a set, are also nodes of that set-tree.
 * */

namespace Utils {
typedef BasicSetRepository::Index Index;

/**
 * Note: Never use KSharedPtr<SetNode> temporarily, because the reference-count is an important property.
 * */
struct SetNode : public KShared
{
  typedef KSharedPtr<SetNode> Ptr;
  
  SetNode() : start(1), end(1), inRepository(false), contiguous(true), parentInRepository(0) {
  }

  ~SetNode() {
    if(inRepository) {
      if(left)
        left->parentInRepository = 0;
      if(right)
        right->parentInRepository = 0;
    }
  }

  //Rule: start <= end
  Index start, end; //This set-node contains all indices starting at start until end, not including end.

  
  //Rule: left->start == start, right->end == end
  //Rule: (left != 0 && right != 0) || (left == 0 && right == 0)
  Ptr left, right;

  //Is this node stored in a string-repository?
  bool inRepository;

  /**If this is true, this set-node contains all indices from start to end.
   * If it is not true, there may be gaps.
   * Always true if inRepository is true.
  */
  bool contiguous;

  //Only filled if inRepository is true
  SetNode* parentInRepository;

  /**
   * Convenience-function for splitting up this node into 2 sub-nodes.
   * @param splitPosition Will be the end of the left, and the start of the right node sub-node.
   * */
  void split(Index splitPosition) {
    Q_ASSERT(splitPosition != start && contains(splitPosition));
    left = new SetNode;
    left->start = start;
    left->end = splitPosition;
    left->inRepository = inRepository;

    right = new SetNode;
    right->start = splitPosition;
    right->end = end;
    right->inRepository = inRepository;

    if(inRepository) {
      left->parentInRepository = this;
      right->parentInRepository = this;
    }

    ifDebug( check(); )
  }

  ///Only here for better readability. Either their is left+right, or neither left nor right.
  inline bool hasSlaves() const {
    return (bool)left;
  }
  
  /**
   * Must only be called on nodes without children.
   * Returns a node representing the range start -> end
   * */
  SetNode* createIntersection(Index start, Index end) {
    Q_ASSERT(end > this->start && start < this->end); //Would be useless calling this with non-intersecting ranges
    Q_ASSERT(!left && !right);
    SetNode* ret = 0;
    //this is only partially covered by this range. Split this up so the intersection can be represented.
    //Since this has no slaves, we do not need to care about them. Just split this up into the right count of sub-nodes.
    if(start > this->start) {
      this->split(start);
      ret = this->right.data();
      Q_ASSERT(ret);
    }

    if(end < this->end) {
      //The end of this range is also within this, so we need to do another cut.

      if(this->right) { //If right is filled, the node was already split above. Do the split in the right side.
        this->right->split(end);
        ret = this->right->left.data();
      } else {
        this->split(end); //Probably start == startNode->start, so this wasn't split yet
        ret = this->left.data();
      }
    }

    ifDebug( localCheck(); )
    return ret ? ret : this;
  }

  void localCheck() {
    Q_ASSERT(start > 0);
    Q_ASSERT(start < end);
    Q_ASSERT((left && right) || (!left && !right));
    Q_ASSERT(!left || (left->start == start && right->end == end));
    Q_ASSERT(!left || (left->end <= right->start));
    Q_ASSERT(!inRepository || contiguous);
  }
  
  void check() {
    localCheck();
    if(left)
      left->check();
    if(right)
      right->check();
  }

  inline bool contains(Index i) {
    return i >= start && i < end;
  }

  /**
   * Move the right side to the given @param newEnd if possible, else returns zero.
   * Adding the indices is not possible if a node that needs to be changed has a reference-count higher than
   * zero, because that means that it is referenced from multiple positions.
   * Since a once referenced node must never be changed, we cannot expand it then.
   *
   * The first appended index will have the index this nodes "end" member had before appendIndices was called.
   * @note newEnd must be bigger then the old end(indices can only be added)
   * */
  bool expandEnd(Index newEnd) {
    Q_ASSERT(newEnd >= end);
    
    if(ref <= 1) {
      if(right) {
        if(right->expandEnd(newEnd)) {
          end = newEnd;
          return true;
        }else{
          if(ref <= 1) {
            //We can expand by adding an intermediate node to the right.
            SetNode::Ptr newRight(new SetNode);
            newRight->left = right;
            newRight->inRepository = inRepository;
            newRight->contiguous = right->contiguous;

            newRight->right = SetNode::Ptr(new SetNode);
            newRight->right->start = right->end;
            newRight->right->end = newEnd;
            newRight->right->inRepository = newRight->inRepository;

            newRight->start = right->start;
            newRight->end = newEnd;
            right = newRight;
            end = newEnd;
            
            newRight->right->parentInRepository = newRight.data();
            newRight->left->parentInRepository = newRight.data();
            newRight->parentInRepository = this;
            
            ifDebug(check());
            return true;
          }else{
            return false;
          }
        }
      }else{
        end = newEnd;
        return true;
      }
    }else{
      //The node cannot be changed, because the ref-count is higher than zero
      return false;
    }
  }

  /**
   * Adds the given count of indices behind this node.
   * If this node cannot be expanded, the expanding will be done by creating a new
   * node and storing it in pnt.
   * @param root Pointer where to store a newly created master-node. Should initially be this.
   *
   * It is guaranteed that after this is called, the node stored in root will have the same content as before,
   * extended by count nodes.
   * */
  Index appendIndices(Ptr& root, int count) {

    Index ret = root->end;
    //Append indices to the right side of root until the right side is as big as the left one(balancing)
    if( (!root->right || root->right->size() < root->left->size()) &&
        root->expandEnd(root->end + count) ) {
      return ret;
    } else {
      //There is already a node references from outside, so it cannot be changed.

      //Create a new root node.
      SetNode::Ptr oldRoot = root;
      root = new SetNode;
      root->left = oldRoot;
      root->inRepository = oldRoot->inRepository;

      //Create new right node, containing the new indices
      root->right = new SetNode;
      root->right->start = root->left->end;
      root->right->end = root->left->end + count;
      root->right->inRepository = oldRoot->inRepository;

      root->contiguous = root->left->contiguous && root->right->contiguous;
      
      if(oldRoot->inRepository) {
        oldRoot->parentInRepository = root.data();
        root->right->parentInRepository = root.data();
      }

      //Correct the bounds of the master node
      root->start = root->left->start;
      root->end = root->right->end;

      ifDebug(root->check();)

      return root->right->start;
    }
  }

  /**
   * Returns the smallest slave set-node that encloses @param position
   * The returned node will have no slaves.
   * If the position is not contained by this set, returns 0.
   * */
  SetNode* findContainer(Index position) {
    Q_ASSERT(contains(position));
    if(!left){
      return this;
    }else{
      if(left->contains(position))
        return left->findContainer(position);
      else if(right->contains(position))
        return right->findContainer(position);
      else
        return 0;
    }
    return 0;
  }

  Index size() const {
    return end - start;
  }

  QString shortLabel() const {
    return QString("n%1_%2").arg(start).arg(end);
  }

  QString dumpDotGraph() const;
private:
  QString dumpDotGraphInternal(bool master=false) const;
};

struct Set::Private : public KShared {
  mutable SetNode::Ptr m_tree; //May be zero for an empty set
};

Set::Set() : d(KSharedPtr<Private>(new Private)) {
}

Set::~Set() {
}

Set::Set(const Set& rhs) {
  d = rhs.d;
}

Set& Set::operator=(const Set& rhs) {
  d = rhs.d;
  return *this;
}

QString Set::dumpDotGraph() const {
  if(d->m_tree)
    return d->m_tree->dumpDotGraph();
  else
    return QString::null;
}

struct BasicSetRepository::Private {
  Private() : m_root(new SetNode), m_debug(true) {
    m_root->inRepository = true;
  }

  //As the set is expanded, the root-node changes.
  SetNode::Ptr m_root;
  bool m_debug;
};

QString SetNode::dumpDotGraphInternal(bool master) const {
  QString color = "blue";
  if(!inRepository)
    color = "black";
  if(master)
    color = "red";

  QString label = QString("%1 -> %2").arg(start).arg(end);
  if(!contiguous)
    label += ", with gaps";
  
  QString ret = QString("%1[label=\"%2\", color=\"%3\"];\n").arg(shortLabel()).arg(label).arg(color);

  if(left) {
    Q_ASSERT(right);
    ret += QString("%1 -> %2;\n").arg(shortLabel()).arg(left->shortLabel());
    ret += QString("%1 -> %2;\n").arg(shortLabel()).arg(right->shortLabel());
    ret += left->dumpDotGraphInternal();
    ret += right->dumpDotGraphInternal();
  }
  
  return ret;
}

QString SetNode::dumpDotGraph() const {
  QString ret = "digraph Repository {\n";
  ret += dumpDotGraphInternal(true);
  ret += "}\n";
  return ret;
}

struct Set::Iterator::IteratorPrivate : public KShared {
  std::list<const SetNode*> nodeStack;
  Index currentIndex;

  /**
   * Pushes the noed on top of the stack, changes currentIndex, and goes as deep as necessary for iteration.
   * */
  void startAtNode(const SetNode* node) {
    currentIndex = node->start;

    do {
      nodeStack.push_back(node);
      if(node->contiguous)
        break; //We need no finer granularity, because the range is contiguous
      node = node->left.data();
    } while(node);
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
  d = new IteratorPrivate(*rhs.d);
  return *this;
}

Set::Iterator::Iterator() : d(new IteratorPrivate) {
}

Set::Iterator::~Iterator() {
}

Set::Iterator::operator bool() const {
  return !d->nodeStack.empty();
}

Set::Iterator& Set::Iterator::operator++() {
  Q_ASSERT(!d->nodeStack.empty());
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
      Q_ASSERT(d->nodeStack.back()->left->end == d->currentIndex);
      d->startAtNode(d->nodeStack.back()->right.data());
    }
  }
  return *this;
}

BasicSetRepository::Index Set::Iterator::operator*() const {
  return d->currentIndex;
}

Set::Iterator Set::iterator() const {
  Set::Iterator ret;
  if(d->m_tree)
    ret.d->startAtNode(d->m_tree.data());
  return ret;
}

///The bounds must already be represented in the repository, which means that there must be sets separated by these bounds.
SetNode* applyBounds(SetNode* node, Index lowerBound, Index upperBound)
{
  if(node->start >= lowerBound && node->end <= upperBound)
    return node;
  if(node->start >= upperBound || node->end <= lowerBound)
    return 0;
  
  if(node->left->end <= lowerBound) //left node is completely out of the bounds, continue with the right
    return applyBounds(node->right.data(), lowerBound, upperBound);
  
  if(node->right->start >= upperBound) //right node is completely out of the bounds, continue with the left
    return applyBounds(node->left.data(), lowerBound, upperBound);

  SetNode* left = applyBounds(node->left.data(), lowerBound, upperBound);
  SetNode* right = applyBounds(node->right.data(), lowerBound, upperBound);
  if(!left)
    return right;
  else if(!right)
    return left;
  
  SetNode* set(new SetNode);
  set->left = SetNode::Ptr(left);
  set->right = SetNode::Ptr(right);
  set->inRepository = false;

  if(!set->left->contiguous || !set->right->contiguous)
    set->contiguous = false;
  else
    set->contiguous = set->left->end == set->right->start;

  set->start = set->left->start;
  set->end = set->right->end;

  ifDebug( set->check() );
  Q_ASSERT(set->end <= upperBound);
  Q_ASSERT(set->start >= lowerBound);
  return set;
}

SetNode* set_union(SetNode* first, SetNode* second, Index lowerBound = 0, Index upperBound = std::numeric_limits<Index>::max())
{
  if(first == second || (first->start == second->start && first->end == second->end && first->contiguous && second->contiguous))
    return applyBounds(first, lowerBound, upperBound);
  
  Index firstStart = first->start, secondEnd=second->end;

  if(firstStart >= upperBound)
    return applyBounds(second, lowerBound, upperBound);
  
  if(firstStart >= secondEnd)
  {
    SetNode* left = applyBounds(second, lowerBound, upperBound);
    SetNode* right = applyBounds(first, lowerBound, upperBound);
    if(!left)
      return right;
    else if(!right)
      return left;
    
    SetNode* set(new SetNode);
    set->left = SetNode::Ptr(left);
    set->right = SetNode::Ptr(right);
    set->inRepository = false;

    if(!set->left->contiguous || !set->right->contiguous)
      set->contiguous = false;
    else
      set->contiguous = set->left->end == set->right->start;

    set->start = set->left->start;
    set->end = set->right->end;

    ifDebug( set->check() );
    Q_ASSERT(set->end <= upperBound);
    Q_ASSERT(set->start >= lowerBound);
    return set;
  }
  
  Index secondStart = second->start, firstEnd = first->end;

  if(firstEnd <= lowerBound)
    return applyBounds(second, lowerBound, upperBound);
  
  if(secondStart >= firstEnd)
  {
    SetNode* left = applyBounds(first, lowerBound, upperBound);
    SetNode* right = applyBounds(second, lowerBound, upperBound);
    if(!left)
      return right;
    else if(!right)
      return left;
    
    SetNode* set(new SetNode);
    set->left = SetNode::Ptr(left);
    set->right = SetNode::Ptr(right);
    set->inRepository = false;

    if(!set->left->contiguous || !set->right->contiguous)
      set->contiguous = false;
    else
      set->contiguous = set->left->end == set->right->start;

    set->start = set->left->start;
    set->end = set->right->end;

    ifDebug( set->check() );
    Q_ASSERT(set->end <= upperBound);
    Q_ASSERT(set->start >= lowerBound);
    return set;
  }
  
  //first and second intersect.

  SetNode* leftUnion;
  SetNode* rightUnion;
  //Always split up the one side that is bigger, so we have a better chance that shared nodes meet each other.
  if(firstEnd - firstStart < secondEnd - secondStart)
  {
    SetNode* left = second->left.data(), *right = second->right.data();
    leftUnion = set_union(first, left, lowerBound, left->end < upperBound ? left->end : upperBound);
    rightUnion = set_union(first, right, left->end > lowerBound ? left->end : lowerBound, upperBound);
    if(leftUnion == left && rightUnion == right)
      return second;
  } else {
    SetNode* left = first->left.data(), *right = first->right.data();
    leftUnion = set_union(second, left, lowerBound, left->end < upperBound ? left->end : upperBound);
    rightUnion = set_union(second, right, left->end > lowerBound ? left->end : lowerBound, upperBound);
    if(leftUnion == left && rightUnion == right)
      return first;
  }

  if(leftUnion && !rightUnion)
    return leftUnion;
  else if(rightUnion && !leftUnion)
    return rightUnion;
  else if(!leftUnion && !rightUnion)
    return 0;

  SetNode* set(new SetNode);
  set->inRepository = false;
  set->left = leftUnion;
  set->right = rightUnion;

  if(!set->left->contiguous || !set->right->contiguous)
    set->contiguous = false;
  else
    set->contiguous = set->left->end == set->right->start;

  set->start = set->left->start;
  set->end = set->right->end;

  ifDebug( set->check() );
  Q_ASSERT(set->end <= upperBound);
  Q_ASSERT(set->start >= lowerBound);
  return set;
}

SetNode* set_intersect(SetNode* first, SetNode* second)
{
  Index firstStart = first->start, secondEnd=second->end;
  if(firstStart >= secondEnd)
    return 0;
  
  Index secondStart = second->start, firstEnd = first->end;
  if(secondStart >= firstEnd)
    return 0;
  
  if(first == second)
    return first;
  
  //first and second intersect.

  SetNode* splitLeftIntersection;
  SetNode* splitRightIntersection;
  //Always split up the one side that is bigger, so we have a better chance that shared nodes meet each other.
  if(firstEnd-firstStart < secondEnd - secondStart)
  {
    SetNode* left = second->left.data(), *right = second->right.data();
    splitLeftIntersection = set_intersect(first, left);
    splitRightIntersection = set_intersect(first, right);
    if(splitLeftIntersection == left && splitRightIntersection == right)
      return second;
  } else {
    SetNode* left = first->left.data(), *right = first->right.data();
    splitLeftIntersection = set_intersect(second, left);
    splitRightIntersection = set_intersect(second, right);
    if(splitLeftIntersection == left && splitRightIntersection == right)
      return first;
  }
  
  if(splitLeftIntersection && splitRightIntersection)
  {
    SetNode* set(new SetNode);
    set->inRepository = false;
    set->left = splitLeftIntersection;
    set->right = splitRightIntersection;

    if(!set->left->contiguous || !set->right->contiguous)
      set->contiguous = false;
    else
      set->contiguous = set->left->end == set->right->start;

    set->start = set->left->start;
    set->end = set->right->end;

    ifDebug( set->check() );
    return set;
  }else if(splitLeftIntersection) {
    return splitLeftIntersection;
  }else{
    return splitRightIntersection;
  }
  
  return 0;
}

SetNode* set_subtract(SetNode* first, SetNode* second)
{
  Index firstStart = first->start, secondEnd=second->end;
  if(firstStart >= secondEnd)
    return first;
  
  Index secondStart = second->start, firstEnd = first->end;
  if(secondStart >= firstEnd)
    return first;
  
  if(first == second)
    return 0;
  
  //first and second intersect.

  SetNode* splitLeftSubtraction;
  SetNode* splitRightSubtraction;
  //Always split up the one side that is bigger, so we have a better chance that shared nodes meet each other.
  if(firstEnd-firstStart < secondEnd - secondStart)
  {
    SetNode* temp = set_subtract(first, second->left.data());
    if(!temp)
      return 0;
    return set_subtract( temp, second->right.data());
  } else {
    SetNode* left = first->left.data(), *right = first->right.data();
    splitLeftSubtraction = set_subtract(left, second);
    splitRightSubtraction = set_subtract(right, second);
    if(splitLeftSubtraction == left && splitRightSubtraction == right)
      return first;
  }

  if(splitLeftSubtraction && splitRightSubtraction)
  {
    SetNode* set(new SetNode);
    set->inRepository = false;
    set->left = splitLeftSubtraction;
    set->right = splitRightSubtraction;

    if(!set->left->contiguous || !set->right->contiguous)
      set->contiguous = false;
    else
      set->contiguous = set->left->end == set->right->start;

    set->start = set->left->start;
    set->end = set->right->end;

    ifDebug( set->check() );
    return set;
  }else if(splitLeftSubtraction) {
    return splitLeftSubtraction;
  }else{
    return splitRightSubtraction;
  }
  
  return 0;
}

/**
 * Creates a binary out-of-repository tree containing the given nodes.
 * @param start A start-iterator of a container that contains SetNode* values, sorted by their index-ranges.
 * @param end The end-iterator.
 * @param count Must either be the exact count of items between start and end, or zero.
*/
template<class Iterator>
SetNode::Ptr createBinaryTree(Iterator start, Iterator end, int count = 0) {

  if(!count)
  {
    for(Iterator counter = start; counter != end; ++counter)
      ++count;
  }

  Q_ASSERT(count);
  if(count == 1)
    return SetNode::Ptr(*start);
  Q_ASSERT(count >= 2);

  SetNode::Ptr set(new SetNode);
  set->inRepository = false;

  //Put he first half to the left side
  Iterator split = start;
  for(int a = 0; a < count/2; a++)
    ++split;
  
  set->left = createBinaryTree(start, split, count/2);
  set->right = createBinaryTree(split, end, count - count/2);

  if(!set->left->contiguous || !set->right->contiguous)
    set->contiguous = false;
  else
    set->contiguous = set->left->end == set->right->start;

  set->start = set->left->start;
  set->end = set->right->end;

  ifDebug( set->localCheck() );
  
  return set;
}

Set BasicSetRepository::createSet(const std::vector<Index>& indices) {
  Index lastIndex = 0;
  std::vector<SetNode*> nodes;

  SetNode* searchNode = d->m_root.data();
  ifDebug(searchNode->check());
  
  for(std::vector<Index>::const_iterator it = indices.begin(); it != indices.end(); ++it) {
    Q_ASSERT(*it >= lastIndex);
    Index start = *it;

    ++it;
    Q_ASSERT(it != indices.end());
    Q_ASSERT(*it >= lastIndex);
    Index end = *it;
    lastIndex = end;

    //Move up in the tree to the first node that contains start
    while(searchNode->end <= start)
      searchNode = searchNode->parentInRepository;

    ifDebug(searchNode->localCheck();)
    
    //Find the bottom node that contains this range
    //findContainer could be used here, but the recursion is too slow.
    
    //SetNode* searchNode = d->m_root.data();
    {
      SetNode* searchNodeLeft = searchNode->left.data();
      while(searchNodeLeft) {
        if(start >= searchNodeLeft->start && start < searchNodeLeft->end)
        {
          searchNode = searchNodeLeft;
          searchNodeLeft = searchNode->left.data();
        } else {
          SetNode* searchNodeRight = searchNode->right.data();
          if(start >= searchNodeRight->start && start < searchNodeRight->end) {
            searchNode = searchNodeRight;
            searchNodeLeft = searchNode->left.data();
          }else{
            //No node found that contains the position
            Q_ASSERT(0);
          }
        }
      }
    }
    
    //Walk through the nodes, using the fact that they are too ordered by indices
    Q_ASSERT(searchNode);
    Q_ASSERT(searchNode->contains(start));

    //Intersection of the range with searchNode
    SetNode* searchNodeIntersection = 0;
    
    if((searchNode->end == end && searchNode->start == start) || (searchNode->end < end && searchNode->start == start)) {
      //searchNode is completely contained by the searched range
      searchNodeIntersection = searchNode;
    }

    if(!searchNodeIntersection) {
      Q_ASSERT(start != searchNode->start || end != searchNode->end);
      searchNodeIntersection = searchNode->createIntersection(start, end);
    }
    Q_ASSERT(searchNodeIntersection);

    //Try to find a parent-node as big as possible that is covered by this range
    while(searchNodeIntersection->parentInRepository &&
          searchNodeIntersection->parentInRepository->start == start &&
          searchNodeIntersection->parentInRepository->end <= end)
      searchNodeIntersection = searchNodeIntersection->parentInRepository;

    Q_ASSERT(searchNodeIntersection->start == start && searchNodeIntersection->end <= end);
    //We've got out first node for this range.
    nodes.push_back(searchNodeIntersection);

    if(searchNodeIntersection->end < end) {
      //Find the nodes that represent the part searchNodeIntersection->end -> end
      Q_ASSERT(searchNodeIntersection->parentInRepository);
      SetNode* previousNode = searchNodeIntersection;
      SetNode* currentNode = searchNodeIntersection->parentInRepository;

      //Seek upwards
      while(!currentNode->contains(end-1))
      {
        Q_ASSERT(currentNode->parentInRepository);
        
        if(previousNode != currentNode->right.data()) //currentNode->right represents previousNode->end -> currentNode->end
        {
          //If we are skipping a node, completely add it.
          Q_ASSERT(previousNode == currentNode->left.data());
          nodes.push_back(currentNode->right.data());
        }

        previousNode = currentNode;
        currentNode = currentNode->parentInRepository;
      }

      Q_ASSERT(currentNode->left == previousNode);
      
      if(currentNode->end == end) {
        nodes.push_back(currentNode->right.data());
      } else {
        //We have found a big node that contains end, now seek down to find the exact bottom-node.
        currentNode = currentNode->right.data();
        
        while(currentNode->hasSlaves()) //The bottom-node has no left
        {
          if(currentNode->left->contains(end-1)) {
            currentNode = currentNode->left.data();
          }else{
            nodes.push_back(currentNode->left.data()); //Collect all skipped nodes. They are represented by this range.
            currentNode = currentNode->right.data();
          }
        }
        nodes.push_back(currentNode->createIntersection(start, end));
      }
    }
  }

  ///nodes now contains all repository-nodes contained by the given ranges.
  ///Now place a hull of non-repository SetNodes over the nodes.
  Set ret;
  if(!nodes.empty())
    ret.d->m_tree = createBinaryTree(nodes.begin(), nodes.end(), nodes.size());
  ifDebug(d->m_root->check();)
  return ret;
}

Set BasicSetRepository::createSet(Index i) {
  std::vector<Index> ranges;
  ranges.push_back(i);
  ranges.push_back(i+1);
  return createSet(ranges);
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

  return createSet(ranges);
}

BasicSetRepository::BasicSetRepository() : d(new Private) {
}

BasicSetRepository::~BasicSetRepository() {
  delete d;
}

Index BasicSetRepository::appendIndices(int count) {
  return d->m_root->appendIndices(d->m_root, count);
}

QString BasicSetRepository::dumpDotGraph() const {
  if(d->m_root)
    return d->m_root->dumpDotGraph();
  else
    return QString::null;
}

Set Set::operator +(const Set& first) const
{
  if(!first.d->m_tree)
    return *this;
  else if(!d->m_tree)
    return first;
  Set ret;
  ret.d->m_tree = SetNode::Ptr(set_union(d->m_tree.data(), first.d->m_tree.data()));
  
  ifDebug(if(ret.d->m_tree)ret.d->m_tree->check());
  return ret;
}

Set& Set::operator +=(const Set& first) {
  if(!first.d->m_tree)
    return *this;
  else if(!d->m_tree) {
    d->m_tree = first.d->m_tree;
    return *this;
  }
  SetNode::Ptr newTree(set_union(d->m_tree.data(), first.d->m_tree.data()));
  d->m_tree = newTree;
  ifDebug(if(d->m_tree)d->m_tree->check());
  return *this;
}

Set Set::operator &(const Set& first) const {
  if(!first.d->m_tree || !d->m_tree)
    return Set();
  Set ret;
  ret.d->m_tree = SetNode::Ptr(set_intersect(d->m_tree.data(), first.d->m_tree.data()));
  ifDebug(if(ret.d->m_tree)ret.d->m_tree->check());
  return ret;
}

Set& Set::operator &=(const Set& first) {
  if(!first.d->m_tree || !d->m_tree) {
    d->m_tree = SetNode::Ptr();
    return *this;
  }
  
  SetNode::Ptr newTree = SetNode::Ptr(set_intersect(d->m_tree.data(), first.d->m_tree.data()));
  d->m_tree = newTree;
  ifDebug(if(d->m_tree)d->m_tree->check());
  return *this;
}

Set Set::operator -(const Set& rhs) const {
  if(!d->m_tree || !rhs.d->m_tree)
    return *this;

  Set ret;
  ret.d->m_tree = SetNode::Ptr(set_subtract(d->m_tree.data(), rhs.d->m_tree.data()));
  ifDebug(if(ret.d->m_tree)ret.d->m_tree->check());
  return ret;
}

Set& Set::operator -=(const Set& rhs) {
  if(!d->m_tree || !rhs.d->m_tree)
    return *this;

  SetNode::Ptr newTree = SetNode::Ptr(set_subtract(d->m_tree.data(), rhs.d->m_tree.data()));

  d->m_tree = newTree;
  ifDebug(if(d->m_tree)d->m_tree->check());
  return *this;
}
}

