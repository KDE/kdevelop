/*
 * file     : codecompletionparser.cpp
 * begin    : 2001
 * copyright: (c) by daniel engelschalt
 * email    : daniel.engelschalt@gmx.net
 * license  : gpl version >= 2
 */

#define ENABLEMESSAGEOUTPUT
#define ENABLEDEBUGOUTPUT

#include "cppccparser.h"
#include "dbg.h"
#include <kdebug.h>


#define PUSH_LEXEM( ) lexemStack.push( new CParsedLexem( lexem, getText( ) ) )

CppCCParser::CppCCParser( )
{
}

CppCCParser::~CppCCParser( )
{
}

bool
CppCCParser::parse( const QString& file, const int iCCLine_ )
{
    ifstream f( file.latin1( ) );
    if( f.is_open( ) ){
        iCCLine = iCCLine_;
        currentFile = file;
        parseObject( f );
        return true;
    }
    else {
        kdDebug(9007) << "EE: CppCCParser::parse file '" << file << "' couldn't opened" << endl;
    }

    return false;
}

bool
CppCCParser::parse( const QString* string, const int iCCLine_ )
{
    if( string ){
        istrstream s( string->latin1( ) );
        iCCLine = iCCLine_;
        currentFile = QString::null;
        parseObject( s );
        return true;
    }
    else {
        kdDebug(9007) << "EE: CppCCParser::parse string = 0" << endl;
    }

    return false;
}

bool
CppCCParser::parse( const QString* string, const QString variable )
{
    if( string ){
        istrstream s( string->latin1( ) );
        iCCLine = 999999;
        currentFile = QString::null;
        m_variable = variable;
        parseObject( s );
        postProcessVariables( );
        return true;
    }
    else {
        kdDebug(9007) << "EE: CppCCParser::parse string = 0" << endl;
    }

    return false;
}

bool
CppCCParser::parse( const QString& file, const QString variable )
{
    ifstream f( file.latin1( ) );
    if( f.is_open( ) ){
        currentFile = file;
        m_variable = variable;
        iCCLine = 999999;
        parseObject( f );
        postProcessVariables( );
        return true;
    }
    else {
        kdDebug(9007) << "EE: CppCCParser::parse file '" << file << "' couldn't opened" << endl;
    }

    return false;
}

void
CppCCParser::parseObject( istream& object )
{
    lexem = -1;
    lexer = new yyFlexLexer( &object );
    parseTopLevel( );
    delete lexer;
}

void
CppCCParser::parseTopLevel( )
{
    // according to Victor and Daniel we will only provide one-method-parsing
    // so that the object only contains one method

    // parsing the function header
    parseFunctionHead( );

    // parsing the function body
    parseFunctionBody( );
}


