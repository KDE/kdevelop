#ifndef INC_TokenStreamIOException_hpp__
#define INC_TokenStreamIOException_hpp__

#include "antlr/config.hpp"
#include "antlr/TokenStreamException.hpp"
#include <exception>

ANTLR_BEGIN_NAMESPACE(antlr)

class TokenStreamIOException : public TokenStreamException {
public:
	ANTLR_USE_NAMESPACE(std)exception io;

	TokenStreamIOException() {}
	TokenStreamIOException(const ANTLR_USE_NAMESPACE(std)exception& e)
		: TokenStreamException(e.what()), io(e) {}
	~TokenStreamIOException() throw() {}
};

ANTLR_END_NAMESPACE

#endif //INC_TokenStreamIOException_hpp__
