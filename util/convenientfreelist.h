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

#ifndef KDEVPLATFORM_CONVENIENTFREELIST_H
#define KDEVPLATFORM_CONVENIENTFREELIST_H

#include <QtCore/QVector>
#include <QtCore/QPair>

#include "embeddedfreetree.h"
#include "kdevvarlengtharray.h"

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

            uint countFreeItems() {
                EmbeddedTreeAlgorithms<Data, Handler> alg(m_data, m_dataSize, m_centralFreeItem);
                return alg.countFreeItems();
            }

            void verify() {
                EmbeddedTreeAlgorithms<Data, Handler> alg(m_data, m_dataSize, m_centralFreeItem);
                alg.verifyTreeConsistent();
                alg.verifyOrder();
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

            ///Returns the first valid item in the range [pos, end), or -1
            int firstValidItem(int start, int end = -1) const {
                if(end == -1)
                    end = (int)m_dataSize;
                for(; start < end; ++start)
                    if(!Handler::isFree(m_data[start]))
                        return start;

                return -1;
            }

            ///Returns the last valid item in the range [pos, end), or -1
            int lastValidItem(int start = 0, int end = -1) const {
                if(end == -1)
                    end = (int)m_dataSize;
                --end;
                for(; end >= start; --end)
                    if(!Handler::isFree(m_data[end]))
                        return end;

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


    ///An iterator that allows efficient matching between two lists with different data type.
    ///Important: It must be possible to extract the data-type of the second list from the items in the first list
    ///The second list must be sorted by that data.
    ///The first list must primarily be sorted by that data, but is allowed to
    ///be sub-ordered by something else, and multiple items in the first list are allowed to match one item in the second.
    ///This iterator iterates through all items in the first list that have extracted key-data that is in represented in the second.
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
#if 0
            //This code works, but it doesn't give a speedup
            int ownFirstValid = this->firstValidItem(ownStart, ownEnd), ownLastValid = this->lastValidItem(ownStart, ownEnd);
            int rhsFirstValid = m_rhs.firstValidItem(rhsStart, rhsEnd), rhsLastValid = m_rhs.lastValidItem(rhsStart, rhsEnd);

            if(ownFirstValid == -1 || rhsFirstValid == -1)
                goto boundsUp;


            Data2 ownFirstValidData = KeyExtractor::extract(this->m_data[ownFirstValid]);
            Data2 ownLastValidData = KeyExtractor::extract(this->m_data[ownLastValid]);

            Data2 commonStart = ownFirstValidData;
            Data2 commonLast = ownLastValidData; //commonLast is also still valid

            if(commonStart < m_rhs.m_data[rhsFirstValid])
                commonStart = m_rhs.m_data[rhsFirstValid];

            if(m_rhs.m_data[rhsLastValid] < commonLast)
                commonLast = m_rhs.m_data[rhsLastValid];

            if(commonLast < commonStart)
                goto boundsUp;
#endif

            while(true) {
                if(ownStart == ownEnd)
                    goto boundsUp;

                int ownMiddle = this->validMiddle(ownStart, ownEnd);
                Q_ASSERT(ownMiddle < 100000);
                if(ownMiddle == -1)
                    goto boundsUp; //No valid items in the range

                Data2 currentData2 = KeyExtractor::extract(this->m_data[ownMiddle]);
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

                if(bound == m_rhs.firstValidItem(rhsStart)) {
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

        //Bounds that yet need to be matched.
        KDevVarLengthArray<QPair<QPair<uint, uint>, QPair<uint, uint> > > boundStack;
        ConvenientEmbeddedSetIterator<Data2, Handler2> m_rhs;
        int m_match;
    };

    ///Filters a list-embedded set by a binary tree set as managed by the SetRepository data structures
    template<class Data, class Handler, class Data2, class TreeSet, class KeyExtractor>
    class ConvenientEmbeddedSetTreeFilterIterator : public ConvenientEmbeddedSetIterator<Data, Handler> {
        public:
        ConvenientEmbeddedSetTreeFilterIterator() : m_match(-1) {
        }
        ///@param noFiltering whether the given input is pre-filtered. If this is true, base will be iterated without skipping any items.
        ConvenientEmbeddedSetTreeFilterIterator(const ConvenientEmbeddedSetIterator<Data, Handler>& base, const TreeSet& rhs, bool noFiltering = false) : ConvenientEmbeddedSetIterator<Data, Handler>(base), m_rhs(rhs), m_match(-1), m_noFiltering(noFiltering) {
            if(rhs.node().isValid()) {
                //Correctly initialize the initial bounds
                int ownStart = lowerBound(rhs.node().firstItem(), 0, this->m_dataSize);
                if(ownStart == -1)
                    return;
                int ownEnd = lowerBound(rhs.node().lastItem(), ownStart, this->m_dataSize);
                if(ownEnd == -1)
                    ownEnd = this->m_dataSize;
                else
                    ownEnd += 1;
                boundStack.append( qMakePair( qMakePair((uint)ownStart, (uint)ownEnd), rhs.node() ) );
            }
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

        ConvenientEmbeddedSetTreeFilterIterator& operator++() {
            Q_ASSERT(m_match != -1);
            go();
            return *this;
        }
        #define CHECK_BOUNDS Q_ASSERT(boundStack.back().first.first < 100000 && boundStack.back().first.second < 10000  && boundStack.back().second.first < 100000 && boundStack.back().second.second < 10000 );

        private:
        void go() {
            if(m_noFiltering) {
                ++m_match;
                if((uint)m_match >= this->m_dataSize)
                    m_match = -1;
                return;
            }

            if(m_match != -1) {
                //Match multiple items in this list to one in the tree
                m_match = this->firstValidItem(m_match+1, this->m_dataSize);
                if(m_match != -1 && KeyExtractor::extract(this->m_data[m_match]) == m_matchingTo)
                    return;
            }
            m_match = -1;

            boundsUp:
            if(boundStack.isEmpty())
                return;
            QPair<QPair<uint, uint>, typename TreeSet::Node > currentBounds = boundStack.back();
            boundStack.pop_back();

            uint ownStart = currentBounds.first.first, ownEnd = currentBounds.first.second;
            typename TreeSet::Node currentNode = currentBounds.second;

            if(ownStart >= ownEnd)
                goto boundsUp;
            if(!currentNode.isValid())
                goto boundsUp;

            while(true) {
                if(ownStart == ownEnd)
                    goto boundsUp;

                if(currentNode.isFinalNode()) {
//                      qCDebug(UTIL) << ownStart << ownEnd << "final node" << currentNode.start() * extractor_div_with << currentNode.end() * extractor_div_with;
                    //Check whether the item is contained
                    int bound = lowerBound(*currentNode, ownStart, ownEnd);
//                      qCDebug(UTIL) << "bound:" << bound << (KeyExtractor::extract(this->m_data[bound]) == *currentNode);
                    if(bound != -1 && KeyExtractor::extract(this->m_data[bound]) == *currentNode) {
                        //Got a match
                        m_match = bound;
                        m_matchingTo = *currentNode;
                        m_matchBound = ownEnd;
                        return;
                    }else{
                        //Mismatch
                        goto boundsUp;
                    }
                }else{
//                     qCDebug(UTIL) << ownStart << ownEnd << "node" << currentNode.start() * extractor_div_with << currentNode.end() * extractor_div_with;
                    //This is not a final node, split up the search into the sub-nodes
                    typename TreeSet::Node leftNode = currentNode.leftChild();
                    typename TreeSet::Node rightNode = currentNode.rightChild();
                    Q_ASSERT(leftNode.isValid());
                    Q_ASSERT(rightNode.isValid());


                    Data2 leftLastItem = leftNode.lastItem();

                    int rightSearchStart = lowerBound(rightNode.firstItem(), ownStart, ownEnd);
                    if(rightSearchStart == -1)
                        rightSearchStart = ownEnd;
                    int leftSearchLast = lowerBound(leftLastItem, ownStart, rightSearchStart != -1 ? rightSearchStart : ownEnd);
                    if(leftSearchLast == -1)
                        leftSearchLast = rightSearchStart-1;

                    bool recurseLeft = false;
                    if(leftSearchLast > (int)ownStart) {
                        recurseLeft = true; //There must be something in the range ownStart -> leftSearchLast that matches the range
                    }else if((int)ownStart == leftSearchLast) {
                        //Check if the one item item under leftSearchStart is contained in the range
                        Data2 leftFoundStartData = KeyExtractor::extract(this->m_data[ownStart]);
                        recurseLeft = leftFoundStartData < leftLastItem || leftFoundStartData == leftLastItem;
                    }

                    bool recurseRight = false;
                    if(rightSearchStart < (int)ownEnd)
                        recurseRight = true;

                    if(recurseLeft && recurseRight) {
                        //Push the right branch onto the stack, and work in the left one
                        boundStack.append( qMakePair( qMakePair( (uint)rightSearchStart, ownEnd ), rightNode) );
                    }

                    if(recurseLeft) {
                        currentNode = leftNode;
                        if(leftSearchLast != -1)
                            ownEnd = leftSearchLast+1;
                    }else if(recurseRight) {
                        currentNode = rightNode;
                        ownStart = rightSearchStart;
                    }else{
                        goto boundsUp;
                    }
                }
            }
        }

        ///Returns the first valid index that has an extracted data-value larger or equal to @param data.
        ///Returns -1 if nothing is found.
        int lowerBound(const Data2& data, int start, int end) {
            int currentBound = -1;
            while(1) {
                if(start >= end)
                    return currentBound;

                int center = (start + end)/2;

                //Skip free items, since they cannot be used for ordering
                for(; center < end; ) {
                    if(!Handler::isFree(this->m_data[center]))
                        break;
                    ++center;
                }

                if(center == end) {
                    end = (start + end)/2; //No non-free items found in second half, so continue search in the other
                }else{
                    Data2 centerData = KeyExtractor::extract(this->m_data[center]);
                    //Even if the data equals we must continue searching to the left, since there may be multiple matching
                    if(data == centerData || data < centerData) {
                        currentBound = center;
                        end = (start + end)/2;
                    }else{
                        //Continue search in second half
                        start = center+1;
                    }
                }
            }
        }

        //Bounds that yet need to be matched. Always a range in the own vector, and a node that all items in the range are contained in
        KDevVarLengthArray<QPair<QPair<uint, uint>, typename TreeSet::Node > > boundStack;
        TreeSet m_rhs;
        int m_match, m_matchBound;
        Data2 m_matchingTo;
        bool m_noFiltering;
    };

    ///Same as above, except that it visits all filtered items with a visitor, instead of iterating over them.
    ///This is more efficient. The visiting is done directly from within the constructor.
    template<class Data, class Handler, class Data2, class TreeSet, class KeyExtractor, class Visitor>
    class ConvenientEmbeddedSetTreeFilterVisitor : public ConvenientEmbeddedSetIterator<Data, Handler> {
        public:
        ConvenientEmbeddedSetTreeFilterVisitor() {
        }

        typedef QPair<QPair<uint, uint>, typename TreeSet::Node > Bounds;

        struct Bound {
            inline Bound(uint s, uint e, const typename TreeSet::Node& n) : start(s), end(e), node(n) {
            }
            Bound() {
            }
            uint start;
            uint end;
            typename TreeSet::Node node;
        };

        ///@param noFiltering whether the given input is pre-filtered. If this is true, base will be iterated without skipping any items.
        ConvenientEmbeddedSetTreeFilterVisitor(Visitor& visitor, const ConvenientEmbeddedSetIterator<Data, Handler>& base, const TreeSet& rhs, bool noFiltering = false) : ConvenientEmbeddedSetIterator<Data, Handler>(base), m_visitor(visitor), m_rhs(rhs), m_noFiltering(noFiltering) {

            if(m_noFiltering) {
                for(uint a = 0; a < this->m_dataSize; ++a)
                    visitor(this->m_data[a]);
                return;
            }

            if(rhs.node().isValid())  {
                //Correctly initialize the initial bounds
                int ownStart = lowerBound(rhs.node().firstItem(), 0, this->m_dataSize);
                if(ownStart == -1)
                    return;
                int ownEnd = lowerBound(rhs.node().lastItem(), ownStart, this->m_dataSize);
                if(ownEnd == -1)
                    ownEnd = this->m_dataSize;
                else
                    ownEnd += 1;

                go( Bound((uint)ownStart, (uint)ownEnd, rhs.node()) );
            }
        }

        private:
        void go( Bound bound ) {

            KDevVarLengthArray<Bound> bounds;

            while(true) {
                if(bound.start >= bound.end)
                    goto nextBound;

                if(bound.node.isFinalNode()) {
                    //Check whether the item is contained
                    int b = lowerBound(*bound.node, bound.start, bound.end);
                    if(b != -1) {
                        const Data2& matchTo(*bound.node);

                        if(KeyExtractor::extract(this->m_data[b]) == matchTo) {
                            while(1) {
                                m_visitor(this->m_data[b]);
                                b = this->firstValidItem(b+1, this->m_dataSize);
                                if(b < (int)this->m_dataSize && b != -1 && KeyExtractor::extract(this->m_data[b]) == matchTo)
                                    continue;
                                else
                                    break;
                            }
                        }
                    }
                    goto nextBound;
                }else{
                    //This is not a final node, split up the search into the sub-nodes
                    typename TreeSet::Node leftNode = bound.node.leftChild();
                    typename TreeSet::Node rightNode = bound.node.rightChild();
                    Q_ASSERT(leftNode.isValid());
                    Q_ASSERT(rightNode.isValid());


                    Data2 leftLastItem = leftNode.lastItem();

                    int rightSearchStart = lowerBound(rightNode.firstItem(), bound.start, bound.end);
                    if(rightSearchStart == -1)
                        rightSearchStart = bound.end;
                    int leftSearchLast = lowerBound(leftLastItem, bound.start, rightSearchStart != -1 ? rightSearchStart : bound.end);
                    if(leftSearchLast == -1)
                        leftSearchLast = rightSearchStart-1;

                    bool recurseLeft = false;
                    if(leftSearchLast > (int)bound.start) {
                        recurseLeft = true; //There must be something in the range bound.start -> leftSearchLast that matches the range
                    }else if((int)bound.start == leftSearchLast) {
                        //Check if the one item item under leftSearchStart is contained in the range
                        Data2 leftFoundStartData = KeyExtractor::extract(this->m_data[bound.start]);
                        recurseLeft = leftFoundStartData < leftLastItem || leftFoundStartData == leftLastItem;
                    }

                    bool recurseRight = false;
                    if(rightSearchStart < (int)bound.end)
                        recurseRight = true;

                    if(recurseLeft && recurseRight)
                        bounds.append( Bound(rightSearchStart, bound.end, rightNode) );

                    if(recurseLeft) {
                        bound.node = leftNode;
                        if(leftSearchLast != -1)
                            bound.end = leftSearchLast+1;
                    }else if(recurseRight) {
                        bound.node = rightNode;
                        bound.start = rightSearchStart;
                    }else{
                        goto nextBound;
                    }
                    continue;
                }
                nextBound:
                    if(bounds.isEmpty()) {
                        return;
                    }else{
                        bound = bounds.back();
                        bounds.pop_back();
                    }
            }
        }

        ///Returns the first valid index that has an extracted data-value larger or equal to @param data.
        ///Returns -1 if nothing is found.
        int lowerBound(const Data2& data, int start, int end) {
            int currentBound = -1;
            while(1) {
                if(start >= end)
                    return currentBound;

                int center = (start + end)/2;

                //Skip free items, since they cannot be used for ordering
                for(; center < end; ) {
                    if(!Handler::isFree(this->m_data[center]))
                        break;
                    ++center;
                }

                if(center == end) {
                    end = (start + end)/2; //No non-free items found in second half, so continue search in the other
                }else{
                    Data2 centerData = KeyExtractor::extract(this->m_data[center]);
                    //Even if the data equals we must continue searching to the left, since there may be multiple matching
                    if(data == centerData || data < centerData) {
                        currentBound = center;
                        end = (start + end)/2;
                    }else{
                        //Continue search in second half
                        start = center+1;
                    }
                }
            }
        }

        //Bounds that yet need to be matched. Always a range in the own vector, and a node that all items in the range are contained in
        Visitor& m_visitor;
        TreeSet m_rhs;
        bool m_noFiltering;
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
