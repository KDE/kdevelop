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

namespace KDevelop {
    
    template<class Data, class Handler>
    class ConvenientEmbeddedSetIterator;
    
    ///A convenience-class for accessing the data in a set managed by the EmbeddedFreeTree algorithms.
    template<class Data, class Handler>
    class ConvenientEmbeddedSet {
        public:
            ConvenientEmbeddedSet(const Data* data, uint count, int centralFreeItem) : m_data(data), m_dataSize(count), m_centralFreeItem(centralFreeItem) {
            }
            
            ///Returns whether the item is contained in this set
            bool contains(const Data& data) const {
                return indexOf(data) != -1;
            }
            
            ///Returns the position of the item in the underlying array, or -1 if it is not contained
            int indexOf(const Data& data) const {
                EmbeddedTreeAlgorithms<Data, Handler> alg(data, m_dataSize, m_centralFreeItem);
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
            
            ConvenientEmbeddedSetIterator<Data, Handler> iterator() const;
            
        protected:
            const Data* m_data;
            uint m_dataSize;
            int m_centralFreeItem;
    };
    
    ///Convenient iterator that automatically skips invalid/free items in the array.
    template<class Data, class Handler>
    class ConvenientEmbeddedSetIterator : public ConvenientEmbeddedSet<Data, Handler> {
        public:
        ConvenientEmbeddedSetIterator(const Data* data, uint count, int centralFreeItem) : ConvenientEmbeddedSet<Data, Handler>(data, count, centralFreeItem), m_pos(0) {
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
        }
        
        private:
            inline void moveToValid() {
                while(this->m_pos < this->m_dataSize && (Handler(this->m_data[this->m_pos]).isFree()))
                    ++this->m_pos;
            }
            uint m_pos;
    };
    
    template<class Data, class Handler>
    ConvenientEmbeddedSetIterator<Data, Handler> ConvenientEmbeddedSet<Data, Handler>::iterator() const {
        return ConvenientEmbeddedSetIterator<Data, Handler>(m_data, m_dataSize, m_centralFreeItem);
    }
    
};

#endif