void
CppCCParser::parseFunctionHead( )
{
    outln( "Parsing function head starting" );

    // here we store what we found
    CParsedVariable* pVar = 0;

    // current Scope - always 1 for "within function scope" ;)
    CScope tmpScope( 1 );
    m_lastScope = tmpScope;

    // skipping up to the arguments
    skipToLexem( '(' );

    // header ends with '{' || ';' || EOF
    while( lexem != '{' && lexem != ';' && lexem != 0 ){
        getNextLexem( );
        // create new variable-data holder and setting current ( = standard ) scope
        if( !pVar ){
            pVar = new CParsedVariable;
            pVar->scope = m_lastScope;
        }

        // check what we found
        switch( lexem ){
            // standard values first
            case CPINT  : outln( " + INT" );
                pVar->iVariableValue = CPINT;
                pVar->iLine = getLineNo( );
                break;

            case CPFLOAT: outln( " + FLOAT" );
                pVar->iVariableValue = CPFLOAT;
                pVar->iLine = getLineNo( );
                break;

            case CPCHAR : outln( " + CHAR" );
                pVar->iVariableValue = CPCHAR;
                pVar->iLine = getLineNo( );
                break;

            case CPBOOL : outln( " + BOOL" );
                pVar->iVariableValue = CPBOOL;
                pVar->iLine = getLineNo( );
                break;

            case CPVOID : outln( " + VOID" );
                pVar->iVariableValue = CPVOID;
                pVar->iLine = getLineNo( );
                break;

	    case CPSTRUCT  : outln( " + STRUCT" );
		pVar->iVariableValue = CPSTRUCT;
		pVar->iLine = getLineNo( );
		break;

            case CPCONST   :
            case CPVOLATILE:
            case CPUNION   : outln( "const, volatile, union found - skipping" ); break;

	    // shouldn't happen
            case CPSTATIC  : kdDebug(9007) << "CPSTATIC found - skipping" << endl;

            case '*'    : outln( " + POINTER"   ); pVar->iVariableType = CPPOINTER  ; break;
            case '&'    : outln( " + REFERENCE" ); pVar->iVariableType = CPREFERENCE; break;

            case '('    :
		kdDebug(9007) << "EE: ( found - shouldn't be, or ?" << endl;
		break;

            // these are the signs for a variable's end
            case ')'    :
		outln(  "Ending function head found: ')'" );
		if( pVar->sVariableName == "" ){
		    pVar->setDefault( );
		    outln( "something like ( const QString& ) has been found" );
		    break;
		}
                // anything about baseclasses, throws doesn't matter
                // !!! something like foo( ); isn't handled !!! --- editor part ?
                skipToLexem( '{' );
		// falling through to case ','

            case ','    :
                if( !pVar->isDefault( ) && pVar->sVariableName == m_variable ){
                    varList.append( pVar );
                    pVar = 0;
                    break;
                }
                else {
                    kdDebug(9007) << "WW: unexpected else in ," << endl;
                }
                break;

            case ':'    :
                // full qualifier found - not handled yet
                kdDebug(9007) << "EE: : found, skipping to '{'" << endl;
                skipToLexem( '{' );
                break;

            default     :
                // here we collect unknown types (classes) and stuff
                outln( " + default '" << getText( ) << "'" );
		if( pVar->isDefault( ) ){
		    pVar->sVariableType = getText( );
                    pVar->iLine = getLineNo( );
                    break;
		}
		else {
		    outln( "default else" );
                    pVar->sVariableName = getText( );
		}
        } // end switch
    } // end while

    // show what we found
//    debugPrint( );
    if( pVar )
        delete pVar;
    outln( "Parsing function head ending" );
}

