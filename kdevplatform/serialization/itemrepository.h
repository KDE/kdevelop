/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_ITEMREPOSITORY_H
#define KDEVPLATFORM_ITEMREPOSITORY_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>

#include <KMessageBox>
#include <KLocalizedString>

#include <algorithm>
#include <memory>
#include <type_traits>

#include "referencecounting.h"
#include "abstractitemrepository.h"
#include "itemrepositoryregistry.h"

//#define DEBUG_MONSTERBUCKETS

// #define DEBUG_ITEMREPOSITORY_LOADING
// #define ifDebugInfiniteRecursion(x) x
#define ifDebugInfiniteRecursion(x)

// #define ifDebugLostSpace(x) x
#define ifDebugLostSpace(x)
// #define DEBUG_INCORRECT_DELETE

//Makes sure that all items stay reachable through the basic hash
// #define DEBUG_ITEM_REACHABILITY

///@todo Dynamic bucket hash size

#ifdef DEBUG_ITEM_REACHABILITY
#define ENSURE_REACHABLE(bucket) Q_ASSERT(allItemsReachable(bucket));
#define IF_ENSURE_REACHABLE(x) x
#else
#define ENSURE_REACHABLE(bucket)
#define IF_ENSURE_REACHABLE(x)
#endif

#define ITEMREPOSITORY_USE_MMAP_LOADING

//Assertion macro that prevents warnings if debugging is disabled
//Only use it to verify values, it should not call any functions, since else the function will even be called in release mode
#ifdef QT_NO_DEBUG
#define VERIFY(X) if (!(X)) {qWarning() << "Failed to verify expression" << # X;}
#else
#define VERIFY(X) Q_ASSERT(X)
#endif

///When this is uncommented, a 64-bit test-value is written behind the area an item is allowed to write into before
///createItem(..) is called, and an assertion triggers when it was changed during createItem(), which means createItem wrote too long.
///The problem: This temporarily overwrites valid data in the following item, so it will cause serious problems if that data is accessed
///during the call to createItem().
// #define DEBUG_WRITING_EXTENTS

class TestItemRepository;
class BenchItemRepository;

namespace KDevelop {
namespace ItemRepositoryUtils {
class FileMap
{
    Q_DISABLE_COPY_MOVE(FileMap)
public:
    explicit FileMap(char* data)
        : m_data(data)
    {
        Q_ASSERT(m_data);
    }

    template<typename T>
    void readValue(T* to)
    {
        Q_ASSERT(to);
        static_assert(std::is_integral_v<T>);

        *to = *reinterpret_cast<T*>(m_data);
        m_data += sizeof(T);
    }

    template<typename T>
    void readArray(T** to, uint size)
    {
        Q_ASSERT(to);
        static_assert(std::is_integral_v<T>);

        *to = reinterpret_cast<T*>(m_data);
        m_data += sizeof(T) * size;
    }

    char* current() const
    {
        return m_data;
    }

private:
    char* m_data;
};

template<class T>
void readValues(QIODevice* file, uint numValues, T* to)
{
    Q_ASSERT(file);
    Q_ASSERT(to);
    static_assert(std::is_integral_v<T>);

    file->read(reinterpret_cast<char*>(to), sizeof(T) * numValues);
}

template<class T>
void readValue(QIODevice* file, T* to)
{
    readValues(file, 1, to);
}

template<typename T>
void readList(QIODevice* file, QVector<T>* to)
{
    Q_ASSERT(to);
    readValues(file, to->size(), to->data());
}

template<class T>
void writeValues(QIODevice* file, uint numValues, const T* from)
{
    Q_ASSERT(file);
    Q_ASSERT(from);
    static_assert(std::is_integral_v<T>);

    file->write(reinterpret_cast<const char*>(from), sizeof(T) * numValues);
}

template<class T>
void writeValue(QIODevice* file, const T& from)
{
    writeValues(file, 1, &from);
}

template<typename T>
void writeList(QIODevice* file, const QVector<T>& from)
{
    writeValues(file, from.size(), from.data());
}
}
/**
 * This file implements a generic bucket-based indexing repository, that can be used for example to index strings.
 *
 * All you need to do is define your item type that you want to store into the repository, and create a request item
 * similar to ExampleItemRequest that compares and fills the defined item type.
 *
 * For example the string repository uses "unsigned short" as item-type, uses that actual value to store the length of the string,
 * and uses the space behind to store the actual string content.
 *
 * @see AbstractItemRepository
 * @see ItemRepository
 *
 * @see ExampleItem
 * @see ExampleItemRequest
 *
 * @see typerepository.h
 * @see stringrepository.h
 * @see indexedstring.h
 */

enum {
    ItemRepositoryBucketSize = 1 << 16,
    ItemRepositoryBucketLimit = 1 << 16,
    ItemRepositoryBucketLinearGrowthFactor = 10,
};

/**
 * Buckets are the memory-units that are used to store the data in an ItemRepository.
 *
 * About monster buckets: Normally a bucket has a size of 64kb, but when an item is
 * allocated that is larger than that, a "monster bucket" is allocated, which spans the
 * space of multiple buckets.
 */
template <class Item, class ItemRequest, bool markForReferenceCounting, uint fixedItemSize>
class Bucket
{
public:
    enum {
        AdditionalSpacePerItem = 2
    };
    enum {
        ObjectMapSize = ((ItemRepositoryBucketSize / ItemRequest::AverageSize) * 3) / 2 + 1,
        MaxFreeItemsForHide = 0, //When less than this count of free items in one buckets is reached, the bucket is removed from the global list of buckets with free items
        MaxFreeSizeForHide = fixedItemSize ? fixedItemSize : 0, //Only when the largest free size is smaller then this, the bucket is taken from the free list
        MinFreeItemsForReuse = 10,//When this count of free items in one bucket is reached, consider re-assigning them to new requests
        MinFreeSizeForReuse = ItemRepositoryBucketSize / 20 //When the largest free item is bigger then this, the bucket is automatically added to the free list
    };
    enum {
        NextBucketHashSize = ObjectMapSize, //Affects the average count of bucket-chains that need to be walked in ItemRepository::index. Must be a multiple of ObjectMapSize
        DataSize = sizeof(char) + sizeof(unsigned int) * 3 + ItemRepositoryBucketSize + sizeof(short unsigned int) *
                   (ObjectMapSize + NextBucketHashSize + 1)
    };
    enum {
        CheckStart = 0xff00ff1,
        CheckEnd = 0xfafcfb
    };
    Bucket()
    {
    }
    Q_DISABLE_COPY_MOVE(Bucket)
    ~Bucket()
    {
        if (m_data != m_mappedData) {
            delete[] m_data;
            delete[] m_nextBucketHash;
            delete[] m_objectMap;
        }
    }

    void initialize(int monsterBucketExtent, std::unique_ptr<short unsigned int[]> nextBucketHashToRestore = {})
    {
        if (!m_data) {
            m_monsterBucketExtent = monsterBucketExtent;
            m_available = ItemRepositoryBucketSize;
            m_data = new char[dataSize()];
#ifndef QT_NO_DEBUG
            std::fill_n(m_data, dataSize(), 0);
#endif
            //The bigger we make the map, the lower the probability of a clash(and thus bad performance). However it increases memory usage.
            // NOTE: the `()` at the end of `new int[...]()` ensures the data is zero-initialized, see e.g.:
            //       https://stackoverflow.com/questions/7546620/operator-new-initializes-memory-to-zero
            m_objectMap = new short unsigned int[ObjectMapSize]();

            if (nextBucketHashToRestore) {
                m_nextBucketHash = nextBucketHashToRestore.release();
            } else {
                m_nextBucketHash = new short unsigned int[NextBucketHashSize]();
            }

            m_changed = true;
            m_dirty = false;
            m_lastUsed = 0;
        }
    }

    void initializeFromMap(char* fileMapData)
    {
        if (m_data) {
            return;
        }

        ItemRepositoryUtils::FileMap fileMap(fileMapData);

        fileMap.readValue(&m_monsterBucketExtent);
        Q_ASSERT(fileMap.current() - fileMapData == 4);
        fileMap.readValue(&m_available);
        fileMap.readArray(&m_objectMap, ObjectMapSize);
        fileMap.readArray(&m_nextBucketHash, NextBucketHashSize);
        fileMap.readValue(&m_largestFreeItem);
        fileMap.readValue(&m_freeItemCount);
        fileMap.readValue(&m_dirty);

        m_data = fileMap.current();
        m_mappedData = m_data;

        m_changed = false;
        m_lastUsed = 0;
        Q_ASSERT(fileMap.current() - fileMapData == DataSize - ItemRepositoryBucketSize);
    }

    void store(QFile* file, size_t offset)
    {
        using namespace ItemRepositoryUtils;

        if (!m_data)
            return;

        if (static_cast<size_t>(file->size()) < offset + (1 + m_monsterBucketExtent) * DataSize)
            file->resize(offset + (1 + m_monsterBucketExtent) * DataSize);

        file->seek(offset);

        writeValue(file, m_monsterBucketExtent);
        writeValue(file, m_available);
        writeValues(file, ObjectMapSize, m_objectMap);
        writeValues(file, NextBucketHashSize, m_nextBucketHash);
        writeValue(file, m_largestFreeItem);
        writeValue(file, m_freeItemCount);
        writeValue(file, m_dirty);
        writeValues(file, dataSize(), m_data);

        if (static_cast<size_t>(file->pos()) != offset + (1 + m_monsterBucketExtent) * DataSize) {
            KMessageBox::error(nullptr, i18n("Failed writing to %1, probably the disk is full", file->fileName()));
            abort();
        }

        m_changed = false;
#ifdef DEBUG_ITEMREPOSITORY_LOADING
        {
            file->flush();
            file->seek(offset);

            uint available, freeItemCount;
            int monsterBucketExtent;
            short unsigned int largestFree;
            bool dirty;

            short unsigned int* m = new short unsigned int[ObjectMapSize];
            short unsigned int* h = new short unsigned int[NextBucketHashSize];

            readValue(file, &monsterBucketExtent);
            Q_ASSERT(monsterBucketExtent == m_monsterBucketExtent);

            char* d = new char[dataSize()];

            readValue(file, &available);
            readValues(file, ObjectMapSize, m);
            readValues(file, NextBucketHashSize, h);
            readValue(file, &largestFree);
            readValue(file, &freeItemCount);
            readValue(file, &dirty);
            readValues(file, dataSize(), d);

            Q_ASSERT(available == m_available);
            Q_ASSERT(std::equal(d, std::next(d, dataSize()), m_data));
            Q_ASSERT(std::equal(m, std::next(m, ObjectMapSize), m_objectMap));
            Q_ASSERT(std::equal(h, std::next(h, NextBucketHashSize), m_nextBucketHash));
            Q_ASSERT(m_largestFreeItem == largestFree);
            Q_ASSERT(m_freeItemCount == freeItemCount);
            Q_ASSERT(m_dirty == dirty);

            Q_ASSERT(static_cast<size_t>(file->pos()) == offset + DataSize + m_monsterBucketExtent * DataSize);

            delete[] d;
            delete[] m;
            delete[] h;
        }
#endif
    }

    inline char* data()
    {
        return m_data;
    }

    inline uint dataSize() const
    {
        return ItemRepositoryBucketSize + m_monsterBucketExtent * DataSize;
    }

