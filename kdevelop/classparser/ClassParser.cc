/***************************************************************************
                          ClassParser.cc  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@syncom.se
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
// 1999-07-27 added kapp->processEvents() to line 1381 - Ralf

//#include <iostream.h>
#include <kdebug.h>
#include <qstring.h>
#include <qregexp.h> 
#include <kapp.h>
#include "ClassParser.h"
#include "ProgrammingByContract.h"

/** Line where a comment starts. */
extern int comment_start;

/** Line where a comment ends. */
extern int comment_end;

/** Last parsed comment. */
extern QString comment;

#define PUSH_LEXEM() lexemStack.push( new CParsedLexem( lexem, getText() ))

enum
{ 
  CP_IS_OTHER,
  CP_IS_OPERATOR, CP_IS_OPERATOR_IMPL,
  CP_IS_ATTRIBUTE, CP_IS_ATTR_IMPL,
  CP_IS_MULTI_ATTRIBUTE, CP_IS_MULTI_ATTR_IMPL,
  CP_IS_METHOD, CP_IS_METHOD_IMPL, CP_IS_STRUCT
};

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CClassParser::CClassParser()
 * CClassParser()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassParser::CClassParser()
{
  reset();
  lexemStack.setAutoDelete(true);
}

/*------------------------------------- CClassParser::~CClassParser()
 * ~CClassParser()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CClassParser::~CClassParser()
{
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------- CClassParser::emptyStack()
 * emptyStack()
 *   Remove all elements from the stack.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::emptyStack()
{
  lexemStack.clear();
//  while( !lexemStack.isEmpty() )
//    delete lexemStack.pop();
}

/*------------------------------------ CClassParser::commentInRange()
 * commentInRange()
 *   Tells if the last parsed comment is in range to be a comment
 *   for the current parsed item.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
bool CClassParser::commentInRange( CParsedItem *aItem )
{
  REQUIRE1( "Valid item", aItem != NULL, false );

  int range;

  range = (aItem->declaredOnLine - ( comment_end - 1 ) );

  return ( range > 0 && range <=2 );
}

/*---------------------------------- CClassParser::skipThrowStatement()
 * skipThrowStatement()
 *   Skip a throw() statement.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::skipThrowStatement()
{
  while( lexem != ')' && lexem != 0 )
    getNextLexem();
}

/*--------------------------- CClassParser::parseStructDeclarations()
 * parseStructDeclarations()
 *   Parse all declarations inside a structure.
 *
 * Parameters:
 *   aStruct        The struct that holds the declarations.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseStructDeclarations( CParsedStruct *aStruct)
{
  REQUIRE( "Valid struct", aStruct != NULL );

  PIExport oldScope = declaredScope;

  declaredScope = PIE_PUBLIC;
  bool bID, bUnion;

  while( lexem != '}' && lexem != 0 ) {
    bID = bUnion = false;
    if( lexem != '}' ) {
      declStart = getLineno();
      switch( lexem ) {
      case ID:
	parseMethodAttributes( aStruct );
	bID = true;
	break;
      case CPENUM:    // enums are completely parsed by parseEnum( )
	parseEnum();
	break;
      case CPUNION:   // unions are NOT completely parsed by parseUnion( )
	parseUnion(); // e.g.: union { } var = 97
	bUnion = true;
	break;
      case CPSTRUCT:  // structs are completely parsed by parseStruct( )
	parseStruct( aStruct );
	break;
      default:
	kdDebug( ) << "--- parseStructDeclarations: case default" << endl;
	kdDebug( ) << "lexem: " << lexem << " & " << char( lexem ) << endl;
	kdDebug( ) << "@line: " << getLineno( ) << endl;
	kdDebug( ) << "Found unknown struct declaration." << endl;
      }
    }
    if( bUnion == true ){ // were just skipping the union stuff
      while( lexem != ';' && lexem != 0 ){ // todo: do that in parseUnion( )
	getNextLexem( );
      }
      if( lexem == ';' )
	getNextLexem( );
      continue;
    }

    if( lexem == '}' && bID == false && bUnion == false )
      break;              // found a '}' which doesn't come from a method- / union end

    if( lexem == '}' && bID == true ){ // found a "}" from a method
      getNextLexem( );                
      if( lexem == ';' )               // there should / could be a ';'
	getNextLexem( );               // if not, go ahead
    }
    else
      getNextLexem( );
  }
  declaredScope=oldScope;
}

/*---------------------------------- CClassParser::fillInParsedStruct()
 * fillInParsedStruct()
 *   Parse a structure using header declaration from stack.
 *
 * Parameters:
 *   aContainer  Container to store the parsed struct in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedStruct( CParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );
  REQUIRE( "Valid lexem", lexem == '{' );

  CParsedStruct *aStruct = new CParsedStruct();

  // Set some info about the struct.
  aStruct->setDeclaredOnLine( declStart );
  aStruct->setDeclaredInFile( currentFile );
  aStruct->setDefinedInFile( currentFile );
  // force this now!!! 20-Apr-2000 jbb
  if( !aContainer->path().isEmpty() )
    aStruct->setDeclaredInScope( aContainer->path() );

  // Check for a name on the stack
  if( !lexemStack.isEmpty() && lexemStack.top()->type == ID )
    aStruct->setName( lexemStack.top()->text );

  // Remove all lexema from the stack.
  emptyStack();

  // Jump to first declaration or to '}'.
  getNextLexem();

  // ends if } was found
  parseStructDeclarations( aStruct );

  // Skip '}'
  // --- ERROR: structs end with <name> <name...> ';'
  CParsedStruct* nStruct;
  QStringList ps;
 
  getNextLexem( );
  while( lexem == ID || lexem == ',' ){  // here we're parsing <name> <name...>
    if( lexem == ID )
      ps.append( getText( ) );
    getNextLexem( );
  }

  if( ps.isEmpty( ) ) // struct x {...}; ? -> use default name
    ps.append( aStruct->name );

  aStruct->setName( ps.first( ) );

  // Set the point where the struct ends.
  aStruct->setDeclarationEndsOnLine( getLineno() );

  // If we find a name here we use the typedef name as the struct name.
  //  if( lexem == ID )                --- commented out by Daniel
  //  aStruct->setName( getText() );   --- commented out by Daniel
  
  if( commentInRange( aStruct ) )
    aStruct->setComment( comment );

  if( aStruct != NULL && !aStruct->name.isEmpty() )
  {
    for( uint i = 0; i < ps.count( ); i++ ){
      nStruct = new CParsedStruct( *aStruct );
      nStruct->setName( ps[ i ] );

      aContainer->addStruct( nStruct );

      // Always add structs to the global container.
      if( aContainer != &store.globalContainer )
	store.globalContainer.addStruct( nStruct );
    }
  }
}

/*---------------------------------------- CClassParser::parseStruct()
 * parseStruct()
 *   Parse a struct.
 *
 * Parameters:
 *   aContainer  Container to store the parsed struct in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseStruct( CParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );

  while( lexem != 0 && lexem != '{' && lexem !=';' )
  {
    PUSH_LEXEM();
    getNextLexem();
  }

  // If we find a forward declaration we just ignore everything.
  if( lexem == ';' || lexem == 0)
    emptyStack();
  else
    fillInParsedStruct( aContainer );
}

/*---------------------------------------- CClassParser::parseEnum()
 * parseEnum()
 *   Parse a enum.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseEnum()
{
  while( lexem != 0 && lexem != ';' )
    getNextLexem();
}

/*----------------------------------- CClassParser::parseNamespace()
 * parseNamespace()
 *   Parse a namespace.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseNamespace( CParsedScopeContainer * scope )
{
  REQUIRE( "Valid scope", scope != NULL );

  CParsedScopeContainer *ns = new CParsedScopeContainer();

  // Set some info about the namespace
  ns->setDeclaredOnLine( declStart );
  ns->setDeclaredInFile( currentFile );
  ns->setDefinedInFile( currentFile );

  if( commentInRange( ns ) )
    ns->setComment( comment );

  getNextLexem();

  // kdDebug() << "parsing namespace" << endl;
  // kdDebug() << "lexem = " << lexem << ":" << static_cast<char>(lexem) << endl;

  if( lexem == '{' ) 
  {
    // anonymous namespace
    // assume 1-1 correspondence between file name and translation unit
    QString short_file_name(currentFile);
    short_file_name.replace( QRegExp(".*/"), "");
    QString trans_unit_ident("(");
    trans_unit_ident += short_file_name;
    trans_unit_ident += ")";

    ns->setName( trans_unit_ident );
  }
  else 
  {
    ns->setName( getText() );

    // check that it is well-formed, moving to beginning of block
    getNextLexem();
    while(lexem != 0 && lexem != '{') {
      kdDebug() << "Bad namespace identifier." << endl;
      getNextLexem();
    }
  }

  // skip over '{'
  getNextLexem();
  // EO start
  // reference to an already declared namespace
  // retrieve it
  CParsedScopeContainer *ns2 = scope->getScopeByName(ns->name);
  if (ns2)
  {
    delete ns;
    ns = ns2;
  }
  else	
  {
    // this is a namespace declaration
    scope->addScope( ns );

    // Always add namespaces to the global container.
    // EO why?
    // it is possible to have namespaces inside namespaces
    //    if( scope != &store.globalContainer )
    //       store.addScope( ns );
  }
  // EO end

  while(lexem != 0 && lexem != '}') 
  {
    declStart = getLineno();
  
    if( isGenericLexem() )
      parseGenericLexem( ns );
    else
      parseTopLevelLexem( ns );

    getNextLexem();
  }

  ASSERT( lexem == '}' );

  // If the parsing went ok, we add the scope.
  if( lexem != 0 )
  {
    ns->setDeclarationEndsOnLine( getLineno() );

//EO    scope->addScope( ns );

    //EO Always add namespaces to the global container.
//EO    if( scope != &store.globalContainer )
//EO      store.addScope( ns );
  }
}