void
CppCCParser::parseFunctionBody( )
{
    outln( "Parsing funtion body starting" );

    // scope++ at '{'; parseFunctionHead parsed until lexem = '{'
    int iScope = 1;

    // scope that counts
    // seems it is useless because it hasn't changed in parseFunctionHead( )
    CScope tmpScope( iScope );
    m_lastScope = tmpScope;

    // here we store what we found
    CParsedVariable* pVar = 0;

    // are we within a variable ?
    bool bWithinVariable = false;

    // parsing the rest until out of lexem
    while( lexem != 0 && iScope != 0 && ( getLineNo( ) <= iCCLine ) ){
        getNextLexem( );

        // create new variable-data holder
        if( !pVar )
            pVar = new CParsedVariable;

        switch( lexem ){
            // checking standard values first
	    // typedefs ????
            case CPBOOL : outln( " + BOOL" );
                pVar->iVariableValue = CPBOOL;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPINT  : outln( " + INT"   );
                pVar->iVariableValue = CPINT;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPFLOAT: outln( " + FLOAT" );
                pVar->iVariableValue = CPFLOAT;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPCHAR : outln( " + CHAR" );
                pVar->iVariableValue = CPCHAR;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPVOID : outln( " + VOID" );
                pVar->iVariableValue = CPVOID;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPCONST   :
            case CPVOLATILE:
            case CPSTATIC  : outln( "const, volatile, static found - skipping" ); break;

            case CPSTRUCT  : // to handle: struct m x;
            case CPUNION   :
                kdDebug(9007) << "### struct / union found +++ not handled yet - ignoring" << endl;
                kdDebug(9007) << "### place structs / unions in header-file !" << endl;
                skipBlock( ); // currently skips to fileend if: struct m x;
                skipToLexem( ';' );
                break;

            case '*'    : outln( " + POINTER"   ); pVar->iVariableType = CPPOINTER  ; break;
            case '&'    : outln( " + REFERENCE" ); pVar->iVariableType = CPREFERENCE; break;

            case CPWHILE: outln( "+ while found" ); skipCommand( ); break;
            case CPFOR  : outln( "+ for found"   ); skipCommand( ); break;

            case '?'    :
                outln( "? found" );
                if( !pVar->isDefault( ) ){
                    outln( "?, var defaulted" );
                    pVar->setDefault( );
                    skipCommand( );
                }
                break;

            case CPDCAST:
                outln( "dynamic cast found" );
                skipCommand( );
                break;

            case CPSCAST:
                outln( "static cast found" );
                skipCommand( );
                break;

            case CPIF   : outln( "+ if found"   ); skipCommand( ); break;
            case CPELSE : outln( "+ else found" ); skipCommand( ); break;
            case CPDO   : outln( "+ do found"   ); skipCommand( ); break;

            case CPSWITCH :
                outln( "+ switch found" );
                skipCommand( );
                break;

            case CPDEFAULT:
                outln( "default found" );
                getNextLexem( ); // = ':'
                break;

            case CPCASE   :
                outln( "case found" );
                skipToLexem( ':' );
                break;

            case CPBREAK  :
                outln( "break found" );
                getNextLexem( ); // = ';'
                break;

            case CPPIPE :
                outln( "operator << or >> found" );
                skipToSemicolon( );
                break;

            // qt-specific ! how to handle that ?
            case CPEMIT   : outln( "+ emit found"   ); skipToSemicolon( ); break;

            case CPRETURN : outln( "+ return found" ); skipToSemicolon( ); break;

            case CLCL   :
                outln( ":: found" );
                if( bWithinVariable == false ){
                    pVar->setDefault( );
                    outln( "-:: var defaulted" );
                    skipCommand( );
                }
                else {
                    kdDebug(9007) << "WW: unexpected else in CLCL" << endl;
                }
                break;

	    case '['    :
		outln( "[ found" );
		if( bWithinVariable == true ){
                    skipToLexem( '[' );
                    break;
		}
		else {
		    pVar->setDefault( );
		    outln( "-[ var defaulted" );
		    skipToSemicolon( );
                    break;
		}

            case ','    :
                outln( ", found" );
                if( bWithinVariable == true && pVar->sVariableName == m_variable ){
                    pVar->scope = tmpScope;
                    varList.append( pVar );
                    pVar = new CParsedVariable( *pVar );
                    outln( "--> , appended '" << pVar->sVariableName << "' <--" );
                    break;
                }
                else {
                    // errln( "WW: unexpected else in ," );
                }
                break;

            case ';'    :
                outln( "; found" );
                if( bWithinVariable == true && pVar->sVariableName == m_variable ){
                    pVar->scope = tmpScope;
                    varList.append( pVar );
                    outln( "--> ; appended '" << pVar->sVariableName << "' <--" );
                    pVar = 0;
                    bWithinVariable = false;
                    break;
                }
                else {
                    // errln( "WW: unexpected else in ;" );
                }
                break;

            case '='    :
                outln( "= found" );
                skipToSemicolon( );
                if( bWithinVariable == true && pVar->sVariableName == m_variable ){
                    pVar->scope = tmpScope;
                    varList.append( pVar );
                    outln( "--> = appended '" << pVar->sVariableName << "' <--" );
                    pVar = 0;
                    bWithinVariable = false;
                    break;
                }
                else {
                    // normale zuweisung ;)
                    pVar->setDefault( );
                    outln( "=, var defaulted" );
                    break;
                }

            case '('    :
                outln( "( found" );
                if( bWithinVariable == false ){
                    // function name = unknown, stored in default
                    pVar->setDefault( );
                    outln( "(, var defaulted" );
                    skipToSemicolon( );
		    break;
                }
		else {
		    // QString file( ... )
		    skipCommand( );
            	    break;
		}

            case '-'    :
                outln( "- found" );
                if( bWithinVariable == false ){
                    pVar->setDefault( );
                    outln( "-, var defaulted" );
                    skipToSemicolon( );
                    break;
                }
                else {
                    kdDebug(9007) << "WW: unexpected else in -" << endl;
                }
                break;

            case '{'    :
                outln( "{ found" );
                m_lastScope.increase( ++iScope );
                tmpScope.increase( iScope );
                outmsg( "entering scope: " ); m_lastScope.debugOutput( ); outln( "" );
                break;

            case '}'    :
                outln( "} found" );
                m_lastScope.decrease( --iScope );
                break;

            case '.'    :
                outln( ". found" );
                if( bWithinVariable == false ){
                    pVar->setDefault( );
                    outln( "-. var defaulted" );
                    skipToSemicolon( );
                    break;
                }
                else {
                    kdDebug(9007) << "WW: unexpected else in ." << endl;
                }
                break;

            default     :
                outln( "unknow lexem found '" << getText( ) << "'" );

                // if variable type = default this is our variable type
                if( pVar->isDefault( ) ){
                    pVar->sVariableType = getText( );
                    pVar->iLine = getLineNo( );
                    break;
                }
                else {
                    kdDebug(9007) << "  parseFunctionBody default value, else" << endl;
                    pVar->sVariableName = getText( );
                    bWithinVariable = true;
                }
        }
    }

    if( pVar )
        delete pVar;

    // show what we found
    debugPrint( );

    outln( "Parsing funtion body ending" );
}

