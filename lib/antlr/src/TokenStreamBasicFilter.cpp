#include "antlr/TokenStreamBasicFilter.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

/** This object is a TokenStream that passes through all
 *  tokens except for those that you tell it to discard.
 *  There is no buffering of the tokens.
 */
TokenStreamBasicFilter::TokenStreamBasicFilter(TokenStream& input_)
: input(&input_)
{
}

void TokenStreamBasicFilter::discard(int ttype)
{
	discardMask.add(ttype);
}

void TokenStreamBasicFilter::discard(const BitSet& mask)
{
	discardMask = mask;
}

RefToken TokenStreamBasicFilter::nextToken()
{
	RefToken tok = input->nextToken();
	while ( tok && discardMask.member(tok->getType()) ) {
		tok = input->nextToken();
	}
	return tok;
}

ANTLR_END_NAMESPACE

