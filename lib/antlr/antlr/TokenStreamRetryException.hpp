#ifndef INC_TokenStreamRetryException_hpp__
#define INC_TokenStreamRetryException_hpp__

#include "antlr/config.hpp"
#include "antlr/TokenStreamException.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

class TokenStreamRetryException : public TokenStreamException {
public:
	TokenStreamRetryException() {}
	~TokenStreamRetryException() throw() {}
};

ANTLR_END_NAMESPACE

#endif //INC_TokenStreamRetryException_hpp__
