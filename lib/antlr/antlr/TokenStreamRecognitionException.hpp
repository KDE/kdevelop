#ifndef INC_TokenStreamRecognitionException_hpp__
#define INC_TokenStreamRecognitionException_hpp__

#include "antlr/config.hpp"
#include "antlr/TokenStreamException.hpp"
#include <exception>

ANTLR_BEGIN_NAMESPACE(antlr)

class TokenStreamRecognitionException : public TokenStreamException {
public:
	RecognitionException recog;

	TokenStreamRecognitionException(RecognitionException& re)
		: TokenStreamException(re.getMessage()), recog(re) {}
	~TokenStreamRecognitionException() throw() {}
};

ANTLR_END_NAMESPACE

#endif //INC_TokenStreamRecognitionException_hpp__
