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

#include <sys/types.h>
#include <limits>
#include <stdlib.h>

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
    ///@todo Better dynamic rebalancing the tree
    
    ///Responsible for handling the items in the list
//     template<class Data>
//     class ExampleItemHandler {
//         public:
//         ExampleItemHandler(const Data& data) : m_data(data) {
//         }
//         int rightChild() const {
//             Q_ASSERT(0);
//         }
//         int leftChild() const {
//             Q_ASSERT(0);
//         }
//         void setLeftChild(int child) {
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

    ///Use this for several constant algorithms on sorted lists with free-trees
    template<class Data, class ItemHandler>
    class EmbeddedTreeAlgorithms {

        public:
            
            EmbeddedTreeAlgorithms(const Data* items, uint itemCount, const int& centralFreeItem) : m_items(items), m_itemCount(itemCount), m_centralFreeItem(centralFreeItem) {
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
                ItemHandler dataHandler(data);
                while(1) {
                    if(start == end)
                        return -1;
                    if(m_items[start] == data)
                        return start;
                    if(end-start == 1) //The range only consists of 'start'
                        return -1;
                    
                    //Skip free items, since they cannot be used for ordering
                    uint center = (start + end)/2;
                    for(; center < end; ) {
                        if(!ItemHandler(m_items[center]).isFree())
                            break;
                        ++center;
                    }
                    if(center == end) {
                        end = (start + end)/2; //No non-free items found in second half, so continue search in the other
                    }else{
                        if(dataHandler < ItemHandler(m_items[center])) {
                            end = (start + end)/2;
                        }else{
                            //Continue search in second half
                            start = center;
                        }
                    }
                }
            }
            
        uint countFreeItems() const {
            return countFreeItemsInternal(m_centralFreeItem);
        }

        private:
        uint countFreeItemsInternal(int item) const {
            if(item == -1)
                return 0;
            ItemHandler current(m_items[item]);
            
            return 1 + countFreeItemsInternal(current.leftChild()) + countFreeItemsInternal(current.rightChild());
        }

           const Data* m_items;
           uint m_itemCount;
           int m_centralFreeItem;
    };
    
    ///Use this to add items.
    ///The added item must not be in the list yet!
    template<class Data, class ItemHandler >
    class EmbeddedTreeAddItem {

        public:
            
            EmbeddedTreeAddItem(Data* items, uint itemCount, int& centralFreeItem, const Data& add) : m_add(add), m_items(items), m_itemCount(itemCount), m_centralFreeItem(centralFreeItem) {
            }
            ~EmbeddedTreeAddItem() {
                apply();
            }

            ///Check this to see whether a new item-count is needed. If this does not equal the given itemCount, then
            ///the data needs to be transferred into a new list using transferData
            uint newItemCount() const {
                if(m_centralFreeItem == -1) {
                    //We have to increase the size. Always increase by 10%.
                    uint newItemCount = m_itemCount + (m_itemCount/10);
                    if(newItemCount == m_itemCount)
                        ++newItemCount;
                    
                    return newItemCount;
                }else{
                    return m_itemCount;
                }
            }
            
            ///Transfers the data into a new item-list. The size of the new item-list must equal newItemCount()
            void transferData(Data* newItems, uint newCount) {
                //We only transfer into a new list when all the free items are used up
                Q_ASSERT(countFreeItems(m_centralFreeItem) == 0);
                
                //Create a new list where the items from m_items are put into newItems, with the free items evenly
                //distributed, and a clean balanced free-tree.
                uint newFreeCount = newCount - m_itemCount;
                volatile uint freeItemRaster = newCount / newFreeCount;
                Q_ASSERT(freeItemRaster);
                uint offset = 0;
                for(uint a = 0; a < newCount; ++a) {
                    //Create new free items at the end of their raster range
                    if(a % freeItemRaster == (freeItemRaster-1)) {
                        //We need to insert a free item
                        ItemHandler::createFreeItem(newItems[a]);
                        ++offset;
                    }else{
                        newItems[a] = m_items[a-offset];
                    }
                }
                Q_ASSERT(m_itemCount+offset == newCount);
                
                m_items = newItems;
                m_itemCount = newCount;

                m_centralFreeItem = buildFreeTree(newFreeCount, freeItemRaster, freeItemRaster-1);
            }
            
            
        private:
            QPair<int, int> leftAndRightRealItems(int pos) {
                Q_ASSERT(item(pos).isFree());
                int left = -1, right = -1;
                for(int a = pos-1; a >= 0; --a) {
                    if(!item(a).isFree()) {
                        left = a;
                        break;
                    }
                }
                for(uint a = pos+1; a < m_itemCount; ++a) {
                    if(!item(a).isFree()) {
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
                    uint central = start + (count / 2) * raster;
                    int leftCount = count / 2;
                    int midCount = 1;
                    int rightCount = count - leftCount - midCount;
                    Q_ASSERT(leftCount + midCount <= count);
                    ItemHandler::createFreeItem(m_items[central]);
                    Q_ASSERT(item(central).isFree());
                    
                    item(central).setLeftChild( buildFreeTree(leftCount, raster, start) );
                    
                    if(rightCount > 0)
                        item(central).setRightChild( buildFreeTree(rightCount, raster, central+raster) );
                    
                    return central;
                }
            }
            
            uint countFreeItems(int item) const {
                if(item == -1)
                    return 0;
                ItemHandler current(m_items[item]);
                
                return 1 + countFreeItems(current.leftChild()) + countFreeItems(current.rightChild());
            }
            
            ItemHandler item(uint number) const {
                return ItemHandler(m_items[number]);
            }
            
           void apply() {
               Q_ASSERT(m_centralFreeItem != -1);
               
               //Find the free item that is nearest to the target position in the item order
               int previousItem = -1;
               int currentItem = m_centralFreeItem;
               int replaceCurrentWith = -1;
               
               //Now go down the chain, always into the items direction
               
               while(1) {
                   QPair<int, int> freeBounds = leftAndRightRealItems(currentItem);
                   ItemHandler current(m_items[currentItem]);
                   if(freeBounds.first != -1 && m_add < item(freeBounds.first)) {
                       //Follow left child
                       if(current.leftChild() != -1) {
                           //Continue traversing
                           previousItem = currentItem;
                           currentItem = current.leftChild();
                       }else{
                           replaceCurrentWith = current.rightChild();
                           break;
                       }
                   }else if(freeBounds.second != -1 && item(freeBounds.second) < m_add) {
                       //Follow right child
                       if(current.rightChild() != -1) {
                           //Continue traversing
                           previousItem = currentItem;
                           currentItem = current.rightChild();
                       }else{
                           replaceCurrentWith = current.leftChild();
                           break;
                       }
                   }else{
                       //We will use this item! So find a replacement for it in the tree, and update the structure
                       
                       int leftReplaceCandidate = -1, rightReplaceCandidate = -1;
                       if(current.leftChild() != -1)
                           leftReplaceCandidate = rightMostChild(current.leftChild());
                       if(current.rightChild() != -1)
                           rightReplaceCandidate = leftMostChild(current.rightChild());
                       
                       ///@todo it's probably better using lowerBound and upperBound like in the "remove" version
                       //Left and right bounds of all children of current
                       int leftChildBound = leftMostChild(currentItem), rightChildBound = rightMostChild(currentItem);
                       Q_ASSERT(leftChildBound != -1 && rightChildBound != -1);
                       int childCenter = (leftChildBound + rightChildBound)/2;
                       
                       if(leftReplaceCandidate == -1 && rightReplaceCandidate == -1) {
                          //We don't have a replace candidate, since there is no children
                          Q_ASSERT(current.leftChild() == -1);
                          Q_ASSERT(current.rightChild() == -1);
                       }else if(rightReplaceCandidate == -1 || abs(leftReplaceCandidate - childCenter) < abs(rightReplaceCandidate - childCenter)) {
                           //pick the left replacement, since it's more central
                           Q_ASSERT(leftReplaceCandidate != -1);
                           replaceCurrentWith = leftReplaceCandidate;
                           
                           ItemHandler replaceWith(m_items[replaceCurrentWith]);
                           
                           if(replaceCurrentWith == current.leftChild()) {
                               //The left child of replaceWith can just stay as it is, and we just need to add the right child
                               Q_ASSERT(replaceWith.rightChild() == -1);
                           }else{
                            takeRightMostChild(current.leftChild());
                            
                            //Since we'll be clearing the item, we have to put this childsomewhere else. 
                            //Either make it our new "left" child, or make it the new left childs "rightmost" child.
                            int addRightMostLeftChild = replaceWith.leftChild();
                            
                            replaceWith.setLeftChild(-1);
                            
                            Q_ASSERT(replaceWith.leftChild() == -1);
                            Q_ASSERT(replaceWith.rightChild() == -1);
                            
                            if(current.leftChild() != -1)
                            {
                                Q_ASSERT(rightMostChild(current.leftChild()) != replaceCurrentWith);
                                replaceWith.setLeftChild(current.leftChild());
                                
                                if(addRightMostLeftChild != -1) {
                                    int rightMostLeft = rightMostChild(replaceWith.leftChild());
                                    Q_ASSERT(rightMostLeft != -1);
                                    Q_ASSERT(item(rightMostLeft).rightChild() == -1);
                                    item(rightMostLeft).setRightChild(addRightMostLeftChild);
                                }
                            }else{
                                replaceWith.setLeftChild(addRightMostLeftChild);
                            }
                           }
                           
                           replaceWith.setRightChild(current.rightChild());
                       }else{
                           //pick the right replacement, since it's more central
                           Q_ASSERT(rightReplaceCandidate != -1);
                           replaceCurrentWith = rightReplaceCandidate;
                           
                           ItemHandler replaceWith(m_items[replaceCurrentWith]);
                           
                           if(replaceCurrentWith == current.rightChild()) {
                               //The right child of replaceWith can just stay as it is, and we just need to add the left child
                               Q_ASSERT(replaceWith.leftChild() == -1);
                           }else{
                            takeLeftMostChild(current.rightChild());
                            
                            //Since we'll be clearing the item, we have to put this childsomewhere else. 
                            //Either make it our new "right" child, or make it the new right childs "leftmost" child.
                            int addLeftMostRightChild = replaceWith.rightChild();
                            
                            replaceWith.setRightChild(-1);
                            
                            Q_ASSERT(replaceWith.rightChild() == -1);
                            Q_ASSERT(replaceWith.leftChild() == -1);
                            
                            if(current.rightChild() != -1)
                            {
                                Q_ASSERT(leftMostChild(current.rightChild()) != replaceCurrentWith);
                                replaceWith.setRightChild(current.rightChild());
                                
                                if(addLeftMostRightChild != -1) {
                                    int leftMostRight = leftMostChild(replaceWith.rightChild());
                                    Q_ASSERT(leftMostRight != -1);
                                    Q_ASSERT(item(leftMostRight).leftChild() == -1);
                                    item(leftMostRight).setLeftChild(addLeftMostRightChild);
                                }
                            }else{
                                replaceWith.setRightChild(addLeftMostRightChild);
                            }
                           }
                           
                           replaceWith.setLeftChild(current.leftChild());
                       }
                       break;
                   }
               }
               
               //We can insert now
               //currentItem and previousItem are the two items that best enclose the target item
                
               //First, take currentItem out of the chain, by replacing it with current.rightChild in the parent
               if(previousItem != -1) {
                   ItemHandler previous(m_items[previousItem]);
                    if(previous.leftChild() == currentItem)
                        previous.setLeftChild(replaceCurrentWith);
                    else if(previous.rightChild() == currentItem)
                        previous.setRightChild(replaceCurrentWith);
                    else {
                        Q_ASSERT(0);
                    }
               } else {
                   m_centralFreeItem = replaceCurrentWith;
               }
               
               m_add.copyTo(m_items[currentItem]);
               updateSorting(currentItem);
           }
           
           int leftMostChild(int pos) const {
               while(1) {
                   ItemHandler handler(m_items[pos]);
                   if(handler.leftChild() != -1)
                       pos = handler.leftChild();
                   else
                       return pos;
               }
           }
           
           int takeLeftMostChild(int pos) const {
               int parent = -1;
               while(1) {
                   ItemHandler handler(m_items[pos]);
                   if(handler.leftChild() != -1) {
                       parent = pos;
                       pos = handler.leftChild();
                   } else {
                       ItemHandler parentHandler(m_items[parent]);
                       parentHandler.setLeftChild(-1);
                       return pos;
                   }
               }
           }

           int rightMostChild(int pos) const {
               while(1) {
                   ItemHandler handler(m_items[pos]);
                   if(handler.rightChild() != -1)
                       pos = handler.rightChild();
                   else
                       return pos;
               }
           }
           
           int takeRightMostChild(int pos) const {
               int parent = -1;
               while(1) {
                   ItemHandler handler(m_items[pos]);
                   if(handler.rightChild() != -1) {
                       parent = pos;
                       pos = handler.rightChild();
                   } else {
                       ItemHandler parentHandler(m_items[parent]);
                       parentHandler.setRightChild(-1);
                       return pos;
                   }
               }
           }

           //Updates the sorting for this item locally, using bubble-sort
           void updateSorting(int pos) {
               while(1) {
                int prev = pos-1;
                int next = pos+1;
                if(prev >= 0 && !item(prev).isFree() && item(pos) < item(prev)) {
                    Data backup(m_items[prev]);
                    m_items[prev] = m_items[pos];
                    m_items[pos] = backup;
                    pos = prev;
                }else if(next < (int)m_itemCount && !item(next).isFree() && item(next) < item(pos)) {
                    Data backup(m_items[next]);
                    m_items[next] = m_items[pos];
                    m_items[pos] = backup;
                    pos = next;
                }else{
                    break;
                }
               }
           }
           
           ItemHandler m_add;
           Data* m_items;
           uint m_itemCount;
           int& m_centralFreeItem;
    };

    ///The removed item must be in the list!
    template<class Data, class ItemHandler >
    class EmbeddedTreeRemoveItem {

        public:
            
            EmbeddedTreeRemoveItem(Data* items, uint itemCount, int& centralFreeItem, const Data& remove) : m_remove(remove), m_items(items), m_itemCount(itemCount), m_centralFreeItem(centralFreeItem) {
                 apply();
            }

            ~EmbeddedTreeRemoveItem() {
            }

            ///Check this to see whether a new item-count is needed. If this does not equal the given itemCount, then
            ///the data needs to be transferred into a new list using transferData
            uint newItemCount() const {
                uint freeCount = countFreeItems(m_centralFreeItem);
                if(freeCount > ((m_itemCount / 7)+1))
                    return m_itemCount - freeCount;
                else
                    return m_itemCount;
            }
            
            ///Transfers the data into a new item-list. The size of the new item-list must equal newItemCount()
            void transferData(Data* newItems, uint newCount) {
                //We only transfer into a new list when all the free items are used up
                
                //Create a list where only the non-free items exist
                uint offset = 0;
                for(uint a = 0; a < m_itemCount; ++a) {
                    if(!item(a).isFree()) {
                        newItems[offset] = m_items[a];
                        ++offset;
                    }
                }
                Q_ASSERT(offset == newCount);

                m_centralFreeItem = -1;
                m_items = newItems;
                m_itemCount = newCount;
            }
            
            
        private:
            
            uint countFreeItems(int item) const {
                if(item == -1)
                    return 0;
                ItemHandler current(m_items[item]);
                
                return 1 + countFreeItems(current.leftChild()) + countFreeItems(current.rightChild());
            }
            
            ItemHandler item(uint number) const {
                Q_ASSERT(number < m_itemCount);
                return ItemHandler(m_items[number]);
            }
            
            int findItem(const Data& data, uint start, uint end) {
                EmbeddedTreeAlgorithms<Data, ItemHandler> alg(m_items, m_itemCount, m_centralFreeItem);
                return alg.indexOf(data, start, end);
            }
            
           void apply() {
               int removeIndex = findItem(m_remove.item(), 0, m_itemCount);
               Q_ASSERT(removeIndex != -1);
               Q_ASSERT(!item(removeIndex).isFree());
               
               //Find the free item that is nearest to the target position in the item order
               int previousItem = -1;
               int currentItem = m_centralFreeItem;
               
               uint lowerBound = 0; //The minimum position the searched item can have
               uint upperBound = m_itemCount; //The lowest known position the searched item can _not_ have
               
               if(m_centralFreeItem == -1) {
                   m_centralFreeItem = removeIndex;
                   Q_ASSERT(m_centralFreeItem != -1);
                   ItemHandler::createFreeItem(m_items[m_centralFreeItem]);
                   return;
               }
               
               //Now go down the chain, always into the items direction
               ///@todo make the structure better: Don't just put left/right child, but also swap when neede
               ///      to balance the tree
               while(1) {
                   Q_ASSERT(removeIndex != currentItem);
                   ItemHandler current(m_items[currentItem]);
                   if(removeIndex < currentItem) {
                       upperBound = currentItem;
                       //Follow left child
                       if(current.leftChild() != -1) {
                           //Continue traversing
                           previousItem = currentItem;
                           currentItem = current.leftChild();
                       }else{
                           //The to-be deleted item is before current, and can be added as left child to current 
                           int item = findItem(m_remove.item(), lowerBound, upperBound);
                           Q_ASSERT(item == removeIndex);
                           ItemHandler::createFreeItem(m_items[item]);
                           current.setLeftChild(item);
                           break;
                       }
                   }else{
                       lowerBound = currentItem+1;
                       //Follow right child
                       if(current.rightChild() != -1) {
                           //Continue traversing
                           previousItem = currentItem;
                           currentItem = current.rightChild();
                       }else{
                           //The to-be deleted item is behind current, and can be added as right child to current 
                           int item = findItem(m_remove.item(), lowerBound, upperBound);
                           Q_ASSERT(item == removeIndex);
                           ItemHandler::createFreeItem(m_items[item]);
                           current.setRightChild(item);
                           break;
                       }
                   }
               }
           }
           
           //Updates the sorting for this item, using bubble-sort
           void updateSorting(int pos) {
               Q_ASSERT(0);
           }
           
           ItemHandler m_remove;
           Data* m_items;
           uint m_itemCount;
           int& m_centralFreeItem;
    };
};

#endif
