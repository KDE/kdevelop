#include "antlr/BitSet.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/** A BitSet to replace java.util.BitSet.
 * Primary differences are that most set operators return new sets
 * as opposed to oring and anding "in place".  Further, a number of
 * operations were added.  I cannot contain a BitSet because there
 * is no way to access the internal bits (which I need for speed)
 * and, because it is final, I cannot subclass to add functionality.
 * Consider defining set degree.  Without access to the bits, I must
 * call a method n times to test the ith bit...ack!
 *
 * Also seems like or() from util is wrong when size of incoming set is bigger
 * than this.length.
 *
 * This is a C++ version of the Java class described above, with only
 * a handful of the methods implemented, because we don't need the
 * others at runtime. It's really just a wrapper around vector<bool>,
 * which should probably be changed to a wrapper around bitset, once
 * bitset is more widely available.
 *
 * @author Terence Parr, MageLang Institute
 * @author <br><a href="mailto:pete@yamuna.demon.co.uk">Pete Wells</a>
 */
BitSet::BitSet(int nbits)
	: storage(nbits)
{
	for (int i=0;i<nbits;i++) {
		storage[i] = false;
	}
}

BitSet::BitSet(const unsigned long* bits_,int nlongs)
	: storage(nlongs*32)
{
	for ( int i = 0 ; i < nlongs*32; i++) {
		storage[i] = (bits_[i>>5] & (1UL << (i&31))) ? true : false;
	}
}

BitSet::~BitSet()
{
}

void BitSet::add(int el)
{
	if ( el < 0 )
		throw ANTLR_USE_NAMESPACE(std)out_of_range(ANTLR_USE_NAMESPACE(std)string("antlr::BitSet.cpp line 49"));

	if( static_cast<unsigned int>(el) >= storage.size() )
		storage.resize( el+1, false );

	storage[el] = true;
}

bool BitSet::member(int el) const
{
	if ( el < 0 || static_cast<unsigned int>(el) >= storage.size())
		return false;

	return storage[el];
}

ANTLR_USE_NAMESPACE(std)vector<int> BitSet::toArray() const
{
	ANTLR_USE_NAMESPACE(std)vector<int> elems;
	for (unsigned int i=0;i<storage.size();i++) {
		if (storage[i])
			elems.push_back(i);
	}

	return elems;
}

ANTLR_END_NAMESPACE
