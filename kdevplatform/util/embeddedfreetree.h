/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef EMBEDDED_FREE_TREE
#define EMBEDDED_FREE_TREE

#include "kdevvarlengtharray.h"

#include <QPair>

#include <iostream>
#include <stdlib.h>
#include <limits>

//Uncomment this to search for tree-inconsistencies, however it's very expensive
// #define DEBUG_FREEITEM_COUNT debugFreeItemCount(); verifyTreeConsistent(*m_centralFreeItem, 0, m_itemCount);
#define DEBUG_FREEITEM_COUNT

/**
 * This file implements algorithms that allow managing a sorted list of items, and managing "free" items
 * for reuse efficiently in that list. Among those free items a tree is built, and they are traversed
 * on insertion/removal to manage free items in the tree.
 *
 * There is specific needs on the embedded items:
 * - They must be markable "invalid", so after they are deleted they can stay in their place as invalid items.
 * - While they are invalid, they still must be able to hold 2 integers, needed for managing the tree of free items.
 * - One integer is needed for each list to hold a pointer to the central free item.
 *
 * Only these functions must be used to manipulate the lists, from the beginning up. First create an empty list
 * and initialize centralFreeItem with -1, and then you start adding items.
 *
 * Since the list is sorted, and each item can be contained only once, these lists actually represent a set.
 *
 * EmbeddedTreeAlgorithms implements an efficient "contains" function that uses binary search within the list.
 */

namespace KDevelop {
    ///Responsible for handling the items in the list
    ///This is an example. ItemHandler::rightChild(..) and ItemHandler::leftChild(..) must be values that must be able to hold the count of positive
    ///values that will be the maximum size of the list, and additionally -1.
//     template<class Data>
//     class ExampleItemHandler {
//         public:
//         ExampleItemHandler(const Data& data) : m_data(data) {
//         }
//         int ItemHandler::rightChild() const {
//             Q_ASSERT(0);
//         }
//         int ItemHandler::leftChild() const {
//             Q_ASSERT(0);
//         }
//         void ItemHandler::setLeftChild(int child) {
//             Q_ASSERT(0);
//         }
//         void setRightChild(int child) {
//             Q_ASSERT(0);
//         }
//         bool operator<(const StandardItemHandler& rhs) const {
//             Q_ASSERT(0);
//         }
//         //Copies this item into the given one
//         void copyTo(Data& data) const {
//             data = m_data;
//         }
//
//         static void createFreeItem(Data& data) {
//             data = Data();
//         }
//
//         bool isFree() const {
//             Q_ASSERT(0);
//         }
//
//         const Data& data() {
//         }
//
//         private:
//             const Data& m_data;
//     };

    /**
     * Use this for several constant algorithms on sorted lists with free-trees
     * */
    template<class Data, class ItemHandler>
    class EmbeddedTreeAlgorithms {

        public:

            EmbeddedTreeAlgorithms(const Data* items, uint itemCount, const int& centralFreeItem) : m_items(items), m_itemCount(itemCount), m_centralFreeItem(&centralFreeItem) {
            }
            ~EmbeddedTreeAlgorithms() {
            }

            ///Efficiently checks whether the item is contained in the set.
            ///If it is contained, returns the index. Else, returns -1.

            int indexOf(const Data& data) {
                return indexOf(data, 0, m_itemCount);
            }

            ///Searches the given item within the specified bounds.
            int indexOf(const Data& data, uint start, uint end) {
                while(1) {
                    if(start >= end)
                        return -1;

                    int center = (start + end)/2;

                    //Skip free items, since they cannot be used for ordering
                    for(; center < (int)end; ) {
                        if(!ItemHandler::isFree(m_items[center]))
                            break;
                        ++center;
                    }

                    if(center == (int)end) {
                        end = (start + end)/2; //No non-free items found in second half, so continue search in the other
                    }else{
                        if(ItemHandler::equals(data, m_items[center])) {
                            return center;
                        }else if(data < m_items[center]) {
                            end = (start + end)/2;
                        }else{
                            //Continue search in second half
                            start = center+1;
                        }
                    }
                }
            }

