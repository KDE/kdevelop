#ifndef INC_CharStreamException_hpp__
#define INC_CharStreamException_hpp__

#include "antlr/config.hpp"
#include "antlr/ANTLRException.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

class CharStreamException : public ANTLRException {
public:
	CharStreamException(const ANTLR_USE_NAMESPACE(std)string& s)
		: ANTLRException(s) {}
	~CharStreamException() throw() {}
};

ANTLR_END_NAMESPACE

#endif //INC_CharStreamException_hpp__
