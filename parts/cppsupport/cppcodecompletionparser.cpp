#include "cppcodecompletionparser.h"
#include <iostream.h>

#define PUSH_LEXEM( ) lexemStack.push( new CParsedLexem( lexem, getText( ) ) )

CppCCParser::CppCCParser( )
{
}

CppCCParser::~CppCCParser( )
{
}

bool
CppCCParser::parse( const char* file, const int _iCCLine )
{
    ifstream f( file );
    if( f.is_open( ) ){
        iCCLine = _iCCLine;
        currentFile = file;
        parseFile( f );
        return true;
    }
    else {
        cerr << "EE: CppCCParser::parse file '" << file << "' couldn't opened" << endl;
    }

    return false;
}

void
CppCCParser::parseFile( ifstream& file )
{
    lexem = -1;
    lexer = new yyFlexLexer( &file );
    parseTopLevel( );
    delete lexer;
}

void
CppCCParser::parseTopLevel( )
{
    // depending on what cc will (have to) provide we could check for global variables
    // first (i.e. pushing the arguments until we find the keyword 'static' or a '('
    // i don't care about that at the moment

    // parsing the function header
    //parseFunctionHead( );

    // parsing the function body
    parseFunctionBody( );
}


void
CppCCParser::parseFunctionHead( )
{
    cout << "Parsing function head starting" << endl;

    // here we store what we found
    CParsedVariable* pVar = 0;

    // current Scope - always 1 for "within function scope" ;)
    CScope currentScope( 1 );

    // string for the function name
    QString fName = "";

    // getting first '(' and then looping until the header ends
    skipToLexem( '(' );

    getNextLexem( );
    while( lexem != ';' && lexem != '{' && lexem != 0 ){
        // create new variable-data holder and setting defaults
        if( !pVar ){
            pVar = new CParsedVariable;
            pVar->scope = currentScope;
        }

        switch( lexem ){
            // checking standard return values first
            case CPVOID : cout << " + VOID"  << endl;
                pVar->iVariableValue = CPVOID;
                pVar->iLine = getLineNo( );
                break;

            case CPINT  : cout << " + INT"   << endl;
                pVar->iVariableValue = CPINT;
                pVar->iLine = getLineNo( );
                break;

            case CPFLOAT: cout << " + FLOAT" << endl;
                pVar->iVariableValue = CPFLOAT;
                pVar->iLine = getLineNo( );
                break;

            case CPCHAR : cout << " + CHAR"  << endl;
                pVar->iVariableValue = CPCHAR;
                pVar->iLine = getLineNo( );
                break;

            case CPBOOL : cout << " + BOOL"  << endl;
                pVar->iVariableValue = CPBOOL;
                pVar->iLine = getLineNo( );
                break;

            case CPCONST   :
            case CPVOLATILE:
            case CPUNION   :
            case CPSTATIC  : cout << "const, volatile, union, static found - skipping" << endl; break;

            case '*'    : cout << " + POINTER"   << endl; pVar->iVariableType = CPPOINTER  ; break;
            case '&'    : cout << " + REFERENCE" << endl; pVar->iVariableType = CPREFERENCE; break;

            case '('    :
		cerr << "EE: ( found - shouldn't be, or ?" << endl;
		break;

            // these are the signs for a variable's end
            case ')'    :
		cout << "Ending function head found: ')'" << endl;
		if( pVar->sVariableName == "" ){
		    pVar->setDefault( );
		    cout << "something like ( const QString& ) has been found" << endl;
		    break;
		}
                // anything about baseclasses, throws doesn't matter
                skipToLexem( '{' );

            // falling through from ')'
            case ','    :
                if( !pVar->isDefault( ) ){
                    varList.append( pVar );
                    pVar = 0;
                    break;
                }
                else {
                    cerr << "WW: unexpected else in ," << endl;
                }
                break;

            // base class found - shouldn't happen
            case ':'    :
                cerr << "EE: found, skipping" << endl;
                skipCommand( );
                break;

            default     :
                // here we collect unknown types and stuff
                cout << " + default '" << getText( ) << "'" << endl;
		if( pVar->isDefault( ) ){
		    pVar->sVariableType = getText( );
                    pVar->iLine = getLineNo( );
                    break;
		}
		else {
		    cout << "default else" << endl;
                    pVar->sVariableName = getText( );
		}
        } // end switch
        getNextLexem( );
    } // end while

    // show what we found
//    debugPrint( );
    if( pVar )
        delete pVar;
    cout << "Parsing function head ending" << endl;
}