            ///Returns the first valid index that has a data-value larger or equal to @p data.
            ///Returns -1 if nothing is found.
            int lowerBound(const Data& data, int start, int end) {
                int currentBound = -1;
                while(1) {
                    if(start >= end)
                        return currentBound;

                    int center = (start + end)/2;

                    //Skip free items, since they cannot be used for ordering
                    for(; center < end; ) {
                        if(!ItemHandler::isFree(m_items[center]))
                            break;
                        ++center;
                    }

                    if(center == end) {
                        end = (start + end)/2; //No non-free items found in second half, so continue search in the other
                    }else{
                        if(ItemHandler::equals(data, m_items[center])) {
                            return center;
                        }else if(data < m_items[center]) {
                            currentBound = center;
                            end = (start + end)/2;
                        }else{
                            //Continue search in second half
                            start = center+1;
                        }
                    }
                }
            }

        uint countFreeItems() const {
            return countFreeItemsInternal(*m_centralFreeItem);
        }
        uint countFreeItemsNaive() const {
            uint ret = 0;
            for(uint a = 0; a < m_itemCount; ++a) {
                if(ItemHandler::isFree(m_items[a]))
                    ++ret;
            }
            return ret;
        }

        void verifyOrder() {
            Data last;

            for(uint a = 0; a < m_itemCount; ++a) {
                if(!ItemHandler::isFree(m_items[a])) {
                    if(!ItemHandler::isFree(last))
                        Q_ASSERT(last < m_items[a]);
                    last = m_items[a];
                }
            }
        }

        void verifyTreeConsistent() {
            verifyTreeConsistentInternal(*m_centralFreeItem, 0, m_itemCount);
            Q_ASSERT(countFreeItems() == countFreeItemsNaive());
        }

        private:
        void verifyTreeConsistentInternal(int position, int lowerBound, int upperBound) {
            if(position == -1)
                return;
            Q_ASSERT(lowerBound <= position && position < upperBound);
            verifyTreeConsistentInternal(ItemHandler::leftChild(m_items[position]), lowerBound, position);
            verifyTreeConsistentInternal(ItemHandler::rightChild(m_items[position]), position+1, upperBound);
        }

        uint countFreeItemsInternal(int item) const {
            if(item == -1)
                return 0;

            return 1 + countFreeItemsInternal(ItemHandler::leftChild(m_items[item])) + countFreeItemsInternal(ItemHandler::rightChild(m_items[item]));
        }

           const Data* m_items;
           uint m_itemCount;
           const int* m_centralFreeItem;
    };

    /**Use this to add items.
     * The added item must not be in the set yet!
     * General usage:
     * - Construct the object
     * - Check if newItemCount() equals the previous item-count. If not, construct
     *   a new list as large as newItemCount, and call object.transferData to transfer the data
     *   into the new list. The new size must match the returned newItemCount.
     * - Either call object.apply(), or let it be called automatically by the destructor.
     * @tparam increaseFraction By what fraction the list is increased when it needs to. For example the size will be increased by 1/5 if it's 5.
     * @tparam rebuildIfInsertionMoreExpensive The structure is rebuilt completely when an insertion needs a moving around of more than rebuildIfInsertionMoreExpensive times
                                              the count of items needed to be moved in worst case in a fresh tree.
     *                                          After rebuilding the tree, the free space is evenly distributed, and thus insertions require much less moving.
     * */
    template<class Data, class ItemHandler, int increaseFraction = 5, int rebuildIfInsertionMoreExpensive = 20>
    class EmbeddedTreeAddItem {

        public:

            EmbeddedTreeAddItem(Data* items, uint itemCount, int& centralFreeItem, const Data& add) : m_add(add), m_items(items), m_itemCount(itemCount), m_centralFreeItem(&centralFreeItem), m_applied(false), m_needResize(false) {
                m_needResize = !apply();
            }
            ~EmbeddedTreeAddItem() {
                if(!m_applied)
                    apply(true);
            }

            ///Check this to see whether a new item-count is needed. If this does not equal the given itemCount, then
            ///the data needs to be transferred into a new list using transferData
            uint newItemCount() const {
                if(!m_applied) {
                    if(*m_centralFreeItem == -1) {
                        uint realItemCount = m_itemCount - countFreeItems(*m_centralFreeItem);
                        uint newItemCount = realItemCount + (realItemCount/increaseFraction);
                        if(newItemCount <= m_itemCount)
                            newItemCount = m_itemCount+1;

                        return newItemCount;
                    }else if(m_needResize) {
                        uint realItemCount = m_itemCount - countFreeItems(*m_centralFreeItem);
                        uint newItemCount = realItemCount + (realItemCount/increaseFraction);

                        return newItemCount;
                    }
                }
                return m_itemCount;
            }