    //Tries to find the index this item has in this bucket, or returns zero if the item isn't there yet.
    unsigned short findIndex(const ItemRequest& request) const
    {
        m_lastUsed = 0;

        unsigned short localHash = request.hash() % ObjectMapSize;
        unsigned short index = m_objectMap[localHash];

        unsigned short follower = 0;
        //Walk the chain of items with the same local hash
        while (index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
            index = follower;

        if (index && request.equals(itemFromIndex(index))) {
            return index; //We have found the item
        }

        return 0;
    }

    //Tries to get the index within this bucket, or returns zero. Will put the item into the bucket if there is room.
    //Created indices will never begin with 0xffff____, so you can use that index-range for own purposes.
    unsigned short index(const ItemRequest& request, unsigned int itemSize)
    {
        m_lastUsed = 0;

        unsigned short localHash = request.hash() % ObjectMapSize;
        unsigned short index = m_objectMap[localHash];
        unsigned short insertedAt = 0;

        const auto createInsertedItem = [&]() {
            const OptionalDUChainReferenceCountingEnabler<markForReferenceCounting> optionalRc(m_data, dataSize());
            request.createItem(reinterpret_cast<Item*>(m_data + insertedAt));
        };

        unsigned short follower = 0;
        //Walk the chain of items with the same local hash
        while (index && (follower = followerIndex(index)) && !(request.equals(itemFromIndex(index))))
            index = follower;

        if (index && request.equals(itemFromIndex(index)))
            return index; //We have found the item

        ifDebugLostSpace(Q_ASSERT(!lostSpace()); )

        prepareChange();

        unsigned int totalSize = itemSize + AdditionalSpacePerItem;

        if (m_monsterBucketExtent) {
            /// This is a monster-bucket. Other rules are applied here:
            /// Only one item can be allocated, and that must be bigger than the bucket data
            if (!m_available)
                return 0;

            Q_ASSERT(totalSize > ItemRepositoryBucketSize);
            m_available = 0;

            insertedAt = AdditionalSpacePerItem;
            setFollowerIndex(insertedAt, 0);
            Q_ASSERT(m_objectMap[localHash] == 0);
            m_objectMap[localHash] = insertedAt;
            createInsertedItem();
            return insertedAt;
        }

        //The second condition is needed, else we can get problems with zero-length items and an overflow in insertedAt to zero
        if (totalSize > m_available || (!itemSize && totalSize == m_available)) {
            //Try finding the smallest freed item that can hold the data
            unsigned short currentIndex = m_largestFreeItem;
            unsigned short previousIndex = 0;

            unsigned short freeChunkSize = 0;

            ///@todo Achieve this without full iteration
            while (currentIndex && freeSize(currentIndex) > itemSize) {
                unsigned short follower = followerIndex(currentIndex);
                if (follower && freeSize(follower) >= itemSize) {
                    //The item also fits into the smaller follower, so use that one
                    previousIndex = currentIndex;
                    currentIndex = follower;
                } else {
                    //The item fits into currentIndex, but not into the follower. So use currentIndex
                    freeChunkSize = freeSize(currentIndex) - itemSize;

                    //We need 2 bytes to store the free size
                    if (freeChunkSize != 0 && freeChunkSize < AdditionalSpacePerItem + 2) {
                        //we can not manage the resulting free chunk as a separate item, so we cannot use this position.
                        //Just pick the biggest free item, because there we can be sure that
                        //either we can manage the split, or we cannot do anything at all in this bucket.

                        freeChunkSize = freeSize(m_largestFreeItem) - itemSize;

                        if (freeChunkSize == 0 || freeChunkSize >= AdditionalSpacePerItem + 2) {
                            previousIndex = 0;
                            currentIndex = m_largestFreeItem;
                        } else {
                            currentIndex = 0;
                        }
                    }
                    break;
                }
            }

            if (!currentIndex || freeSize(currentIndex) < (totalSize - AdditionalSpacePerItem))
                return 0;

            if (previousIndex)
                setFollowerIndex(previousIndex, followerIndex(currentIndex));
            else
                m_largestFreeItem = followerIndex(currentIndex);

            --m_freeItemCount; //Took one free item out of the chain

            ifDebugLostSpace(Q_ASSERT(( uint )lostSpace() == ( uint )(freeSize(currentIndex) + AdditionalSpacePerItem));
            )

            if (freeChunkSize) {
                Q_ASSERT(freeChunkSize >= AdditionalSpacePerItem + 2);
                unsigned short freeItemSize = freeChunkSize - AdditionalSpacePerItem;

                unsigned short freeItemPosition;
                //Insert the resulting free chunk into the list of free items, so we don't lose it
                if (isBehindFreeSpace(currentIndex)) {
                    //Create the free item at the beginning of currentIndex, so it can be merged with the free space in front
                    freeItemPosition = currentIndex;
                    currentIndex += freeItemSize + AdditionalSpacePerItem;
                } else {
                    //Create the free item behind currentIndex
                    freeItemPosition = currentIndex + itemSize + AdditionalSpacePerItem;
                }
                setFreeSize(freeItemPosition, freeItemSize);
                insertFreeItem(freeItemPosition);
            }

            insertedAt = currentIndex;
            Q_ASSERT(( bool )m_freeItemCount == ( bool )m_largestFreeItem);
        } else {
            //We have to insert the item
            insertedAt = ItemRepositoryBucketSize - m_available;

            insertedAt += AdditionalSpacePerItem; //Room for the prepended follower-index

            m_available -= totalSize;
        }

        ifDebugLostSpace(Q_ASSERT(lostSpace() == totalSize); )

        Q_ASSERT(!index || !followerIndex(index));

        Q_ASSERT(!m_objectMap[localHash] || index);

        if (index)
            setFollowerIndex(index, insertedAt);
        setFollowerIndex(insertedAt, 0);

        if (m_objectMap[localHash] == 0)
            m_objectMap[localHash] = insertedAt;

#ifdef DEBUG_CREATEITEM_EXTENTS
        char* borderBehind = m_data + insertedAt + (totalSize - AdditionalSpacePerItem);

        quint64 oldValueBehind = 0;
        if (m_available >= 8) {
            oldValueBehind = *( quint64* )borderBehind;
            *(( quint64* )borderBehind) = 0xfafafafafafafafaLLU;
        }
#endif

        //Last thing we do, because createItem may recursively do even more transformation of the repository
        createInsertedItem();

#ifdef DEBUG_CREATEITEM_EXTENTS
        if (m_available >= 8) {
            //If this assertion triggers, then the item writes a bigger range than it advertised in
            Q_ASSERT(*(( quint64* )borderBehind) == 0xfafafafafafafafaLLU);
            *(( quint64* )borderBehind) = oldValueBehind;
        }
#endif

        Q_ASSERT(itemFromIndex(insertedAt)->hash() == request.hash());
        Q_ASSERT(itemFromIndex(insertedAt)->itemSize() == itemSize);

        ifDebugLostSpace(if (lostSpace()) qDebug() << "lost space:" << lostSpace(); Q_ASSERT(!lostSpace()); )

        return insertedAt;
    }

    /// @param modulo Returns whether this bucket contains an item with (hash % modulo) == (item.hash % modulo)
    ///               The default-parameter is the size of the next-bucket hash that is used by setNextBucketForHash and nextBucketForHash
    /// @note modulo MUST be a multiple of ObjectMapSize, because (b-a) | (x * h1) => (b-a) | h2, where a|b means a is a multiple of b.
    ///               This allows efficiently computing the clashes using the local object map hash.
    bool hasClashingItem(size_t hash, uint modulo)
    {
        Q_ASSERT(modulo % ObjectMapSize == 0);

        m_lastUsed = 0;

        uint hashMod = hash % modulo;
        unsigned short localHash = hash % ObjectMapSize;
        unsigned short currentIndex = m_objectMap[localHash];

        if (currentIndex == 0)
            return false;

        while (currentIndex) {
            auto currentHash = itemFromIndex(currentIndex)->hash();

            Q_ASSERT(currentHash % ObjectMapSize == localHash);

            if (currentHash % modulo == hashMod)
                return true; //Clash
            currentIndex = followerIndex(currentIndex);
        }
        return false;
    }

    void countFollowerIndexLengths(uint& usedSlots, uint& lengths, uint& slotCount, uint& longestInBucketFollowerChain)
    {
        for (uint a = 0; a < ObjectMapSize; ++a) {
            unsigned short currentIndex = m_objectMap[a];
            ++slotCount;
            uint length = 0;

            if (currentIndex) {
                ++usedSlots;

                while (currentIndex) {
                    ++length;
                    ++lengths;
                    currentIndex = followerIndex(currentIndex);
                }
                if (length > longestInBucketFollowerChain) {
//             qDebug() << "follower-chain at" << a << ":" << length;

                    longestInBucketFollowerChain = length;
                }
            }
        }
    }

    //Returns whether the given item is reachabe within this bucket, through its hash.
    bool itemReachable(const Item* item, size_t hash) const
    {
        unsigned short localHash = hash % ObjectMapSize;
        unsigned short currentIndex = m_objectMap[localHash];

        while (currentIndex) {
            if (itemFromIndex(currentIndex) == item)
                return true;

            currentIndex = followerIndex(currentIndex);
        }
        return false;
    }

    template<class Repository>
    void deleteItem(unsigned short index, size_t hash, Repository& repository)
    {
        ifDebugLostSpace(Q_ASSERT(!lostSpace()); )

        m_lastUsed = 0;
        prepareChange();

        unsigned int size = itemFromIndex(index)->itemSize();
        //Step 1: Remove the item from the data-structures that allow finding it: m_objectMap
        unsigned short localHash = hash % ObjectMapSize;
        unsigned short currentIndex = m_objectMap[localHash];
        unsigned short previousIndex = 0;

        //Fix the follower-link by setting the follower of the previous item to the next one, or updating m_objectMap
        while (currentIndex != index) {
            previousIndex = currentIndex;
            currentIndex = followerIndex(currentIndex);
            //If this assertion triggers, the deleted item was not registered under the given hash
            Q_ASSERT(currentIndex);
        }
        Q_ASSERT(currentIndex == index);

        if (!previousIndex)
            //The item was directly in the object map
            m_objectMap[localHash] = followerIndex(index);
        else
            setFollowerIndex(previousIndex, followerIndex(index));

        Item* item = const_cast<Item*>(itemFromIndex(index));

        {
            const OptionalDUChainReferenceCountingEnabler<markForReferenceCounting> optionalRc(m_data, dataSize());
            ItemRequest::destroy(item, repository);
        }

#ifndef QT_NO_DEBUG
#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 800)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
        //For debugging, so we notice the data is wrong
        std::fill_n(reinterpret_cast<char*>(item), size, 0);
#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && (((__GNUC__ * 100) + __GNUC_MINOR__) >= 800)
#pragma GCC diagnostic pop
#endif
#endif

        if (m_monsterBucketExtent) {
            ///This is a monster-bucket. Make it completely empty again.
            Q_ASSERT(!m_available);
            m_available = ItemRepositoryBucketSize;

            //Items are always inserted into monster-buckets at a fixed position
            Q_ASSERT(currentIndex == AdditionalSpacePerItem);
            Q_ASSERT(m_objectMap[localHash] == 0);
        } else {
            ///Put the space into the free-set
            setFreeSize(index, size);

            //Try merging the created free item to other free items around, else add it into the free list
            insertFreeItem(index);

            if (m_freeItemCount == 1 && freeSize(m_largestFreeItem) + m_available == ItemRepositoryBucketSize) {
                //Everything has been deleted, there is only free space left. Make the bucket empty again,
                //so it can later also be used as a monster-bucket.
                m_available = ItemRepositoryBucketSize;
                m_freeItemCount = 0;
                m_largestFreeItem = 0;
            }
        }

        Q_ASSERT(( bool )m_freeItemCount == ( bool )m_largestFreeItem);
        ifDebugLostSpace(Q_ASSERT(!lostSpace()); )
#ifdef DEBUG_INCORRECT_DELETE
        //Make sure the item cannot be found any more
        {
            unsigned short localHash = hash % ObjectMapSize;
            unsigned short currentIndex = m_objectMap[localHash];

            while (currentIndex && currentIndex != index) {
                previousIndex = currentIndex;
                currentIndex = followerIndex(currentIndex);
            }
            Q_ASSERT(!currentIndex); //The item must not be found
        }
#endif
//       Q_ASSERT(canAllocateItem(size));
    }

    ///@warning The returned item may be in write-protected memory, so never try doing a const_cast and changing some data
    ///         If you need to change something, use dynamicItemFromIndex
    ///@warning When using multi-threading, mutex() must be locked as long as you use the returned data
    inline const Item* itemFromIndex(unsigned short index) const
    {
        m_lastUsed = 0;
        return reinterpret_cast<Item*>(m_data + index);
    }

    bool isEmpty() const
    {
        return m_available == ItemRepositoryBucketSize;
    }

    ///Returns true if this bucket has no nextBucketForHash links
    bool noNextBuckets() const
    {
        if (!m_nextBucketHash)
            return true;

        for (int a = 0; a < NextBucketHashSize; ++a)
            if (m_nextBucketHash[a])
                return false;

        return true;
    }

    std::unique_ptr<short unsigned int[]> takeNextBucketHash()
    {
        if (m_mappedData == m_data) {
            // mmapped data, we need to copy the next bucket hash
            auto ret = std::make_unique<short unsigned int[]>(NextBucketHashSize);
            std::copy_n(m_nextBucketHash, NextBucketHashSize, ret.get());
            std::fill_n(m_nextBucketHash, NextBucketHashSize, 0);
            return ret;
        }

        // otherwise we can just take the pointer directly
        return std::unique_ptr<short unsigned int[]>(std::exchange(m_nextBucketHash, nullptr));
    }

    uint available() const
    {
        return m_available;
    }

    uint usedMemory() const
    {
        return ItemRepositoryBucketSize - m_available;
    }

    template <class Visitor>
    bool visitAllItems(Visitor& visitor) const
    {
        m_lastUsed = 0;
        for (uint a = 0; a < ObjectMapSize; ++a) {
            uint currentIndex = m_objectMap[a];
            while (currentIndex) {
                //Get the follower early, so there is no problems when the current
                //index is removed

                if (!visitor(reinterpret_cast<const Item*>(m_data + currentIndex)))
                    return false;

                currentIndex = followerIndex(currentIndex);
            }
        }

        return true;
    }

    ///Returns whether something was changed
    template <class Repository>
    int finalCleanup(Repository& repository)
    {
        int changed = 0;

        while (m_dirty) {
            m_dirty = false;

            for (uint a = 0; a < ObjectMapSize; ++a) {
                uint currentIndex = m_objectMap[a];
                while (currentIndex) {
                    //Get the follower early, so there is no problems when the current
                    //index is removed

                    const Item* item = reinterpret_cast<const Item*>(m_data + currentIndex);

                    if (!ItemRequest::persistent(item)) {
                        changed += item->itemSize();
                        deleteItem(currentIndex, item->hash(), repository);
                        m_dirty = true; //Set to dirty so we re-iterate
                        break;
                    }

                    currentIndex = followerIndex(currentIndex);
                }
            }
        }
        return changed;
    }

    unsigned short nextBucketForHash(size_t hash) const
    {
        m_lastUsed = 0;
        return m_nextBucketHash[hash % NextBucketHashSize];
    }

    void setNextBucketForHash(size_t hash, unsigned short bucket)
    {
        m_lastUsed = 0;
        prepareChange();
        m_nextBucketHash[hash % NextBucketHashSize] = bucket;
    }

    uint freeItemCount() const
    {
        return m_freeItemCount;
    }

    short unsigned int totalFreeItemsSize() const
    {
        short unsigned int ret = 0;
        short unsigned int currentIndex = m_largestFreeItem;
        while (currentIndex) {
            ret += freeSize(currentIndex);
            currentIndex = followerIndex(currentIndex);
        }
        return ret;
    }

    //Size of the largest item that could be inserted into this bucket
    short unsigned int largestFreeSize() const
    {
        short unsigned int ret = 0;
        if (m_largestFreeItem)
            ret = freeSize(m_largestFreeItem);
        if (m_available > ( uint )(AdditionalSpacePerItem + ( uint )ret)) {
            ret = m_available - AdditionalSpacePerItem;
            Q_ASSERT(ret == (m_available - AdditionalSpacePerItem));
        }
        return ret;
    }

    bool canAllocateItem(unsigned int size) const
    {
        short unsigned int currentIndex = m_largestFreeItem;
        while (currentIndex) {
            short unsigned int currentFree = freeSize(currentIndex);
            if (currentFree < size)
                return false;
            //Either we need an exact match, or 2 additional bytes to manage the resulting gap
            if (size == currentFree || currentFree - size >= AdditionalSpacePerItem + 2)
                return true;
            currentIndex = followerIndex(currentIndex);
        }

        return false;
    }

    void tick() const
    {
        ++m_lastUsed;
    }

    //How many ticks ago the item was last used
    int lastUsed() const
    {
        return m_lastUsed;
    }

    //Whether this bucket was changed since it was last stored
    bool changed() const
    {
        return m_changed;
    }

    void prepareChange()
    {
        m_changed = true;
        m_dirty = true;
        makeDataPrivate();
    }

    bool dirty() const
    {
        return m_dirty;
    }

    ///Returns the count of following buckets that were merged onto this buckets data array
    int monsterBucketExtent() const
    {
        return m_monsterBucketExtent;
    }

    //Counts together the space that is neither accessible through m_objectMap nor through the free items
    uint lostSpace()
    {
        if (m_monsterBucketExtent)
            return 0;

        uint need = ItemRepositoryBucketSize - m_available;
        uint found = 0;

        for (uint a = 0; a < ObjectMapSize; ++a) {
            uint currentIndex = m_objectMap[a];
            while (currentIndex) {
                found += reinterpret_cast<const Item*>(m_data + currentIndex)->itemSize() + AdditionalSpacePerItem;

                currentIndex = followerIndex(currentIndex);
            }
        }

        uint currentIndex = m_largestFreeItem;
        while (currentIndex) {
            found += freeSize(currentIndex) + AdditionalSpacePerItem;

            currentIndex = followerIndex(currentIndex);
        }
        return need - found;
    }

private:

    void makeDataPrivate()
    {
        if (m_mappedData == m_data) {
            short unsigned int* oldObjectMap = m_objectMap;
            short unsigned int* oldNextBucketHash = m_nextBucketHash;

            m_data = new char[dataSize()];
            m_objectMap = new short unsigned int[ObjectMapSize];
            m_nextBucketHash = new short unsigned int[NextBucketHashSize];

            std::copy_n(m_mappedData, dataSize(), m_data);
            std::copy_n(oldObjectMap, ObjectMapSize, m_objectMap);
            std::copy_n(oldNextBucketHash, NextBucketHashSize, m_nextBucketHash);
        }
    }

    ///Merges the given index item, which must have a freeSize() set, to surrounding free items, and inserts the result.
    ///The given index itself should not be in the free items chain yet.
    ///Returns whether the item was inserted somewhere.
    void insertFreeItem(unsigned short index)
    {
        //If the item-size is fixed, we don't need to do any management. Just keep a list of free items. Items of other size will never be requested.
        if (!fixedItemSize) {
            unsigned short currentIndex = m_largestFreeItem;
            unsigned short previousIndex = 0;

            while (currentIndex) {
                Q_ASSERT(currentIndex != index);

#ifndef QT_NO_DEBUG
                unsigned short currentFreeSize = freeSize(currentIndex);
#endif

                ///@todo Achieve this without iterating through all items in the bucket(This is very slow)
                //Merge behind index
                if (currentIndex == index + freeSize(index) + AdditionalSpacePerItem) {
                    //Remove currentIndex from the free chain, since it's merged backwards into index
                    if (previousIndex && followerIndex(currentIndex))
                        Q_ASSERT(freeSize(previousIndex) >= freeSize(followerIndex(currentIndex)));

                    if (previousIndex)
                        setFollowerIndex(previousIndex, followerIndex(currentIndex));
                    else
                        m_largestFreeItem = followerIndex(currentIndex);

                    --m_freeItemCount; //One was removed

                    //currentIndex is directly behind index, touching its space. Merge them.
                    setFreeSize(index, freeSize(index) + AdditionalSpacePerItem + freeSize(currentIndex));

                    //Recurse to do even more merging
                    insertFreeItem(index);
                    return;
                }

                //Merge before index
                if (index == currentIndex + freeSize(currentIndex) + AdditionalSpacePerItem) {
                    if (previousIndex && followerIndex(currentIndex))
                        Q_ASSERT(freeSize(previousIndex) >= freeSize(followerIndex(currentIndex)));

                    //Remove currentIndex from the free chain, since insertFreeItem wants
                    //it not to be in the chain, and it will be inserted in another place
                    if (previousIndex)
                        setFollowerIndex(previousIndex, followerIndex(currentIndex));
                    else
                        m_largestFreeItem = followerIndex(currentIndex);

                    --m_freeItemCount; //One was removed

                    //index is directly behind currentIndex, touching its space. Merge them.
                    setFreeSize(currentIndex, freeSize(currentIndex) + AdditionalSpacePerItem + freeSize(index));

                    //Recurse to do even more merging
                    insertFreeItem(currentIndex);
                    return;
                }

                previousIndex = currentIndex;
                currentIndex = followerIndex(currentIndex);
#ifndef QT_NO_DEBUG
                if (currentIndex)
                    Q_ASSERT(freeSize(currentIndex) <= currentFreeSize);
#endif
            }
        }
        insertToFreeChain(index);
    }

    ///Only inserts the item in the correct position into the free chain. index must not be in the chain yet.
    void insertToFreeChain(unsigned short index)
    {
        if (!fixedItemSize) {
            ///@todo Use some kind of tree to find the correct position in the chain(This is very slow)
            //Insert the free item into the chain opened by m_largestFreeItem
            unsigned short currentIndex = m_largestFreeItem;
            unsigned short previousIndex = 0;

            unsigned short size = freeSize(index);

            while (currentIndex && freeSize(currentIndex) > size) {
                Q_ASSERT(currentIndex != index); //must not be in the chain yet
                previousIndex = currentIndex;
                currentIndex = followerIndex(currentIndex);
            }

            if (currentIndex)
                Q_ASSERT(freeSize(currentIndex) <= size);

            setFollowerIndex(index, currentIndex);

            if (previousIndex) {
                Q_ASSERT(freeSize(previousIndex) >= size);
                setFollowerIndex(previousIndex, index);
            } else
                //This item is larger than all already registered free items, or there are none.
                m_largestFreeItem = index;
        } else {
            Q_ASSERT(freeSize(index) == fixedItemSize);
            //When all items have the same size, just prepent to the front.
            setFollowerIndex(index, m_largestFreeItem);
            m_largestFreeItem = index;
        }

        ++m_freeItemCount;
    }

    /// Returns true if the given index is right behind free space, and thus can be merged to the free space.
    bool isBehindFreeSpace(unsigned short index) const
    {
        // TODO: Without iteration!
        unsigned short currentIndex = m_largestFreeItem;

        while (currentIndex) {
            if (index == currentIndex + freeSize(currentIndex) + AdditionalSpacePerItem)
                return true;

            currentIndex = followerIndex(currentIndex);
        }
        return false;
    }

    /// @param index the index of an item @return The index of the next item in the chain of items with a same local hash, or zero
    inline unsigned short followerIndex(unsigned short index) const
    {
        Q_ASSERT(index >= 2);
        return *reinterpret_cast<unsigned short*>(m_data + (index - 2));
    }

    void setFollowerIndex(unsigned short index, unsigned short follower)
    {
        Q_ASSERT(index >= 2);
        *reinterpret_cast<unsigned short*>(m_data + (index - 2)) = follower;
    }
    // Only returns the current value if the item is actually free
    inline unsigned short freeSize(unsigned short index) const
    {
        return *reinterpret_cast<unsigned short*>(m_data + index);
    }

    //Convenience function to set the free-size, only for freed items
    void setFreeSize(unsigned short index, unsigned short size)
    {
        *reinterpret_cast<unsigned short*>(m_data + index) = size;
    }

private:
    int m_monsterBucketExtent = 0; //If this is a monster-bucket, this contains the count of follower-buckets that belong to this one
    unsigned int m_available = 0;
    char* m_data = nullptr; //Structure of the data: <Position of next item with same hash modulo ItemRepositoryBucketSize>(2 byte), <Item>(item.size() byte)
    char* m_mappedData  = nullptr; //Read-only memory-mapped data. If this equals m_data, m_data must not be written
    short unsigned int* m_objectMap  = nullptr; //Points to the first object in m_data with (hash % ObjectMapSize) == index. Points to the item itself, so subtract 1 to get the pointer to the next item with same local hash.
    short unsigned int m_largestFreeItem  = 0; //Points to the largest item that is currently marked as free, or zero. That one points to the next largest one through followerIndex
    unsigned int m_freeItemCount  = 0;

    unsigned short* m_nextBucketHash  = nullptr;

    bool m_dirty = false; //Whether the data was changed since the last finalCleanup
    bool m_changed  = false; //Whether this bucket was changed since it was last stored to disk
    mutable int m_lastUsed = 0; //How many ticks ago this bucket was last accessed
};

///This object needs to be kept alive as long as you change the contents of an item
///stored in the repository. It is needed to correctly track the reference counting
///within disk-storage.
template <class Item, bool markForReferenceCounting>
class DynamicItem : public OptionalDUChainReferenceCountingEnabler<markForReferenceCounting>
{
public:
    explicit DynamicItem(Item* i, const void* start, unsigned size)
        : OptionalDUChainReferenceCountingEnabler<markForReferenceCounting>(start, size)
        , m_item{i}
    {
//       qDebug() << "enabling" << i << "to" << (void*)(((char*)i)+size);
    }

