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
  currentClass->setName( getText() );
  currentClass->setDefinedOnLine( getLineno() );
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
  bool exit = false;

  if( lexem == CONST )
  {
    isConst = true;
    getNextLexem();
  }

  *aStr = getText();
  *aStr += " ";

  // Check for modifiers.
  if( strcmp( getText(), "unsigned" ) == 0 ||
      strcmp( getText(), "short" ) == 0 )
  {
    getNextLexem();
    *aStr += getText();
    *aStr += " ";
  }

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
          *aStr += getText();
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
        type += getText();
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

      // Skip defaultvalues.
      if( lexem == '=' )
        while( lexem != ',' && lexem != ')' )
          getNextLexem();
      
      arg = new CParsedArgument();
      // If the argument has a name, we set it
      if( lexem != ')' && lexem != ',' )
        arg->setName( getText() );

      arg->setType( type );
      method->addArgument( arg );
    }
  }

  // Check for const declaration.
  getNextLexem();

  if( lexem == CONST )
    method->setIsConst( true );
  else if( isDefiningClass && lexem == '=' ) // Pure virtual
    while( lexem != ';' )
      getNextLexem();

  // If this klass call other initializers, skip to start of block.
  if( !isDefiningClass && lexem == ':' )
    while( lexem != '{' )
      getNextLexem();

  if( lexem == '{' )
    scopedepth++;
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
      case CPVIRTUAL:
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
      if( c != NULL )
      {
        c->setImplFilename( currentFile );

        // Try to fetch a method with the same signature.
        m = c->getMethod( *method );

        if( m != NULL )
        {
          m->setDeclaredInFile( currentFile );
          m->setDeclaredOnLine( method->declaredOnLine );
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

/*------------------------------------ CClassParser::parseVariableList()
 * parseVariableList()
 *   Parse a whole list of variables declared on one line.
 *
 * Parameters:
 *   type             The original type of the attributes.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseVariableList( QString &type )
{
  QString realType = type;
  QString currentT;
  CParsedMethod *aAttr;
  
  // Remove all special chars from the type.
  realType = realType.replace( "[\*&]", "" );

  getNextLexem();

  // Parse until we find an ';'.
  while( lexem != ';' )
  {
    currentT = realType;
    while( lexem != ID )
    {
      currentT.append( getText() );
      getNextLexem();
    }

    // Add the new attribute.
    aAttr = new CParsedMethod();
    aAttr->setDefinedInFile( currentFile );
    aAttr->setDeclaredInFile( currentFile );
    aAttr->setExport( declaredScope );
    aAttr->setType( currentT );
    aAttr->setName( getText() );
    aAttr->setDeclaredOnLine( getLineno() );
    aAttr->setDefinedOnLine( getLineno() );
    addDeclaration( aAttr, true, false );

    getNextLexem();

    // Skip ','. We can't just skip two chars since it could be ';'.
    if( lexem == ',' )
      getNextLexem();
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
  method->setDefinedInFile( currentFile );
  method->setDeclaredInFile( currentFile );
  method->setExport( declaredScope );

  // Check modifiers( virtual/static ).
  parseModifiers( method );

  // Parse the type
  parseType( &type );

  // Function with no returntype.
  if( lexem == '(' || ( !isDefiningClass && lexem == CLCL) )
  {
    declName = type;
    type = "";
  }

  // Set the line on which the function is defined/declared.
  declLine = getLineno();

  // Check for variable declarations with classreferences.
  if( isDefiningClass && lexem == CLCL )
  {
    classPrefix = type;
    getNextLexem();

    parseType( &type );
    type = classPrefix + "::" + type;
  }

  // Save the variable/functionname.
  if( lexem == ID )
  {
    declName = getText();
    getNextLexem();
  }

  switch( lexem )
  {
    case ',': // Variable with multiple declarations on one row.
      isAttr = true;
      parseVariableList( type );
      break;
    case '=': // Global variable with default value assignment.
      isAttr = true;
      while( lexem != ';' )
        getNextLexem();
      break;
    case ';': // Variable 
      isAttr = true;
      break;
    case ':': // Bit declared variable
      isAttr = true;
      // Skip bit specification.
      getNextLexem();
      break;
    case '[': // Array start
      isAttr = true;
      type += "[";
      getNextLexem();
      while( lexem != ']' )
      {
        type += getText();
        getNextLexem();
      }
      type += "]";
      break;
    case '(': // Function parameter
      isAttr = false;
      parseFunctionArgs( method );

      skip = ( isDefiningClass && lexem != ';' ) ||
             ( !isDefiningClass && lexem != '{' );
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
      
      declName = getText();
      
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
  method->setDeclaredOnLine( declLine );
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
  aSS->setSignal( getText() );

  // Skip the comma.
  getNextLexem();
  assert( lexem == ',' );

  // Fetch the slotname.
  getNextLexem();
  assert( lexem == ID );
  m->setName( getText() );

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
  aST->setSignal( getText() );

  // Skip the comma.
  getNextLexem();
  assert( lexem == ',' );

  while( lexem != ')' )
  {
    getNextLexem();
    txt += getText();
  }

  aST->setText( txt );

  currentClass->addSignalTextMap( aST );
}

/*---------------------------------------- CClassParser::parseStruct()
 * parseStruct()
 *   Parse a struct header.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void CClassParser::parseStruct()
{
  CParsedStruct *aStruct = new CParsedStruct();

  // Set the name
  getNextLexem();
  assert( lexem == ID );
  aStruct->setName( getText() );

  getNextLexem();
  assert( lexem == '{' );

  // TODO! Add parsing of members by adding isParsingStruct and
  // currentStruct.
  while( lexem != '}' )
    getNextLexem();

  store.addGlobalStruct( aStruct );
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
      case CPBEGINSTATUSMSG:
        while( lexem != CPENDSTATUSMSG )
          getNextLexem();
        break;
      case CPSTRUCT:
        parseStruct();
        break;
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
      case CPFRIEND:
        if( isDefiningClass )
        {
          getNextLexem();
          if( lexem == CLASS )
          {
            getNextLexem();
            currentClass->addFriend( getText() );
          }
          else if( declaredScope != -1 && scopedepth == 1 )
            parseDeclaration();
        }
        break;
      case CPVIRTUAL: // Virtual indicates start of a declaration.
      case CONST: // Const indicates start of a declaration.
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