            ///Transfers the data into a new item-list. The size of the new item-list must equal newItemCount()
            void transferData(Data* newItems, uint newCount, int* newCentralFree = nullptr) {
                DEBUG_FREEITEM_COUNT

                uint currentRealCount = m_itemCount - countFreeItems(*m_centralFreeItem);
//                 Q_ASSERT(currentRealCount + (currentRealCount/increaseFraction) == newCount);

                //Create a new list where the items from m_items are put into newItems, with the free items evenly
                //distributed, and a clean balanced free-tree.
                uint newFreeCount = newCount - currentRealCount;
                volatile uint freeItemRaster;
                if(newFreeCount)
                    freeItemRaster = newCount / newFreeCount;
                else {
                    freeItemRaster = newCount+1; //No free items
                }

                ///@todo Do not iterate through all items, instead use the free-tree and memcpy for the ranges between free items.
                ///Ideally, even the free-tree would be built on-the-fly.
                Q_ASSERT(freeItemRaster);
                uint offset = 0;
                uint insertedValidCount = 0;
                for(uint a = 0; a < newCount; ++a) {
                    //Create new free items at the end of their raster range
                    if(a % freeItemRaster == (freeItemRaster-1)) {
                        //We need to insert a free item
                        ItemHandler::createFreeItem(newItems[a]);
                        ++offset;
                    }else{
                        ++insertedValidCount;
                        while(ItemHandler::isFree(m_items[a-offset]) && a-offset < m_itemCount)
                            --offset;
                        Q_ASSERT(a-offset < m_itemCount);
                        newItems[a] = m_items[a-offset];
//                         Q_ASSERT(!ItemHandler::isFree(newItems[a]));
                    }
                }
                Q_ASSERT(insertedValidCount == m_itemCount - countFreeItems(*m_centralFreeItem));
//                  qCDebug(UTIL) << m_itemCount << newCount << offset;
//                 Q_ASSERT(m_itemCount == newCount-offset);

                m_items = newItems;
                m_itemCount = newCount;

                if(newCentralFree)
                    m_centralFreeItem  = newCentralFree;

                *m_centralFreeItem = buildFreeTree(newFreeCount, freeItemRaster, freeItemRaster-1);

//                 qCDebug(UTIL) << "count of new free items:" << newFreeCount;

//                 Q_ASSERT(countFreeItems( *m_centralFreeItem ) == newFreeCount);

                DEBUG_FREEITEM_COUNT
            }

