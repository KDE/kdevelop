#include "ParsedVariable.h"

CParsedVariable::CParsedVariable( )
{
    // setting default values
    setDefault( );
}

CParsedVariable::CParsedVariable( CParsedVariable& cp )
{
    *this = cp;
}

const CParsedVariable&
CParsedVariable::operator = ( CParsedVariable& cp )
{
    iVariableValue = cp.iVariableValue;
    iVariableType  = cp.iVariableType;
    sVariableName  = cp.sVariableName;
    sVariableType  = cp.sVariableType;
    iLine          = cp.iLine;
    scope          = cp.scope;
}

void
CParsedVariable::setDefault( )
{
    iVariableValue = CPUNKNOWN;
    iVariableType  = CPNONE;
    sVariableName  = "";
    sVariableType  = "";
    iLine          = 0;
    scope.reset( );
}

bool
CParsedVariable::isDefault( )
{
    return ( iVariableValue ==  CPUNKNOWN && sVariableType == "" ? true : false );
}



