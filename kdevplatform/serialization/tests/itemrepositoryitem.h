#pragma once

#include "serialization/indexedstring.h"
#include "serialization/itemrepository.h"

struct TestItem
{
    TestItem(uint hash, uint dataSize);
    TestItem& operator=(const TestItem& rhs) = delete;
    unsigned int hash() const;
    unsigned int itemSize() const;
    bool equals(const TestItem* rhs) const;
    std::string_view as_string_view() const;
    static void freeItem(TestItem* ptr);

    uint m_hash;
    uint m_dataSize;
};

struct TestItemDeleter
{
    void operator()(TestItem* ptr)
    {
        TestItem::freeItem(ptr);
    }
};

using TestItemPtr = std::unique_ptr<TestItem, TestItemDeleter>;

struct TestItemRequest
{
    TestItem& m_item;
    bool m_compareData;

    TestItemRequest(TestItem& item, bool compareData = false);

    enum {
        AverageSize = 700 //This should be the approximate average size of an Item
    };

    uint hash() const;

    //Should return the size of an item created with createItem
    uint itemSize() const;

    void createItem(TestItem* item) const;

    static void destroy(TestItem* /*item*/, KDevelop::AbstractItemRepository&);

    static bool persistent(const TestItem* /*item*/);

    //Should return whether the here requested item equals the given item
    bool equals(const TestItem* item) const;
};

TestItem* createItem(uint id, uint size);
