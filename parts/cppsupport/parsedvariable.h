/*
 * file     : parsedvariable.h
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@htw-dresden.de
 * license  : gpl version >= 2
 *
 */

#ifndef _PARSEDVARIABLE_H
#define _PARSEDVARIABLE_H

#include <qstring.h>
#include "tokenizer-cc.h"
#include "scope.h"

/**
 * this class represents a parsed variable
 */
class CParsedVariable {
public:

    /**
    * constructor
    */
    CParsedVariable( );

    /**
    * copy constructor
    */
    CParsedVariable( CParsedVariable& cp );

    /**
    * operator =
    */
    const CParsedVariable& operator = ( CParsedVariable& cp );

    /**
    * set default values
    */
    void setDefault( );

    /**
    * returns true if only defaults are stored
    */
    bool isDefault( );

    /** iVariableValue : standard types (int, char,... ), CPUNKNOWN */
    int iVariableValue;

    /** iVariableType : pointer, reference, CPNONE */
    int iVariableType;

    /** sName = variable name -> "myVar", ...  */
    QString sVariableName;

    /** sVariableType: if CPUNKNOWN ( = "" ) -> "QString", ... */
    QString sVariableType;

    /** iLine: found at line */
    int iLine;

    /** scope: numbered scope that variable belongs to */
    CScope scope;
};

#endif
