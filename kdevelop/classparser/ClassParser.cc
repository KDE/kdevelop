/********************************************************************
* Name    : Implementation of the classparser.                      *
* ------------------------------------------------------------------*
* File    : ClassParser.cc                                          *
* Author  : Jonas Nordin (jonas.nordin@cenacle.se)                  *
* Date    : Mon Mar 15 14:18:46 CET 1999                            *
*                                                                   *
* ------------------------------------------------------------------*
* Purpose :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Usage   :                                                         *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Functions:                                                        *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Modifications:                                                    *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
*********************************************************************/

#include <iostream.h>
#include <qregexp.h> 
#include <assert.h>
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

/*---------------------------------------- CClassParser::parseStruct()
 * parseStruct()
 *   Parse a structure.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseStruct( CParsedContainer *aContainer )
{
  assert( aContainer != NULL );

  CParsedStruct *aStruct = new CParsedStruct();

  // Set some info about the struct.
  aStruct->setDefinedOnLine( getLineno() );
  aStruct->setDefinedInFile( currentFile );

  getNextLexem();

  // Check if this struct has a name
  if( lexem == ID )
  {
    aStruct->setName( getText() );
    getNextLexem();
  }

  switch( lexem )
  {
    case '{': // A struct definition.
      // Jump to first declaration or to '}'.
      getNextLexem();

      parseStructDeclarations( aStruct );

      // Skip '}'
      getNextLexem();

      // If we find a name here we use the typedef name as the struct name.
      if( lexem == ID )
        aStruct->setName( getText() );
      break;
    case ';': // Forward declaration.
      delete aStruct;
      aStruct = NULL;
      break;
  }
  
  if( aStruct != NULL && !aStruct->name.isEmpty() )
    aContainer->addStruct( aStruct );
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

  anAttr->setDefinedInFile( currentFile );
  anAttr->setDefinedOnLine( getLineno() );
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

  while( lexem != ')' )
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

    if( lexem == ',' )
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
  if( lexem != ')' )
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

  // Skip implementation.
  if( lexem == '{' )
    skipBlock();
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
  while( lexem != '(' )
  {
    PUSH_LEXEM();
    getNextLexem();
  }

  fillInParsedMethod( aMethod );

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

  // Remove all other classname'::' declarations.
  while( lexemStack.top() && lexemStack.top()->type == CLCL )
  {
    // Delete '::'
    aLexem = lexemStack.pop();
    delete aLexem;

    // Delete classname
    aLexem = lexemStack.pop();
    delete aLexem;
  }

  // To make things easier we push the methodname again.
  lexemStack.push( new CParsedLexem( ID, name ) );

  // Get the method declaration.
  fillInParsedMethod( &aMethod );

  // Try to move the values to the declared method.
  aClass = store.getClassByName( className );
  if( aClass != NULL )
  {
    pm = aClass->getMethod( aMethod );
    if( pm != NULL )
    {
      aClass->setDeclaredInFile( currentFile );
      pm->setIsInHFile( false );
      pm->setDeclaredInFile( currentFile );
      pm->setDeclaredOnLine( declLine );
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
  bool isOperator = false;
  bool isMultiDecl = false;
  int retVal = CP_IS_OTHER;
  bool exit = false;

  while( !exit )
  {
    switch( lexem )
    {
      case CLCL:
        isImpl = true;
        break;
      case CPOPERATOR:
        isOperator = true;
        break;
      case ',':
        isMultiDecl = true;
        break;
    }

    PUSH_LEXEM();
    getNextLexem();

    exit = ( ( isOperator && lexem == '(' && lexemStack.top()->type != CPOPERATOR ) || 
             ( !isOperator && ( lexem == '(' || lexem == ';' || lexem == '=' ) ) ||
             ( lexem == 0 ) ); 
  }

  // If we find a '(' it's a function of some sort.
  if( lexem == '(' )
  {
    if( isOperator )
      retVal = ( isImpl ? CP_IS_OPERATOR_IMPL : CP_IS_OPERATOR );
    else
      retVal = ( isImpl ? CP_IS_METHOD_IMPL : CP_IS_METHOD );
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
  while( lexem != '{' )
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
    while( lexem != '{' && lexem != ',' )
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

  // Skip to the identifier
  if( lexem == CPCLASS )
    getNextLexem();

  // Ok, this seems to be a class definition so allocate the
  // the new object and set some values.
  aClass = new CParsedClass();
  aClass->setName( getText() );
  aClass->setDefinedOnLine( getLineno() );
  aClass->setDefinedInFile( currentFile );
  aClass->setDeclaredInFile( currentFile );
  
  getNextLexem();

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

/*----------------------------- CClassParser::parseClassDeclarations()
 * parseClassDeclarations()
 *   Parse the declarations of a class.
 *
 * Parameters:
 *   aClass         The class which declarations we are parsing.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseClassDeclarations( CParsedClass *aClass )
{
  CParsedClass *childClass;
  CParsedMethod *aMethod;
  bool exit = false;

  declaredScope = PIE_GLOBAL;

  while( !exit )
  {
    getNextLexem();
    
    switch( lexem )
    {
      case PUBLIC:
        declaredScope = PIE_PUBLIC;
        methodType = 0;
        break;
      case PROTECTED:
        declaredScope = PIE_PROTECTED;
        methodType = 0;
      case PRIVATE:
        declaredScope = PIE_PRIVATE;
        methodType = 0;
        break;
      case QTSIGNAL:
      case QTSLOT:
        methodType = lexem;
        break;
      case CPENUM:
        parseEnum();
        break;
      case CPUNION:
        parseUnion();
        break;
      case CPTEMPLATE:
        parseTemplate();
        break;
      case CPCLASS:
        childClass = parseClass();
        if( childClass != NULL )
        {
          store.addClass( childClass );
          aClass->addChildClass( childClass->name );
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
      case STATIC:
        isStatic = true;
        break;
      case CPSTRUCT:
        parseStruct( &store.globalContainer );
        break;
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
              aClass->addSignal( aMethod );
            if( aMethod && methodType == QTSLOT)
              aClass->addSlot( aMethod );
          }
          else
            parseMethodAttributes( aClass );
        }
        isStatic=false;
        break;
      case '}':
        exit = true;
        break;
      case 0:
        exit = true;
        break;
      default:
        break;
    }
  }
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

  aClass = parseClassHeader();

  if( aClass != NULL )
    parseClassDeclarations( aClass );

  return aClass;
}

/*********************************************************************
 *                                                                   *
 *                         TOP LEVEL METHODS                         *
 *                                                                   *
 ********************************************************************/

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
  int declType;

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
        aContainer->addMethod( aMethod );
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
  CParsedClass *aClass;

  // Ok... Here we go.
  lexem = -1;

  while( lexem != 0 )
  {
    getNextLexem();

    switch( lexem )
    {
      case CPTYPEDEF:
        getNextLexem();
        switch( lexem )
        {
          case CPSTRUCT:
            parseStruct( &store.globalContainer );
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
      case CPENUM:
        parseEnum();
        break;
      case CPUNION:
        parseUnion();
        break;
      case CPTEMPLATE:
        parseTemplate();
        break;
      case CPCLASS:
        aClass = parseClass();
        if( aClass != NULL )
        {
          if( !store.hasClass( aClass->name ) )
            store.addClass( aClass );
          else
            debug( "Found new definition of class %s", aClass->name.data() );
        }
        break;
      case '{': // Skip implementation blocks
        skipBlock();
        break;
      case STATIC:
        break;
      case CPSTRUCT:
        parseStruct( &store.globalContainer );
        break;
      case CONST:
      case ID:
        parseMethodAttributes( &store.globalContainer );
        break;
      default:
        break;
    }
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
}

/*------------------------------ CClassParser::removeWithReferences()
 * removeWithReferences()
 *   Remove all items in the store with references to the file.
 *
 * Parameters:
 *   file          The file.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::removeWithReferences( const char *aFile )
{
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
 *   Parse the two file and add found classes to the list.
 *
 * Parameters:
 *   hFile          Path to the .h file.
 *   defFile        Path to the .cc/.cpp whatever file.
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
  removeWithReferences( file );

  // Parse the file.
  parseFile( f );

  return true;
}

/*-------------------------------------------- CClassParser::wipeout()
 * wipeout()
 *   Remove all parsed classes and reset the state.
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
