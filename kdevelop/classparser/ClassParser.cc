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
#include <assert.h>
#include "ClassParser.h"

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
  delete lexer;
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- CClassParser::getNextLexem()
 * getNextLexem()
 *   Fetch the next lexem from the lexer and store it in the 
 *   attribute lexem.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::getNextLexem()
{
  lexem = lexer->yylex();
}

/*------------------------------ CClassParser::parseClassHeader()
 * parseClassHeader()
 *   Parse a class header, i.e find out classname and possible
 *   parents
 *
 * Parameters:
 *   list           List of classes to add the new class to.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseClassHeader()
{
  int export;             // Export status(private/public/protected).
  CParsedParent *aParent; // A parent of this class.

  // Ok, this seems to be a class definition so allocate the
  // the new object and set some values.
  currentClass = new CParsedClass();
  currentClass->setName( lexer->YYText() );
  currentClass->setDefinedOnLine( lexer->lineno() );
  currentClass->setHFilename( currentFile );
  currentClass->setImplFilename( currentFile );
  
  // Check for inheritance
  getNextLexem();
  switch( lexem )
  {
    case '{': // It was a class. Yiieha!
      scopedepth++;
      isDefiningClass = true;
      declaredScope = -1;
      break;
    case ':': // Class with inheritance
      isDefiningClass = true;
      declaredScope = -1;
      // Add parents until we find a {
      while( lexem != '{' )
      {
        // Fetch next lexem.
        getNextLexem();

        // Find out type of inheritance.
        export = lexem;
        
        // Read the name of the parent.
        getNextLexem();
        assert( lexem == ID );
        
        // Add the parent.
        aParent = new CParsedParent();
        aParent->setName( lexer->YYText() );
        aParent->setExport( export );
        
        currentClass->addParent( aParent );
        
        // Fetch next lexem.
        getNextLexem();
        assert( lexem == '{' || lexem == ',' );
      }
      scopedepth++;
      break;
    default: // Bogus class definition..
      delete currentClass;
      currentClass = NULL;
      break;
  }

  // If this was a classdefinition we add it to the list.
  if( currentClass != NULL )
    store.addClass( currentClass );
}

/*----------------------------------------- CClassParser::parseType()
 * parseType()
 *   Parse a type declaration of some sort. Works for both function
 *   return types, attribute declarations and function parameters.
 *
 * Parameters:
 *   aStr           String to store the typedeclaration in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseType( QString *aStr )
{
  bool isConst = false;
  bool exit=false;

  if( lexem == CONST )
  {
    getNextLexem();
  }

  // Start off with saving the type.
  *aStr = lexer->YYText();
  *aStr += " ";

  while( !exit )
  {
    getNextLexem();

    switch( lexem )
    {
      case '<': // Template
        *aStr += "<";
        getNextLexem();
        while( lexem != '>' )
        {
          *aStr += lexer->YYText();
          getNextLexem();
        }
        *aStr += ">";
        break;
      case '*': // Pointer
        *aStr += "*";
        break;
      case '&': // Pointer(sort of...)
        *aStr += "&";
        break;
      default:  // No more known lexem found, let's exit.
        exit = true;
        break;
    }
  }

  if( (*aStr)[(*aStr).length() - 1] == ' ' )
    (*aStr).remove( (*aStr).length() - 1, 1 );

  // if this is a const declaration, add the word.
  if( isConst )
    *aStr = "const " + *aStr;
}

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
  QString type;
  CParsedArgument *arg = NULL;

  while( lexem != ')' )
  {
    getNextLexem();

    // Array declaration from the last argument.
    if( lexem == '[' )
    {
      type += "[";
      getNextLexem();
      while( lexem != ']' )
      {
        type += lexer->YYText();
        getNextLexem();
      }
      type += "]";

      arg->setType( type );

      // Skip the ].
      getNextLexem();
    }

    // Skip defaultvalues.
    if( lexem == '=' )
      while( lexem != ',' && lexem != ')' )
        getNextLexem();

    // Skip commas.
    if( lexem == ',' )
      getNextLexem();

    if( lexem != ')' )
    {
      parseType( &type );

      arg = new CParsedArgument();
      // If the argument has a name, we set it
      if( lexem != ')' && lexem != ',' )
        arg->setName( lexer->YYText() );

      arg->setType( type );
      method->addArgument( arg );
    }
  }

  // Check for const declaration.
  getNextLexem();

  if( lexem == CONST )
      method->setIsConst( true );

  // If this isn't a class definition skip to the start of the method.
  if( !isDefiningClass )
    while( lexem != '{' )
      getNextLexem();

  if( lexem == '{' )
    scopedepth++;
}

/*--------------------------- CClassParser::isConstructorDestructor()
 * isConstructorDestructor()
 *   Is the string the name of a constructor or destructor?
 *
 * Parameters:
 *   str          String to check.
 *
 * Returns:
 *   bool         The result.
 *-----------------------------------------------------------------*/
