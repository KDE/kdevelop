#ifndef INC_TokenBuffer_hpp__
#define INC_TokenBuffer_hpp__

/* ANTLR Translator Generator
 * Project led by Terence Parr at http://www.jGuru.com
 * Software rights: http://www.antlr.org/RIGHTS.html
 *
 */

#include <antlr/config.hpp>
#include <antlr/TokenStream.hpp>
#include <antlr/CircularQueue.hpp>

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
namespace antlr {
#endif

/**A Stream of Token objects fed to the parser from a TokenStream that can
 * be rewound via mark()/rewind() methods.
 * <p>
 * A dynamic array is used to buffer up all the input tokens.  Normally,
 * "k" tokens are stored in the buffer.  More tokens may be stored during
 * guess mode (testing syntactic predicate), or when LT(i>k) is referenced.
 * Consumption of tokens is deferred.  In other words, reading the next
 * token is not done by conume(), but deferred until needed by LA or LT.
 * <p>
 *
 * @todo see if we can integrate this one with InputBuffer into one template
 * or so.
 *
 * @see antlr.Token
 * @see antlr.TokenStream
 * @see antlr.TokenQueue
 */
class ANTLR_API TokenBuffer {
public:
	/** Create a token buffer */
	TokenBuffer(TokenStream& input_);

	/// Reset the input buffer to empty state
	inline void reset( void )
	{
		nMarkers = 0;
		markerOffset = 0;
		numToConsume = 0;
		queue.clear();
	}

	/** Get a lookahead token value */
	int LA(int i);

	/** Get a lookahead token */
	RefToken LT(int i);

	/** Return an integer marker that can be used to rewind the buffer to
	 * its current state.
	 */
	int mark();

	/**Rewind the token buffer to a marker.
	 * @param mark Marker returned previously from mark()
	 */
	void rewind(int mark);

	/** Mark another token for deferred consumption */
	inline void consume()
	{
		numToConsume++;
	}
private:
	/** Ensure that the token buffer is sufficiently full */
	void fill(int amount);
	/** Sync up deferred consumption */
	void syncConsume();

protected:
	// Token source
	TokenStream& input;

private:
	// Number of active markers
	int nMarkers;

	// Additional offset used when markers are active
	int markerOffset;

	// Number of calls to consume() since last LA() or LT() call
	int numToConsume;

	// Circular queue
	CircularQueue<RefToken> queue;

private:
	TokenBuffer(const TokenBuffer& other);
	const TokenBuffer& operator=(const TokenBuffer& other);
};

/** Sync up deferred consumption */
inline void TokenBuffer::syncConsume()
{
	if (numToConsume > 0)
	{
		if (nMarkers > 0)
			markerOffset += numToConsume;
		else
			queue.removeItems( numToConsume );

		numToConsume = 0;
	}
}

#ifdef ANTLR_CXX_SUPPORTS_NAMESPACE
}
#endif

#endif //INC_TokenBuffer_hpp__
