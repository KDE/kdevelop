/***************************************************************************
                          ClassParser.cpp  -  description
                             -------------------
    begin                : Mon Mar 15 1999
    copyright            : (C) 1999 by Jonas Nordin
    email                : jonas.nordin@cenacle.se
   
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

#include <iostream.h>
#include <qregexp.h> 
#include <assert.h>
#include <kapp.h>
#include "ClassParser.h"

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
  while( lexem != '}' && lexem != 0 )
  {
    if( lexem != '}' )
    {
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
          PUSH_LEXEM();
          // Check if it's a variable declaration or a struct declaration.
          getNextLexem();

          if( lexem == '{' ) // Struct declaration.
          {
            skipBlock();
            // Goto the end of the declaration.
            while( lexem != ';' && lexem != 0 )
              getNextLexem();
          }
          else
            PUSH_LEXEM();
          break;
        default:
          debug( "Found unknown struct declaration." );
      }

      if( lexem != '}' )
        getNextLexem();
    }
  }
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
  assert( aContainer != NULL );
  assert( lexem == '{' );

  CParsedStruct *aStruct = new CParsedStruct();

  // Set some info about the struct.
  aStruct->setDeclaredOnLine( getLineno() );
  aStruct->setDeclaredInFile( currentFile );
  aStruct->setDefinedInFile( currentFile );

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
  
  if( aStruct != NULL && !aStruct->name.isEmpty() )
    aContainer->addStruct( aStruct );
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
           lexem == '}' || lexem == 0 );
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
  bool exit=false;
  CParsedLexem *aLexem;
  QString arrayDecl;

  // Check for an array declaration.
  if( lexemStack.top()->type == ']' )
  {
    exit = false;
    while( !exit )
    {
      aLexem = lexemStack.pop();
      arrayDecl = aLexem->text + arrayDecl;

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
  if( !lexemStack.isEmpty() && lexemStack.top()->type == ID && lexemStack.count() > 1 )
  {
    aLexem = lexemStack.pop();
    anAttr->setName( aLexem->text );
    delete aLexem;
  }

  anAttr->setDeclaredInFile( currentFile );
  anAttr->setDeclaredOnLine( getLineno() );
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
  QString type;

  // Get the everything exept the type
  fillInParsedVariableHead( anAttr );

  // Get the type
  fillInParsedType( type );

  // Set values in the variable.
  if( !type.isEmpty() )
    anAttr->setType( type + anAttr->type );

  // Skip default values
  if( lexem == '=' )
    while( lexem != ';' && lexem != 0 )
      getNextLexem();

  // If we reach end of file, this is not a valid attribute.
  if( lexem == 0 )
    anAttr->setName( "" );
  else // Set the end of this variable declaration.
    anAttr->setDeclarationEndsOnLine( getLineno() );
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
  bool exit = false;
  CParsedAttribute *anAttr;
  CParsedLexem *aLexem;
  QString type;
  QList<CParsedAttribute> list;

  // Make sure no attributes gets deleted.
  list.setAutoDelete( false );

  while( !exit )
  {
    anAttr = new CParsedAttribute();

    // Get the variable name.
    fillInParsedVariableHead( anAttr );

    // Add pointer stuff
    while( lexemStack.top()->type != ID && lexemStack.top()->type != ',' )
    {
      aLexem = lexemStack.pop();
      anAttr->type = aLexem->text + anAttr->type;
      delete aLexem;
    }

    exit = ( lexemStack.top()->type == ID );

    // Remove separating ','
    if( lexemStack.top()->type == ',' )
      delete lexemStack.pop();

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
      skip = true;
    else if( !skip && lexem == '(' ) // Check for function pointer
    {
      getNextLexem();
      if( lexem == '*' )
      {
        lexemStack.push( new CParsedLexem( '(', "(" ));
        PUSH_LEXEM();

        // Push variable name.
        getNextLexem();
        PUSH_LEXEM();

        // Push ')'
        getNextLexem();
        PUSH_LEXEM();

        getNextLexem();

        // We'll take a chance that this declaration doesn't include function pointers.
        while( lexem != 0 && lexem != ')' )
        {
          PUSH_LEXEM();
          getNextLexem();
        }
        
        // Add ')'
        PUSH_LEXEM();
      }
    }
    else if( !skip && !isEndOfVarDecl() )
    {
      PUSH_LEXEM();
    }

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
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedMethod(CParsedMethod *aMethod, bool isOperator)
{
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
  aMethod->setDefinedOnLine( getLineno() );
  aMethod->setDeclaredOnLine( getLineno() );
  aMethod->setDefinedInFile( currentFile );
  aMethod->setDeclaredInFile( currentFile );
  aMethod->setExport( declaredScope );
  aMethod->setIsStatic( isStatic );

  getNextLexem();

  // Const method
  if( lexem == CONST )
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

  // Add all lexems on the stack until we find the start of the
  // parameter list.
  while( lexem != '(' && lexem != ';' && lexem != 0 )
  {
    PUSH_LEXEM();
    getNextLexem();
  }

  if( lexem == '(' )
    fillInParsedMethod( aMethod );
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
void CClassParser::parseMethodImpl(bool isOperator)
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
  while( lexemStack.top()->type != CLCL )
    delete lexemStack.pop();

  // Delete '::'
  aLexem = lexemStack.pop();
  delete aLexem;

  // Get the classname
  aLexem = lexemStack.pop();
  className = aLexem->text;
  delete aLexem;

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
  fillInParsedMethod( &aMethod );

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
        warning( "No method by the name %s found in class %s", 
                 name.data(), className.data() );
        aMethod.out();
      }
    }
    else
      warning( "No class by the name %s found", className.data() );
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
  bool exit = false;
  CParsedLexem *aLexem;

  declStart = getLineno();

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
    }

    PUSH_LEXEM();
    getNextLexem();

    exit =
      ( isStruct && ( lexem == '(' || lexem == ';' || lexem == '=' || lexem == '{' ) )||
      ( isOperator && lexem == '(' && lexemStack.top()->type != CPOPERATOR ) || 
      ( !isOperator && ( lexem == '(' || lexem == ';' || lexem == '=' ) ) ||
      ( lexem == 0 ); 
  }

  // Pop the top lexem to check for ::. Then put it back again.
  aLexem = lexemStack.pop();
  isImpl = ( !lexemStack.isEmpty() && lexemStack.top()->type == CLCL );
  lexemStack.push( aLexem );

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
void CClassParser::parseClassInheritance( CParsedClass *aClass )
{
  CParsedParent *aParent; // A parent of this class.
  QString cname;          // Name of inherited class.
  int export=-1;          // Parent import status(private/public/protected).

  // Add parents until we find a {
  while( lexem != '{' && lexem != 0 )
  {
    // Fetch next lexem.
    getNextLexem();
    
    // For classes with no scope identifier at inheritance.
    if( lexem == ID )
      export = PRIVATE;
    else if( lexem == PUBLIC || lexem == PROTECTED || lexem == PRIVATE )
    {
      export = lexem;
      getNextLexem();
    }
    
    cname = "";
    while( lexem != '{' && lexem != ',' && lexem != 0 )
    {
      cname += getText();
      getNextLexem();
    }
    
    // Add the parent.
    if( export != -1 )
    {
      aParent = new CParsedParent();
      aParent->setName( cname );
      aParent->setExport( export );
      
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
  QString parentTree;
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
  aClass->setName( aLexem->text );
  delete aLexem;

  // Check if this a implementation of a nested class.
  if( foundCLCL )
  {
    while( !lexemStack.isEmpty() && lexemStack.top()->type == CLCL )
    {
      // Skip ::
      delete lexemStack.pop();

      // Append parent.
      aLexem = lexemStack.pop();
      
      // Only add . if the string contains something.
      if( !parentTree.isEmpty() )
        parentTree = "." + parentTree;

      parentTree = aLexem->text + parentTree;
      delete aLexem;
    }
    
    aClass->setDeclaredInClass( parentTree );
  }

  // Check for inheritance
  if( lexem == ':' )
    parseClassInheritance( aClass );
  else if( lexem != '{' ) // Bogus class definition..
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
  CParsedClass *childClass;
  CParsedMethod *aMethod;
  QString childMap;
  bool exit = false;

  switch( lexem )
  {
    case PUBLIC:
      declaredScope = PIE_PUBLIC;
      methodType = 0;
      break;
    case PROTECTED:
      declaredScope = PIE_PROTECTED;
      methodType = 0;
      break;
    case PRIVATE:
      declaredScope = PIE_PRIVATE;
      methodType = 0;
      break;
    case QTSIGNAL:
    case QTSLOT:
      methodType = lexem;
      break;
    case CPCLASS:
      childClass = parseClass();
      if( childClass != NULL )
      {
        childClass->setDeclaredInClass( aClass->name );
        aClass->addClass( childClass );
        childMap = aClass->name + "." + childClass->name;
        store.globalContainer.addSubClass( childMap, childClass );
      }
      break;
    case CPVIRTUAL:
      // Skip the virtual token.
      getNextLexem();

      // Get the method.
      aMethod = parseMethodDeclaration();

      if( aMethod != NULL )
      {
        aMethod->setIsVirtual( true );
        aClass->addMethod( aMethod );
      }
      break;        
    case CPSTRUCT:
    case CONST:
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
CParsedClass *CClassParser::parseClass()
{
  CParsedClass *aClass;
  bool exit = false;

  aClass = parseClassHeader();

  if( aClass != NULL )
  {
    declaredScope = PIE_GLOBAL;

    // Iterate until we find the end of the class.
    while( !exit )
    {
      getNextLexem();

      if( isGenericLexem() )
        parseGenericLexem( aClass );
      else
        exit = parseClassLexem( aClass );
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
  return ( lexem == CPENUM || lexem == CPUNION || lexem == STATIC ||
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
      parseMethodImpl( declType == CP_IS_OPERATOR_IMPL );
      break;
    case CP_IS_OPERATOR:
    case CP_IS_METHOD:
      aMethod = new CParsedMethod();
      fillInParsedMethod( aMethod, declType == CP_IS_OPERATOR );

      if( !aMethod->name.isEmpty() )
      {
        // If this method already exists we just set some attributes.
        oldMethod = aContainer->getMethod( *aMethod );
        if( oldMethod != NULL )
        {
          oldMethod->setIsInHFile( false );
          oldMethod->setDefinedInFile( aMethod->definedInFile );
          oldMethod->setDefinedOnLine( aMethod->definedOnLine );
          oldMethod->setDefinitionEndsOnLine( aMethod->definitionEndsOnLine );

          delete aMethod;
        }
        else
          aContainer->addMethod( aMethod );
      }
      else
        delete aMethod;
      break;
    case CP_IS_MULTI_ATTR_IMPL:
      debug( "Found multi attr implementation." );
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
  assert( aContainer != NULL );

  switch( lexem )
  {
    case CPENUM:
      parseEnum();
      break;
    case CPUNION:
      parseUnion();
      break;
    case STATIC:
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
      debug( "Found throw statement." );
      break;
  }
}
  
/*--------------------------------- CClassParser::parseTopLevelLexem()
 * parseTopLevelLexem()
 *   Take care of lexem in a top-level context.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseTopLevelLexem()
{
  CParsedClass *aClass;
  CParsedClass *parentClass;
  QString key;

  switch( lexem )
  {
    case CPCLASS:
      aClass = parseClass();
      if( aClass != NULL )
      {
        if( aClass->isSubClass() )
        {
          // Get the parent class;
          parentClass = store.getClassByName( aClass->declaredInClass );

          if( parentClass != NULL )
            parentClass->addClass( aClass );
          else
            debug( "Didn't find parentclass: '%s'", aClass->declaredInClass.data() );

          // Create the subclass hierarchy.
          key = aClass->declaredInClass;
          key += ".";
          key += aClass->name;
          
          store.globalContainer.addSubClass( key, aClass );
        }
        else if( !store.hasClass( aClass->name ) )
          store.addClass( aClass );
        else
          debug( "Found new definition of class %s", aClass->name.data() );
      }
      break;
    case '{': // Skip implementation blocks
      skipBlock();
      break;
    case CPSTRUCT:
    case CONST:
    case ID:
      parseMethodAttributes( &store.globalContainer );
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
    if( isGenericLexem() )
      parseGenericLexem( &store.globalContainer );
    else
      parseTopLevelLexem();

    //    kapp->processEvents();
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
bool CClassParser::parse( const char *file )
{
  assert( file != NULL );

  ifstream f( file );
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