           ///Tries to put the item into the list. If the insertion would be too inefficient or is not possible, returns false, unless @param force is true
           bool apply(bool force = false) {
               if(m_applied)
                   return true;

               if(*m_centralFreeItem == -1) {
                   Q_ASSERT(!force);
                   return false;
               }

               //Find the free item that is nearest to the target position in the item order
               int previousItem = -1;
               int currentItem = *m_centralFreeItem;
               int replaceCurrentWith = -1;

               //In currentLowerBound and currentUpperBound, we count the smallest contiguous range between free
               //items that the added items needs to be sorted into. If the range is empty, the item can just be inserted.
               int currentLowerBound = 0;
               int currentUpperBound = m_itemCount;

               //Now go down the chain, always into the items direction

               while(1) {
                   QPair<int, int> freeBounds = leftAndRightRealItems(currentItem);
                   const Data& current(m_items[currentItem]);
                   if(freeBounds.first != -1 && m_add < m_items[freeBounds.first]) {
                       //Follow left child
                       currentUpperBound = freeBounds.first+1;

                       if(ItemHandler::leftChild(current) != -1) {
                           //Continue traversing
                           previousItem = currentItem;
                           currentItem = ItemHandler::leftChild(current);
                       }else{
                           replaceCurrentWith = ItemHandler::rightChild(current);
                           break;
                       }
                   }else if(freeBounds.second != -1 && m_items[freeBounds.second] < m_add) {
                       //Follow right child
                       currentLowerBound = freeBounds.second;

                       if(ItemHandler::rightChild(current) != -1) {
                           //Continue traversing
                           previousItem = currentItem;
                           currentItem = ItemHandler::rightChild(current);
                       }else{
                           replaceCurrentWith = ItemHandler::leftChild(current);
                           break;
                       }
                   }else{
                       //We will use this item! So find a replacement for it in the tree, and update the structure
                       force = true;
                       currentLowerBound = currentUpperBound = currentItem;

                       int leftReplaceCandidate = -1, rightReplaceCandidate = -1;
                       if(ItemHandler::leftChild(current) != -1)
                           leftReplaceCandidate = rightMostChild(ItemHandler::leftChild(current));
                       if(ItemHandler::rightChild(current) != -1)
                           rightReplaceCandidate = leftMostChild(ItemHandler::rightChild(current));

                       ///@todo it's probably better using lowerBound and upperBound like in the "remove" version
                       //Left and right bounds of all children of current
                       int leftChildBound = leftMostChild(currentItem), rightChildBound = rightMostChild(currentItem);
                       Q_ASSERT(leftChildBound != -1 && rightChildBound != -1);
                       int childCenter = (leftChildBound + rightChildBound)/2;

                       if(leftReplaceCandidate == -1 && rightReplaceCandidate == -1) {
                          //We don't have a replace candidate, since there is no children
                          Q_ASSERT(ItemHandler::leftChild(current) == -1);
                          Q_ASSERT(ItemHandler::rightChild(current) == -1);
                       }else if(rightReplaceCandidate == -1 || abs(leftReplaceCandidate - childCenter) < abs(rightReplaceCandidate - childCenter)) {
                           //pick the left replacement, since it's more central
                           Q_ASSERT(leftReplaceCandidate != -1);
                           replaceCurrentWith = leftReplaceCandidate;

                           Data& replaceWith(m_items[replaceCurrentWith]);

                           if(replaceCurrentWith == ItemHandler::leftChild(current)) {
                               //The left child of replaceWith can just stay as it is, and we just need to add the right child
                               Q_ASSERT(ItemHandler::rightChild(replaceWith) == -1);
                           }else{
                            takeRightMostChild(ItemHandler::leftChild(current));

                            //Since we'll be clearing the item, we have to put this childsomewhere else.
                            // Either make it our new "left" child, or make it the new left children "rightmost" child.
                            int addRightMostLeftChild = ItemHandler::leftChild(replaceWith);

                            ItemHandler::setLeftChild(replaceWith, -1);

                            Q_ASSERT(ItemHandler::leftChild(replaceWith) == -1);
                            Q_ASSERT(ItemHandler::rightChild(replaceWith) == -1);

                            if(ItemHandler::leftChild(current) != -1)
                            {
                                Q_ASSERT(rightMostChild(ItemHandler::leftChild(current)) != replaceCurrentWith);
                                Q_ASSERT(ItemHandler::leftChild(current) == -1 || ItemHandler::leftChild(current) < replaceCurrentWith);
                                ItemHandler::setLeftChild(replaceWith, ItemHandler::leftChild(current));

                                if(addRightMostLeftChild != -1) {
                                    int rightMostLeft = rightMostChild(ItemHandler::leftChild(replaceWith));
                                    Q_ASSERT(rightMostLeft != -1);
//                                     Q_ASSERT(item(rightMostLeft).ItemHandler::rightChild() == -1);
                                    Q_ASSERT(rightMostLeft < addRightMostLeftChild);
                                    ItemHandler::setRightChild(m_items[rightMostLeft], addRightMostLeftChild);
                                }
                            }else{
                                Q_ASSERT(addRightMostLeftChild == -1 || addRightMostLeftChild < replaceCurrentWith);
                                ItemHandler::setLeftChild(replaceWith, addRightMostLeftChild);
                            }
                           }

                           Q_ASSERT(ItemHandler::rightChild(current) == -1 || replaceCurrentWith < ItemHandler::rightChild(current));
                           ItemHandler::setRightChild(replaceWith, ItemHandler::rightChild(current));
                       }else{
                           //pick the right replacement, since it's more central
                           Q_ASSERT(rightReplaceCandidate != -1);
                           replaceCurrentWith = rightReplaceCandidate;

                           Data& replaceWith(m_items[replaceCurrentWith]);

                           if(replaceCurrentWith == ItemHandler::rightChild(current)) {
                               //The right child of replaceWith can just stay as it is, and we just need to add the left child
                               Q_ASSERT(ItemHandler::leftChild(replaceWith) == -1);
                           }else{
                            takeLeftMostChild(ItemHandler::rightChild(current));

                            //Since we'll be clearing the item, we have to put this childsomewhere else.
                            // Either make it our new "right" child, or make it the new right children "leftmost" child.
                            int addLeftMostRightChild = ItemHandler::rightChild(replaceWith);

                            ItemHandler::setRightChild(replaceWith, -1);

                            Q_ASSERT(ItemHandler::rightChild(replaceWith) == -1);
                            Q_ASSERT(ItemHandler::leftChild(replaceWith) == -1);

                            if(ItemHandler::rightChild(current) != -1)
                            {
                                Q_ASSERT(leftMostChild(ItemHandler::rightChild(current)) != replaceCurrentWith);
                                Q_ASSERT(ItemHandler::rightChild(current) == -1 || replaceCurrentWith < ItemHandler::rightChild(current));
                                ItemHandler::setRightChild(replaceWith, ItemHandler::rightChild(current));

                                if(addLeftMostRightChild != -1) {
                                    int leftMostRight = leftMostChild(ItemHandler::rightChild(replaceWith));
                                    Q_ASSERT(leftMostRight != -1);
                                    Q_ASSERT(ItemHandler::leftChild(m_items[leftMostRight]) == -1);
                                    Q_ASSERT(addLeftMostRightChild < leftMostRight);
                                    ItemHandler::setLeftChild(m_items[leftMostRight], addLeftMostRightChild);
                                }
                            }else{
                                Q_ASSERT(addLeftMostRightChild == -1 || replaceCurrentWith < addLeftMostRightChild);
                                ItemHandler::setRightChild(replaceWith, addLeftMostRightChild);
                            }
                           }

                           Q_ASSERT(ItemHandler::leftChild(current) == -1 || ItemHandler::leftChild(current) < replaceCurrentWith);
                           ItemHandler::setLeftChild(replaceWith, ItemHandler::leftChild(current));
                       }
                       break;
                   }
               }

               //We can insert now
               //currentItem and previousItem are the two items that best enclose the target item

//                for(int a = currentLowerBound; a < currentUpperBound; ++a) {
//                        Q_ASSERT(!ItemHandler::isFree(m_items[a]));
//                }

               Q_ASSERT(currentItem < currentLowerBound || currentItem >= currentUpperBound);

               //If the current item is on a border of the bounds, it needs to be inserted in the right position.
               //Else, the current position already is right, and we only need to copy it in.
               if(currentLowerBound < currentUpperBound && (currentItem == currentLowerBound-1 || currentItem == currentUpperBound)) {
                 if(!insertSorted(m_add, currentItem, currentLowerBound, currentUpperBound, force)) {
                     return false;
                 }
               }else{
                 ItemHandler::copyTo(m_add, m_items[currentItem]);
               }

               m_applied = true;

               //First, take currentItem out of the chain, by replacing it with current.rightChild in the parent
               if(previousItem != -1) {
                   Data& previous(m_items[previousItem]);
                    if(ItemHandler::leftChild(previous) == currentItem) {
                        Q_ASSERT(replaceCurrentWith == -1 || replaceCurrentWith < previousItem);
                        ItemHandler::setLeftChild(previous, replaceCurrentWith);
                    } else if(ItemHandler::rightChild(previous) == currentItem) {
                        Q_ASSERT(replaceCurrentWith == -1 || previousItem < replaceCurrentWith);
                        ItemHandler::setRightChild(previous, replaceCurrentWith);
                    } else {
                        Q_ASSERT(0);
                    }
               } else {
                   *m_centralFreeItem = replaceCurrentWith;
               }

               return true;

               DEBUG_FREEITEM_COUNT
           }