void
CppCCParser::postProcessVariables( )
{
    kdDebug(9007) << "-- post-processing started --" << endl;
    // if we found only one or none vars we just copy the one var
    CParsedVariable* pVar;
    if( varList.count( ) <= 1 ){
        pVar = varList.first( );
        if( pVar )
            variableList.append( pVar );
        return;
    }

    errmsg( "   `-> m_lastScope: '" ); m_lastScope.debugOutput( ); errln( "'" );
    for( int i = 0; i < varList.count( ); i++ ){
        pVar = varList.at( i );
        if( !pVar->scope.isValidIn( m_lastScope ) ) {
            errmsg( "   `-> scope '" ); pVar->scope.debugOutput( ); errln( "' won't be added" );
        }
        else {
            errmsg( "   `-> scope '" ); pVar->scope.debugOutput( ); errln( "' will be added" );
            variableList.append( pVar );
        }
    }
    varList.clear( );
    kdDebug(9007) << "-- post-processing ended --" << endl;
}

void
CppCCParser::debugPrint( )
{
    kdDebug(9007) << "-- debugPrint start --" << endl;

    errln( "Number of variables (" << varList.count( ) << ")" );
    for( int i = 0; i < varList.count( ); i++ ){
        errln( "Variable found @line: "      << varList.at( i )->iLine );
        errln( "  Variable name          : " << varList.at( i )->sVariableName );
        errmsg  ( "  Variable value         : " << varList.at( i )->iVariableValue );
        switch( varList.at( i )->iVariableValue ){
            case CPBOOL   : errln( " ( bool )"  ); break;
            case CPVOID   : errln( " ( void )"  ); break;
            case CPINT    : errln( " ( int )"   ); break;
            case CPFLOAT  : errln( " ( float )" ); break;
            case CPCHAR   : errln( " ( char )"  ); break;
            case CPNONE   : errln( " ( none )"  ); break;
            case CPUNKNOWN: errln( " ( unknown )" ); break;
            default       : errln( " *** iReturnValue default" );
        }

        errmsg( "  Variable integer type  : " << varList.at( i )->iVariableType );
        switch( varList.at( i )->iVariableType ){
            case CPPOINTER  : errln( " ( pointer )"   ); break;
            case CPREFERENCE: errln( " ( reference )" ); break;
            case CPNONE     : errln( " ( none )"      ); break;
            default         : errln( " *** iType default" );
        }
        errmsg( "  Variable string  type  : " );
        if( varList.at( i )->sVariableType == "" )
            errln( "( standard type )" );
        else
            errln( varList.at( i )->sVariableType );
        errmsg( "  Scope                  : " ); varList.at( i )->scope.debugOutput( ); errln ( "" );
    }

    kdDebug(9007) << "-- debugPrint end --" << endl;
}

void
CppCCParser::skipBlock( )
{
    int  iDepth;
    ( lexem == '{' ) ? iDepth = 1 : iDepth = 0;
    bool bExit  = false;

    while( !bExit )
    {
        getNextLexem( );

        if( lexem == '{' )
            iDepth++;

        if( lexem == '}' )
        {
            iDepth--;
            bExit = ( iDepth == 0 );
        }

        // Always exit if lexem == 0 -> EOF.
        bExit = bExit || ( lexem == 0 );
    }
}

void
CppCCParser::skipToSemicolon( )
{
    while( lexem != ';' && lexem != 0 ){
        outln( " - skipping: " << getText( ) );
        getNextLexem( );
    }
}

void
CppCCParser::skipCommand( )
{
    outln( " + skipCommand start" );
    int  iDepth;
    ( lexem == '(' ) ? iDepth = 1 : iDepth = 0;
    bool bExit  = false;

    while( !bExit ){
	getNextLexem( );
        outln( " - skipping: " << getText( ) );

	if( lexem == '(' )
	    iDepth++;

	if( lexem == ')' ){
	    iDepth--;
	    bExit = ( iDepth == 0 );
	}

        if( lexem == '{' )
            bExit = true;

	if( lexem == ';' && iDepth == 0 )
	    bExit = true;

	if( lexem == ',' && iDepth == 0 )
	    bExit = true;


	bExit = bExit || ( lexem == 0 );
    }

    outln( " + skipCommand end" );
}

void
CppCCParser::skipToLexem( const char c )
{
    do {
        getNextLexem( );
    } while( lexem != c );
}
