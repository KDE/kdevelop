#ifndef INC_CharStreamIOException_hpp__
#define INC_CharStreamIOException_hpp__

#include "antlr/config.hpp"
#include "antlr/CharStreamException.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

class CharStreamIOException : public CharStreamException {
public:
	ANTLR_USE_NAMESPACE(std)exception io;

	CharStreamIOException(ANTLR_USE_NAMESPACE(std)exception& e)
		: CharStreamException(e.what()), io(e) {}
	~CharStreamIOException() throw() {}
};

ANTLR_END_NAMESPACE

#endif //INC_CharStreamIOException_hpp__