        private:
           void verifyTreeConsistent(int position, int lowerBound, int upperBound) {
               if(position == -1)
                   return;
               Q_ASSERT(lowerBound <= position && position < upperBound);
               verifyTreeConsistent(ItemHandler::leftChild(m_items[position]), lowerBound, position);
               verifyTreeConsistent(ItemHandler::rightChild(m_items[position]), position+1, upperBound);
           }

            void debugFreeItemCount() {
                uint count = 0;
                for(uint a = 0; a < m_itemCount; ++a) {
                    if(isFree(m_items[a]))
                        ++count;
                }
                uint counted = countFreeItems(*m_centralFreeItem);
                Q_ASSERT(count == counted);
                Q_UNUSED(counted);
            }

            QPair<int, int> leftAndRightRealItems(int pos) {
                Q_ASSERT(ItemHandler::isFree(m_items[pos]));
                int left = -1, right = -1;
                for(int a = pos-1; a >= 0; --a) {
                    if(!ItemHandler::isFree(m_items[a])) {
                        left = a;
                        break;
                    }
                }
                for(uint a = pos+1; a < m_itemCount; ++a) {
                    if(!ItemHandler::isFree(m_items[a])) {
                        right = a;
                        break;
                    }
                }
                return qMakePair(left, right);
            }