bool CClassParser::isConstructorDestructor( const char *str )
{
  assert( str != NULL && strlen( str ) > 0 );

  bool retVal = false;

  if( isDefiningClass )
  {
    if( str[0] == '~' && currentClass->name == (const char *)&str[1] )
      retVal = true;
    else
      retVal = ( currentClass->name == str );
  }
  else
    retVal = ( store.getClassByName( ( str[0] == '~' ? (const char *)str[1] : str ) ) != NULL );

  return retVal;
}

/*------------------------------------- CClassParser::parseModifier()
 * parseModifier()
 *   Checks for virtal/static... declarations.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseModifiers( CParsedMethod *method )
{
  while( lexem != ID && lexem != CONST )
  {
    switch( lexem )
    {
      case STATIC:
        method->setIsStatic( true );
        break;
      case VIRTUAL:
        method->setIsVirtual( true );
        break;
    }

    getNextLexem();
  }
}

/*------------------------------------ CClassParser::addDeclaration()
 * addDeclaration()
 *   Add the parsed declaration to the correct list.
 *
 * Parameters:
 *   method           The parsed method/attribute.
 *   isAttr           Is this an attribute?
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::addDeclaration( CParsedMethod *method, 
                                   bool isAttr, bool isImpl )
{
  // If we're defining a class we add it to the current class.
  if( isDefiningClass )
  {
    // Set the values.
    method->setIsInHFile( true );

    // Add it to the correct list.
    if( isAttr )
      currentClass->addAttribute( method );
    else
    {
      switch( methodType )
      {
        case QTSIGNAL:
          currentClass->addSignal( method );
          break;
        case QTSLOT:
          currentClass->addSlot( method );
          break;
        default:
          currentClass->addMethod( method );
          break;
      }
    }
  }
  else // Either an implementation or a global function/variable.
  {
    method->setIsInHFile( false );

    // If this is a implementation we need to find the correct
    // method i.e the one with the same signature.
    if( isImpl )
    {
      CParsedClass *c;
      CParsedMethod *m;

      // Fetch the class.
      c = store.getClassByName( method->declaredInClass );
      c->setImplFilename( currentFile );
      if( c != NULL )
      {
        // Try to fetch a method with the same signature.
        m = c->getMethod( *method );
        if( m != NULL )
        {
          m->setDeclaredInFile( currentFile );
          m->setDefinedOnLine( method->definedOnLine );
          m->setIsInHFile( false );
        }
      }
    }
    else
      if( isAttr )
        store.addGlobalVar( method );
      else
        store.addGlobalFunction( method );
  }
}

/*---------------------------------- CClassParser::parseDeclaration()
 * parseDeclaration()
 *   Parses some sort of declaration(methods/attributes/function/vars).
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseDeclaration()
{
  QString type = "";
  QString classPrefix;
  QString declName;
  int declLine;  // Line of the declaration.
  bool skip = false;
  bool isAttr = false;
  bool isImpl = false; // Is this a method implementation?
  CParsedMethod *method = new CParsedMethod();

  // Initialize the object.
  method->setDeclaredInFile( currentFile );
  method->setExport( declaredScope );

  // Check modifiers( virtual/static ).
  parseModifiers( method );

  // Check if this is a constructor or destructor.
  if( !isConstructorDestructor( lexer->YYText() ) )
    // Get the type declaration.
    parseType( &type );

  declLine = lexer->lineno() - 1;

  // Check for variable declarations with classreferences.
  if( isDefiningClass && lexem == CLCL )
  {
    classPrefix = type;
    getNextLexem();

    parseType( &type );
    type.stripWhiteSpace();
    type = classPrefix + "::" + type;
  }

  // Save the variable/functionname.
  declName = lexer->YYText();
  
  getNextLexem();

  switch( lexem )
  {
    case ';': // Variable 
      isAttr = true;
      break;
    case '[': // Array start
      isAttr = true;
      type += "[";
      getNextLexem();
      while( lexem != ']' )
      {
        type += lexer->YYText();
        getNextLexem();
      }
      type += "]";
      break;
    case '(': // Function parameter
      isAttr = false;
      parseFunctionArgs( method );
      break;
    case CLCL: // Method implementation
      isAttr = false;
      isImpl = true;
      
      // Set the classname.
      method->setDeclaredInClass( declName );
      
      // Set the current class using the classname.
      currentClass = store.getClassByName( declName );
      
      // Fetch lexem for the name
      getNextLexem();
      assert( lexem == ID );
      
      declName = lexer->YYText();
      
      getNextLexem();
      if( lexem == '(' )
        parseFunctionArgs( method );
      else
      {
        // Skip until end of declaration.
        while( lexem != ';' )
          getNextLexem();
            
        currentClass = NULL;

        skip = true;
      }

      break;
  }

  // Set the common values.
  method->setType( type );
  method->setName( declName );
  method->setDefinedOnLine( declLine );

  // Add the declaration to the correct list.
  if( !skip )
    addDeclaration( method, isAttr, isImpl );
  else
    delete method;
}

/*-------------------------------- CClassParser::parseSignalSlotMap()
 * parseSignalSlotMap()
 *   Parse the and create the relation between a slot a signal.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseSignalSlotMap()
{
  assert( currentClass != NULL );

  CParsedSignalSlot *aSS = new CParsedSignalSlot();
  CParsedMethod *m = new CParsedMethod();

  aSS->setSlot( m );

  // Skip the paranthesis
  getNextLexem();
  assert( lexem = '(' );

  // Fetch the signalname.
  getNextLexem();
  assert( lexem == ID );
  aSS->setSignal( lexer->YYText() );

  // Skip the comma.
  getNextLexem();
  assert( lexem == ',' );

  // Fetch the slotname.
  getNextLexem();
  assert( lexem == ID );
  m->setName( lexer->YYText() );

  // Add the mapping.
  currentClass->addSignalSlotMap( aSS );
}

/*------------------------------------- CClassParser::parseSignalTextMap()
 * parseSignalTextMap()
 *   Parse the and create the relation between a slot a text.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseSignalTextMap()
{
  assert( currentClass != NULL );

  CParsedSignalText *aST = new CParsedSignalText();
  QString txt="";

  // Skip the paranthesis
  getNextLexem();
  assert( lexem = '(' );

  // Fetch the signalname.
  getNextLexem();
  assert( lexem == ID );
  aST->setSignal( lexer->YYText() );

  // Skip the comma.
  getNextLexem();
  assert( lexem == ',' );

  while( lexem != ')' )
  {
    getNextLexem();
    txt += lexer->YYText();
  }

  aST->setText( txt );

  currentClass->addSignalTextMap( aST );
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
  bool isConst = false;

  // Ok... Here we go.
  lexem = -1;
  prevLexem = -1;

  while( lexem != 0 )
  {
    prevLexem = lexem;
    getNextLexem();

    // To parse toplevel definitions.
    switch( lexem )
    {
      case CLASS:
        getNextLexem();
        if( lexem == ID )
          parseClassHeader();
        break;
      case PUBLIC:
      case PROTECTED:
      case PRIVATE:
        if( isDefiningClass )
          declaredScope = lexem;
        break;
      case QTSIGNAL:
      case QTSLOT:
        if( isDefiningClass )
          methodType = lexem;
        break;
      case CPENUM: // Skip all enums
        while( lexem != '}' )
          getNextLexem();
        break;
      case CPTYPEDEF: // Skip all typedefs
        while( lexem != ';' )
          getNextLexem();
        break;
      case ':':
        if( isDefiningClass && 
            ( prevLexem == PUBLIC || 
              prevLexem == PRIVATE || 
              prevLexem == PROTECTED ) )
          methodType = 0;
        break;
      case SIGNALSLOT_MAP:
        if( !isDefiningClass )
          parseSignalSlotMap();
        break;
      case SIGNALTEXT_MAP:
        if( !isDefiningClass )
          parseSignalTextMap();
        break;
      case '{':
        scopedepth++;
        break;
      case '}':
        scopedepth--;

        if( scopedepth == 0 )
        {
          // We don't have any scope, so we can't have a class.
          currentClass = NULL;

          // No scope means global declarations.
          declaredScope = CPGLOBAL;

          // If we're defining a class and reach scopedepth 0
          // the class definition is over.
          if( isDefiningClass )
            isDefiningClass = false;
        }
        break;
      case CONST: // Const indicates start of a declaration
      case ID:
        // Only threat declarations within a class or at 
        // toplevel.
        if( ( isDefiningClass && declaredScope != -1 && scopedepth == 1 ) || 
            ( !isDefiningClass && scopedepth == 0 ) )
          parseDeclaration();
        break;
      default:
        isConst = false;
    }
  }
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
  lexer = NULL;
  lexem = -1;
  declaredScope = CPGLOBAL;
  currentClass = NULL;
  isDefiningClass = false;
  scopedepth = 0;
  methodType = -1;
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
