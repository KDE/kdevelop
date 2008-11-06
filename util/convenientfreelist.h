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

#ifndef CONVENIENTFREELIST_H
#define CONVENIENTFREELIST_H

#include "embeddedfreetree.h"
#include <qvector.h>
#include "kdevvarlengtharray.h"
#include <qpair.h>
#include <kdebug.h>

namespace KDevelop {
    
    template<class Data, class Handler>
    class ConvenientEmbeddedSetIterator;
    template<class Data, class Handler, class Data2, class Handler2, class KeyExtractor>
    class ConvenientEmbeddedSetFilterIterator;
    
    ///A convenience-class for accessing the data in a set managed by the EmbeddedFreeTree algorithms.
    template<class Data, class Handler>
    class ConstantConvenientEmbeddedSet {
        public:
            ConstantConvenientEmbeddedSet() : m_data(0), m_dataSize(0), m_centralFreeItem(-1) {
            }
            ConstantConvenientEmbeddedSet(const Data* data, uint count, int centralFreeItem) : m_data(data), m_dataSize(count), m_centralFreeItem(centralFreeItem) {
            }
            
            ///Returns whether the item is contained in this set
            bool contains(const Data& data) const {
                return indexOf(data) != -1;
            }
            
            ///Returns the position of the item in the underlying array, or -1 if it is not contained
            int indexOf(const Data& data) const {
                EmbeddedTreeAlgorithms<Data, Handler> alg(m_data, m_dataSize, m_centralFreeItem);
                return alg.indexOf(data);
            }
            
            ///Returns the size of the underlying array
            uint dataSize() const {
                return m_dataSize;
            }
            
            ///Returns the underlying array. That array may contain invalid/free items.
            const Data* data() const {
                return m_data;
            }
            
            ///Returns the first valid index that has a data-value larger or equal to @param data.
            ///Returns -1 if nothing is found.
            int lowerBound(const Data& data) const {
                EmbeddedTreeAlgorithms<Data, Handler> alg(m_data, m_dataSize, m_centralFreeItem);
                return alg.lowerBound(data, 0, m_dataSize);
            }

            ///Returns the first valid index that has a data-value larger or equal to @param data,
            ///and that is in the range [start, end).
            ///Returns -1 if nothing is found.
            int lowerBound(const Data& data, uint start, uint end) const {
                EmbeddedTreeAlgorithms<Data, Handler> alg(m_data, m_dataSize, m_centralFreeItem);
                return alg.lowerBound(data, start, end);
            }

            ///Finds a valid most central in the range [start, end).
            ///Returns -1 if no such item exists.
            int validMiddle(uint start, uint end) {
                if(end <= start)
                    return -1;
                
                int firstTry = ((end-start)/2) + start;
                
                int thisTry = firstTry;
                while(thisTry < (int)end && Handler::isFree(m_data[thisTry]))
                    ++thisTry;
                
                if(thisTry != (int)end)
                    return thisTry;
                
                //Nothing find on right side of middle, try the other direction
                thisTry = firstTry-1;
                while(thisTry >= (int)start && Handler::isFree(m_data[thisTry]))
                    --thisTry;
                
                if(thisTry >= (int)start)
                    return thisTry;
                else
                    return -1;
            }
            
            int nextValidItem(int pos) {
                for(; pos < (int)m_dataSize; ++pos)
                    if(!Handler::isFree(m_data[pos]))
                        return pos;

                return -1;
            }
            
            typedef ConvenientEmbeddedSetIterator<Data, Handler> Iterator;
            
            ConvenientEmbeddedSetIterator<Data, Handler> iterator() const;
            
//         protected:
            const Data* m_data;
            uint m_dataSize;
            int m_centralFreeItem;
    };
    
    ///Convenient iterator that automatically skips invalid/free items in the array.
    template<class Data, class Handler>
    class ConvenientEmbeddedSetIterator : public ConstantConvenientEmbeddedSet<Data, Handler> {
        public:
        ConvenientEmbeddedSetIterator(const Data* data = 0, uint count = 0, int centralFreeItem = -1) : ConstantConvenientEmbeddedSet<Data, Handler>(data, count, centralFreeItem), m_pos(0) {
            //Move to first valid position
            moveToValid();
        }
        