            int buildFreeTree(int count, uint raster, int start) {
                Q_ASSERT((start % raster) == (raster-1));
                Q_ASSERT(count != 0);
                Q_ASSERT(count <= (int)m_itemCount);
                if(count == 1) {
                    ItemHandler::createFreeItem(m_items[start]);
                    return start;
                }else{
                    int central = start + (count / 2) * raster;
                    int leftCount = count / 2;
                    int midCount = 1;
                    int rightCount = count - leftCount - midCount;
                    Q_ASSERT(leftCount + midCount <= count);
                    ItemHandler::createFreeItem(m_items[central]);
                    Q_ASSERT(ItemHandler::isFree(m_items[central]));

                    int leftFreeTree = buildFreeTree(leftCount, raster, start);
                    Q_ASSERT(leftFreeTree == -1 || leftFreeTree < central);
                    ItemHandler::setLeftChild(m_items[central],  leftFreeTree );

                    if(rightCount > 0) {
                        int rightFreeTree = buildFreeTree(rightCount, raster, central+raster);
                        Q_ASSERT(rightFreeTree == -1 || central < rightFreeTree);
                        ItemHandler::setRightChild(m_items[central], rightFreeTree );
                    }

                    return central;
                }
            }

            uint countFreeItems(int item) const {
                if(item == -1)
                    return 0;
                const Data& current(m_items[item]);

                return 1 + countFreeItems(ItemHandler::leftChild(current)) + countFreeItems(ItemHandler::rightChild(current));
            }

           int leftMostChild(int pos) const {
               while(1) {
                   if(ItemHandler::leftChild(m_items[pos]) != -1)
                       pos = ItemHandler::leftChild(m_items[pos]);
                   else
                       return pos;
               }
           }

           int takeLeftMostChild(int pos) const {
               int parent = -1;
               while(1) {
                   if(ItemHandler::leftChild(m_items[pos]) != -1) {
                       parent = pos;
                       pos = ItemHandler::leftChild(m_items[pos]);
                   } else {
                       ItemHandler::setLeftChild(m_items[parent], -1);
                       return pos;
                   }
               }
           }

           int rightMostChild(int pos) const {
               while(1) {
                   if(ItemHandler::rightChild(m_items[pos]) != -1)
                       pos = ItemHandler::rightChild(m_items[pos]);
                   else
                       return pos;
               }
           }

           int takeRightMostChild(int pos) const {
               int parent = -1;
               while(1) {
                   if(ItemHandler::rightChild(m_items[pos]) != -1) {
                       parent = pos;
                       pos = ItemHandler::rightChild(m_items[pos]);
                   } else {
                       ItemHandler::setRightChild(m_items[parent], -1);
                       return pos;
                   }
               }
           }

           ///Maximum "moving" out of the way of items without forcing a complete rebuild of the list
           inline int maxMoveAround() const {
               return increaseFraction * rebuildIfInsertionMoreExpensive;
           }