/*---------------------------------------- CClassParser::parseUnion()
 * parseUnion()
 *   Parse an union.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseUnion()
{
  // Get the name
  // ToDo: parse a complete! union
  // e.g.: union {...} u = 97;
  getNextLexem();

  if( lexem == ID )
    getNextLexem();

  if( lexem == '{' )
    skipBlock();
}

/*----------------------------------- CClassParser::fillInParsedType()
 * fillInParsedType()
 *   Create a type using the arguments on the stack.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedType(QString &type)
{
  CParsedLexem *aLexem;
  type = "";

  while( !lexemStack.isEmpty() )
  {
    aLexem = lexemStack.pop();
    type = aLexem->text + " " + type;
    delete aLexem;
  }
}

/*----------------------------------- CClassParser::skipBlock()
 * skipBlock()
 *   Skip all lexems inside a '{' '}' block.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::skipBlock()
{
  int depth=1;
  bool exit=false;

  while( !exit )
  {
    getNextLexem();

    if( lexem == '{' )
      depth++;
    if( lexem == '}' )
    {
      depth--;
      exit = ( depth == 0 );
    }

    // Always exit if lexem == 0 -> EOF.
    exit = exit || ( lexem == 0 );
  }
}

/*-------------------------------- CClassParser::parseClassTemplate()
 * parseClassTemplate()
 *   Skip a template declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseTemplate()
{
  bool exit = false;
  int depth = 0;

  // If we're currently at an <, increase the depth.
  if( lexem == '<' )
    depth++;

  while( !exit )
  {
    getNextLexem();
    switch( lexem )
    {
      case '<':
        depth++;
        break;
      case '>':
        depth--;
        break;
      default:
        break;
    }
  
    exit = ( depth == 0 || lexem == 0 );
  }
}

/*********************************************************************
 *                                                                   *
 *                         VARIABLE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ CClassParser::isEndOfVarDecl()
 * isEndOfVarDecl()
 *   Tells if the current lexem is the end of a variable declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   bool           Is it an end?
 *-----------------------------------------------------------------*/
bool CClassParser::isEndOfVarDecl()
{
  return ( lexem == ';' || lexem == ',' || lexem == ')' || 
          // lexem == '}' ||
     lexem == 0 );
}

