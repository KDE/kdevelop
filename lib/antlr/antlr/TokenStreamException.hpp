#ifndef INC_TokenStreamException_hpp__
#define INC_TokenStreamException_hpp__

#include "antlr/config.hpp"
#include "antlr/ANTLRException.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

class TokenStreamException : public ANTLRException {
public:
	TokenStreamException() {}
	TokenStreamException(const ANTLR_USE_NAMESPACE(std)string& s)
		: ANTLRException(s) {}
	virtual ~TokenStreamException() throw() {}
};

ANTLR_END_NAMESPACE

#endif //INC_TokenStreamException_hpp__
