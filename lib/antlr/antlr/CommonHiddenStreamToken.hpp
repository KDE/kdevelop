#ifndef INC_CommonHiddenStreamToken_hpp__
#define INC_CommonHiddenStreamToken_hpp__

#include "antlr/config.hpp"
#include "antlr/CommonToken.hpp"

ANTLR_BEGIN_NAMESPACE(antlr)

class CommonHiddenStreamToken : public CommonToken {
protected:
	RefToken hiddenBefore;
	RefToken hiddenAfter;

public:
	CommonHiddenStreamToken();
	CommonHiddenStreamToken(int t, const ANTLR_USE_NAMESPACE(std)string& txt);
	CommonHiddenStreamToken(const ANTLR_USE_NAMESPACE(std)string& s);

	RefToken getHiddenAfter();
	RefToken getHiddenBefore();

   static RefToken factory();

	void setHiddenAfter(RefToken t);
	void setHiddenBefore(RefToken t);
};

ANTLR_END_NAMESPACE

#endif //INC_CommonHiddenStreamToken_hpp__