           ///Inserts the given data item into position @p pos, and updates the sorting
           ///@param data can be another empty item, that together with @p pos represents the closest enclosure of the target position
           ///@return Whether the item could be inserted. It is not inserted if
           bool insertSorted(const Data& data, int pos, int start, int end, bool force) {

               if(pos < start)
                   start = pos;
               if(pos >= end)
                   end = pos+1;

/*               for(int a = start; a < end; ++a) {
                   if(a != pos) {
                       Q_ASSERT(!ItemHandler::isFree(m_items[a]));
                   }
               }*/
               EmbeddedTreeAlgorithms<Data, ItemHandler> alg(m_items, m_itemCount, *m_centralFreeItem);
               int bound = alg.lowerBound(data, start, end);
               //Now find the position that should be taken
               if(bound == -1)
                   bound = end;

               //Now our item should end up right before bound

               int target;
               //bound cannot be pos, because pos is invalid
               Q_ASSERT(bound != pos);

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 800)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
                //Shuffle around the item at the free pos, so reference counting in constructors/destructors is not screwed up
                char backup[sizeof(Data)];
                memcpy(backup, m_items+pos, sizeof(Data));

               if(bound < pos) {
                   if(!force && pos-bound > maxMoveAround()) {
//                         qCDebug(UTIL) << "increasing because" << pos-bound << ">" << maxMoveAround() << "left free items:" << countFreeItems(*m_centralFreeItem) << "target free items:" << (m_itemCount-countFreeItems(*m_centralFreeItem))/increaseFraction;
                       return false;
                   }
                   //Move [bound, pos) one to right, and insert at bound
                    memmove(m_items+bound+1, m_items+bound, sizeof(Data)*(pos-bound));
                    target = bound;
               }else {
                   Q_ASSERT(bound > pos);
                   if(!force && bound-pos-1 > maxMoveAround()) {
//                         qCDebug(UTIL) << "increasing because" << bound-pos-1 << ">" << maxMoveAround() << "left free items:" << countFreeItems(*m_centralFreeItem)<< "target free items:" << (m_itemCount-countFreeItems(*m_centralFreeItem))/increaseFraction;
                       return false;
                   }
                   //Move (pos, bound) one to left, and insert at bound-1
                    memmove(m_items+pos, m_items+pos+1, sizeof(Data)*(bound-pos-1));
                    target = bound-1;
               }
               memcpy(m_items+target, backup, sizeof(Data));
#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 800)
#pragma GCC diagnostic pop
#endif

               ItemHandler::copyTo(data, m_items[target]);
               return true;
           }

