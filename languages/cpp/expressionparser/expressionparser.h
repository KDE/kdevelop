#ifndef EXPRESSIONPARSER_H
#define EXPRESSIONPARSER_H

#include <ksharedptr.h>
#include "cppexpressionparserexport.h"

class TranslationUnitAST;
class AST;
namespace KDevelop {
class DUContext;
  
class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionEvaluationResult : public KShared {
  public:
    ExpressionEvaluationResult() : ast(0), isMacro(0) {
    }
    ~ExpressionEvaluationResult() {
      delete ast;
    }

    AST* ast;
    bool isMacro;
    typedef KSharedPtr<ExpressionEvaluationResult> Pointer;
};

class KDEVCPPEXPRESSIONPARSER_EXPORT ExpressionParser {
  public:
    /**
     * Evaluates the type of an expression given as a string within a given context
     *
     * @param exp The expression to evaluate
     * @param context the context within which the expression should be evaluated
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult::Pointer evaluateType( const QByteArray& exp, DUContext* context, bool debug = true );
    /**
     * Evaluates the type of an expression given as an AST.
     *
     * @param ast the AST. It's context must be built already(context-member filled).
     * @param debug whether additional output to kdDebug should be issued
    */
    ExpressionEvaluationResult::Pointer evaluateType( AST* ast, bool debug = true );
};

}
#endif
