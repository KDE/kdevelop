#ifndef INC_CircularQueue_hpp__
#define INC_CircularQueue_hpp__

/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include <antlr/config.hpp>
#include <vector>

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

// Resize every 5000 items
#define OFFSET_MAX_RESIZE 5000

template <class T>
class ANTLR_API CircularQueue {
public:
	CircularQueue()
	: storage(), m_offset(0)
	{
	}
	~CircularQueue()
	{
	}

	/// Clear the queue
	inline void clear( void )
	{
		m_offset = 0;
		storage.clear();
	}

	/// @todo this should use at or
	inline T elementAt(int idx) const
	{
		return storage[idx+m_offset];
	}
	void removeFirst()
	{
		if (m_offset >= OFFSET_MAX_RESIZE)
		{
			storage.erase( storage.begin(), storage.begin() + m_offset + 1 );
			m_offset = 0;
		}
		else
			++m_offset;
	}
	inline void removeItems( int nb )
	{
		if (m_offset >= OFFSET_MAX_RESIZE)
		{
			storage.erase( storage.begin(), storage.begin() + m_offset + nb );
			m_offset = 0;
		}
		else
			m_offset+=nb;
	}
	inline void append(const T& t)
	{
		storage.push_back(t);
	}
	inline int entries() const
	{
		return storage.size()-m_offset;
	}

private:
	typename ANTLR_USE_NAMESPACE(std)vector<T> storage;
	int m_offset;

	CircularQueue(const CircularQueue&);
	const CircularQueue& operator=(const CircularQueue&);
};

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif

#endif //INC_CircularQueue_hpp__