/*------------------------------ CClassParser::fillInParsedVariable()
 * fillInParsedVariable()
 *   Initialize a attribute, except the type, using the arguments on
 *   the stack.
 *
 * Parameters:
 *   anAttr        The attribute to initialize.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedVariableHead( CParsedAttribute *anAttr )
{
  REQUIRE( "Valid attribute", anAttr != NULL );

  bool exit=false;
  CParsedLexem *aLexem;
  QString addDecl;

  // Check for a function pointer declaration.
  if( lexemStack.top()->type == ')' )
  {
    int depth=0;
    bool nextIsVarName=false;
    bool skipText=false;

    exit = false;
    while( !exit )
    {
      bool thisIsVarName=nextIsVarName;
      aLexem = lexemStack.pop();
      if (aLexem->type == ')')
      {
         depth++;
         if (!lexemStack.isEmpty() && lexemStack.top()->text=="void")
           nextIsVarName=true;
      }

      // check if the next parsed entry could be the var name
      // or all inside [ ]
      if (aLexem->type=='[')
        skipText=false;

      if (thisIsVarName || skipText)
      {
        nextIsVarName=false;
      }
      else
      {
        thisIsVarName= (!lexemStack.isEmpty() && aLexem->type == ID &&
                       (lexemStack.top()->type==ID ||
                        lexemStack.top()->type=='*' ||
                        lexemStack.top()->type=='}' ||
                        lexemStack.top()->type=='&'));
      }

      // save everything but the var names or single "void" combination
      if (!thisIsVarName && !skipText)
      {
        if (aLexem->type=='(')
          addDecl = aLexem->text + addDecl;
        else
          addDecl = aLexem->text + " " +addDecl;
      }

      if (aLexem->type == '(')
        depth--;
      if (aLexem->type==']')
        skipText=true;
      exit = ( lexemStack.isEmpty() || depth<=0 );
      delete aLexem;
    }

    if (!lexemStack.isEmpty() && lexemStack.top()->type==')')
    {
      aLexem = lexemStack.pop();
      addDecl = aLexem->text + addDecl;
      delete aLexem;
    }
  }


  // Check for an single or multiple array declaration.
  while ( !lexemStack.isEmpty() && lexemStack.top()->type == ']' )
  {
    exit = false;
    while( !exit )
    {
      aLexem = lexemStack.pop();
      if (aLexem->type == ']' || aLexem->type == '[')
        addDecl = aLexem->text + addDecl;
      exit = ( lexemStack.isEmpty() || aLexem->type == '[' );
      delete aLexem;
    }
  }

  // Check if this variable has a bit definition. If so remove it.
  if( !lexemStack.isEmpty() && lexemStack.top()->type == NUM )
  {
    while( !lexemStack.isEmpty() && lexemStack.top()->type != ID )
      delete lexemStack.pop();
  }

  // Initial checks if this variable declaration just has a type.

  //  don't forget about "const", "register", "volatile" specifier in
  //   an argument
  // now if there are two ID's ... the last supposed to be the
  //   var name
  if( !lexemStack.isEmpty() && lexemStack.top()->type==ID )
  {
    aLexem = lexemStack.pop();
    if (!lexemStack.isEmpty() &&
        (lexemStack.top()->type==ID
         || lexemStack.top()->type=='}'
         || lexemStack.top()->type=='*'
         || lexemStack.top()->type==','
         || lexemStack.top()->type=='&'))
    {
      /* if the next item is also an ID or it is something like
         struct test {int a, int b} x, int &x
         in aLexem it has to be the var name
       */
      anAttr->setName( aLexem->text );
      delete aLexem;
    }
    else
    {
      // in this case the ID must be a type spec...
      lexemStack.push(aLexem);
      anAttr->setName( "" );
    }
  }

  anAttr->type+=addDecl;
  anAttr->setDeclaredInFile( currentFile );
  anAttr->setDeclaredOnLine( /* declStart */ getLineno());
  anAttr->setExport( declaredScope );
}

/*------------------------------ CClassParser::fillInParsedVariable()
 * fillInParsedVariable()
 *   Initialize a attribute using the arguments on the stack.
 *
 * Parameters:
 *   anAttr        The attribute to initialize.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedVariable( CParsedAttribute *anAttr )
{
  REQUIRE( "Valid attribute", anAttr != NULL );

  QString type;

  // Get the everything exept the type
  fillInParsedVariableHead( anAttr );

  // Get the type
  fillInParsedType( type );

  // Set values in the variable.
  if( !type.isEmpty() )
    anAttr->setType( type + anAttr->type );

  anAttr->setNamePos( type.length() );

  // Skip default values
  if( lexem == '=' )
    while( lexem != ';' && lexem != 0 )
      getNextLexem();

  // If we reach end of file, this is not a valid attribute.
  if( lexem == 0 )
    anAttr->setName( "" );
  else // Set the end of this variable declaration.
    anAttr->setDeclarationEndsOnLine( getLineno() );

  // Set the comment if in range.
  if( commentInRange( anAttr ) )
    anAttr->setComment( comment );
}

/*----------------------------- CClassParser::fillInMultipleVariable()
 * fillInMultipleVariable()
 *   Take what's on the stack and return as a list of variables. 
 *   Works for variable declarations like int foo, bar, baz....
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedAttribute>  List of variables
 *-----------------------------------------------------------------*/
void CClassParser::fillInMultipleVariable( CParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );

  bool exit = false;
  CParsedAttribute *anAttr;
  CParsedLexem *aLexem;
  QString type;
  QList<CParsedAttribute> list;

  // Make sure no attributes gets deleted.
  list.setAutoDelete( false );

  while( !lexemStack.isEmpty() && !exit )
  {
    anAttr = new CParsedAttribute();

    // Get the variable name.
    fillInParsedVariableHead( anAttr );

    if (!lexemStack.isEmpty())
    {
      // Add pointer stuff
      while(  lexemStack.top()->type != ID && lexemStack.top()->type != ',' )
      {
        aLexem = lexemStack.pop();
        anAttr->type = aLexem->text + anAttr->type;
        delete aLexem;
      }

      exit = (lexemStack.isEmpty() || lexemStack.top()->type == ID );

      // Remove separating ','
      if( !lexemStack.isEmpty() && lexemStack.top()->type == ',' )
        delete lexemStack.pop();
    }

    // Set endpoint of declaration.
    anAttr->setDeclarationEndsOnLine( getLineno() );

    list.append( anAttr );
  }

  // Get the type
  fillInParsedType( type );

  // Iterate over all attributes, set the type and add it to the container.
  for( anAttr = list.first();
       anAttr != NULL;
       anAttr = list.next() )
  {
    // Only add attributes that have a name.
    if( !anAttr->name.isEmpty() )
    {
      anAttr->type = type + anAttr->type;

      aContainer->addAttribute( anAttr );
    }
    else
      delete anAttr;
  }
}

