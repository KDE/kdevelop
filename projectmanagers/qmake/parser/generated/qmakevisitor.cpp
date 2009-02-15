// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmakevisitor.h"

namespace QMake
{

Visitor::ParserFuncType Visitor::sParserTable[] =
{
    reinterpret_cast<ParserFuncType>(&Visitor::visitArgumentList),
    reinterpret_cast<ParserFuncType>(&Visitor::visitFunctionArguments),
    reinterpret_cast<ParserFuncType>(&Visitor::visitItem),
    reinterpret_cast<ParserFuncType>(&Visitor::visitOp),
    reinterpret_cast<ParserFuncType>(&Visitor::visitOrOperator),
    reinterpret_cast<ParserFuncType>(&Visitor::visitProject),
    reinterpret_cast<ParserFuncType>(&Visitor::visitScope),
    reinterpret_cast<ParserFuncType>(&Visitor::visitScopeBody),
    reinterpret_cast<ParserFuncType>(&Visitor::visitStatement),
    reinterpret_cast<ParserFuncType>(&Visitor::visitValue),
    reinterpret_cast<ParserFuncType>(&Visitor::visitValueList),
    reinterpret_cast<ParserFuncType>(&Visitor::visitVariableAssignment)
}; // sParserTable[]

} // end of namespace QMake

