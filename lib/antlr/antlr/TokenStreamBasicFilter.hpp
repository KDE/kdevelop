#ifndef INC_TokenStreamBasicFilter_hpp__
#define INC_TokenStreamBasicFilter_hpp__

#include "antlr/config.hpp"
#include "antlr/BitSet.hpp"
#include "antlr/TokenStream.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/** This object is a TokenStream that passes through all
 *  tokens except for those that you tell it to discard.
 *  There is no buffering of the tokens.
 */
class TokenStreamBasicFilter : public TokenStream {
	/** The set of token types to discard */
protected:
	BitSet discardMask;

	/** The input stream */
protected:
	TokenStream* input;

public:
	TokenStreamBasicFilter(TokenStream& input_);

	void discard(int ttype);

	void discard(const BitSet& mask);

	RefToken nextToken();
};

ANTLR_END_NAMESPACE

#endif //INC_TokenStreamBasicFilter_hpp__
