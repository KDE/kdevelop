/*
 * file     : parsedvariable.h
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@htw-dresden.de
 * license  : gpl version >= 2
 *
 */

#define ENABLEMESSAGEOUTPUT
#define ENABLEDEBUGOUTPUT
#include "dbg.h"

#include "parsedvariable.h"

CParsedVariable::CParsedVariable( )
{
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

    return *this;
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
    return ( iVariableValue == CPUNKNOWN && sVariableType == "" ? true : false );
}