        ///Returns true of this iterator has a value to return
        operator bool() const {
            return m_pos != this->m_dataSize;
        }
        
        const Data* operator->() const {
            return &this->m_data[m_pos];
        }
        
        const Data& operator*() const {
            return this->m_data[m_pos];
        }
        
        ConvenientEmbeddedSetIterator& operator++() {
            ++m_pos;
            moveToValid();
            return *this;
        }
        
        protected:
            inline void moveToValid() {
                while(this->m_pos < this->m_dataSize && (Handler::isFree(this->m_data[this->m_pos])))
                    ++this->m_pos;
            }
            uint m_pos;
    };
    
    
    ///An iterator that allows matching between two lists with different data type.
    ///Important: It must be possible to extract the data-type of the second list from this list,
    ///and the this list must also be primarily sorted by that data. However the list is allowed to
    ///be sub-ordered by something else, and multiple items are allowed to match one item in the second.
    ///This iterator iterates through all items in this list that have extracted key-data that is in represented in the second.
    template<class Data, class Handler, class Data2, class Handler2, class KeyExtractor>
    class ConvenientEmbeddedSetFilterIterator : public ConvenientEmbeddedSetIterator<Data, Handler> {
        public:
        ConvenientEmbeddedSetFilterIterator() : m_match(-1) {
        }
        ConvenientEmbeddedSetFilterIterator(const ConvenientEmbeddedSetIterator<Data, Handler>& base, const ConvenientEmbeddedSetIterator<Data2, Handler2>& rhs) : ConvenientEmbeddedSetIterator<Data, Handler>(base), m_rhs(rhs), m_match(-1) {
            boundStack.append( qMakePair( qMakePair(0u, this->m_dataSize), qMakePair(0u, rhs.m_dataSize) ) );
            go();
        }
        
        operator bool() const {
            return m_match != -1;
        }
        
        const Data* operator->() const {
            Q_ASSERT(m_match != -1);
            return &this->m_data[m_match];
        }

        const Data& operator*() const {
            Q_ASSERT(m_match != -1);
            return this->m_data[m_match];
        }
        
        ConvenientEmbeddedSetFilterIterator& operator++() {
            Q_ASSERT(m_match != -1);
            go();
            return *this;
        }
        #define CHECK_BOUNDS Q_ASSERT(boundStack.back().first.first < 100000 && boundStack.back().first.second < 10000  && boundStack.back().second.first < 100000 && boundStack.back().second.second < 10000 );

        private:
        void go() {
            m_match = -1;
            
            boundsUp:
            if(boundStack.isEmpty())
                return;
            CHECK_BOUNDS
            QPair<QPair<uint, uint>, QPair<uint, uint> > currentBounds = boundStack.back();
            boundStack.pop_back();

            uint ownStart = currentBounds.first.first, ownEnd = currentBounds.first.second;
            uint rhsStart = currentBounds.second.first, rhsEnd = currentBounds.second.second;

            if(ownStart >= ownEnd)
                goto boundsUp;
            if(rhsStart >= rhsEnd)
                goto boundsUp;
            
            while(true) {
                if(ownStart == ownEnd)
                    goto boundsUp;
                
                int ownMiddle = this->validMiddle(ownStart, ownEnd);
                Q_ASSERT(ownMiddle < 100000);
                if(ownMiddle == -1)
                    goto boundsUp; //No valid items in the range
                
                Data2 currentData2 = extractData2(this->m_data[ownMiddle]);
                Q_ASSERT(!Handler2::isFree(currentData2));
                
                int bound = m_rhs.lowerBound(currentData2, rhsStart, rhsEnd);
                if(bound == -1) {
                    //Release second half of the own range
//                     Q_ASSERT(ownEnd > ownMiddle);
                    ownEnd = ownMiddle;
                    continue;
                }

                if(currentData2 == m_rhs.m_data[bound]) {
                    //We have a match
                    this->m_match = ownMiddle;
                    //Append the ranges that need to be matched next, without the matched item
                    boundStack.append( qMakePair( qMakePair( (uint)ownMiddle+1, ownEnd ), qMakePair((uint)bound, rhsEnd)) );
                    if(ownMiddle)
                        boundStack.append( qMakePair( qMakePair( ownStart, (uint)ownMiddle ), qMakePair(rhsStart, (uint)bound+1)) );
                    return;
                }

                if(bound == m_rhs.nextValidItem(rhsStart)) {
                    //The bound is the first valid item of the second range.
                    //Discard left side and the matched left item, and continue.
                    
                    ownStart = ownMiddle+1;
                    rhsStart = bound;
                    continue;
                }
                
                //Standard: Split both sides into 2 ranges that will be checked next
                boundStack.append( qMakePair( qMakePair( (uint)ownMiddle+1, ownEnd ), qMakePair((uint)bound, rhsEnd)) );
//                 Q_ASSERT(ownMiddle <= ownEnd);
                ownEnd = ownMiddle; //We loose the item at 'middle' here, but that's fine, since it hasn't found a match.
                rhsEnd = bound+1;
            }
        }
        Data2 extractData2(const Data& data) const {
            return KeyExtractor::extract(data);
        }
        //Bounds that yet need to be matched.
        KDevVarLengthArray<QPair<QPair<uint, uint>, QPair<uint, uint> > > boundStack;
        ConvenientEmbeddedSetIterator<Data2, Handler2> m_rhs;
        int m_match;
    };    
    
