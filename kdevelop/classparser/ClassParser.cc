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
#define CP_IS_OTHER 0
#define CP_IS_ATTRIBUTE 1
#define CP_IS_METHOD 2
#define CP_IS_METHOD_IMPL 3
#define CP_IS_ATTR_IMPL 4
#define CP_IS_STRUCT 5

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

/*---------------------------------------- CClassParser::parseStruct()
 * parseStruct()
 *   Parse a structure.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
CParsedStruct *CClassParser::parseStruct()
{
  CParsedStruct *aStruct = NULL;
  //  CParsedAttribute *aAttr;

  // Save the STRUCT keyword on the stack.
  PUSH_LEXEM();

  // Set the name
  getNextLexem();

  PUSH_LEXEM();

  if( lexem == ID )
  {
    aStruct = new CParsedStruct();
    aStruct->setName( getText() );
    aStruct->setDefinedOnLine( getLineno() );
    aStruct->setDefinedInFile( currentFile );

    // Skip the '{'
    getNextLexem();

    if( lexem == '{' )
    {
      // This is really a struct so we don't need the stuff on the stack.
      emptyStack();

      skipBlock();

      // Skip to the ';'
      while( lexem != ';' )
        getNextLexem();
    }
    else if( lexem == ';' ) // Forward declaration
    {
      emptyStack();

      delete aStruct;
      aStruct = NULL;
    }
    else // Part of variable declaration. 
    {
      delete aStruct;
      aStruct = NULL;
    }
  }
  else if( lexem == '{' ) // Anonymous struct
    skipBlock();

  /*  getNextLexem();
      

  while( aAttr != NULL )
  {
    aAttr = parseVariable();
    if( aAttr != NULL )
    {
      aStruct->addMember( aAttr );
      getNextLexem();
    }
    }*/
  
  return aStruct;
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
  while( lexem != ';' )
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
  assert( lexem == ID );

  // Skip the '{'
  getNextLexem();
  assert( lexem == '{' );

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
  return lexem == ';' || lexem == ',' || lexem == ')' || lexem == '}';
}