           const Data& m_add;
           Data* m_items;
           uint m_itemCount;
           int* m_centralFreeItem;
           bool m_applied, m_needResize;
    };

    /**Use this to add items.
     * The removed item must be in the set!
     * General usage:
     * - Construct the object
     * - Check if newItemCount() equals the previous item-count. If not, construct
     *   a new list as large as newItemCount, and call object.transferData to transfer the data
     *   into the new list. The new size must match the returned newItemCount.
     * However this may also be ignored if the memory-saving is not wanted in that moment.
     * */
    template<class Data, class ItemHandler, int increaseFraction = 5 >
    class EmbeddedTreeRemoveItem {

        public:

            EmbeddedTreeRemoveItem(Data* items, uint itemCount, int& centralFreeItem, const Data& remove) : m_remove(remove), m_items(items), m_itemCount(itemCount), m_centralFreeItem(&centralFreeItem), m_insertedAtDepth(0) {
                 apply();
            }

            ~EmbeddedTreeRemoveItem() {
            }

            ///Check this to see whether a new item-count is needed. If this does not equal the given itemCount, then
            ///the data needs to be transferred into a new list using transferData
            uint newItemCount() const {
                uint maxFreeItems = ((m_itemCount / increaseFraction)*3)/2 + 1;
                //First we approximate the count of free items using the insertion depth
                if((1u << m_insertedAtDepth) >= maxFreeItems) {
                    uint freeCount = countFreeItems(*m_centralFreeItem);
                    if(freeCount > maxFreeItems || freeCount == m_itemCount) {
                        return m_itemCount - freeCount;
                    }
                }

                return m_itemCount;
            }

            ///Transfers the data into a new item-list. The size of the new item-list must equal newItemCount()
            void transferData(Data* newItems, uint newCount, int* newCentralFree = nullptr) {
                DEBUG_FREEITEM_COUNT
                //We only transfer into a new list when all the free items are used up

                //Create a list where only the non-free items exist
                uint offset = 0;
                for(uint a = 0; a < m_itemCount; ++a) {
                    if(!ItemHandler::isFree(m_items[a])) {
                        newItems[offset] = m_items[a];
                        ++offset;
                    }
                }
                Q_ASSERT(offset == newCount);

                if(newCentralFree)
                    m_centralFreeItem = newCentralFree;
                *m_centralFreeItem = -1;
                m_items = newItems;
                m_itemCount = newCount;
                DEBUG_FREEITEM_COUNT
            }

        private:
           void verifyTreeConsistent(int position, int lowerBound, int upperBound) {
                if(position == -1)
                    return;
                Q_ASSERT(lowerBound <= position && position < upperBound);
                verifyTreeConsistent(ItemHandler::leftChild(m_items[position]), lowerBound, position);
                verifyTreeConsistent(ItemHandler::rightChild(m_items[position]), position+1, upperBound);
           }

            uint countFreeItems(int item) const {
                if(item == -1)
                    return 0;
                const Data& current(m_items[item]);

                return 1 + countFreeItems(ItemHandler::leftChild(current)) + countFreeItems(ItemHandler::rightChild(current));
            }

            int findItem(const Data& data, uint start, uint end) {
                EmbeddedTreeAlgorithms<Data, ItemHandler> alg(m_items, m_itemCount, *m_centralFreeItem);
                return alg.indexOf(data, start, end);
            }

           void apply() {
               DEBUG_FREEITEM_COUNT

               int removeIndex = findItem(m_remove, 0, m_itemCount);
               Q_ASSERT(removeIndex != -1);
               Q_ASSERT(!ItemHandler::isFree(m_items[removeIndex]));

               //Find the free item that is nearest to the target position in the item order
               int currentItem = *m_centralFreeItem;

               int lowerBound = 0; //The minimum position the searched item can have
               int upperBound = m_itemCount; //The lowest known position the searched item can _not_ have

               if(*m_centralFreeItem == -1) {
                   *m_centralFreeItem = removeIndex;
                   Q_ASSERT(*m_centralFreeItem != -1);
                   ItemHandler::createFreeItem(m_items[*m_centralFreeItem]);
                   return;
               }

               //Now go down the chain, always into the items direction
               ///@todo make the structure better: Don't just put left/right child, but also swap when neede
               ///      to balance the tree
               while(1) {
                   Q_ASSERT(removeIndex != currentItem);
                   Data& current(m_items[currentItem]);
                   ++m_insertedAtDepth;
                   if(removeIndex < currentItem) {
                       upperBound = currentItem;
                       //Follow left child
                       if(ItemHandler::leftChild(current) != -1) {
                           //Continue traversing
                           currentItem = ItemHandler::leftChild(current);
                           Q_ASSERT(currentItem >= lowerBound && currentItem < upperBound);
                       }else{
                           //The to-be deleted item is before current, and can be added as left child to current
                           int item = findItem(m_remove, lowerBound, upperBound);
                           Q_ASSERT(item == removeIndex);
                           ItemHandler::createFreeItem(m_items[item]);
                           Q_ASSERT(item == -1 || item < currentItem);
                           ItemHandler::setLeftChild(current, item);
                           Q_ASSERT(item >= lowerBound && item < upperBound);
                           break;
                       }
                   }else{
                       lowerBound = currentItem+1;
                       //Follow right child
                       if(ItemHandler::rightChild(current) != -1) {
                           //Continue traversing
                           currentItem = ItemHandler::rightChild(current);
                           Q_ASSERT(currentItem >= lowerBound && currentItem < upperBound);
                       }else{
                           //The to-be deleted item is behind current, and can be added as right child to current
                           int item = findItem(m_remove, lowerBound, upperBound);
                           Q_ASSERT(item == removeIndex);
                           ItemHandler::createFreeItem(m_items[item]);
                           Q_ASSERT(item == -1 || currentItem < item);
                           ItemHandler::setRightChild(current, item);
                           Q_ASSERT(item >= lowerBound && item < upperBound);
                           break;
                       }
                   }
               }

               DEBUG_FREEITEM_COUNT
           }

           void debugFreeItemCount() {
               uint count = 0;
               for(uint a = 0; a < m_itemCount; ++a) {
                   if(ItemHandler::isFree(m_items[a]))
                       ++count;
               }
               uint counted = countFreeItems(*m_centralFreeItem);
               Q_ASSERT(count == counted);
               Q_UNUSED(counted);
           }

           const Data& m_remove;
           Data* m_items;
           uint m_itemCount;
           int* m_centralFreeItem;
           int m_insertedAtDepth;
    };
}

#endif