    Item* operator->() const { return m_item; }

private:
    Item* const m_item;
};

struct ItemRepositoryStatistics {
    uint loadedBuckets = -1;
    uint currentBucket = -1;
    uint usedMemory = -1;
    uint loadedMonsterBuckets = -1;
    uint usedSpaceForBuckets = -1;
    uint freeSpaceInBuckets = -1;
    uint lostSpace = -1;
    uint freeUnreachableSpace = -1;
    uint hashClashedItems = -1;
    uint totalItems = -1;
    uint emptyBuckets;
    uint hashSize = -1; // How big the hash is
    uint hashUse = -1; // How many slots in the hash are used
    uint averageInBucketHashSize = -1;
    uint averageInBucketUsedSlotCount = -1;
    float averageInBucketSlotChainLength = -1;
    uint longestInBucketChain = -1;

    uint longestNextBucketChain = -1;
    uint totalBucketFollowerSlots = -1; // Total count of used slots in the nextBucketForHash structure
    float averageNextBucketForHashSequenceLength
        = -1; // Average sequence length of a nextBucketForHash sequence(If not empty)

    QString print() const
    {
        QString ret;
        ret += QStringLiteral("loaded buckets: %1 current bucket: %2 used memory: %3 loaded monster buckets: %4")
                   .arg(loadedBuckets)
                   .arg(currentBucket)
                   .arg(usedMemory)
                   .arg(loadedMonsterBuckets);
        ret += QStringLiteral("\nbucket hash clashed items: %1 total items: %2").arg(hashClashedItems).arg(totalItems);
        ret += QStringLiteral("\nused space for buckets: %1 free space in buckets: %2 lost space: %3")
                   .arg(usedSpaceForBuckets)
                   .arg(freeSpaceInBuckets)
                   .arg(lostSpace);
        ret += QStringLiteral("\nfree unreachable space: %1 empty buckets: %2")
                   .arg(freeUnreachableSpace)
                   .arg(emptyBuckets);
        ret += QStringLiteral("\nhash size: %1 hash slots used: %2").arg(hashSize).arg(hashUse);
        ret += QStringLiteral("\naverage in-bucket hash size: %1 average in-bucket used hash slot count: %2 average "
                              "in-bucket slot chain length: %3 longest in-bucket follower chain: %4")
                   .arg(averageInBucketHashSize)
                   .arg(averageInBucketUsedSlotCount)
                   .arg(averageInBucketSlotChainLength)
                   .arg(longestInBucketChain);
        ret += QStringLiteral("\ntotal count of used next-bucket-for-hash slots: %1 average next-bucket-for-hash "
                              "sequence length: %2 longest next-bucket chain: %3")
                   .arg(totalBucketFollowerSlots)
                   .arg(averageNextBucketForHashSequenceLength)
                   .arg(longestNextBucketChain);
        return ret;
    }
    operator QString() const { return print(); }
};

/**
 * The ItemRepository is essentially an on-disk key/value hash map
 *
 * Access to this structure is assumed to happen in a thread safe manner, e.g.
 * by locking a mutex externally. The API can then call itself without having
 * to re-lock anything internally, thus is capable to work with a non-recursive mutex.
 * If desired, a recursive mutex can be used too though as needed.
 *
 * @tparam Item See ExampleItem
 * @tparam ItemRequest See ExampleReqestItem
 * @tparam fixedItemSize When this is true, all inserted items must have the same size.
 *                      This greatly simplifies and speeds up the task of managing free items within the buckets.
 * @tparam markForReferenceCounting Whether the data within the repository should be marked for reference-counting.
 *                                 This costs a bit of performance, but must be enabled if there may be data in the
 *                                 repository that does on-disk reference counting, like IndexedString,
 *                                 IndexedIdentifier, etc.
 * @tparam Mutex The mutex type to use internally. It has to be locked externally before accessing the item repository
 *               from multiple threads.
 */

template <class Item, class ItemRequest, bool markForReferenceCounting = true, typename Mutex = QMutex,
          uint fixedItemSize = 0, unsigned int targetBucketHashSize = 524288 * 2>
class ItemRepository : public AbstractItemRepository
{
    using MyBucket = Bucket<Item, ItemRequest, markForReferenceCounting, fixedItemSize>;