/*-------------------------------------- CClassParser::parseVariable()
 * parseVariable()
 *   Parse a variable declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedAttribute *CClassParser::parseVariable()
{
  bool skip = false;
  CParsedAttribute *anAttr = NULL;

  while( !isEndOfVarDecl() )
  {
    // Skip all default values.
    if( lexem == '=' )
    {
      skip = true;
    }
    else if( !skip && lexem == '(' ) // Check for function pointer
    {
      getNextLexem();
      if( lexem == '*' )
      {
        lexemStack.push( new CParsedLexem( '(', "(" ));
        PUSH_LEXEM();

        // Push variable name.(but maybe there's no var name)
        getNextLexem();
        if (lexem!=0 && lexem==ID)
        {
          PUSH_LEXEM();
          getNextLexem();
        }

        // search for ')' and push it
        //      (0 or ; means something went wrong)
        // it can be something like
        //   int (*fp[10][]) (char, int)
        while (lexem!=0 && lexem!=')' && lexem!=';')
        {
          PUSH_LEXEM();
          getNextLexem();
        }

        // Add last element... hoping this was a ')'
        PUSH_LEXEM();

        // now we should be at int (*fp...)   -> going ahead to (char, int)
        if (lexem==')')
        {
          int depth=1;
          getNextLexem();  // now it should be a (

          if ( lexem == '(')
          {
            while( lexem != 0 && depth>0  && lexem!=';')
            {
              PUSH_LEXEM();
              if (lexem==')')
                depth--;
              getNextLexem();
              if (lexem=='(')
                depth++;
            }
          }
        }
      }
    }
    else if( !skip && !isEndOfVarDecl() )
    {
      PUSH_LEXEM();
    }

    if (!isEndOfVarDecl())
     getNextLexem();
  }

  if( !lexemStack.isEmpty() )
  {
    anAttr = new CParsedAttribute();
    fillInParsedVariable( anAttr );
    anAttr->setDeclarationEndsOnLine( getLineno() );
  }

  return anAttr;
}

/*********************************************************************
 *                                                                   *
 *                          METHOD METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------- CClassParser::parseFunctionArgs()
 * parseFunctionArgs()
 *   Parse and add all arguments to a function.
 *
 * Parameters:
 *   method         The method to which we should add arguments.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseFunctionArgs( CParsedMethod *method )
{
  REQUIRE( "Valid method", method != NULL );

  CParsedAttribute *anAttr;
  CParsedArgument *anArg;
  bool exit = false;

  while( !exit )
  {
    // Get the variable
    anAttr = parseVariable();

    if( anAttr )
    {
      // Move the values to the argument object.
      anArg = new CParsedArgument();
      if( !anAttr->name.isEmpty() )
        anArg->setName( anAttr->name );

      if( !anAttr->type.isEmpty() )
        anArg->setType( anAttr->type );
      
      anArg->setNamePos( anAttr->posName );
      // Add the argument to the method.
      method->addArgument( anArg );
      delete anAttr;
    }

    // Loop while we find ,
    exit = !(lexem == ',');

    if( !exit )
      getNextLexem();
  }
}

/*--------------------------------- CClassParser::fillInParsedMethod()
 * fillInParsedMethod()
 *   Initialize a method using the arguments on the stack.
 *
 * Parameters:
 *   aMethod        The method to initialize.
 *   isOperator     this is not used all the operator related processing
 *  		    has already been done
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedMethod(CParsedMethod *aMethod, bool isOperator)
{
  REQUIRE( "Valid method", aMethod != NULL );

  QString name;
  CParsedLexem *aLexem;
  QString type;

  // Set the method name, depending on if it's a operator or not.
  if( !isOperator )
  {
    aLexem = lexemStack.pop();
    name = aLexem->text;
    delete aLexem;
  }
  else
  {
    // Make the name of all symbols until the operator keyword.
    while( lexemStack.top()->type != CPOPERATOR )
    {
      aLexem = lexemStack.pop();
      name = aLexem->text + name;
      delete aLexem;
    }

    // Delete the operator symbol.
    delete lexemStack.pop();
  }

  // Set the operator name
  aMethod->setName( name );

  // Set the type of the method.
  fillInParsedType( type );
  if( !type.isEmpty() )
    aMethod->setType( type );

  // Jump to first argument or ')'
  getNextLexem();

  // Set the arguments of the method, if there are any.
  if( lexem != ')' && lexem != ';')
    parseFunctionArgs( aMethod );

  // Set some attributes of the parsed method.
  aMethod->setDefinedOnLine( declStart );
  aMethod->setDeclaredOnLine( declStart );
  aMethod->setDefinedInFile( currentFile );
  aMethod->setDeclaredInFile( currentFile );
  aMethod->setExport( declaredScope );
  aMethod->setIsStatic( isStatic );

  getNextLexem();

  // Const method
  if( lexem == CPCONST )
  {
    aMethod->setIsConst( true );
    getNextLexem();
  }

  // Skip throw statements
  if( lexem == CPTHROW ){
    skipThrowStatement( );
    getNextLexem( );
  }

  // Other initializers
  if( lexem == ':' )
    while( lexem != 0 && lexem != '{' )
      getNextLexem();

  // commented out by Daniel
  // because past the initializers the implementation starts
  // and is expected (see 5 lines above)
  //  Skip throw statements
  //  if( lexem == CPTHROW )
  //  skipThrowStatement();

  // Skip implementation.
  if( lexem == '{' )
    skipBlock();

  // Set end of declaration and definition.
  aMethod->setDefinitionEndsOnLine( getLineno() );
  aMethod->setDeclarationEndsOnLine( getLineno() );  

  // Set the comment if in range.
  if( commentInRange( aMethod ) )
    aMethod->setComment( comment );
}

/*---------------------------- CClassParser::parseMethodDeclaration()
 * parseMethodDeclaration()
 *   Parse a method declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedMethod *CClassParser::parseMethodDeclaration()
{
  CParsedMethod *aMethod = new CParsedMethod();
  bool isOperator=false;

  // Add all lexems on the stack until we find the start of the
  // parameter list.
  while( lexem != '(' && lexem != ';' && lexem != 0 )
  {
    if (lexem==CPOPERATOR)
      isOperator=true;
    PUSH_LEXEM();
    getNextLexem();
  }

  if( lexem == '(' )
    fillInParsedMethod( aMethod, isOperator);
  else 
  {
    emptyStack();
    delete aMethod;
    aMethod = NULL;
  }

  return aMethod;
}

/*------------------------------------ CClassParser::parseMethodImpl()
 * parseMethodImpl()
 *   Parse a method implementation.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseMethodImpl(bool isOperator,CParsedContainer *scope)
{
  CParsedClass *aClass;
  CParsedLexem *aLexem;
  QString name;
  QString className;
  CParsedMethod aMethod;
  CParsedMethod *pm;
  int declLine = getLineno();

  // The two first objects on the stack is the name and then the class.

  // Get the method name.
  if( !isOperator )
  {
    aLexem = lexemStack.pop();
    name = aLexem->text;
    delete aLexem;
  }
  else
  {
    // Add all symbols until we find the operator keyword.
    while( lexemStack.top()->type != CPOPERATOR )
    {
      aLexem = lexemStack.pop();
      name = aLexem->text + name;
      delete aLexem;
    }

    // Remove the operator lexem from the stack.
    delete lexemStack.pop();
  }

  // Remove strange things until we find a '::'
  while( !lexemStack.isEmpty() && lexemStack.top()->type != CLCL )
   delete lexemStack.pop();

  if (!lexemStack.isEmpty())
  {
   // Delete '::'
   aLexem = lexemStack.pop();
   delete aLexem;

   // Get the classname
   aLexem = lexemStack.pop();
   className = aLexem->text;
   delete aLexem;
  }

  // Append all other classname'::' declarations.
  while( !lexemStack.isEmpty() && lexemStack.top()->type == CLCL )
  {
    // Delete '::'
    aLexem = lexemStack.pop();
    delete aLexem;

    // Get next classname and append it.
    aLexem = lexemStack.pop();
    className = aLexem->text + "." + className;
    delete aLexem;
  }

  // To make things easier we push the methodname again.
  lexemStack.push( new CParsedLexem( ID, name ) );

  // Get the method declaration.
  fillInParsedMethod( &aMethod);

  // Skip forward declarations.
  if( lexem != ';' )
  { 
    // Try to move the values to the declared method.
    aClass = store.getClassByName( className );
    if( aClass != NULL)
    {
      pm = aClass->getMethod( aMethod );
      if( pm != NULL )
      {
        aClass->setDefinedInFile( currentFile );
        pm->setIsInHFile( false );
        pm->setDefinedInFile( currentFile );
        pm->setDefinedOnLine( declLine );
        pm->setDefinitionEndsOnLine( getLineno() );
      }
      else
      {
	// see below
        kdWarning() << "No method by the name " << name << " found in class " << className << endl;
        aMethod.out();
      }
    }
    else
// EO
  	{
  	  QString path = className;
  	  if (scope->path())        // Don't produce bad class names
  	    path = scope->path() + "." + path;
  	
      kdDebug() << "scope path is " << path << endl;
      aClass = store.getClassByName( path );
      if( aClass != NULL)
      {
        pm = aClass->getMethod( aMethod );
        if( pm != NULL )
        {
          aClass->setDefinedInFile( currentFile );
          pm->setIsInHFile( false );
          pm->setDefinedInFile( currentFile );
          pm->setDefinedOnLine( declLine );
          pm->setDefinitionEndsOnLine( getLineno() );
        }
        else
        {
	  // again, if class x{ x::y( ) }; is found the classparser thinks it's an implementation
	  // and we get a warning right here -> bug 2374
          kdWarning() << "No method by the name " << name << " found in class " << path << endl;
          aMethod.out();
        }
      }
      else
        kdWarning() << "No class by the name " << path << " found" << endl;
    }
  }
}

/*********************************************************************
 *                                                                   *
 *                           CLASS METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- CClassParser::checkClassDecl()
 * checkClassDecl()
 *   Push lexems on the stack until we find something we know and 
 *   return what we found.
 *
 * Parameters:
 *   -
 * Returns:
 *   int          Type of element found.
 *-----------------------------------------------------------------*/