void
CppCCParser::parseFunctionBody( )
{
    // btw: we could stop parsing until a given line has been analyzed
    cout << "Parsing funtion body starting" << endl;

    // scope++ at '{'; parseFunctionHead parses until '{'
    int iScope = 1;

    // testscope
    CScope currentScope( iScope );

    // here we store what we found
    CParsedVariable* pVar = 0;

    // are we within a variable ?
    bool bWithinVariable = false;

    // parsing the rest until out of lexem
    while( lexem != 0 && iScope != 0 && ( getLineNo( ) <= iCCLine ) ){
        getNextLexem( );

        // create new variable-data holder and setting defaults
        if( !pVar )
            pVar = new CParsedVariable;

        switch( lexem ){
            // checking standard values first
            case CPBOOL : cout << " + BOOL" << endl;
                pVar->iVariableValue = CPBOOL;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPVOID : cout << " + VOID"  << endl;
                pVar->iVariableValue = CPVOID;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPINT  : cout << " + INT"   << endl;
                pVar->iVariableValue = CPINT;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPFLOAT: cout << " + FLOAT" << endl;
                pVar->iVariableValue = CPFLOAT;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPCHAR : cout << " + CHAR"  << endl;
                pVar->iVariableValue = CPCHAR;
                pVar->iLine = getLineNo( );
                bWithinVariable = true;
                break;

            case CPCONST   :
            case CPVOLATILE:
            case CPSTATIC  : cout << "const, volatile, static found - skipping" << endl; break;

            case CPSTRUCT  :
            case CPUNION   :
                cerr << "### struct / union found +++ not handled yet - ignoring" << endl;
                cerr << "### place structs / unions in header-file !" << endl;
                skipBlock( );
                skipToLexem( ';' );
                break;

            case '*'    : cout << " + POINTER"   << endl; pVar->iVariableType = CPPOINTER  ; break;
            case '&'    : cout << " + REFERENCE" << endl; pVar->iVariableType = CPREFERENCE; break;

            case CPWHILE: cout << "+ while found" << endl; skipCommand( ); break;
            case CPFOR  : cout << "+ for found"   << endl; skipCommand( ); break;

            case '?'    :
                cout << "? found" << endl;
                if( !pVar->isDefault( ) ){
                    cout << "?, var defaulted" << endl;
                    pVar->setDefault( );
                    skipCommand( );
                }
                break;

            case CPDCAST:
                cout << "dynamic cast found" << endl;
                skipCommand( );
                break;

            case CPSCAST:
                cout << "static cast found" << endl;
                skipCommand( );
                break;

            case CPIF   : cout << "+ if found" << endl; skipCommand( ); break;
            case CPELSE : cout << "+ else found" << endl; skipCommand( ); break;
            case CPDO   : cout << "+ do found" << endl; skipCommand( ); break;

            case CPSWITCH :
                cout << "+ switch found" << endl;
                skipCommand( );
                break;

            case CPDEFAULT:
                cout << "default found" << endl;
                getNextLexem( ); // = :
                break;

            case CPCASE   :
                cout << "case found" << endl;
                skipToLexem( ':' );
                break;

            case CPBREAK  :
                cout << "break found" << endl;
                getNextLexem( ); // = ;
                break;

            case CPPIPE :
                cout << "operator << or >> found" << endl;
                skipToSemicolon( );
                break;

            case CPEMIT : cout << "+ emit found" << endl; skipToSemicolon( ); break;

            case CPRETURN :
                cout << "+ return found";
                skipToSemicolon( );
                break;

            case CLCL   :
                cout << ":: found" << endl;
                if( bWithinVariable == false ){
                    pVar->setDefault( );
                    cout << "-:: var defaulted" << endl;
                    skipCommand( );
                }
                else {
                    cerr << "WW: unexpected else in CLCL" << endl;
                }
                break;

	    case '['    :
		cout << "[ found" << endl;
		if( bWithinVariable == true ){
                    skipToLexem( '[' );
                    break;
		}
		else {
		    pVar->setDefault( );
		    cout << "-[ var defaulted" << endl;
		    skipToSemicolon( );
                    break;
		}

            case ','    :
                cout << ", found" << endl;
                if( bWithinVariable == true ){
                    pVar->scope = currentScope;
                    varList.append( pVar );
                    pVar = new CParsedVariable( *pVar );
                    break;
                }
                else {
                    cerr << "WW: unexpected else in ," << endl;
                }

                break;

            case ';'    :
                cout << "; found" << endl;
                if( bWithinVariable == true ){
                    pVar->scope = currentScope;
                    varList.append( pVar );
                    pVar = 0;
                    bWithinVariable = false;
                    break;
                }
                else {
                    cerr << "WW: unexpected else in ;" << endl;
                }
                break;

            case '='    :
                cout << "= found" << endl;
                skipToSemicolon( );
                if( bWithinVariable == true ){
                    pVar->scope = currentScope;
                    varList.append( pVar );
                    pVar = 0;
                    bWithinVariable = false;
                    break;
                }
                else {
                    // normale zuweisung ;)
                    pVar->setDefault( );
                    cout << "=, var defaulted" << endl;
                    break;
                }

            case '('    :
                cout << "( found" << endl;
                if( bWithinVariable == false ){
                    // function name = unknown, stored in default
                    pVar->setDefault( );
                    cout << "(, var defaulted" << endl;
                    skipToSemicolon( );
		    break;
                }
		else {
		    // QString file( ... )
		    skipCommand( );
            	    break;
		}

            case '-'    :
                cout << "- found" << endl;
                if( bWithinVariable == false ){
                    pVar->setDefault( );
                    cout << "-, var defaulted" << endl;
                    skipToSemicolon( );
                    break;
                }
                else {
                    cerr << "WW: unexpected else in -" << endl;
                }
                break;

            case '{'    :
                cout << "{ found" << endl;
                currentScope.increase( ++iScope );
                currentScope.debugOutput( ); cerr << endl;
                break;

            case '}'    :
                cout << "} found" << endl;
                iScope--;
                break;

            case '.'    :
                cout << ". found" << endl;
                if( bWithinVariable == false ){
                    pVar->setDefault( );
                    cout << "-. var defaulted" << endl;
                    skipToSemicolon( );
                    break;
                }
                else {
                    cerr << "WW: unexpected else in ." << endl;
                }
                break;

            default     :
                cout << "unknow lexem found '" << getText( ) << "'" << endl;

                // if variable type = default this is our variable type
                if( pVar->isDefault( ) ){
                    pVar->sVariableType = getText( );
                    pVar->iLine = getLineNo( );
                    break;
                }
                else {
                    cerr << "  parseFunctionBody default value, else" << endl;
                    pVar->sVariableName = getText( );
                    bWithinVariable = true;
                }
        }
    }

    if( pVar )
        delete pVar;
    // show what we found
    debugPrint( );

    cout << "Parsing funtion body ending" << endl;
}