/*------------------------------ CClassParser::fillInParsedVariable()
 * fillInParsedVariable()
 *   Initialize a attribute using the arguments on the stack.
 *
 * Parameters:
 *   aMethod        The method to initialize.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::fillInParsedVariable( CParsedAttribute *anAttr )
{
  CParsedLexem *aLexem;
  QString type;
  QString arrayDecl;
  int lexType;
  bool exit;

  // Check for an array declaration.
  if( lexemStack.top()->type == ']' )
  {
    exit = false;
    while( !exit )
    {
      aLexem = lexemStack.pop();
      arrayDecl = aLexem->text + arrayDecl;

      exit = ( aLexem->type == '[' );
      delete aLexem;
    }
  }

  // Initial checks if this variable declaration just has a type.
  lexType = lexemStack.top()->type;
  if( lexType == ID && lexemStack.count() > 1 )
  {
    aLexem = lexemStack.pop();
    anAttr->setName( aLexem->text );
    delete aLexem;
  }

  // Get the type
  fillInParsedType( type );
  type += arrayDecl;

  // Set values in the variable.
  anAttr->setType( type );
  anAttr->setDefinedInFile( currentFile );
  anAttr->setDefinedOnLine( getLineno() );
  anAttr->setExport( declaredScope );

  // Skip default values
  if( lexem == '=' )
    while( lexem != ';' )
      getNextLexem();
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
void CClassParser::fillInParsedMethod(CParsedMethod *aMethod)
{
  CParsedLexem *aLexem;
  QString type;

  // Set the method name
  aLexem = lexemStack.pop();
  aMethod->setName( aLexem->text );
  delete aLexem;

  // Set the type of the method.
  fillInParsedType( type );
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
    while( lexem != '{' )
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
void CClassParser::parseMethodImpl()
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
  aLexem = lexemStack.pop();
  name = aLexem->text;
  delete aLexem;

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
      pm->setIsInHFile( false );
      pm->setDeclaredInFile( currentFile );
      pm->setDeclaredOnLine( declLine );
    }
    else
      warning( "No method by the name %s found in class %s", 
               name.data(), className.data() );
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

  while( lexem != '(' && lexem != ';' && lexem != '=' )
  {
    if( lexem == CLCL )
      isImpl = true;
    PUSH_LEXEM();
    getNextLexem();
  }

  return ( lexem == '(' ? 
           ( isImpl ? CP_IS_METHOD_IMPL : CP_IS_METHOD ) :
           ( isImpl ? CP_IS_ATTR_IMPL : CP_IS_ATTRIBUTE ) );
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
  int export;             // Parent import status(private/public/protected).

  // Add parents until we find a {
  while( lexem != '{' )
  {
    // Fetch next lexem.
    getNextLexem();
    
    // For classes with no scope identifier at inheritance.
    if( lexem == ID )
      export = PRIVATE;
    else
    {
      // Find out type of inheritance.
      export = lexem;
          
      // Read the name of the parent.
      getNextLexem();
      assert( lexem == ID );
    }
        
    // Add the parent.
    aParent = new CParsedParent();
    aParent->setName( getText() );
    aParent->setExport( export );
    
    aClass->addParent( aParent );
    
    // Fetch next lexem.
    getNextLexem();
    assert( lexem == '{' || lexem == ',' );
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
  aClass->setHFilename( currentFile );
  aClass->setImplFilename( currentFile );
  
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
  CParsedStruct *aStruct;
  CParsedAttribute *anAttr;
  CParsedMethod *aMethod;
  bool exit = false;

  declaredScope = CPGLOBAL;

  while( !exit )
  {
    getNextLexem();
    
    switch( lexem )
    {
      case PUBLIC:
      case PROTECTED:
      case PRIVATE:
        declaredScope = lexem;
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
        aStruct = parseStruct();
        if( aStruct )
        {
          store.addGlobalStruct( aStruct );
          break;
        }
        else if( aStruct == NULL && lexemStack.isEmpty() )
          break;
      case CONST:
      case ID:
        // Ignore everything that hasn't got any scope declarator.
        if( declaredScope != CPGLOBAL )
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
            switch( checkClassDecl() )
            {
              case CP_IS_ATTRIBUTE:
                anAttr = new CParsedAttribute();
                fillInParsedVariable( anAttr );
                if( !anAttr->name.isEmpty() )
                  aClass->addAttribute( anAttr );
                else
                  delete anAttr;
                break;
              case CP_IS_METHOD:
                aMethod = new CParsedMethod();
                fillInParsedMethod( aMethod );
                if( !aMethod->name.isEmpty() )
                  aClass->addMethod( aMethod );
                else 
                  delete aMethod;
                break;
            }
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
  CParsedStruct *aStruct;
  CParsedAttribute *anAttr;
  CParsedMethod *aMethod;

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
            aStruct = parseStruct();
            if( aStruct != NULL )
              store.addGlobalStruct( aStruct );
            break;
          case CPENUM:
            parseEnum();
            break;
          case CPUNION:
            parseUnion();
            break;
        }

        // Skip the typedef name.
        while( lexem != ';' )
          getNextLexem();

        emptyStack();
        break;
      case CPENUM:
        parseEnum();
        break;
      case CPUNION:
        parseUnion();
        break;
      case CPCLASS:
        aClass = parseClass();
        if( aClass != NULL )
            store.addClass( aClass );
        break;
      case '{': // Skip implementation blocks
        skipBlock();
        break;
      case STATIC:
        break;
      case CPSTRUCT:
        aStruct = parseStruct();
        if( aStruct )
        {
          store.addGlobalStruct( aStruct );
          break;
        }
        else if( aStruct == NULL && lexemStack.isEmpty() )
          break;
      case CONST:
      case ID:
        switch( checkClassDecl() )
        {
          case CP_IS_ATTR_IMPL:
            // Empty the stack
            emptyStack();
            break;
          case CP_IS_METHOD_IMPL:
            parseMethodImpl();
            break;
          case CP_IS_ATTRIBUTE:
            anAttr = new CParsedAttribute();
            fillInParsedVariable( anAttr );
            if( !anAttr->name.isEmpty() )
              store.addGlobalVar(  anAttr );
            else
              delete anAttr;
            break;
          case CP_IS_METHOD:
            aMethod = new CParsedMethod();
            fillInParsedMethod( aMethod );
            if( !aMethod->name.isEmpty() )
              store.addGlobalFunction( aMethod );
            else
              delete aMethod;
            break;
        }
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
  declaredScope = CPGLOBAL;
  isStatic=false;
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