int CClassParser::checkClassDecl()
{
  bool isImpl = false;
  bool isStruct = false;
  bool isOperator = false;
  bool isMultiDecl = false;
  int retVal = CP_IS_OTHER;
//  bool exit = false;
//  CParsedLexem *aLexem;

//  while( !exit )
  while( true )
  {
    switch( lexem )
    {
      case '<':
        // Only skip templates when we're not declaring an operator.
        if( !isOperator )
//        {
          parseTemplate();
//          if( lexem == '>' )
//            getNextLexem();
//        }
          // MW eating an extra token here is the wrong way to
          // prevent the '>' from beeing pushed on the lexemStack
        else
          // MW the RightWay (tm) is to only push if no template
          // declaration was skipped.
          PUSH_LEXEM();
        break;
      case CPSTRUCT:
        isStruct = true;
        PUSH_LEXEM();
        break;
      case CPOPERATOR:
        isOperator = true;
        isStruct = false;
        PUSH_LEXEM();
        break;
      case ',':
        isMultiDecl = true;
        isStruct = false;
        PUSH_LEXEM();
        break;
      case CLCL:
        isImpl = true;
        PUSH_LEXEM();
        break;
      default:
        PUSH_LEXEM();
        break;
    }

//    PUSH_LEXEM();
    getNextLexem();

    // MW at least my compiler generates wrong code with the original exit condition.
    // SuSE 7.0, gcc 2.95.2, so nothing really exotic..
    // caveat: most of the time it's better not trying to be toooo clever.
    // this will result in more portable code.
    if ( isStruct && ( lexem == '(' || lexem == ';' || lexem == '=' || lexem == '{' ) )
      break;
    if ( isOperator && lexem == '(' && lexemStack.top()->type != CPOPERATOR )
      break;
    if ( !isOperator && ( lexem == '(' || lexem == ';' || lexem == '=' ) )
      break;
    if ( lexem == 0 )
      break;

//    exit =
//      ( isStruct && ( lexem == '(' || lexem == ';' || lexem == '=' || lexem == '{' ) )||
//      ( isOperator && lexem == '(' && lexemStack.top()->type != CPOPERATOR ) ||
//      ( !isOperator && ( lexem == '(' || lexem == ';' || lexem == '=' ) ) ||
//      ( lexem == 0 );
  }

//this is not correct when considering an operator implementation
  // Pop the top lexem to check for ::. Then put it back again.
//  aLexem = lexemStack.pop();
//  isImpl = ( !lexemStack.isEmpty() && lexemStack.top()->type == CLCL );
//  lexemStack.push( aLexem );

  // If we find a '(' it's a function of some sort.
  if( lexem == '(' )
  {
    if( isOperator )
      retVal = ( isImpl ? CP_IS_OPERATOR_IMPL : CP_IS_OPERATOR );
    else {
      if (m_isParsingClassDeclaration) {
         retVal = CP_IS_METHOD;//( isImpl ? CP_IS_METHOD_IMPL : CP_IS_METHOD );
      }
      else {
         retVal = ( isImpl ? CP_IS_METHOD_IMPL : CP_IS_METHOD );
      }
    }
  }
  else if( lexem == '{' )
  {
    if( isStruct )
      retVal = CP_IS_STRUCT;
  }
  else if( lexem != 0 ) // Attribute
  {
    // If isImpl is true, it is simply a leading namespace in the attribute type (Falk)
    if( isMultiDecl )
      retVal = CP_IS_MULTI_ATTRIBUTE; // ( isImpl ? CP_IS_MULTI_ATTR_IMPL : CP_IS_MULTI_ATTRIBUTE );
    else
      retVal = CP_IS_ATTRIBUTE; // ( isImpl ? CP_IS_ATTR_IMPL : CP_IS_ATTRIBUTE );
  }

  return retVal;
}