void
CppCCParser::debugPrint( )
{
    cerr << "-- debugPrint start --" << endl;

    cerr << "Number of variables (" << varList.count( ) << ")" << endl;
    for( int i = 0; i < varList.count( ); i++ ){
        cerr << "Variable found @line: " << varList.at( i )->iLine << endl;
        cerr << "  Variable name          : " << varList.at( i )->sVariableName << endl;
        cerr << "  Variable value         : " <<  varList.at( i )->iVariableValue;
        switch( varList.at( i )->iVariableValue ){
            case CPBOOL   : cerr << " ( bool )"  << endl; break;
            case CPVOID   : cerr << " ( void )"  << endl; break;
            case CPINT    : cerr << " ( int )"   << endl; break;
            case CPFLOAT  : cerr << " ( float )" << endl; break;
            case CPCHAR   : cerr << " ( char )"  << endl; break;
            case CPNONE   : cerr << " ( none )"  << endl; break;
            case CPUNKNOWN: cerr << " ( unknown )" << endl; break;
            default       : cerr << " *** iReturnValue default" << endl;
        }

        cerr << "  Variable integer type  : " <<  varList.at( i )->iVariableType;
        switch( varList.at( i )->iVariableType ){
            case CPPOINTER  : cerr << " ( pointer )"   << endl; break;
            case CPREFERENCE: cerr << " ( reference )" << endl; break;
            case CPNONE     : cerr << " ( none )"      << endl; break;
            default         : cerr << " *** iType default" << endl;
        }
        cerr << "  Variable string  type  : ";
        if( varList.at( i )->sVariableType == "" )
            cerr << "( standard type )" << endl;
        else
            cerr << varList.at( i )->sVariableType << endl;
        cerr << "  Scope                  : "; varList.at( i )->scope.debugOutput( ); cerr << endl;
    }

    cerr << "-- debugPrint end --" << endl;
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
        cout << " - skipping: " << getText( ) << endl;
        getNextLexem( );
    }
}

void
CppCCParser::skipCommand( )
{
    cout << " + skipCommand start" << endl;
    int  iDepth;
    ( lexem == '(' ) ? iDepth = 1 : iDepth = 0;
    bool bExit  = false;

    while( !bExit ){
	getNextLexem( );
        cout << " - skipping: " << getText( ) << endl;

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

    cout << " + skipCommand end" << endl;
}

void
CppCCParser::skipToLexem( const char c )
{
    do {
        getNextLexem( );
    } while( lexem != c );
}