    enum {
        //Must be a multiple of Bucket::ObjectMapSize, so Bucket::hasClashingItem can be computed
        //Must also be a multiple of Bucket::NextBucketHashSize, for the same reason.(Currently those are same)
        bucketHashSize = (targetBucketHashSize / MyBucket::ObjectMapSize) * MyBucket::ObjectMapSize
    };

    enum {
        BucketStartOffset = sizeof(uint) * 7 + sizeof(short unsigned int) * bucketHashSize //Position in the data where the bucket array starts
    };

    Q_DISABLE_COPY_MOVE(ItemRepository)
public:
    ///@param registry May be zero, then the repository will not be registered at all. Else, the repository will
    /// register itself to that registry.
    ///                If this is zero, you have to care about storing the data using store() and/or close() by
    ///                yourself. It does not happen automatically. For the global standard registry, the storing/loading
    ///                is triggered from within duchain, so you don't need to care about it.
    explicit ItemRepository(const QString& repositoryName, Mutex* mutex,
                            ItemRepositoryRegistry* registry = &globalItemRepositoryRegistry(),
                            uint repositoryVersion = 1)
        : m_repositoryName(repositoryName)
        , m_repositoryVersion(repositoryVersion)
        , m_mutex(mutex)
        , m_registry(registry)
    {
        if (m_registry)
            m_registry->registerRepository(this);
    }

    ~ItemRepository() override
    {
        if (m_registry)
            m_registry->unRegisterRepository(this);

        close();
    }

    ///Unloading of buckets is enabled by default. Use this to disable it. When unloading is enabled, the data
    ///gotten from must only itemFromIndex must not be used for a long time.
    void setUnloadingEnabled(bool enabled)
    {
        m_unloadingEnabled = enabled;
    }

