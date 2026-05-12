#include "itemrepositoryitem.h"

TestItem::TestItem(uint hash, uint dataSize)
    : m_hash(hash)
    , m_dataSize(dataSize)
{
}

//Every item has to implement this function, and return a valid hash.
//Must be exactly the same hash value as ExampleItemRequest::hash() has returned while creating the item.
unsigned int TestItem::hash() const
{
    return m_hash;
}

//Every item has to implement this function, and return the complete size this item takes in memory.
//Must be exactly the same value as ExampleItemRequest::itemSize() has returned while creating the item.
unsigned int TestItem::itemSize() const
{
    return sizeof(TestItem) + m_dataSize;
}
std::string_view TestItem::as_string_view() const
{
    return std::string_view(reinterpret_cast<const char*>(this) + sizeof(*this), m_dataSize);
}

bool TestItem::equals(const TestItem* rhs) const
{
    return rhs->m_hash == m_hash && itemSize() == rhs->itemSize()
        && memcmp(reinterpret_cast<const char*>(this), rhs, itemSize()) == 0;
}

void TestItem::freeItem(TestItem* ptr)
{
    if (!ptr)
        return;
    // No op, but needed for correctness.
    ptr->~TestItem();
    // ptr was allocated with "new char[]", so we must use delete[] (char*) to free it.
    delete[] reinterpret_cast<char*>(ptr);
}

TestItemRequest::TestItemRequest(TestItem& item, bool compareData)
    : m_item(item)
    , m_compareData(compareData)
{
}
uint TestItemRequest::hash() const
{
    return m_item.hash();
}

//Should return the size of an item created with createItem
uint TestItemRequest::itemSize() const
{
    return m_item.itemSize();
}

void TestItemRequest::createItem(TestItem* item) const
{
    memcpy(reinterpret_cast<void*>(item), &m_item, m_item.itemSize());
}

void TestItemRequest::destroy(TestItem* /*item*/, KDevelop::AbstractItemRepository&)
{
    //Nothing to do
}

bool TestItemRequest::persistent(const TestItem* /*item*/)
{
    return true;
}

//Should return whether the here requested item equals the given item
bool TestItemRequest::equals(const TestItem* item) const
{
    return hash() == item->hash() && (!m_compareData || m_item.equals(item));
}

TestItem* createItem(uint id, uint size)
{
    TestItem* ret;
    char* data = new char[size];
    uint dataSize = size - sizeof(TestItem);
    ret = new (data) TestItem(id, dataSize);

    //Fill in same random pattern
    for (uint a = 0; a < dataSize; ++a)
        data[sizeof(TestItem) + a] = (char)(a + id);

    return ret;
}