/*----------------------------- CClassParser::parseClassInheritance()
 * parseClassInheritance()
 *   Parse the inheritance clause of a class declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseClassInheritance( CParsedClass *aClass )
{
  REQUIRE( "Valid class", aClass != NULL );

  CParsedParent *aParent; // A parent of this class.
  QString cname;          // Name of inherited class.
  int exportit=-1;          // Parent import status(private/public/protected).

  // Add parents until we find a {
  while( lexem != '{' && lexem != 0 )
  {
    // Fetch next lexem.
    getNextLexem();
    
    // For classes with no scope identifier at inheritance.
    if( lexem == ID )
      exportit = CPPRIVATE;
    else if( lexem == CPPUBLIC || lexem == CPPROTECTED || lexem == CPPRIVATE )
    {
      exportit = lexem;
      getNextLexem();
    }
    
    cname = "";
    while( lexem != '{' && lexem != ',' && lexem != 0 )
    {
      cname += getText();
      getNextLexem();
    }
    
    // Add the parent.
    if( exportit != -1 )
    {
      aParent = new CParsedParent();
      aParent->setName( cname );
      aParent->setExport( exportit );
      
      aClass->addParent( aParent );
    }
  }
}

/*------------------------------ CClassParser::parseClassHeader()
 * parseClassHeader()
 *   Parse a class header, i.e find out classname and possible
 *   parents.
 *
 * Parameters:
 *   list           List of classes to add the new class to.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClass *CClassParser::parseClassHeader()
{
  CParsedClass *aClass;
  QString scopeStr;
  CParsedLexem *aLexem;
  bool foundCLCL = false;

  // Skip to the identifier
  if( lexem == CPCLASS )
    getNextLexem();

  // Ok, this seems to be a class definition so allocate the
  // the new object and set some values.
  aClass = new CParsedClass();
  aClass->setDeclaredOnLine( getLineno() );
  aClass->setDefinedInFile( currentFile );
  aClass->setDeclaredInFile( currentFile );

  // Skip stuff before the real classname.
  while( lexem != 0 && ( lexem == ID || lexem == CLCL ) )
  {
    if( lexem == CLCL )
      foundCLCL = true;

    PUSH_LEXEM();
    getNextLexem();
  }

  // The classname is at the top of the stack.
  aLexem = lexemStack.pop();

  if(aLexem == 0) {
    kdDebug() << "ERROR in classparser: CParsedClass *CClassParser::parseClassHeader()" << endl;
    return 0;
  }

  aClass->setName( aLexem->text );
  delete aLexem;

  // Check if this a implementation of a nested class.
  if( foundCLCL )
  {
    while( !lexemStack.isEmpty() && lexemStack.top()->type == CLCL )
    {
      // Skip ::
      delete lexemStack.pop();

      // Fetch the name of the parent.
      aLexem = lexemStack.pop();
      if(aLexem == 0) {
        kdDebug() << "ERROR in classparser: CParsedClass *CClassParser::parseClassHeader()" << endl;
        return 0;
      }

      // Only add . if the string contains something.
      if( !scopeStr.isEmpty() )
        scopeStr = "." + scopeStr;

      scopeStr = aLexem->text + scopeStr;
      delete aLexem;
    }

    // Set the scope of the class.
    aClass->setDeclaredInScope( scopeStr );
  }

  if( lexem != ':' && lexem != '{' ) // Bogus class definition..
  {
    delete aClass;
    aClass = NULL;
  }

  return aClass;
}

/*------------------------------------ CClassParser::parseClassLexem()
 * parseClassLexem()
 *   Handle lexem that are specific of a class.
 *
 * Parameters:
 *   aClass         The class which declarations we are parsing.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
bool CClassParser::parseClassLexem( CParsedClass *aClass )
{
  REQUIRE1( "Valid class", aClass != NULL, false );

  CParsedClass *childClass;
  CParsedMethod *aMethod;
  QString childMap;
  bool exit = false;

  switch( lexem )
  {
    case CPPUBLIC:
      declaredScope = PIE_PUBLIC;
      methodType = 0;
      break;
    case CPPROTECTED:
      declaredScope = PIE_PROTECTED;
      methodType = 0;
      break;
    case CPPRIVATE:
      declaredScope = PIE_PRIVATE;
      methodType = 0;
      break;
    case QTSIGNAL:
    case QTSLOT:
      methodType = lexem;
      break;
    case CPCLASS:
      childClass = parseClassHeader();
      if( childClass != NULL )
      {
        childClass->setDeclaredInScope( aClass->path() );

        if( store.hasClass( childClass->path() ) ) {
  	      CParsedClass *	parsedClassRef = store.getClassByName( childClass->path() );
  	      parsedClassRef->setDeclaredOnLine( childClass->declaredOnLine );
  	      parsedClassRef->setDeclaredInFile( childClass->declaredInFile );
  	      parsedClassRef->setDeclaredInScope( childClass->declaredInScope );
  	      delete childClass;
  	      childClass = parsedClassRef;
        } else {
          store.addClass( childClass );
        }

        // When the childclass gets added to its parent class
        // the declaredInScope attribute gets set which gives it the
        // correct path.
        aClass->addClass( childClass );

        // Check for inheritance
        if( lexem == ':' ) {
          parseClassInheritance( childClass );
        }
        parseClass( childClass );
      }
      break;
    case CPVIRTUAL:
      // Skip the virtual token.
      getNextLexem();

      // Get the method.
      aMethod = parseMethodDeclaration();

      if( aMethod != NULL )
      {
		CParsedMethod *	pm = aClass->getMethod(*aMethod);
		
		if (pm != NULL) {
          aMethod->setDefinedInFile( pm->definedInFile );
          aMethod->setDefinedOnLine( pm->definedOnLine );
          aMethod->setDefinitionEndsOnLine( pm->definitionEndsOnLine );
          aClass->removeMethod(pm);
		}
        	
        	aMethod->setIsVirtual( true );
		aClass->addMethod(aMethod);
      }
      break;
    case CPSTRUCT:
    case CPCONST:
    case ID:
      // Ignore everything that hasn't got any scope declarator.
      if( declaredScope != PIE_GLOBAL )
      {
        // If the type is signal or slot we KNOW it's a method.
        if( methodType != 0 )
        {
          aMethod = parseMethodDeclaration();
          if( aMethod && methodType == QTSIGNAL)
          {
            aMethod->setIsSignal( true );
            aClass->addSignal( aMethod );
          }
          if( aMethod && methodType == QTSLOT)
          {
//          (void) printf("slot: %s\n", aMethod->name.data());
	    
	    CParsedMethod *	pm = aClass->getMethod(*aMethod);
	    if (pm != NULL) {
	      aMethod->setDefinedInFile( pm->definedInFile );
	      aMethod->setDefinedOnLine( pm->definedOnLine );
	      aMethod->setDefinitionEndsOnLine( pm->definitionEndsOnLine );
	      aClass->removeMethod(pm);
	    }
            aMethod->setIsSlot( true );
            aClass->addSlot( aMethod );
          }
        }
        else {
          parseMethodAttributes( aClass );
	}
      }
      isStatic=false;
      break;
    case '}':
      exit = true;
      aClass->setDeclarationEndsOnLine( getLineno() );
      break;
    case 0:
      exit = true;
      break;
    default:
      break;
  }

  return exit;
}

/*---------------------------------------- CClassParser::parseClass()
 * parseClass()
 *   Parse a class declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedClass *CClassParser::parseClass( CParsedClass * aClass)
{
  bool exit = false;
  PIExport oldScope=declaredScope;

  if( aClass != NULL )
  {
    // Set the comment if in range.
    if( commentInRange( aClass ) )
      aClass->setComment( comment );

    declaredScope = PIE_PRIVATE;

    // Iterate until we find the end of the class.
    while( !exit )
    {
      getNextLexem();

      declStart = getLineno();

      if( isGenericLexem() )
        parseGenericLexem( aClass );
      else
        exit = parseClassLexem( aClass );
    }    
  }

  declaredScope=oldScope;
  return aClass;
}

/*********************************************************************
 *                                                                   *
 *                         TOP LEVEL METHODS                         *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassParser::isGenericLexem()
 * isGenericLexem()
 *   Tells if the current lexem is generic and needs no special
 *   handling depending on the current scope.
 *
 * Parameters:
 *   -
 * Returns:
 *   bool       Is the current lexem generic?
 *-----------------------------------------------------------------*/
