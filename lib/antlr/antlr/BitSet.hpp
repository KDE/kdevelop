#ifndef INC_BitSet_hpp__
#define INC_BitSet_hpp__

#include "antlr/config.hpp"
#include <vector>
#include <stdexcept>

ANTLR_BEGIN_NAMESPACE(antlr)

/**A BitSet to replace java.util.BitSet.
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
class BitSet {
private:
	ANTLR_USE_NAMESPACE(std)vector<bool> storage;

public:
	BitSet(int nbits=64);
	BitSet(const unsigned long* bits_,int nlongs);
	~BitSet();

	void add(int el);

	bool member(int el) const;

	ANTLR_USE_NAMESPACE(std)vector<int> toArray() const;
};

ANTLR_END_NAMESPACE

#endif //INC_BitSet_hpp__