    template<class Data, class Handler>
    ConvenientEmbeddedSetIterator<Data, Handler> ConstantConvenientEmbeddedSet<Data, Handler>::iterator() const {
        return ConvenientEmbeddedSetIterator<Data, Handler>(m_data, m_dataSize, m_centralFreeItem);
    }

    ///This is a simple set implementation based on the embedded free tree algorithms.
    ///The core advantage of the whole thing is that the wole set is represented by a consecutive
    ///memory-area, and thus can be stored or copied using a simple memcpy.
    ///However in many cases it's better using the algorithms directly in such cases.
    ///
    ///However even for normal tasks this implementation does have some advantages over std::set:
    ///- Many times faster iteration through contained data
    ///- Lower memory-usage if the objects are small, since there is no heap allocation overhead
    ///- Can be combined with other embedded-free-list based sets using algorithms in ConstantConvenientEmbeddedSet
    ///Disadvantages:
    ///- Significantly slower insertion
    
    template<class Data, class Handler>
    class ConvenientFreeListSet {
        public:
            
            typedef ConvenientEmbeddedSetIterator<Data, Handler> Iterator;
            
            ConvenientFreeListSet() : m_centralFree(-1) {
            }
            
            ///Re-construct a set from its components
            ConvenientFreeListSet(int centralFreeItem, QVector<Data> data) : m_data(data), m_centralFree(centralFreeItem) {
            }
            
            ///You can use this to store the set to disk and later give it together with data() to the constructor,  thus reconstructing it.
            int centralFreeItem() const {
                return m_centralFree;
            }
            
            const QVector<Data>& data() const {
                return m_data;
            }
            
            void insert(const Data& item) {
                if(contains(item))
                    return;
                KDevelop::EmbeddedTreeAddItem<Data, Handler> add(m_data.data(), m_data.size(), m_centralFree, item);
                
                if((int)add.newItemCount() != (int)m_data.size()) {
                    QVector<Data> newData;
                    newData.resize(add.newItemCount());
                    add.transferData(newData.data(), newData.size());
                    m_data = newData;
                }
            }
            
            Iterator iterator() const {
                return Iterator(m_data.data(), m_data.size(), m_centralFree);
            }
            
            bool contains(const Data& item) const {
                KDevelop::EmbeddedTreeAlgorithms<Data, Handler> alg(m_data.data(), m_data.size(), m_centralFree);
                return alg.indexOf(Data(item)) != -1;
            }
            
            void remove(const Data& item) {
                KDevelop::EmbeddedTreeRemoveItem<Data, Handler> remove(m_data.data(), m_data.size(), m_centralFree, item);
                
                if((int)remove.newItemCount() != (int)m_data.size()) {
                    QVector<Data> newData;
                    newData.resize(remove.newItemCount());
                    remove.transferData(newData.data(), newData.size());
                    m_data = newData;
                }
            }
                 
        private:
            int m_centralFree;
            QVector<Data> m_data;
    };
}

#endif