bool CClassParser::isGenericLexem()
{
  return ( lexem == CPENUM || lexem == CPUNION || lexem == CPSTATIC ||
           lexem == CPTYPEDEF || lexem == CPTEMPLATE || lexem == CPTHROW );
}


/*----------------------------- CClassParser::parseMethodAttributes()
 * parseMethodAttributes()
 *   Parse and add method and attribute declarations.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseMethodAttributes( CParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );

  QList<CParsedAttribute> list;
  CParsedAttribute *anAttr;
  CParsedMethod *aMethod;
  CParsedMethod *oldMethod;
  int declType;
  QString str;

  declType = checkClassDecl();
  switch( declType )
  {
    case CP_IS_ATTRIBUTE:
      anAttr = new CParsedAttribute();
      fillInParsedVariable( anAttr );
      if( !anAttr->name.isEmpty() )
        aContainer->addAttribute( anAttr );
      else
        delete anAttr;
      break;
    case CP_IS_ATTR_IMPL:
      // Empty the stack
      emptyStack();
      break;
    case CP_IS_OPERATOR_IMPL:
    case CP_IS_METHOD_IMPL:
      parseMethodImpl( declType == CP_IS_OPERATOR_IMPL,aContainer );
      break;
    case CP_IS_OPERATOR:
    case CP_IS_METHOD:
      aMethod = new CParsedMethod();
      fillInParsedMethod( aMethod, declType == CP_IS_OPERATOR );

      if( !aMethod->name.isEmpty() )
      {
        // If this method already exists we just get the attributes
        // for the definition, and copy them into the new parsed method
        // Then remove the old method.
        oldMethod = aContainer->getMethod( *aMethod );
        if( oldMethod != NULL )
        {
          aMethod->setDefinedInFile( oldMethod->definedInFile );
          aMethod->setDefinedOnLine( oldMethod->definedOnLine );
          aMethod->setDefinitionEndsOnLine( oldMethod->definitionEndsOnLine );
          aContainer->removeMethod(oldMethod);
        }
        aContainer->addMethod( aMethod );
      }
      else
        delete aMethod;
      break;
    case CP_IS_MULTI_ATTR_IMPL:
      kdDebug() << "Found multi attr implementation." << endl;
      emptyStack();
      break;
    case CP_IS_MULTI_ATTRIBUTE:
      fillInMultipleVariable( aContainer );
      break;
    case CP_IS_STRUCT:
      fillInParsedStruct( aContainer );
      break;
    case CP_IS_OTHER:
      emptyStack();
      break;
  }
}

/*--------------------------------- CClassParser::parseGenericLexem()
 * parseGenericLexem()
 *   Take care of generic lexem.
 *
 * Parameters:
 *   aContainer  Container to store parsed items in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseGenericLexem(  CParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );
 
  switch( lexem )
  {
    case CPENUM:
      parseEnum();
      break;
    case CPUNION:
      parseUnion();
      break;
    case CPSTATIC:
      isStatic = true;
      break;
    case CPTYPEDEF:
      getNextLexem();
      switch( lexem )
      {
        case CPSTRUCT:
          parseStruct( aContainer );
          break;
        case CPENUM:
          parseEnum();
          break;
        case CPUNION:
          parseUnion();
          break;
      }
      
      // Skip the typedef name.
      while( lexem != ';' && lexem != 0)
        getNextLexem();
      
      emptyStack();
      break;
    case CPTEMPLATE:
      parseTemplate();
      break;
    case CPTHROW:
      kdDebug() << "Found throw statement." << endl;
      break;
  }
}
  
/*--------------------------------- CClassParser::parseTopLevelLexem()
 * parseTopLevelLexem()
 *   Take care of lexem in a top-level context.
 *
 * Parameters:
 *   scope      The current scope.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseTopLevelLexem( CParsedScopeContainer *scope )
{
  REQUIRE( "Valid scope", scope != NULL );

  CParsedClass *aClass;
  CParsedClass *parentClass;
  CParsedScopeContainer *parentScope;
  QString key;

  switch( lexem )
  {
    case CPNAMESPACE:
      parseNamespace( scope );
      break;
    case CPCLASS:
      aClass = parseClassHeader();
      
      if( aClass != NULL )
      {
      	// Build up the 'declared in scope' path for the class. But
      	// don't put the class into the scope hierarchy yet, in case
      	// it is already in the store (when 'aClass' will need to be
      	// deleted, and the existing parsed class used instead).
        QString savedClassPath = QString( aClass->declaredInScope );
        QString classPath = aClass->declaredInScope;

        if( classPath.isEmpty() && !scope->path().isEmpty() )
          classPath = scope->path();
        else if( !scope->path().isEmpty() )
        {
          // Get the parent class;
          parentClass = store.getClassByName( scope->path() );

          // If we didn't find a parent class, try to find a namespace.
          if( parentClass == NULL )
          {
            parentScope = store.getScopeByName( scope->path() );

            if( parentScope != NULL )
              classPath = parentScope->path();
          }
          else
            classPath = parentClass->path();
        }

        aClass->setDeclaredInScope( classPath );

        kdDebug() << "Storing class with path: " << aClass->path() << endl;
        
        // Check if class is in the global store, add it if missing
        if( store.hasClass( aClass->path() ) ) {
  	      CParsedClass *	parsedClassRef = store.getClassByName( aClass->path() );
  	      parsedClassRef->setDeclaredOnLine( aClass->declaredOnLine );
  	      parsedClassRef->setDeclaredInFile( aClass->declaredInFile );
  	      delete aClass;
  	      aClass = parsedClassRef;
        } else {
          store.addClass( aClass );
        }

        // Restore the 'declared in scope' path, so that 'aClass'
        // can be given the correct parent in the scope hierarchy
        aClass->setDeclaredInScope( savedClassPath );
        QString scopePath = scope->path();

        if( aClass->declaredInScope.isEmpty() && !scopePath.isEmpty() )
        {
          aClass->setDeclaredInScope(scopePath);
          scope->addClass( aClass );
        }
        else if( !scopePath.isEmpty() )
        {
          // Get the parent class;
          parentClass = store.getClassByName( scopePath );

          // If we didn't find a parent class, try to find a namespace.
          if( parentClass == NULL )
          {
            parentScope = store.getScopeByName( scopePath );

            if( parentScope != NULL )
              parentScope->addClass( aClass );
          }
          else
            parentClass->addClass( aClass );
        }

        // Check for inheritance
        if( lexem == ':' ) {
          parseClassInheritance( aClass );
        }

        m_isParsingClassDeclaration = true;
        parseClass( aClass );
      }
      break;
    case '{': // Skip implementation blocks
      skipBlock();
      break;
    case CPSTRUCT:
    case CPCONST:
    case ID:
      m_isParsingClassDeclaration = false;
      parseMethodAttributes( scope );
      break;
    default:
      break;
  }
}

/*------------------------------------- CClassParser::parseToplevel()
 * parseToplevel()
 *   Parse and add all toplevel definitions i.e classes, global
 *   functions and variables.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseToplevel()
{
  // Ok... Here we go.
  lexem = -1;

  getNextLexem();

  // Loop until we're out of lexem.
  while( lexem != 0 )
  {
    declStart = getLineno();

    if( isGenericLexem() )
      parseGenericLexem( &store.globalContainer );
    else
      parseTopLevelLexem( &store.globalContainer );

  	kapp->processEvents(500);
    getNextLexem();
  }
}

/*--------------------------------------------- CClassParser::reset()
 * reset()
 *   Reset the variables in the class.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::reset()
{
  lexem = -1;
  declaredScope = PIE_GLOBAL;
  isStatic=false;
  declStart = -1;
  comment_start = -1;
  comment_end = -1;
}

/*----------------------------------------- CClassParser::parseFile()
 * parseFile()
 *   Parse one file and add its' results to the internal structures.
 *
 * Parameters:
 *   file          The file to parse.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseFile( std::ifstream &file )
{
  reset();

  lexer = new yyFlexLexer( &file );
  parseToplevel();
  delete lexer;
}

/*********************************************************************
 *                                                                   *
 *                          PUBLIC METHODS                           *
 *                                                                   *
 ********************************************************************/

/*--------------------------------------------- CClassParser::parse()
 * parse()
 *   Parse a file and add found items to the store.
 *
 * Parameters:
 *   file           Path of the file.
 *
 * Returns:
 *   bool           Was the parsing successful.
 *-----------------------------------------------------------------*/
bool CClassParser::parse( const char *file )
{
  REQUIRE1( "Valid filename", file != NULL, false );
  REQUIRE1( "Valid filename length", strlen( file ) > 0, false );

  std::ifstream f( file );
  currentFile = file;

  // Remove all items with references to this file.
  store.removeWithReferences( file );

  // Parse the file.
  parseFile( f );

  return true;
}

/*-------------------------------------------- CClassParser::wipeout()
 * wipeout()
 *   Remove all parsed items and reset the state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::wipeout()
{
  store.wipeout();

  reset();
}
