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
// 2002-02-08 added simple enum parsing - daniel

#include <qstring.h>
#include <qregexp.h>
#include <qfile.h>
#include <kapplication.h>
#include <kdebug.h>
#include "classparser.h"
#include "programmingbycontract.h"

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
CClassParser::CClassParser(ClassStore *classstore)
{
  store = classstore;
  reset();
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
  while( !lexemStack.isEmpty() )
    delete lexemStack.pop();
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
bool CClassParser::commentInRange( ParsedItem *aItem )
{
  REQUIRE1( "Valid item", aItem != NULL, false );

  int range;

  range = (aItem->declaredOnLine() - ( comment_end - 1 ) );

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
void CClassParser::parseStructDeclarations( ParsedStruct *aStruct)
{
  REQUIRE( "Valid struct", aStruct != NULL );

  PIAccess oldAccess = declaredAccess;

  declaredAccess=PIE_PUBLIC;

  while( lexem != '}' && lexem != 0 )
  {
    if( lexem != '}' )
    {
      declStart = getLineno();
      switch( lexem )
      {
        case ID:
          parseMethodAttributes( aStruct );
          break;
        case CPENUM:
          parseEnum();
          break;
        case CPUNION:
          parseUnion();
          break;
        case CPSTRUCT:
          parseStruct( aStruct );
          break;
        default:
          kdDebug(9007) << "Found unknown struct declaration." << endl;
      }

      if( lexem != '}' )
        getNextLexem();
    }
  }
  declaredAccess=oldAccess;
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
void CClassParser::fillInParsedStruct( ParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );
  REQUIRE( "Valid lexem", lexem == '{' );

  ParsedStruct *aStruct = new ParsedStruct();

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

  parseStructDeclarations( aStruct );

  // Skip '}'
  getNextLexem();

  // Set the point where the struct ends.
  aStruct->setDeclarationEndsOnLine( getLineno() );

  // If we find a name here we use the typedef name as the struct name.
  if( lexem == ID )
    aStruct->setName( getText() );
  
  if( commentInRange( aStruct ) )
    aStruct->setComment( comment );

  if( aStruct != NULL && !aStruct->name().isEmpty() )
  {
    aContainer->addStruct( aStruct );

    // This was done in prior versions to maintain a list
    // of all structs in all namespaces. Doesn't seem
    // need currently.
    // Always add structs to the global container.
    //    if( aContainer != &store->globalContainer )
    //      store->globalContainer.addStruct( aStruct );

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
void CClassParser::parseStruct( ParsedContainer *aContainer )
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

/*--- CClassParser::parseEnum( )
 * parseEnum( ParsedContainer* aContainer )
 * tries to parse an enum right
 *
 * Parameters: a valid container
 *
 * Returns   : -
 *
 */
void
CClassParser::parseEnum( ParsedContainer* aContainer )
{
    ParsedAttribute* attr = new ParsedAttribute( );

    // skip {
    getNextLexem( );
    // name
    getNextLexem( );
    while( lexem != '}' && lexem != 0 ){
	if( lexem == '=' )
	    getNextLexem( );
	else if( lexem == ',' ){
		aContainer->addAttribute( attr );
		attr = new ParsedAttribute( );
	     }
	     else
		attr->setName( getText( ) );
	getNextLexem( );
    }
    aContainer->addAttribute( attr );
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
void CClassParser::parseNamespace( ParsedScopeContainer *scope )
{
  REQUIRE( "Valid scope", scope != NULL );

  ParsedScopeContainer *ns = new ParsedScopeContainer();

  // Set some info about the namespace
  ns->setDeclaredOnLine( declStart );
  ns->setDeclaredInFile( currentFile );
  ns->setDefinedInFile( currentFile );

  if( commentInRange( ns ) )
    ns->setComment( comment );

  getNextLexem();

  // cerr << "parsing namespace" << endl;
  // cerr << "lexem = " << lexem << ":" << static_cast<char>(lexem) << endl;

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
      kdDebug(9007) << "Bad namespace identifier." << endl;
      getNextLexem();
    }
  }

  // skip over '{'
  getNextLexem();
  // EO start
  // reference to an already declared namespace
  // retrieve it
  ParsedScopeContainer *ns2 = scope->getScopeByName(ns->name());
  if (ns2)
  {
    delete ns;
    ns = ns2;
  }
  else	
  {
    // this is a namespace declaration
    scope->addScope( ns );

    // Register namespace in index
    store->addScope( ns );
  }

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
void CClassParser::fillInParsedVariableHead( ParsedAttribute *anAttr )
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

  if (!addDecl.isEmpty()) {
    anAttr->setType(addDecl);
  }
  anAttr->setDeclaredInFile( currentFile );
  anAttr->setDeclaredOnLine( /* declStart */ getLineno());
  anAttr->setDefinedInFile( currentFile );
  anAttr->setDefinedOnLine( /* declStart */ getLineno());
  anAttr->setAccess( declaredAccess );
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
void CClassParser::fillInParsedVariable( ParsedAttribute *anAttr )
{
  REQUIRE( "Valid attribute", anAttr != NULL );

  QString type;

  // Get the everything exept the type
  fillInParsedVariableHead( anAttr );

  // Get the type
  fillInParsedType( type );

  // Set values in the variable.
  if( !type.isEmpty() )
    anAttr->setType( type + anAttr->type() );

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
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInMultipleVariable( ParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );

  bool exit = false;
  ParsedAttribute *anAttr;
  CParsedLexem *aLexem;
  QString type;
  QList<ParsedAttribute> list;

  // Make sure no attributes gets deleted.
  list.setAutoDelete( false );

  while( !lexemStack.isEmpty() && !exit )
  {
    anAttr = new ParsedAttribute();

    // Get the variable name.
    fillInParsedVariableHead( anAttr );

    if (!lexemStack.isEmpty())
    {
      // Add pointer stuff
      while(  lexemStack.top()->type != ID && lexemStack.top()->type != ',' )
      {
        aLexem = lexemStack.pop();
        anAttr->setType(aLexem->text + anAttr->type());
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
    if( !anAttr->name().isEmpty() )
    {
      anAttr->setType(type + anAttr->type());

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
ParsedAttribute *CClassParser::parseVariable()
{
  bool skip = false;
  ParsedAttribute *anAttr = NULL;

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
    anAttr = new ParsedAttribute();
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
void CClassParser::parseFunctionArgs( ParsedMethod *method )
{
  REQUIRE( "Valid method", method != NULL );

  ParsedAttribute *anAttr;
  ParsedArgument *anArg;
  bool exit = false;

  while( !exit )
  {
    // Get the variable
    anAttr = parseVariable();

    if( anAttr )
    {
      // Move the values to the argument object.
      anArg = new ParsedArgument();
      if( !anAttr->name().isEmpty() )
        anArg->setName( anAttr->name() );

      if( !anAttr->type().isEmpty() )
        anArg->setType( anAttr->type() );
      
      anArg->setNamePos( anAttr->namePos() );
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
 *   isOperator			this is not used all the operator related processing
 *  								has already been done
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedMethod(ParsedMethod *aMethod, bool isOperator)
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
  aMethod->setAccess( declaredAccess );
  aMethod->setIsStatic( isStatic );

  getNextLexem();

  // Const method
  if( lexem == CPCONST )
  {
    aMethod->setIsConst( true );
    getNextLexem();
  }

  // Other initializers
  if( lexem == ':' )
    while( lexem != 0 && lexem != '{' )
      getNextLexem();

  // Skip throw statements
  if( lexem == CPTHROW )
    skipThrowStatement();

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
ParsedMethod *CClassParser::parseMethodDeclaration()
{
  ParsedMethod *aMethod = new ParsedMethod();
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
void CClassParser::parseMethodImpl(bool isOperator, ParsedContainer *scope)
{
  ParsedClass *aClass;
  CParsedLexem *aLexem;
  QString name;
  QString className;
  ParsedMethod aMethod;
  ParsedMethod *pm;
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
    aClass = store->getClassByName( className );
    if( aClass != NULL)
    {
      pm = aClass->getMethod( &aMethod );
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
        kdDebug(9007) << "parseMethodImpl: No method by the name " << name
                      << "found in class " << className << endl;
        aMethod.out();
      }
    }
    else
// EO
  	{
  	  QString path = className;
  	  if (!scope->path().isEmpty())        // Don't produce bad class names
  	    path = scope->path() + "." + path;
  	
      kdDebug(9007) << "parseMethodImpl: scope path is " << path << endl;
      aClass = store->getClassByName( path );
      if( aClass != NULL)
      {
        pm = aClass->getMethod( &aMethod );
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
            kdDebug(9007) << "parseMethodImpl: No method by the name " << name
                           << "found in class " << path << endl;
          aMethod.out();
        }
      }
      else
        kdDebug(9007) << "parseMethodImpl: No class by the name " << path << " found" << endl;
    }
  }
}

/*--------------------------------- CClassParser::fillInParsedObjcMethod()
 * fillInParsedObjcMethod()
 *   Initialize an Objective-C method.
 *
 * Parameters:
 *   aMethod        The method to initialize.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedObjcMethod(ParsedMethod *aMethod)
{
	QString			methodSignature;
	CParsedLexem *	aLexem;
	QString			type;

	PUSH_LEXEM();
	aLexem = lexemStack.pop();
	methodSignature = aLexem->text;
	delete aLexem;

	// Set some attributes of the parsed method.
	aMethod->setAccess(declaredAccess);
	aMethod->setIsVirtual(true);
	aMethod->setIsObjectiveC(true);

	getNextLexem();

	// Skip any type specifier
	if (lexem == '(') {
		while (lexem != 0 && lexem != ')') {
			getNextLexem();
		}
		getNextLexem();
	}

	while (lexem != 0 && lexem != ';' && lexem != '{') {
		PUSH_LEXEM();
		aLexem = lexemStack.pop();
		methodSignature += aLexem->text;
		delete aLexem;
		getNextLexem();
 	
		if (lexem == 0 || lexem == ';' || lexem == '{') {
			break;
		}
  	
		while (lexem == ':') {
			methodSignature += ":";
			getNextLexem();
			
			// Skip any type specifier
			if (lexem == '(') {
				while (lexem != 0 && lexem != ')') {
					getNextLexem();
				}
				getNextLexem();
			}
			
			getNextLexem();
		}

		// Ignore any comma followed by three periods in varargs style methods
		if (lexem == ',') {
			if (lexem == '.')
				getNextLexem();
			if (lexem == '.')
				getNextLexem();
			if (lexem == '.')
				getNextLexem();
			break;
		}
	}
	
	// Set the method name
	aMethod->setName(methodSignature);
	
	return;
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
  bool exit = false;
//  CParsedLexem *aLexem;

  while( !exit )
  {
    switch( lexem )
    {
      case '<':
        // Only skip templates when we're not declaring an operator.
        if( !isOperator )
        {
          parseTemplate();
          if( lexem == '>' )
            getNextLexem();
        }
        break;
      case CPSTRUCT:
        isStruct = true;
        break;
      case CPOPERATOR:
        isOperator = true;
        isStruct = false;
        break;
      case ',':
        isMultiDecl = true;
        isStruct = false;
        break;
      case CLCL:
        isImpl = true;
        break;
    }

    PUSH_LEXEM();
    getNextLexem();

    exit =
      ( isStruct && ( lexem == '(' || lexem == ';' || lexem == '=' || lexem == '{' ) )||
      ( isOperator && lexem == '(' && lexemStack.top()->type != CPOPERATOR ) ||
      ( !isOperator && ( lexem == '(' || lexem == ';' || lexem == '=' ) ) ||
      ( lexem == 0 );
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
    else
      retVal = ( isImpl ? CP_IS_METHOD_IMPL : CP_IS_METHOD );
  }
  else if( lexem == '{' )
  {
    if( isStruct )
      retVal = CP_IS_STRUCT;
  }
  else if( lexem != 0 ) // Attribute
  {
    if( isMultiDecl )
      retVal = ( isImpl ? CP_IS_MULTI_ATTR_IMPL : CP_IS_MULTI_ATTRIBUTE );
    else
      retVal = ( isImpl ? CP_IS_ATTR_IMPL : CP_IS_ATTRIBUTE );
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
void CClassParser::parseClassInheritance( ParsedClass *aClass )
{
  REQUIRE( "Valid class", aClass != NULL );

  ParsedParent *aParent; // A parent of this class.
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
      aParent = new ParsedParent();
      aParent->setName( cname );
      PIAccess access;
      access = (exportit == CPPUBLIC)? PIE_PUBLIC
          :  (exportit == CPPROTECTED)? PIE_PROTECTED
          :  PIE_PRIVATE;
      aParent->setAccess( access );

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
ParsedClass *CClassParser::parseClassHeader()
{
  ParsedClass *aClass;
  QString scopeStr;
  CParsedLexem *aLexem;
  bool foundCLCL = false;

  // Skip to the identifier
  if( lexem == CPCLASS )
    getNextLexem();

  // Ok, this seems to be a class definition so allocate the
  // the new object and set some values.
  aClass = new ParsedClass();
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
    kdDebug(9007) << "ERROR in classparser: ParsedClass *CClassParser::parseClassHeader()" << endl;
    delete aClass;
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
#if 1
      if(aLexem == 0) {
         kdDebug() << "ERROR in classparser: CParsedClass *CClassParser::parseClassHeader()" << endl;
         return 0;
      }
#endif

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
bool CClassParser::parseClassLexem( ParsedClass *aClass )
{
  REQUIRE1( "Valid class", aClass != NULL, false );

  ParsedClass *childClass;
  ParsedMethod *aMethod;
  QString childMap;
  bool exit = false;

  switch( lexem )
  {
    case CPPUBLIC:
      declaredAccess = PIE_PUBLIC;
      methodType = 0;
      break;
    case CPPROTECTED:
      declaredAccess = PIE_PROTECTED;
      methodType = 0;
      break;
    case CPPRIVATE:
      declaredAccess = PIE_PRIVATE;
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

        bool inStore = store->hasClass( childClass->path() );
        if (inStore) {
  	      ParsedClass *	parsedClassRef = store->getClassByName( childClass->path() );
  	      parsedClassRef->setDeclaredOnLine( childClass->declaredOnLine() );
  	      parsedClassRef->setDeclaredInFile( childClass->declaredInFile() );
  	      parsedClassRef->setDeclaredInScope( childClass->declaredInScope() );
  	      delete childClass;
  	      childClass = parsedClassRef;
        }

        // When the childclass gets added to its parent class
        // the declaredInScope attribute gets set which gives it the
        // correct path.
        aClass->addClass( childClass );

        // Do we put nested classes into the index?
        //        if (!inStore)
        //            store->addClass( childClass );
        
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
		ParsedMethod *	pm = aClass->getMethod(aMethod);
		
		if (pm != NULL) {
          aMethod->setDefinedInFile( pm->definedInFile() );
          aMethod->setDefinedOnLine( pm->definedOnLine() );
          aMethod->setDefinitionEndsOnLine( pm->definitionEndsOnLine() );
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
      if( declaredAccess != PIE_GLOBAL )
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
            kdDebug(9007) << "slot: " << aMethod->name().data() << endl;

            ParsedMethod *pm = aClass->getMethod(aMethod);
            if (pm != NULL) {
               aMethod->setDefinedInFile( pm->definedInFile() );
               aMethod->setDefinedOnLine( pm->definedOnLine() );
               aMethod->setDefinitionEndsOnLine( pm->definitionEndsOnLine() );
               aClass->removeMethod(pm);
	    }
            aMethod->setIsSlot( true );
            aClass->addSlot( aMethod );
          }
        }
        else
          parseMethodAttributes( aClass );
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
ParsedClass *CClassParser::parseClass( ParsedClass * aClass)
{
  bool exit = false;
  PIAccess oldAccess = declaredAccess;

  if( aClass != NULL )
  {
    // Set the comment if in range.
    if( commentInRange( aClass ) )
      aClass->setComment( comment );

    declaredAccess = PIE_PRIVATE;

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

  declaredAccess = oldAccess;
  return aClass;
}

 /*------------------------------------ CClassParser::parseObjcCategory()
 * parseObjcCategory()
 *   Handle lexem for a category.
 *
 * Parameters:
 *   aClass         The name of the class which we are parsing.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass * CClassParser::parseObjcCategory(QString &aClassName)
{
	ParsedClass *	aClass;
	ParsedParent *	aParent;
	CParsedLexem *	aCategoryName;
	QString 			aName;
  	
	// A category
	getNextLexem();
	PUSH_LEXEM();
	aCategoryName = lexemStack.pop();
  	getNextLexem();
  	
	if (lexem == ')') {
  	  getNextLexem();
 	}
   	
	aName = aClassName + "(" + aCategoryName->text + ")";
	aClass = store->getClassByName(aName);

    if (aClass == NULL) {
		aClass = new ParsedClass();
		aClass->setName(aName);
		aClass->setDeclaredOnLine(declStart);
		aClass->setDeclaredInFile(currentFile);
	}
  	
	// Assume a category is a kind of subclass
	if (!aClass->hasParent(aClassName)) {
		aParent = new ParsedParent();
		aParent->setName(aClassName);
		aParent->setAccess(PIE_PUBLIC);
		aClass->addParent(aParent);
    }
  	
  	return aClass;
}

 /*------------------------------------ CClassParser::parseObjcImplemention()
 * parseObjcImplementation()
 *   Handle lexem for instance variables of an Objective-C class.
 *
 * Parameters:
 *   aClass         The class which we are parsing.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass *CClassParser::parseObjcImplementation()
{
	ParsedClass *	aClass;
	CParsedLexem *	aLexem;
	ParsedMethod *	aMethod;
	ParsedMethod *	pm;

	// Skip to the identifier
	if (lexem == CPOBJCIMPLEMENTATION) {
		getNextLexem();
	}

	declStart = getLineno();
	
	// Get the classname.
	PUSH_LEXEM();
	aLexem = lexemStack.pop();
	
	getNextLexem();

	if (lexem == '(') {
		// A category name in brackets after the classname
		aClass = parseObjcCategory(aLexem->text);
	} else {
		aClass = store->getClassByName(aLexem->text);

		if (aClass == NULL) {
			aClass = new ParsedClass();
			aClass->setName(aLexem->text);
			aClass->setDeclaredOnLine(declStart);
			aClass->setDeclaredInFile(currentFile);
		}
	}

	delete aLexem;
	aClass->setDefinedOnLine(declStart);
	aClass->setDefinedInFile(currentFile);
	declaredAccess = PIE_PUBLIC;

	// Iterate through the method definitions
	while (lexem != 0 && lexem != CPOBJCEND) {
		if (lexem == '+' || lexem == '-') {
			aMethod = new ParsedMethod();
			declStart = getLineno();
			fillInParsedObjcMethod(aMethod);
 	
			// A semi-colon after a method definition, before the opening curly brace
			//	should really be a syntax error in Objective-C, but it's not..
			if (lexem == ';') {
				getNextLexem();
			}
			
			// Skip implementation.
			if (lexem == '{') {
				skipBlock();
			}
			
			pm = aClass->getMethod(aMethod);

			if (pm == NULL) {
				aClass->addMethod(aMethod);
				pm = aMethod;
				pm->setDeclaredInFile(currentFile);
				pm->setDeclaredOnLine(declStart);
				pm->setDeclarationEndsOnLine(getLineno());
			} else {
				delete aMethod;
			}

			pm->setDefinedInFile(currentFile);
			pm->setDefinedOnLine(declStart);
			pm->setDefinitionEndsOnLine(getLineno());

			// Set the comment if in range.
			if (commentInRange(pm)) {
				pm->setComment(comment);
			}
		} else if (isGenericLexem()) {
			parseGenericLexem(aClass);
		}
		
		getNextLexem();
	}

	return aClass;
}

 /*------------------------------------ CClassParser::parseObjcClassLexem()
 * parseObjcClassLexem()
 *   Handle lexem for instance variables of an Objective-C class.
 *
 * Parameters:
 *   aClass         The class which instance variables we are parsing.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
bool CClassParser::parseObjcClassLexem(ParsedClass *aClass)
{
	bool exit = false;

	switch(lexem) {
	case CPOBJCPUBLIC:
		declaredAccess = PIE_PUBLIC;
		methodType = 0;
		break;
	case CPOBJCPROTECTED:
		declaredAccess = PIE_PROTECTED;
		methodType = 0;
		break;
	case CPOBJCPRIVATE:
		declaredAccess = PIE_PRIVATE;
		methodType = 0;
		break;
    case CPSTRUCT:
    case CPCONST:
    case ID:
		parseMethodAttributes(aClass);
		isStatic=false;
		break;
    case 0:
		exit = true;
		break;
    default:
		break;
	}

	return exit;
}

 /*------------------------------ CClassParser::parseObjcClassHeader()
 * parseObjcClassHeader()
 *   Parse an Objective-C class header, i.e find out classname and possible
 *   parents.
 *
 * Parameters:
 *   list
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass *CClassParser::parseObjcClassHeader()
{
	ParsedClass *	aClass;
	QString       	parentTree;
	ParsedParent *	aParent;
	CParsedLexem *	aLexem;

	// Skip to the identifier
	if (lexem == CPOBJCINTERFACE || lexem == CPOBJCPROTOCOL) {
		getNextLexem();
	}

	declStart = getLineno();
	
	// Get the classname.
	PUSH_LEXEM();
	getNextLexem();
	aLexem = lexemStack.pop();

	if (lexem == '(') {
		aClass = parseObjcCategory(aLexem->text);
	} else {
		aClass = store->getClassByName(aLexem->text);

		if (aClass == NULL) {
			aClass = new ParsedClass();
			aClass->setName(aLexem->text);
		}
	}

	delete aLexem;
	aClass->setDeclaredOnLine(declStart);
	aClass->setDeclaredInFile(currentFile);

	if (lexem == ':' ) {
		// Check for inheritance
		getNextLexem();
		PUSH_LEXEM();
		aLexem = lexemStack.pop();
		aParent = new ParsedParent();
		aParent->setName(aLexem->text);
		aParent->setAccess(PIE_PUBLIC);
		aClass->addParent(aParent);
		delete aLexem;
		getNextLexem();
	}

	// Step through the protocol list
	if (lexem != 0 && lexem == '<' ) {
		getNextLexem();

		while (lexem != 0 && lexem != '>') {
			PUSH_LEXEM();
			aLexem = lexemStack.pop();
			// Assume a protocol is a kind of superclass
			//	...but not for now unfortunately - the graphical class view
			//	works for small diagrams with multiple inheritance, but
			//	gets stuck with large ones. So stick with just single
			//	inheritance, and ignore any protocol hierarchy.
//			aParent = new CParsedParent();
//			aParent->setName(aLexem->text);
//			aParent->setExport(PIE_PUBLIC);
//			aClass->addParent(aParent);
			delete aLexem;
			getNextLexem();

			if ( lexem == ',' ) {
				getNextLexem();
			}
		}

		getNextLexem();
	}

	return aClass;
}

/*---------------------------------------- CClassParser::parseObjcClass()
 * parseObjcClass()
 *   Parse an Objective-C class declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass *CClassParser::parseObjcClass()
{
	ParsedClass *	aClass;
	ParsedMethod *	aMethod;
	ParsedMethod *	pm;

	aClass = parseObjcClassHeader();

	if (aClass == NULL) {
		return NULL;
	}
	
	// Set the comment if in range.
	if (commentInRange(aClass)) {
		aClass->setComment(comment);
	}

	declaredAccess = PIE_PUBLIC;

    // Iterate until the end of the instance variables.
	if (lexem == '{') {
		getNextLexem();
		
		while (lexem != 0 && lexem != '}' && lexem != CPOBJCEND) {
			declStart = getLineno();

			if (isGenericLexem()) {
				parseGenericLexem(aClass);
			} else {
				parseObjcClassLexem(aClass);
			}
			
			getNextLexem();
		}
	}

	// Iterate through the method declarations until the end of the class.
	declaredAccess = PIE_PUBLIC;

	while (lexem != 0 && lexem != CPOBJCEND) {
		if ( lexem == '+' || lexem == '-' ) {
			declStart = getLineno();
			aMethod = new ParsedMethod();
			fillInParsedObjcMethod(aMethod);
			pm = aClass->getMethod(aMethod);

			if (pm == NULL) {
				aClass->addMethod(aMethod);
				pm = aMethod;
				pm->setDefinedInFile(currentFile);
				pm->setDefinedOnLine(declStart);
				pm->setDefinitionEndsOnLine(getLineno());
			} else {
				delete aMethod;
			}

			// Set end of declaration.
			pm->setDeclaredInFile(currentFile);
			pm->setDeclaredOnLine(declStart);
			pm->setDeclarationEndsOnLine(getLineno());

			// Set the comment if in range.
			if (commentInRange(pm)) {
				pm->setComment(comment);
			}
		} else {
			getNextLexem();
		}
	}

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
void CClassParser::parseMethodAttributes( ParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );

  QList<ParsedAttribute> list;
  ParsedAttribute *anAttr;
  ParsedMethod *aMethod;
  ParsedMethod *oldMethod;
  int declType;
  QString str;

  declType = checkClassDecl();
  switch( declType )
  {
    case CP_IS_ATTRIBUTE:
      anAttr = new ParsedAttribute();
      fillInParsedVariable( anAttr );
      if( !anAttr->name().isEmpty() )
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
      aMethod = new ParsedMethod();
      fillInParsedMethod( aMethod, declType == CP_IS_OPERATOR );

      if( !aMethod->name().isEmpty() )
      {
        // If this method already exists we just get the attributes
        // for the definition, and copy them into the new parsed method
        // Then remove the old method.
        oldMethod = aContainer->getMethod( aMethod );
        if( oldMethod != NULL )
        {
          aMethod->setDefinedInFile( oldMethod->definedInFile() );
          aMethod->setDefinedOnLine( oldMethod->definedOnLine() );
          aMethod->setDefinitionEndsOnLine( oldMethod->definitionEndsOnLine() );
          aContainer->removeMethod(oldMethod);
        }
        aContainer->addMethod( aMethod );
      }
      else
        delete aMethod;
      break;
    case CP_IS_MULTI_ATTR_IMPL:
      kdDebug(9007) << "Found multi attr implementation." << endl;
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
void CClassParser::parseGenericLexem( ParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );
 
  switch( lexem )
  {
    case CPENUM:
      // daniel -- test for parsing enums right
      parseEnum( aContainer );
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
      kdDebug(9007) << "Found throw statement." << endl;
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
void CClassParser::parseTopLevelLexem( ParsedScopeContainer *scope )
{
  REQUIRE( "Valid scope", scope != NULL );

  ParsedClass *aClass;
  ParsedClass *parentClass;
  ParsedScopeContainer *parentScope;
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
        QString savedClassPath = aClass->declaredInScope();
        QString classPath = aClass->declaredInScope();

        if( classPath.isEmpty() && !scope->path().isEmpty() )
          classPath = scope->path();
        else if( !scope->path().isEmpty() )
        {
          // Get the parent class;
          parentClass = store->getClassByName( scope->path() );

          // If we didn't find a parent class, try to find a namespace.
          if( parentClass == NULL )
          {
            parentScope = store->getScopeByName( scope->path() );

            if( parentScope != NULL )
              classPath = parentScope->path();
          }
          else
            classPath = parentClass->path();
        }

        aClass->setDeclaredInScope( classPath );

        kdDebug(9007) << "parseTopLevelLexem: Storing class with path: " << aClass->path() << endl;
        
        // Check if class is in the global store, add it if missing
        bool inStore = store->hasClass( aClass->path() );
        if (inStore) {
            ParsedClass *parsedClassRef = store->getClassByName( aClass->path() );
            parsedClassRef->setDeclaredOnLine( aClass->declaredOnLine() );
            parsedClassRef->setDeclaredInFile( aClass->declaredInFile() );
            delete aClass;
            aClass = parsedClassRef;
        }

        // Restore the 'declared in scope' path, so that 'aClass'
        // can be given the correct parent in the scope hierarchy
        aClass->setDeclaredInScope( savedClassPath );
        QString scopePath = scope->path();

        if( aClass->declaredInScope().isEmpty())
        {
          scope->addClass( aClass );
        }
        else
        {
          // Get the parent class;
          parentClass = store->getClassByName( scopePath );

          // If we didn't find a parent class, try to find a namespace.
          if( parentClass != NULL )
            parentClass->addClass( aClass );
          else
          {
            parentScope = store->getScopeByName( scopePath );

            if( parentScope != NULL )
              parentScope->addClass( aClass );
            else
                // formerly uncommented
                store->globalScope()->addClass( aClass );
          }
        }
        
        if (!inStore)
            store->addClass( aClass );

        // Check for inheritance
        if( lexem == ':' ) {
          parseClassInheritance( aClass );
        }

        parseClass( aClass );
      }
      break;
    case '{': // Skip implementation blocks
      skipBlock();
      break;
    case CPSTRUCT:
    case CPCONST:
    case ID:
      parseMethodAttributes( scope );
      break;
	case CPOBJCIMPLEMENTATION:
		aClass = parseObjcImplementation();
		
		if (aClass != NULL && !store->hasClass(aClass->name())) {
                    kdDebug(9007) << "Storing objective implementation with path: "
                                  << aClass->path() << endl;
                    store->addClass(aClass);
		}
		break;
	case CPOBJCINTERFACE:
	case CPOBJCPROTOCOL:
		aClass = parseObjcClass();
		
		if (aClass != NULL && !store->hasClass(aClass->name())) {
                    kdDebug(9007) << "Storing objective interface with path: "
                                  << aClass->path() << endl;
                    store->addClass(aClass);
		}
    		break;
    case CPOBJCCLASS:
      // Skip the @class list.
      while( lexem != ';' && lexem != 0)
        getNextLexem();
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
      parseGenericLexem( store->globalScope() );
    else
      parseTopLevelLexem( store->globalScope() );

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
  declaredAccess = PIE_GLOBAL;
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
void CClassParser::parseFile( ifstream &file )
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
bool CClassParser::parse( const QString &file )
{
  ifstream f( QFile::encodeName(file) );
  currentFile = file;

  kdDebug(9007) << "Parsing file " << file << endl;
  
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
  store->wipeout();

  reset();
}
/** get a list of all classes */
ClassStore * CClassParser::getClassStore(){
  return  store;
}