    ///Returns the index for the given item. If the item is not in the repository yet, it is inserted.
    ///The index can never be zero. Zero is reserved for your own usage as invalid
    ///@param request Item to retrieve the index from
    unsigned int index(const ItemRequest& request)
    {
        const size_t hash = request.hash();
        const uint size = request.itemSize();

        // Bucket indexes tracked while walking the bucket chain for this request hash
        unsigned short bucketInChainWithSpace = 0;
        unsigned short lastBucketWalked = 0;

        const ushort foundIndexInBucket = walkBucketChain(hash, [&](ushort bucketIdx, const MyBucket* bucketPtr) {
                lastBucketWalked = bucketIdx;

                const ushort found = bucketPtr->findIndex(request);

                if (!found && !bucketInChainWithSpace && bucketPtr->canAllocateItem(size)) {
                    bucketInChainWithSpace = bucketIdx;
                }

                return found;
            });

        if (foundIndexInBucket) {
            // 'request' is already present, return the existing index
            Q_ASSERT(m_currentBucket);
            Q_ASSERT(m_currentBucket < m_buckets.size());
            return createIndex(lastBucketWalked, foundIndexInBucket);
        }

        /*
         * Disclaimer: Writer of comment != writer of code, believe with caution
         *
         * Requested item does not yet exist. Need to find a place to put it...
         *
         * First choice is to place it in an existing bucket in the chain for the request hash
         * Second choice is to find an existing bucket anywhere with enough space
         * Otherwise use m_currentBucket (the latest unused bucket)
         *
         * If the chosen bucket fails to allocate the item, merge buckets to create a monster (dragon?)
         *
         * Finally, if the first option failed or the selected bucket failed to allocate, add the
         * bucket which the item ended up in to the chain of buckets for the request's hash
         */

        m_metaDataChanged = true;

        const bool pickedBucketInChain = bucketInChainWithSpace;
        int useBucket = bucketInChainWithSpace;

        if (!pickedBucketInChain) {
            //Try finding an existing bucket with deleted space to store the data into
            for (int a = 0; a < m_freeSpaceBuckets.size(); ++a) {
                MyBucket* bucketPtr = bucketForIndex(m_freeSpaceBuckets[a]);
                Q_ASSERT(bucketPtr->largestFreeSize());

                if (bucketPtr->canAllocateItem(size)) {
                    //The item fits into the bucket.
                    useBucket = m_freeSpaceBuckets[a];
                    break;
                }
            }
        }

        auto advanceToNextBucket = [&]() {
            //This should never happen when we picked a bucket for re-use
            Q_ASSERT(!pickedBucketInChain);
            Q_ASSERT(useBucket == m_currentBucket);
            // note: the bucket may be empty and/or in the free list, but it might still be too small
            // for a monster bucket request

            ++m_currentBucket;
            Q_ASSERT(m_currentBucket < ItemRepositoryBucketLimit);
            useBucket = m_currentBucket;
        };

        //The item isn't in the repository yet, find a new bucket for it
        while (1) {
            if (useBucket >= m_buckets.size()) {
                if (m_buckets.size() >= 0xfffe) { //We have reserved the last bucket index 0xffff for special purposes
                    //the repository has overflown.
                    qWarning() << "Found no room for an item in" << m_repositoryName << "size of the item:" <<
                        request.itemSize();
                    return 0;
                } else {
                    allocateNextBuckets(ItemRepositoryBucketLinearGrowthFactor);
                }
            }

            if (!useBucket) {
                Q_ASSERT(m_currentBucket);
                useBucket = m_currentBucket;
            }

            // don't put an item into the tail of a monster bucket
            if (m_monsterBucketTailMarker[useBucket]) {
                advanceToNextBucket();
                continue;
            }

            auto* bucketPtr = bucketForIndex(useBucket);

            ENSURE_REACHABLE(useBucket);
            Q_ASSERT_X(!bucketPtr->findIndex(
                           request), Q_FUNC_INFO,
                       "found item in unexpected bucket, ensure your ItemRequest::equals method is correct. Note: For custom AbstractType's e.g. ensure you have a proper equals() override");

            unsigned short indexInBucket = bucketPtr->index(request, size);

            //If we could not allocate the item in an empty bucket, then we need to create a monster-bucket that
            //can hold the data.
            if (bucketPtr->isEmpty() && !indexInBucket) {
                ///@todo Move this compound statement into an own function
                uint totalSize = size + MyBucket::AdditionalSpacePerItem;

                Q_ASSERT((totalSize > ItemRepositoryBucketSize));

                useBucket = 0;
                //The item did not fit in, we need a monster-bucket(Merge consecutive buckets)
                ///Step one: Search whether we can merge multiple empty buckets in the free-list into one monster-bucket
                int rangeStart = -1;
                int rangeEnd = -1;
                for (int a = 0; a < m_freeSpaceBuckets.size(); ++a) {
                    MyBucket* bucketPtr = bucketForIndex(m_freeSpaceBuckets[a]);
                    if (bucketPtr->isEmpty()) {
                        //This bucket is a candidate for monster-bucket merging
                        int index = ( int )m_freeSpaceBuckets[a];
                        if (rangeEnd != index) {
                            rangeStart = index;
                            rangeEnd = index + 1;
                        } else {
                            ++rangeEnd;
                        }
                        if (rangeStart != rangeEnd) {
                            uint extent = rangeEnd - rangeStart - 1;
                            uint totalAvailableSpace = bucketForIndex(rangeStart)->available() +
                                                       MyBucket::DataSize* (rangeEnd - rangeStart - 1);
                            if (totalAvailableSpace > totalSize) {
                                Q_ASSERT(extent);
                                ///We can merge these buckets into one monster-bucket that can hold the data
                                Q_ASSERT((uint)m_freeSpaceBuckets[a - extent] == (uint)rangeStart);
                                useBucket = rangeStart;
                                convertMonsterBucket(rangeStart, extent);

                                break;
                            }
                        }
                    }
                }

                if (!useBucket) {
                    //Create a new monster-bucket at the end of the data
                    int needMonsterExtent = (totalSize - ItemRepositoryBucketSize) / MyBucket::DataSize + 1;
                    Q_ASSERT(needMonsterExtent);
                    const auto currentBucketIncrease = needMonsterExtent + 1;
                    Q_ASSERT(m_currentBucket);
                    if (m_currentBucket + currentBucketIncrease >= m_buckets.size()) {
                        allocateNextBuckets(ItemRepositoryBucketLinearGrowthFactor + currentBucketIncrease);
                    }
                    useBucket = m_currentBucket;

                    convertMonsterBucket(useBucket, needMonsterExtent);
                    m_currentBucket += currentBucketIncrease;
                    Q_ASSERT(m_currentBucket < ItemRepositoryBucketLimit);
                    Q_ASSERT(m_buckets[useBucket]);
                    Q_ASSERT(m_buckets[useBucket]->monsterBucketExtent() == needMonsterExtent);
                }
                Q_ASSERT(useBucket);
                bucketPtr = bucketForIndex(useBucket);

                indexInBucket = bucketPtr->index(request, size);
                Q_ASSERT(indexInBucket);
            }

            Q_ASSERT(m_currentBucket);

            if (indexInBucket) {
                ++m_statItemCount;

                const int previousBucketNumber = lastBucketWalked;
                unsigned short* const bucketHashPosition = m_firstBucketForHash + (hash % bucketHashSize);

                if (!(*bucketHashPosition)) {
                    Q_ASSERT(!previousBucketNumber);
                    (*bucketHashPosition) = useBucket;
                    ENSURE_REACHABLE(useBucket);
                } else if (!pickedBucketInChain && previousBucketNumber && previousBucketNumber != useBucket) {
                    //Should happen rarely
                    ++m_statBucketHashClashes;

                    ///Debug: Detect infinite recursion
                    ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, previousBucketNumber));
                    )

                    //Find the position where the paths of useBucket and *bucketHashPosition intersect, and insert useBucket
                    //there. That way, we don't create loops.
                    QPair<unsigned int, unsigned int> intersect = hashChainIntersection(*bucketHashPosition, useBucket,
                                                                                        hash);

                    Q_ASSERT(m_buckets[previousBucketNumber]->nextBucketForHash(hash) == 0);

                    if (!intersect.second) {
                        ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(*bucketHashPosition, hash, useBucket)); )
                        ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(useBucket, hash, previousBucketNumber)); )

                        Q_ASSERT(m_buckets[previousBucketNumber]->nextBucketForHash(hash) == 0);

                        m_buckets[previousBucketNumber]->setNextBucketForHash(hash, useBucket);
                        ENSURE_REACHABLE(useBucket);
                        ENSURE_REACHABLE(previousBucketNumber);

                        ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, useBucket)); )
                    } else if (intersect.first) {
                        ifDebugInfiniteRecursion(Q_ASSERT(bucketForIndex(intersect.first)->nextBucketForHash(hash) ==
                                                          intersect.second); )
                        ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(*bucketHashPosition, hash, useBucket)); )
                        ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, intersect.second));
                        )
                        ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, intersect.first));
                        )
                        ifDebugInfiniteRecursion(Q_ASSERT(bucketForIndex(intersect.first)->nextBucketForHash(hash) ==
                                                          intersect.second); )

                        ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(useBucket, hash, intersect.second)); )
                        ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(useBucket, hash, intersect.first)); )

                        bucketForIndex(intersect.first)->setNextBucketForHash(hash, useBucket);

                        ENSURE_REACHABLE(useBucket);
                        ENSURE_REACHABLE(intersect.second);

                        ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, useBucket)); )
                        ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(*bucketHashPosition, hash, intersect.second));
                        )
                    } else {
                        //State: intersect.first == 0 && intersect.second != 0. This means that whole complete
                        //chain opened by *bucketHashPosition with the hash-value is also following useBucket,
                        //so useBucket can just be inserted at the top

                        ifDebugInfiniteRecursion(Q_ASSERT(!walkBucketLinks(*bucketHashPosition, hash, useBucket)); )
                        ifDebugInfiniteRecursion(Q_ASSERT(walkBucketLinks(useBucket, hash, *bucketHashPosition)); )
                        unsigned short oldStart = *bucketHashPosition;

                        *bucketHashPosition = useBucket;

                        ENSURE_REACHABLE(useBucket);
                        ENSURE_REACHABLE(oldStart);
                        Q_UNUSED(oldStart);
                    }
                }

                const auto reOrderFreeSpaceBucketIndex = m_freeSpaceBuckets.indexOf(useBucket);
                if (reOrderFreeSpaceBucketIndex != -1)
                    updateFreeSpaceOrder(reOrderFreeSpaceBucketIndex);

                Q_ASSERT(m_currentBucket < m_buckets.size());
                return createIndex(useBucket, indexInBucket);
            } else {
                advanceToNextBucket();
            }
        }
        //We haven't found a bucket that already contains the item, so append it to the current bucket

        qWarning() << "Found no bucket for an item in" << m_repositoryName;
        return 0;
    }

    ///Returns zero if the item is not in the repository yet
    unsigned int findIndex(const ItemRequest& request) const
    {
        return walkBucketChain(request.hash(), [this, &request](ushort bucketIdx, const MyBucket* bucketPtr) {
                const ushort indexInBucket = bucketPtr->findIndex(request);
                return indexInBucket ? createIndex(bucketIdx, indexInBucket) : 0u;
            });
    }

    /// Returns nullptr if the item is not in the repository yet
    const Item* findItem(const ItemRequest& request) const
    {
        auto index = findIndex(request);
        if (!index) {
            return nullptr;
        }
        return itemFromIndex(index);
    }

    ///Deletes the item from the repository.
    void deleteItem(unsigned int index)
    {
        verifyIndex(index);

        m_metaDataChanged = true;

        const size_t hash = itemFromIndex(index)->hash();
        const ushort bucket = (index >> 16);

        //Apart from removing the item itself, we may have to recreate the nextBucketForHash link, so we need the previous bucket
        MyBucket* previousBucketPtr = nullptr;
        MyBucket* const bucketPtr = walkBucketChain(hash,
                                                    [bucket, &previousBucketPtr](ushort bucketIdx,
                                                                                 MyBucket* bucketPtr) -> MyBucket* {
                if (bucket != bucketIdx) {
                    previousBucketPtr = bucketPtr;
                    return nullptr;
                }
                return bucketPtr; // found bucket, stop looking
            });

        //Make sure the index was reachable through the hash chain
        Q_ASSERT(bucketPtr);

        --m_statItemCount;

        bucketPtr->deleteItem(index, hash, *this);

        /**
         * Now check whether the link root/previousBucketNumber -> bucket is still needed.
         */

        if (!previousBucketPtr) {
            // This bucket is linked in the m_firstBucketForHash array, find the next clashing bucket in the chain
            // There may be items in the chain that clash only with MyBucket::NextBucketHashSize, skipped here
            m_firstBucketForHash[hash %
                                 bucketHashSize] = walkBucketChain(hash, [hash](ushort bucketIdx, MyBucket* bucketPtr) {
                    if (bucketPtr->hasClashingItem(hash, bucketHashSize)) {
                        return bucketIdx;
                    }
                    return static_cast<ushort>(0);
                });
        } else if (!bucketPtr->hasClashingItem(hash, MyBucket::NextBucketHashSize)) {
            // TODO: Skip clashing items reachable from m_firstBucketForHash
            // (see note in usePermissiveModuloWhenRemovingClashLinks() test)

            ENSURE_REACHABLE(bucket);

            previousBucketPtr->setNextBucketForHash(hash, bucketPtr->nextBucketForHash(hash));

            Q_ASSERT(m_buckets[bucketPtr->nextBucketForHash(hash)] != previousBucketPtr);
        }

        ENSURE_REACHABLE(bucket);

        if (bucketPtr->monsterBucketExtent()) {
            //Convert the monster-bucket back to multiple normal buckets, and put them into the free list
            Q_ASSERT(bucketPtr->isEmpty());
            if (!previousBucketPtr) {
                // see https://bugs.kde.org/show_bug.cgi?id=272408
                // the monster bucket will be deleted and new smaller ones created
                // the next bucket for this hash is invalid anyways as done above
                // but calling the below unconditionally leads to other issues...
                bucketPtr->setNextBucketForHash(hash, 0);
            }
            convertMonsterBucket(bucket, 0);
        } else {
            putIntoFreeList(bucket, bucketPtr);
        }

        Q_ASSERT(m_currentBucket);
        Q_ASSERT(m_currentBucket < m_buckets.size());
    }

    using MyDynamicItem = DynamicItem<Item, markForReferenceCounting>;

    ///This returns an editable version of the item. @warning: Never change an entry that affects the hash,
    ///or the equals(..) function. That would completely destroy the consistency.
    ///@param index The index. It must be valid(match an existing item), and nonzero.
    ///@warning If you use this, make sure you lock mutex() before calling,
    ///         and hold it until you're ready using/changing the data..
    MyDynamicItem dynamicItemFromIndex(unsigned int index)
    {
        verifyIndex(index);

        unsigned short bucket = (index >> 16);

        auto* bucketPtr = bucketForIndex(bucket);
        bucketPtr->prepareChange();
        unsigned short indexInBucket = index & 0xffff;
        return MyDynamicItem(const_cast<Item*>(bucketPtr->itemFromIndex(indexInBucket)),
                             bucketPtr->data(), bucketPtr->dataSize());
    }

    ///This returns an editable version of the item. @warning: Never change an entry that affects the hash,
    ///or the equals(..) function. That would completely destroy the consistency.
    ///@param index The index. It must be valid(match an existing item), and nonzero.
    ///@warning If you use this, make sure you lock mutex() before calling,
    ///         and hold it until you're ready using/changing the data..
    ///@warning If you change contained complex items that depend on reference-counting, you
    ///         must use dynamicItemFromIndex(..) instead of dynamicItemFromIndexSimple(..)
    Item* dynamicItemFromIndexSimple(unsigned int index)
    {
        verifyIndex(index);

        unsigned short bucket = (index >> 16);

        auto* bucketPtr = bucketForIndex(bucket);
        bucketPtr->prepareChange();
        unsigned short indexInBucket = index & 0xffff;
        return const_cast<Item*>(bucketPtr->itemFromIndex(indexInBucket));
    }

    ///@param index The index. It must be valid(match an existing item), and nonzero.
    const Item* itemFromIndex(unsigned int index) const
    {
        verifyIndex(index);

        unsigned short bucket = (index >> 16);

        const auto* bucketPtr = bucketForIndex(bucket);
        unsigned short indexInBucket = index & 0xffff;
        Q_ASSERT(m_currentBucket);
        Q_ASSERT(m_currentBucket < m_buckets.size());
        return bucketPtr->itemFromIndex(indexInBucket);
    }

    QString printStatistics() const final
    {
        return statistics().print();
    }

    ItemRepositoryStatistics statistics() const
    {
        Q_ASSERT(!m_currentBucket || m_currentBucket < m_buckets.size());
        ItemRepositoryStatistics ret;
        uint loadedBuckets = 0;
        for (auto* bucket : m_buckets) {
            if (bucket) {
                ++loadedBuckets;
            }
        }

#ifdef DEBUG_MONSTERBUCKETS
        for (int a = 0; a < m_freeSpaceBuckets.size(); ++a) {
            if (a > 0) {
                uint prev = a - 1;
                uint prevLargestFree = bucketForIndex(m_freeSpaceBuckets[prev])->largestFreeSize();
                uint largestFree = bucketForIndex(m_freeSpaceBuckets[a])->largestFreeSize();
                Q_ASSERT((prevLargestFree < largestFree) || (prevLargestFree == largestFree &&
                                                             m_freeSpaceBuckets[prev] < m_freeSpaceBuckets[a]));
            }
        }

#endif
        ret.hashSize = bucketHashSize;
        ret.hashUse = 0;
        for (uint a = 0; a < bucketHashSize; ++a)
            if (m_firstBucketForHash[a])
                ++ret.hashUse;

        ret.emptyBuckets = 0;

        uint loadedMonsterBuckets = 0;
        for (auto* bucket : m_buckets) {
            if (bucket && bucket->monsterBucketExtent()) {
                loadedMonsterBuckets += bucket->monsterBucketExtent() + 1;
            }
        }

        uint usedBucketSpace = MyBucket::DataSize* m_currentBucket;
        uint freeBucketSpace = 0, freeUnreachableSpace = 0;
        uint lostSpace = 0; //Should be zero, else something is wrong
        uint totalInBucketHashSize = 0, totalInBucketUsedSlotCount = 0, totalInBucketChainLengths = 0;
        uint bucketCount = 0;

        ret.totalBucketFollowerSlots = 0;
        ret.averageNextBucketForHashSequenceLength = 0;
        ret.longestNextBucketChain = 0;
        ret.longestInBucketChain = 0;

        for (int a = 1; a < m_currentBucket + 1; ++a) {
            Q_ASSERT(!m_monsterBucketTailMarker[a]);

            MyBucket* bucket = bucketForIndex(a);
            if (bucket) {
                ++bucketCount;

                bucket->countFollowerIndexLengths(totalInBucketUsedSlotCount, totalInBucketChainLengths,
                                                  totalInBucketHashSize, ret.longestInBucketChain);

                for (uint aa = 0; aa < MyBucket::NextBucketHashSize; ++aa) {
                    uint length = 0;
                    uint next = bucket->nextBucketForHash(aa);
                    if (next) {
                        ++ret.totalBucketFollowerSlots;
                        while (next) {
                            ++length;
                            ++ret.averageNextBucketForHashSequenceLength;
                            next = bucketForIndex(next)->nextBucketForHash(aa);
                        }
                    }
                    if (length > ret.longestNextBucketChain) {
//             qDebug() << "next-bucket-chain in" << a << aa << ":" << length;
                        ret.longestNextBucketChain = length;
                    }
                }

                uint bucketFreeSpace = bucket->totalFreeItemsSize() + bucket->available();
                freeBucketSpace += bucketFreeSpace;
                if (m_freeSpaceBuckets.indexOf(a) == -1)
                    freeUnreachableSpace += bucketFreeSpace;

                if (bucket->isEmpty()) {
                    ++ret.emptyBuckets;
                    Q_ASSERT(!bucket->monsterBucketExtent());
#ifdef DEBUG_MONSTERBUCKETS
                    Q_ASSERT(m_freeSpaceBuckets.contains(a));
#endif
                }

                lostSpace += bucket->lostSpace();
                a += bucket->monsterBucketExtent();
            }
        }

        if (ret.totalBucketFollowerSlots)
            ret.averageNextBucketForHashSequenceLength /= ret.totalBucketFollowerSlots;

        ret.loadedBuckets = loadedBuckets;
        ret.currentBucket = m_currentBucket;
        ret.usedMemory = usedMemory();
        ret.loadedMonsterBuckets = loadedMonsterBuckets;

        ret.hashClashedItems = m_statBucketHashClashes;
        ret.totalItems = m_statItemCount;
        ret.usedSpaceForBuckets = usedBucketSpace;
        ret.freeSpaceInBuckets = freeBucketSpace;
        ret.lostSpace = lostSpace;

        ret.freeUnreachableSpace = freeUnreachableSpace;
        ret.averageInBucketHashSize = bucketCount ? (totalInBucketHashSize / bucketCount) : 0;
        ret.averageInBucketUsedSlotCount = bucketCount ? (totalInBucketUsedSlotCount / bucketCount) : 0;
        ret.averageInBucketSlotChainLength = float( totalInBucketChainLengths ) / totalInBucketUsedSlotCount;

        //If m_statBucketHashClashes is high, the bucket-hash needs to be bigger
        return ret;
    }

    ///This can be used to safely iterate through all items in the repository
    ///@param visitor Should be an instance of a class that has a bool operator()(const Item*) member function,
    ///               that returns whether more items are wanted.
    ///@param onlyInMemory If this is true, only items are visited that are currently in memory.
    template <class Visitor>
    void visitAllItems(Visitor& visitor, bool onlyInMemory = false) const
    {
        for (int a = 1; a <= m_currentBucket; ++a) {
            if (!onlyInMemory || m_buckets.at(a)) {
                auto bucket = bucketForIndex(a);
                if (bucket && !bucket->visitAllItems(visitor))
                    return;
            }
        }
    }

    QString repositoryName() const final { return m_repositoryName; }

    Mutex* mutex() const { return m_mutex; }

    void lock() final { m_mutex->lock(); }
    void unlock() final { m_mutex->unlock(); }

private:
    void writeMetadata()
    {
        using namespace ItemRepositoryUtils;

        Q_ASSERT(m_file);
        Q_ASSERT(m_dynamicFile);

        m_file->seek(0);
        writeValue(m_file, m_repositoryVersion);
        uint hashSize = bucketHashSize;
        writeValue(m_file, hashSize);
        uint itemRepositoryVersion = staticItemRepositoryVersion();
        writeValue(m_file, itemRepositoryVersion);
        writeValue(m_file, m_statBucketHashClashes);
        writeValue(m_file, m_statItemCount);

        const uint bucketCount = static_cast<uint>(m_buckets.size());
        writeValue(m_file, bucketCount);
        writeValue(m_file, m_currentBucket);
        writeValues(m_file, bucketHashSize, m_firstBucketForHash);
        Q_ASSERT(m_file->pos() == BucketStartOffset);

        m_dynamicFile->seek(0);
        writeValue(m_dynamicFile, static_cast<uint>(m_freeSpaceBuckets.size()));
        writeList(m_dynamicFile, m_freeSpaceBuckets);

        Q_ASSERT(m_buckets.size() == m_monsterBucketTailMarker.size());
        writeList(m_dynamicFile, m_monsterBucketTailMarker);
    }

    ///Synchronizes the state on disk to the one in memory, and does some memory-management.
    ///Should be called on a regular basis. Can be called centrally from the global item repository registry.
    void store() final
    {
        if (m_file) {
            if (!m_file->open(QFile::ReadWrite) || !m_dynamicFile->open(QFile::ReadWrite)) {
                qFatal("cannot re-open repository file for storing");
                return;
            }

            for (int a = 0; a < m_buckets.size(); ++a) {
                auto& bucket = m_buckets[a];
                if (bucket) {
                    if (bucket->changed()) {
                        storeBucket(a);
                    }
                    if (m_unloadingEnabled) {
                        const int unloadAfterTicks = 2;
                        if (bucket->lastUsed() > unloadAfterTicks) {
                            delete bucket;
                            bucket = nullptr;
                        } else {
                            bucket->tick();
                        }
                    }
                }
            }

            if (m_metaDataChanged) {
                writeMetadata();
            }
            //To protect us from inconsistency due to crashes. flush() is not enough. We need to close.
            m_file->close();
            m_dynamicFile->close();
            Q_ASSERT(!m_file->isOpen());
            Q_ASSERT(!m_dynamicFile->isOpen());
        }
    }

    bool open(const QString& path) final
    {
        using namespace ItemRepositoryUtils;

        close();
        // qDebug() << "opening repository" << m_repositoryName << "at" << path;
        QDir dir(path);
        m_file = new QFile(dir.absoluteFilePath(m_repositoryName));
        m_dynamicFile = new QFile(dir.absoluteFilePath(m_repositoryName + QLatin1String("_dynamic")));
        if (!m_file->open(QFile::ReadWrite) || !m_dynamicFile->open(QFile::ReadWrite)) {
            delete m_file;
            m_file = nullptr;
            delete m_dynamicFile;
            m_dynamicFile = nullptr;
            return false;
        }

        m_metaDataChanged = true;
        if (m_file->size() == 0) {
            m_statBucketHashClashes = m_statItemCount = 0;

            Q_ASSERT(m_buckets.isEmpty());
            Q_ASSERT(m_freeSpaceBuckets.isEmpty());
            allocateNextBuckets(ItemRepositoryBucketLinearGrowthFactor);

            std::fill_n(m_firstBucketForHash, bucketHashSize, 0);

            // Skip the first bucket, we won't use it so we have the zero indices for special purposes
            Q_ASSERT(m_currentBucket == 1);

            // -1 because the 0 bucket is reserved for special purposes
            Q_ASSERT(m_freeSpaceBuckets.size() == m_buckets.size() - 1);

            writeMetadata();

            // We have completely initialized the file now
            if (m_file->pos() != BucketStartOffset) {
                KMessageBox::error(nullptr,
                                   i18n("Failed writing to %1, probably the disk is full", m_file->fileName()));
                abort();
            }

        } else {
            m_file->close();
            bool res = m_file->open(QFile::ReadOnly); // Re-open in read-only mode, so we create a read-only m_fileMap
            VERIFY(res);
            // Check that the version is correct
            uint storedVersion = 0, hashSize = 0, itemRepositoryVersion = 0;

            readValue(m_file, &storedVersion);
            readValue(m_file, &hashSize);
            readValue(m_file, &itemRepositoryVersion);
            readValue(m_file, &m_statBucketHashClashes);
            readValue(m_file, &m_statItemCount);

            if (storedVersion != m_repositoryVersion || hashSize != bucketHashSize
                || itemRepositoryVersion != staticItemRepositoryVersion()) {
                qDebug() << "repository" << m_repositoryName << "version mismatch in" << m_file->fileName()
                         << ", stored: version " << storedVersion << "hashsize" << hashSize << "repository-version"
                         << itemRepositoryVersion << " current: version" << m_repositoryVersion << "hashsize"
                         << bucketHashSize << "repository-version" << staticItemRepositoryVersion();
                delete m_file;
                m_file = nullptr;
                delete m_dynamicFile;
                m_dynamicFile = nullptr;
                return false;
            }
            m_metaDataChanged = false;

            uint bucketCount = 0;
            readValue(m_file, &bucketCount);

            Q_ASSERT(m_buckets.isEmpty());
            Q_ASSERT(m_freeSpaceBuckets.isEmpty());
            // not calling allocateNextBuckets here, as we load buckets from file here, not new/next ones
            m_buckets.resize(bucketCount);

            readValue(m_file, &m_currentBucket);
            Q_ASSERT(m_currentBucket);

            readValues(m_file, bucketHashSize, m_firstBucketForHash);

            Q_ASSERT(m_file->pos() == BucketStartOffset);

            uint freeSpaceBucketsSize = 0;
            readValue(m_dynamicFile, &freeSpaceBucketsSize);
            m_freeSpaceBuckets.resize(freeSpaceBucketsSize);
            readList(m_dynamicFile, &m_freeSpaceBuckets);

            m_monsterBucketTailMarker.resize(bucketCount);
            readList(m_dynamicFile, &m_monsterBucketTailMarker);
        }

        m_fileMapSize = 0;
        m_fileMap = nullptr;

#ifdef ITEMREPOSITORY_USE_MMAP_LOADING
        if (m_file->size() > BucketStartOffset) {
            m_fileMap = m_file->map(BucketStartOffset, m_file->size() - BucketStartOffset);
            Q_ASSERT(m_file->isOpen());
            Q_ASSERT(m_file->size() >= BucketStartOffset);
            if (m_fileMap) {
                m_fileMapSize = m_file->size() - BucketStartOffset;
            } else {
                qWarning() << "mapping" << m_file->fileName() << "FAILED!";
            }
        }
#endif
        // To protect us from inconsistency due to crashes. flush() is not enough.
        m_file->close();
        m_dynamicFile->close();

        return true;
    }

    ///@warning by default, this does not store the current state to disk.
    void close(bool doStore = false) final
    {
        if (doStore)
            store();

        if (m_file)
            m_file->close();
        delete m_file;
        m_file = nullptr;
        m_fileMap = nullptr;
        m_fileMapSize = 0;

        if (m_dynamicFile)
            m_dynamicFile->close();
        delete m_dynamicFile;
        m_dynamicFile = nullptr;

        qDeleteAll(m_buckets);
        m_buckets.clear();

        std::fill_n(m_firstBucketForHash, bucketHashSize, 0);
    }

    int finalCleanup() final
    {
        int changed = 0;
        for (int a = 1; a <= m_currentBucket; ++a) {
            MyBucket* bucket = bucketForIndex(a);
            if (bucket && bucket->dirty()) { ///@todo Faster dirty check, without loading bucket
                changed += bucket->finalCleanup(*this);
            }
            a += bucket->monsterBucketExtent(); // Skip buckets that are attached as tail to monster-buckets
        }

        return changed;
    }

    uint usedMemory() const
    {
        uint used = 0;
        for (auto* bucket : m_buckets) {
            if (bucket) {
                used += bucket->usedMemory();
            }
        }

        return used;
    }

    uint createIndex(ushort bucketIndex, ushort indexInBucket) const
    {
        //Combine the index in the bucket, and the bucket number into one index
        const uint index = (bucketIndex << 16) + indexInBucket;
        verifyIndex(index);
        return index;
    }

    /**
     * Walks through all buckets clashing with @p hash
     *
     * Will return the value returned by the lambda, returning early if truthy
     */
    template<typename Visitor>
    auto walkBucketChain(size_t hash, const Visitor& visitor) const -> decltype(visitor(0, nullptr))
    {
        unsigned short bucketIndex = m_firstBucketForHash[hash % bucketHashSize];

        while (bucketIndex) {
            auto* bucketPtr = bucketForIndex(bucketIndex);

            if (auto visitResult = visitor(bucketIndex, bucketPtr)) {
                return visitResult;
            }

            bucketIndex = bucketPtr->nextBucketForHash(hash);
        }

        return {}; // clazy:exclude=returning-void-expression
    }

    ///Makes sure the order within m_freeSpaceBuckets is correct, after largestFreeSize has been changed for m_freeSpaceBuckets[index].
    ///If too few space is free within the given bucket, it is removed from m_freeSpaceBuckets.
    void updateFreeSpaceOrder(uint index)
    {
        m_metaDataChanged = true;

        unsigned int* freeSpaceBuckets = m_freeSpaceBuckets.data();

        Q_ASSERT(index < static_cast<uint>(m_freeSpaceBuckets.size()));
        MyBucket* bucketPtr = bucketForIndex(freeSpaceBuckets[index]);

        unsigned short largestFreeSize = bucketPtr->largestFreeSize();

        if (largestFreeSize == 0 ||
            (bucketPtr->freeItemCount() <= MyBucket::MaxFreeItemsForHide &&
             largestFreeSize <= MyBucket::MaxFreeSizeForHide)) {
            //Remove the item from freeSpaceBuckets
            m_freeSpaceBuckets.remove(index);
        } else {
            while (1) {
                int prev = index - 1;
                int next = index + 1;
                if (prev >= 0 && (bucketForIndex(freeSpaceBuckets[prev])->largestFreeSize() > largestFreeSize ||
                                  (bucketForIndex(freeSpaceBuckets[prev])->largestFreeSize() == largestFreeSize &&
                                   freeSpaceBuckets[index] < freeSpaceBuckets[prev]))
                ) {
                    //This item should be behind the successor, either because it has a lower largestFreeSize, or because the index is lower
                    uint oldPrevValue = freeSpaceBuckets[prev];
                    freeSpaceBuckets[prev] = freeSpaceBuckets[index];
                    freeSpaceBuckets[index] = oldPrevValue;
                    index = prev;
                } else if (next < m_freeSpaceBuckets.size()
                           && (bucketForIndex(freeSpaceBuckets[next])->largestFreeSize() < largestFreeSize
                               || (bucketForIndex(freeSpaceBuckets[next])->largestFreeSize() == largestFreeSize
                                   && freeSpaceBuckets[index] > freeSpaceBuckets[next]))) {
                    //This item should be behind the successor, either because it has a higher largestFreeSize, or because the index is higher
                    uint oldNextValue = freeSpaceBuckets[next];
                    freeSpaceBuckets[next] = freeSpaceBuckets[index];
                    freeSpaceBuckets[index] = oldNextValue;
                    index = next;
                } else {
                    break;
                }
            }
        }
    }

    ///Does conversion from monster-bucket to normal bucket and from normal bucket to monster-bucket
    ///The bucket @param bucketNumber must already be loaded and empty. the "extent" buckets behind must also be loaded,
    ///and also be empty.
    ///The created buckets are not registered anywhere. When converting from monster-bucket to normal bucket,
    ///oldExtent+1 normal buckets are created, that must be registered somewhere.
    ///@warning During conversion, all the touched buckets are deleted and re-created
    ///@param extent When this is zero, the bucket is converted from monster-bucket to normal bucket.
    ///              When it is nonzero, it is converted to a monster-bucket.
    MyBucket* convertMonsterBucket(int bucketNumber, int extent)
    {
        m_metaDataChanged = true;

        Q_ASSERT(bucketNumber);
        auto* bucketPtr = bucketForIndex(bucketNumber);

        // the bucket may have encountered hash clashes in the past, we need to keep that data alive
        // note that all following buckets that got merged with the first bucket to create the monster
        // are guaranteed to _not_ have any next buckets, which is asserted in `deleteBucket`
        auto oldNextBucketHash = bucketPtr->takeNextBucketHash();

        if (extent) {
            //Convert to monster-bucket
            Q_ASSERT(bucketPtr->isEmpty());

            const auto monsterStart = bucketNumber;
            const auto monsterEnd = monsterStart + extent + 1;

            // NOTE: see assertion below, when we get here then the entry for `bucketNumber in `m_freeSpaceBuckets`
            //       will be followed by the entries for the following buckets, because they are all free and thus
            //       the second level ordering by bucket index is all that matters
            const auto freeSpaceIndex = m_freeSpaceBuckets.indexOf(bucketNumber);
            Q_ASSERT(freeSpaceIndex != -1);

#ifdef DEBUG_MONSTERBUCKETS
            for (int offset = 0; offset < extent + 1; ++offset) {
                auto bucket = bucketForIndex(bucketNumber + offset);
                Q_ASSERT(bucket->isEmpty());
                Q_ASSERT(!bucket->monsterBucketExtent());
                Q_ASSERT(!m_monsterBucketTailMarker[bucketNumber + offset]);
                // verify that the m_freeSpaceBuckets is ordered the way we expect it to
                Q_ASSERT(m_freeSpaceBuckets[freeSpaceIndex + offset] == static_cast<uint>(bucketNumber + offset));
            }
#endif

            // the following buckets are not free anymore, they get merged into the monster
            // NOTE: we assert above that the order of the entries is correct
            m_freeSpaceBuckets.remove(freeSpaceIndex, extent + 1);

            for (int index = monsterStart; index < monsterEnd; ++index)
                deleteBucket(index);

            bucketPtr = new MyBucket();
            bucketPtr->initialize(extent, std::move(oldNextBucketHash));

            Q_ASSERT(!m_buckets[bucketNumber]);
            m_buckets[bucketNumber] = bucketPtr;

            // mark the tail of the monster bucket
            std::fill(std::next(m_monsterBucketTailMarker.begin(), monsterStart + 1),
                      std::next(m_monsterBucketTailMarker.begin(), monsterEnd), true);

#ifdef DEBUG_MONSTERBUCKETS
            // all following buckets are deleted and not marked as free anymore
            for (int index = monsterStart + 1; index < monsterEnd; ++index) {
                Q_ASSERT(!m_buckets[index]);
                Q_ASSERT(!m_freeSpaceBuckets.contains(index));

                // all tail buckets are marked as part of the monster now
                Q_ASSERT(m_monsterBucketTailMarker[index]);
            }
#endif

            // but the new monster bucket is still free
            Q_ASSERT(bucketPtr->isEmpty());
            // and it itself is not marked as a tail, to allow us to still look things up in it directly
            Q_ASSERT(!m_monsterBucketTailMarker[bucketNumber]);
            // monster buckets don't get put into the m_freeSpaceBuckets list
            Q_ASSERT(!m_freeSpaceBuckets.contains(bucketNumber));
        } else {
            const auto oldExtent = bucketPtr->monsterBucketExtent();
            const auto oldMonsterStart = bucketNumber;
            const auto oldMonsterEnd = oldMonsterStart + oldExtent + 1;

            Q_ASSERT(bucketPtr->monsterBucketExtent());
            Q_ASSERT(bucketPtr->isEmpty());
            // while empty, a monster bucket never is put into the m_freeSpaceBuckets list
            Q_ASSERT(!m_freeSpaceBuckets.contains(bucketNumber));
            // the monster itself is not a tail
            Q_ASSERT(!m_monsterBucketTailMarker[bucketNumber]);

#ifdef DEBUG_MONSTERBUCKETS
            // all buckets that are part of the monster are marked as such
            for (int index = oldMonsterStart + 1; index < oldMonsterEnd; ++index) {
                Q_ASSERT(m_monsterBucketTailMarker[index]);
            }
#endif

            // delete the monster bucket
            deleteBucket(bucketNumber);

            // remove markers for the tail of the monster bucket
            std::fill(std::next(m_monsterBucketTailMarker.begin(), oldMonsterStart + 1),
                      std::next(m_monsterBucketTailMarker.begin(), oldMonsterEnd), false);

            // recreate non-monster buckets
            for (int index = oldMonsterStart; index < oldMonsterEnd; ++index) {
                auto& bucket = m_buckets[index];
                Q_ASSERT(!bucket);

                bucket = new MyBucket();

                if (index == oldMonsterStart) {
                    bucket->initialize(0, std::move(oldNextBucketHash));
                    bucketPtr = bucket;
                } else {
                    bucket->initialize(0);
                }

                Q_ASSERT(!bucket->monsterBucketExtent());
                Q_ASSERT(!m_freeSpaceBuckets.contains(index));

                putIntoFreeList(index, bucket);

                Q_ASSERT(m_freeSpaceBuckets.contains(index));
                Q_ASSERT(!m_monsterBucketTailMarker[index]);
            }
        }

        Q_ASSERT(bucketPtr == m_buckets[bucketNumber]);
        return bucketPtr;
    }

    MyBucket* bucketForIndex(short unsigned int index) const
    {
        Q_ASSERT(index);
        MyBucket* bucketPtr = m_buckets.at(index);
        if (!bucketPtr) {
            bucketPtr = initializeBucket(index);
        }
        return bucketPtr;
    }

    struct AllItemsReachableVisitor {
        explicit AllItemsReachableVisitor(ItemRepository* rep)
            : repository(rep)
        {
        }

        bool operator()(const Item* item) { return repository->itemReachable(item); }

        ItemRepository* repository;
    };

    // Returns whether the given item is reachable through its hash
    bool itemReachable(const Item* item) const
    {
        const size_t hash = item->hash();

        return walkBucketChain(hash, [=](ushort /*bucketIndex*/, const MyBucket* bucketPtr) {
            return bucketPtr->itemReachable(item, hash);
        });
    }

    // Returns true if all items in the given bucket are reachable through their hashes
    bool allItemsReachable(unsigned short bucket)
    {
        if (!bucket)
            return true;

        MyBucket* bucketPtr = bucketForIndex(bucket);

        AllItemsReachableVisitor visitor(this);
        return bucketPtr->visitAllItems(visitor);
    }

    inline MyBucket* initializeBucket(int bucketNumber) const
    {
        using namespace ItemRepositoryUtils;

        Q_ASSERT(bucketNumber);
#ifdef DEBUG_MONSTERBUCKETS
        // ensure that the previous N buckets are no monster buckets that overlap the requested bucket
        for (int offset = 1; offset < 5; ++offset) {
            int test = bucketNumber - offset;
            if (test >= 0 && m_buckets[test]) {
                Q_ASSERT(m_buckets[test]->monsterBucketExtent() < offset);
            }
        }

#endif

        auto& bucket = m_buckets[bucketNumber];
        if (!bucket) {
            bucket = new MyBucket();

            bool doMMapLoading = ( bool )m_fileMap;

            uint offset = ((bucketNumber - 1) * MyBucket::DataSize);
            if (m_file && offset < m_fileMapSize && doMMapLoading &&
                *reinterpret_cast<uint*>(m_fileMap + offset) == 0) {
//         qDebug() << "loading bucket mmap:" << bucketNumber;
                bucket->initializeFromMap(reinterpret_cast<char*>(m_fileMap + offset));
            } else if (m_file) {
                //Either memory-mapping is disabled, or the item is not in the existing memory-map,
                //so we have to load it the classical way.
                bool res = m_file->open(QFile::ReadOnly);

                if (offset + BucketStartOffset < m_file->size()) {
                    VERIFY(res);
                    offset += BucketStartOffset;
                    m_file->seek(offset);
                    int monsterBucketExtent;
                    readValue(m_file, &monsterBucketExtent);
                    m_file->seek(offset);
                    ///FIXME: use the data here instead of copying it again in prepareChange
                    QByteArray data = m_file->read((1 + monsterBucketExtent) * MyBucket::DataSize);
                    bucket->initializeFromMap(data.data());
                    bucket->prepareChange();
                } else {
                    bucket->initialize(0);
                }

                m_file->close();
            } else {
                bucket->initialize(0);
            }
        } else {
            bucket->initialize(0);
        }

        return bucket;
    }

    ///Can only be called on empty buckets
    void deleteBucket(int bucketNumber)
    {
        // NOTE: use bucketForIndex in the assertions here, as the bucket may not be initialized
        //       we still want to verify that we only delete empty buckets though
        Q_ASSERT(bucketForIndex(bucketNumber)->isEmpty());
        Q_ASSERT(bucketForIndex(bucketNumber)->noNextBuckets());
        Q_ASSERT(!m_freeSpaceBuckets.contains(bucketNumber));

        auto& bucket = m_buckets[bucketNumber];
        delete bucket;
        bucket = nullptr;
    }

    //m_file must be opened
    void storeBucket(int bucketNumber) const
    {
        if (!m_file) {
            return;
        }
        auto& bucket = m_buckets[bucketNumber];
        if (bucket) {
            bucket->store(m_file, BucketStartOffset + (bucketNumber - 1) * MyBucket::DataSize);
        }
    }

    /// If mustFindBucket is zero, the whole chain is just walked. This is good for debugging for infinite recursion.
    /// @return whether @p mustFindBucket was found
    bool walkBucketLinks(uint checkBucket, size_t hash, uint mustFindBucket = 0) const
    {
        bool found = false;
        while (checkBucket) {
            if (checkBucket == mustFindBucket)
                found = true;

            checkBucket = bucketForIndex(checkBucket)->nextBucketForHash(hash);
        }
        return found || (mustFindBucket == 0);
    }

    /// Computes the bucket where the chains opened by the buckets @p mainHead and @p intersectorHead
    /// with hash @p hash meet each other.
    /// @return <predecessor of first shared bucket in mainHead, first shared bucket>
    QPair<unsigned int, unsigned int> hashChainIntersection(uint mainHead, uint intersectorHead, size_t hash) const
    {
        uint previous = 0;
        uint current = mainHead;
        while (current) {
            ///@todo Make this more efficient
            if (walkBucketLinks(intersectorHead, hash, current))
                return qMakePair(previous, current);

            previous = current;
            current = bucketForIndex(current)->nextBucketForHash(hash);
        }
        return qMakePair(0u, 0u);
    }

    void putIntoFreeList(unsigned short bucket, MyBucket* bucketPtr)
    {
        Q_ASSERT(bucket);

        Q_ASSERT(!bucketPtr->monsterBucketExtent());
        int indexInFree = m_freeSpaceBuckets.indexOf(bucket);

        if (indexInFree == -1 &&
            (bucketPtr->freeItemCount() >= MyBucket::MinFreeItemsForReuse ||
             bucketPtr->largestFreeSize() >= MyBucket::MinFreeSizeForReuse)) {
            //Add the bucket to the list of buckets from where to re-assign free space
            //We only do it when a specific threshold of empty items is reached, because that way items can stay "somewhat" semantically ordered.
            Q_ASSERT(bucketPtr->largestFreeSize());
            int insertPos;
            for (insertPos = 0; insertPos < m_freeSpaceBuckets.size(); ++insertPos) {
                if (bucketForIndex(m_freeSpaceBuckets[insertPos])->largestFreeSize() > bucketPtr->largestFreeSize())
                    break;
            }

            m_freeSpaceBuckets.insert(insertPos, bucket);

            updateFreeSpaceOrder(insertPos);
        } else if (indexInFree != -1) {
            ///Re-order so the order in m_freeSpaceBuckets is correct(sorted by largest free item size)
            updateFreeSpaceOrder(indexInFree);
        }

        // empty buckets definitely should be in the free space list
        Q_ASSERT(!bucketPtr->isEmpty() || m_freeSpaceBuckets.contains(bucket));
    }

    void verifyIndex(uint index) const
    {
        // We don't use zero indices
        Q_ASSERT(index);
        int bucket = (index >> 16);
        // nor zero buckets
        Q_ASSERT(bucket);
        Q_ASSERT_X(bucket < m_buckets.size(), Q_FUNC_INFO,
                   qPrintable(QStringLiteral("index %1 gives invalid bucket number %2, current count is: %3")
                              .arg(index)
                              .arg(bucket)
                              .arg(m_buckets.size())));

        // don't trigger compile warnings in release mode
        Q_UNUSED(bucket);
        Q_UNUSED(index);
    }

    void allocateNextBuckets(int numNewBuckets)
    {
        Q_ASSERT(numNewBuckets > 0);
        const auto oldSize = m_buckets.size();
        m_buckets.resize(oldSize + numNewBuckets);
        m_monsterBucketTailMarker.resize(m_buckets.size());

        // the buckets we just created can by definition not yet exist and must be empty
        // meaning we can bypass loading from file _and_ we must add them to the freeSpaceBuckets too
        for (int i = oldSize; i < (oldSize + numNewBuckets); ++i) {
            if (i == 0) // see below, the zero bucket is reserved for special purposes
                continue;

            auto& bucket = m_buckets[i];
            Q_ASSERT(!bucket);

            bucket = new MyBucket;
            bucket->initialize(0);

            Q_ASSERT(bucket->isEmpty());
            Q_ASSERT(!bucket->monsterBucketExtent());
            Q_ASSERT(!m_monsterBucketTailMarker[i]);

            putIntoFreeList(i, bucket);
            Q_ASSERT(m_freeSpaceBuckets.contains(i));
        }

        // Skip the first bucket, we won't use it so we have the zero indices for special purposes
        if (m_currentBucket == 0)
            m_currentBucket = 1;
    }

    bool m_metaDataChanged = true;
    bool m_unloadingEnabled = true;
    // an unused, empty repo has no bucket yet
    // on first use this will then get incremented directly as we never use the zero-bucket index
    // that value is reserved for special purposes instead
    mutable int m_currentBucket = 0;

    //List of buckets that have free space available that can be assigned. Sorted by size: Smallest space first. Second order sorting: Bucket index
    QVector<uint> m_freeSpaceBuckets;
    // every bucket that is part of a monster bucket but not the "main" bucket gets marked in this list,
    // this allows us to ensure we don't try to put an entry into such a bucket later on, which would corrupt data
    QVector<bool> m_monsterBucketTailMarker;
    //List of hash map buckets that actually hold the data of the item repository
    mutable QVector<MyBucket*> m_buckets;
    uint m_statBucketHashClashes = 0;
    uint m_statItemCount = 0;
    //Maps hash-values modulo 1<<bucketHashSizeBits to the first bucket such a hash-value appears in
    short unsigned int m_firstBucketForHash[bucketHashSize] = { 0 };

    //File that contains the buckets
    QFile* m_file = nullptr;
    uchar* m_fileMap = nullptr;
    uint m_fileMapSize = 0;
    //File that contains more dynamic data, like the list of buckets with deleted items
    QFile* m_dynamicFile = nullptr;

    const QString m_repositoryName;
    const uint m_repositoryVersion;
    Mutex* const m_mutex;
    ItemRepositoryRegistry* const m_registry;

    friend class ::TestItemRepository;
    friend class ::BenchItemRepository;
};

template<typename Context>
class ItemRepositoryFor;

struct LockedItemRepository {
    template<typename Context, typename Op>
    static auto read(Op&& op)
    {
        const auto& repo = ItemRepositoryFor<Context>::repo();

        QMutexLocker lock(repo.mutex());
        return op(repo);
    }

    template<typename Context, typename Op>
    static auto write(Op&& op)
    {
        auto& repo = ItemRepositoryFor<Context>::repo();

        QMutexLocker lock(repo.mutex());
        return op(repo);
    }

    template<typename Context>
    static void initialize()
    {
        ItemRepositoryFor<Context>::repo();
    }
};
}

#endif
