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

#define PUSH_LEXEM() lexemStack.push( new JavaParsedLexem( lexem, getText() ))

enum
{
  JAVA_IS_OTHER,
  JAVA_IS_ATTRIBUTE,
  JAVA_IS_MULTI_ATTRIBUTE,
  JAVA_IS_METHOD
};

/*********************************************************************
 *                                                                   *
 *                     CREATION RELATED METHODS                      *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- JavaClassParser::JavaClassParser()
 * JavaClassParser()
 *   Constructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
JavaClassParser::JavaClassParser(ClassStore *classstore)
{
  store = classstore;
  reset();
}

/*------------------------------------- JavaClassParser::~JavaClassParser()
 * ~JavaClassParser()
 *   Destructor.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
JavaClassParser::~JavaClassParser()
{
}

/*********************************************************************
 *                                                                   *
 *                          PRIVATE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*---------------------------------------- JavaClassParser::emptyStack()
 * emptyStack()
 *   Remove all elements from the stack.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::emptyStack()
{
  while( !lexemStack.isEmpty() )
    delete lexemStack.pop();
}

/*------------------------------------ JavaClassParser::commentInRange()
 * commentInRange()
 *   Tells if the last parsed comment is in range to be a comment
 *   for the current parsed item.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
bool JavaClassParser::commentInRange( ParsedItem *aItem )
{
  REQUIRE1( "Valid item", aItem != NULL, false );

  int range;

  range = (aItem->declaredOnLine() - ( comment_end - 1 ) );

  return ( range > 0 && range <=2 );
}

/*---------------------------------- JavaClassParser::skipThrowStatement()
 * skipThrowStatement()
 *   Skip a throw() statement.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::skipThrowStatement()
{
  while( lexem != ';' && lexem != '{' && lexem != 0 )
    getNextLexem();
}



/*----------------------------------- JavaClassParser::fillInParsedType()
 * fillInParsedType()
 *   Create a type using the arguments on the stack.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::fillInParsedType(QString &type)
{
  JavaParsedLexem *aLexem;
  type = "";

  // Check for an single or multiple array declaration.
  while ( !lexemStack.isEmpty() && lexemStack.top()->type == ']' )
  {
    bool exit = false;
    while( !exit )
    {
      aLexem = lexemStack.pop();
      if (aLexem->type == ']' || aLexem->type == '[')
        type = aLexem->text + type;
      exit = ( lexemStack.isEmpty() || aLexem->type == '[' );
      delete aLexem;
    }
  }

  while( !lexemStack.isEmpty() )
  {
    aLexem = lexemStack.pop();
    type = aLexem->text + " " + type;
    delete aLexem;
  }
}

/*----------------------------------- JavaClassParser::skipBlock()
 * skipBlock()
 *   Skip all lexems inside a '{' '}' block.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::skipBlock()
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


/*********************************************************************
 *                                                                   *
 *                         VARIABLE METHODS                          *
 *                                                                   *
 ********************************************************************/

/*------------------------------------ JavaClassParser::isEndOfVarDecl()
 * isEndOfVarDecl()
 *   Tells if the current lexem is the end of a variable declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   bool           Is it an end?
 *-----------------------------------------------------------------*/
bool JavaClassParser::isEndOfVarDecl()
{
  return ( lexem == ';' || lexem == ',' || lexem == ')' ||
          // lexem == '}' ||
     lexem == 0 );
}

/*------------------------------ JavaClassParser::fillInParsedVariable()
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
void JavaClassParser::fillInParsedVariableHead( ParsedAttribute *anAttr )
{
  REQUIRE( "Valid attribute", anAttr != NULL );

  bool exit=false;
  JavaParsedLexem *aLexem;
  JavaParsedLexem *attrName;
  QString addDecl;


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

  // now if there are two ID's ... the last supposed to be the
  //   var name
  if( !lexemStack.isEmpty() && lexemStack.top()->type==ID )
  {
    attrName = lexemStack.pop();

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

    if (!lexemStack.isEmpty() &&
        (lexemStack.top()->type==ID
         || lexemStack.top()->type=='}'
         || lexemStack.top()->type==',') )
    {
      /* if the next item is also an ID
         in aLexem it has to be the var name
       */
      anAttr->setName( attrName->text );
      delete attrName;
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

/*------------------------------ JavaClassParser::fillInParsedVariable()
 * fillInParsedVariable()
 *   Initialize a attribute using the arguments on the stack.
 *
 * Parameters:
 *   anAttr        The attribute to initialize.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::fillInParsedVariable( ParsedAttribute *anAttr )
{
  REQUIRE( "Valid attribute", anAttr != NULL );

  QString type;

  // Get the everything exept the type
  fillInParsedVariableHead( anAttr );

  // Get the type
  fillInParsedType( type );

  // Set values in the variable.
  if( !type.isEmpty() ) {
    anAttr->setType( type + anAttr->type() );
  }
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

/*----------------------------- JavaClassParser::fillInMultipleVariable()
 * fillInMultipleVariable()
 *   Take what's on the stack and return as a list of variables.
 *   Works for variable declarations like int foo, bar, baz....
 *
 * Parameters:
 *   -
 * Returns:
 *   QList<CParsedAttribute>  List of variables
 *-----------------------------------------------------------------*/
void JavaClassParser::fillInMultipleVariable( ParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );

  bool exit = false;
  ParsedAttribute *anAttr;
  JavaParsedLexem *aLexem;
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

/*-------------------------------------- JavaClassParser::parseVariable()
 * parseVariable()
 *   Parse a variable declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedAttribute *JavaClassParser::parseVariable()
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
    else if( !skip && lexem == '(' )
    {
      getNextLexem();

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

/*-------------------------------- JavaClassParser::parseFunctionArgs()
 * parseFunctionArgs()
 *   Parse and add all arguments to a function.
 *
 * Parameters:
 *   method         The method to which we should add arguments.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseFunctionArgs( ParsedMethod *method )
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

      if( !anAttr->type().isEmpty() ) {
        anArg->setType( anAttr->type() );
	  }

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

/*--------------------------------- JavaClassParser::fillInParsedMethod()
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
void JavaClassParser::fillInParsedMethod(ParsedMethod *aMethod)
{
  REQUIRE( "Valid method", aMethod != NULL );

  QString name;
  JavaParsedLexem *aLexem;
  QString type;

  // Set the method name.
  aLexem = lexemStack.pop();
  name = aLexem->text;
  delete aLexem;

  // Set the operator name
  aMethod->setName( name );

  // Set the type of the method.
  fillInParsedType( type );
  if( !type.isEmpty() ) {
    aMethod->setType( type );
  }
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

  // Set end of declaration and definition.
  aMethod->setDefinitionEndsOnLine( getLineno() );
  aMethod->setDeclarationEndsOnLine( getLineno() );

  // Set the comment if in range.
  if( commentInRange( aMethod ) )
    aMethod->setComment( comment );
}


/*------------------------------------ JavaClassParser::parseMethodImpl()
 * parseMethodImpl()
 *   Parse a method implementation.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseMethodImpl(ParsedContainer *scope)
{
  ParsedClass *aClass;
  QString className;
  ParsedMethod *pm;
  int declLine = getLineno();
	
	// Get the method declaration
	pm = new ParsedMethod();
 	fillInParsedMethod( pm);

 	if ( lexem == JAVATHROWS ) {
 		skipThrowStatement();
 	}
 	
	// Skip native method declarations.
	if( lexem == ';' ) {
		getNextLexem();
	}

	// Skip implementation.
	if (lexem == '{') {
		skipBlock();
	}

	aClass = store->getClassByName( scope->path() );
	if ( aClass == NULL ) {
        warning( "No class '%s' found",
                 (const char *) scope->path() );
		return;
	}

	aClass->addMethod(pm);
	pm->setDeclaredInFile(currentFile);
	pm->setDeclaredOnLine(declStart);
	pm->setDeclarationEndsOnLine(getLineno());
	pm->setDefinedInFile(currentFile);
	pm->setDefinedOnLine(declStart);
	pm->setDefinitionEndsOnLine(getLineno());

	// Set the comment if in range.
	if (commentInRange(pm)) {
		pm->setComment(comment);
	}

}


/*********************************************************************
 *                                                                   *
 *                           CLASS METHODS                           *
 *                                                                   *
 ********************************************************************/

/*-------------------------------------- JavaClassParser::checkClassDecl()
 * checkClassDecl()
 *   Push lexems on the stack until we find something we know and
 *   return what we found.
 *
 * Parameters:
 *   -
 * Returns:
 *   int          Type of element found.
 *-----------------------------------------------------------------*/
int JavaClassParser::checkClassDecl()
{
  bool isMultiDecl = false;
  int retVal = JAVA_IS_OTHER;
  bool exit = false;
//  JavaParsedLexem *aLexem;

  while( !exit )
  {
    switch( lexem )
    {
      case ',':
        isMultiDecl = true;
        break;
    }

    PUSH_LEXEM();
    getNextLexem();

    exit = ( lexem == '(' || lexem == ';' || lexem == '=' ) ||
           ( lexem == 0 );
  }

  // If we find a '(' it's a method.
  if( lexem == '(' )
  {
      retVal = JAVA_IS_METHOD;
  }
  else if( lexem == '{' )
  {
  }
  else if( lexem != 0 ) // Attribute
  {
    if( isMultiDecl )
      retVal = JAVA_IS_MULTI_ATTRIBUTE;
    else
      retVal = JAVA_IS_ATTRIBUTE;
  }

  return retVal;
}

/*----------------------------- JavaClassParser::parseClassInheritance()
 * parseClassInheritance()
 *   Parse the inheritance clause of a class declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseClassInheritance( ParsedClass *aClass )
{
  REQUIRE( "Valid class", aClass != NULL );

  ParsedParent *aParent; // A parent of this class.
  QString cname;          // Name of inherited class.
  int exportit=-1;          // Parent import status(private/public/protected).

  if ( lexem != JAVAEXTENDS && lexem != JAVAIMPLEMENTS ) {
  	return;
  }

  // Fetch next lexem.
  getNextLexem();

  // Add parents until we find a {
  while( lexem != '{' && lexem != 0 )
  {
    if( lexem != ID )
      return;

    cname = getText();

    // Add the parent.
    if( exportit != -1 )
    {
      aParent = new ParsedParent();
      aParent->setName( cname );
      PIAccess exportattr;
      exportattr = (exportit == JAVAPUBLIC)? PIE_PUBLIC
          :  (exportit == JAVAPROTECTED)? PIE_PROTECTED
          :  PIE_PACKAGE;
      aParent->setAccess( exportattr );

      aClass->addParent( aParent );
    }

    // Fetch next lexem.
    getNextLexem();
    if (lexem == ',') {
      getNextLexem();
    }
  }
}

/*------------------------------ JavaClassParser::parseClassHeader()
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
ParsedClass *JavaClassParser::parseClassHeader()
{
  ParsedClass *aClass;
  QString scopeStr;
  JavaParsedLexem *aLexem;

  // Skip the modifier
  if( lexem != JAVACLASS && lexem != JAVAINTERFACE )
    return 0;

  // Skip to the identifier
  getNextLexem();
  PUSH_LEXEM();

  // Ok, this seems to be a class definition so allocate the
  // the new object and set some values.
  aClass = new ParsedClass();
  aClass->setDeclaredOnLine( getLineno() );
  aClass->setDefinedInFile( currentFile );
  aClass->setDeclaredInFile( currentFile );

  // The classname is at the top of the stack.
  aLexem = lexemStack.pop();

  if(aLexem == 0) {
    cerr << "ERROR in classparser: ParsedClass *JavaClassParser::parseClassHeader()\n";
    return 0;
  }

  aClass->setName( aLexem->text );
  delete aLexem;

  getNextLexem();

  return aClass;
}

/*------------------------------------ JavaClassParser::parseClassLexem()
 * parseClassLexem()
 *   Handle lexem that are specific of a class.
 *
 * Parameters:
 *   aClass         The class which declarations we are parsing.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
bool JavaClassParser::parseClassLexem( ParsedClass *aClass )
{
  REQUIRE1( "Valid class", aClass != NULL, false );

  ParsedClass *childClass;
  ParsedMethod *aMethod;
  QString childMap;
  bool exit = false;

  switch( lexem )
  {
    case JAVASTATIC:
	  isStatic = true;
      break;
    case JAVAPUBLIC:
      declaredAccess = PIE_PUBLIC;
      break;
    case JAVAPROTECTED:
      declaredAccess = PIE_PROTECTED;
      break;
    case JAVAPRIVATE:
      declaredAccess = PIE_PRIVATE;
      break;
    case JAVAINTERFACE:
	  if (declaredAccess == PIE_PACKAGE) {
        declaredAccess = PIE_PUBLIC;
      }
    case JAVACLASS:
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

        if (!inStore)
            store->addClass( childClass );

        // Check for class inheritance
        if ( lexem == JAVAEXTENDS ) {
          parseClassInheritance( childClass );
        }

        // Check for interface inheritance
        if ( lexem == JAVAIMPLEMENTS ) {
          parseClassInheritance( childClass );
        }

        parseClass( childClass );
      }
      break;
    case '{': // Skip implementation blocks
      skipBlock();
      break;
    case ID:
      parseMethodAttributes( aClass );
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

/*---------------------------------------- JavaClassParser::parseClass()
 * parseClass()
 *   Parse a class declaration.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
ParsedClass *JavaClassParser::parseClass( ParsedClass * aClass)
{
  bool exit = false;
  PIAccess oldAccess=declaredAccess;

  if( aClass != NULL )
  {
    // Set the comment if in range.
    if( commentInRange( aClass ) )
      aClass->setComment( comment );

    declaredAccess = PIE_PACKAGE;

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

  declaredAccess=oldAccess;
  return aClass;
}



/*********************************************************************
 *                                                                   *
 *                         TOP LEVEL METHODS                         *
 *                                                                   *
 ********************************************************************/

/*------------------------------------- JavaClassParser::isGenericLexem()
 * isGenericLexem()
 *   Tells if the current lexem is generic and needs no special
 *   handling depending on the current scope.
 *
 * Parameters:
 *   -
 * Returns:
 *   bool       Is the current lexem generic?
 *-----------------------------------------------------------------*/
bool JavaClassParser::isGenericLexem()
{
  return (	lexem == JAVANATIVE
  			|| lexem == JAVAABSTRACT
  			|| lexem == JAVASYNCHRONIZED
  			|| lexem == JAVATRANSIENT
  			|| lexem == JAVAVOLATILE
  			|| lexem == JAVASTRICTFP
  			|| lexem == JAVATHROW );
}


/*----------------------------- JavaClassParser::parseMethodAttributes()
 * parseMethodAttributes()
 *   Parse and add method and attribute declarations.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseMethodAttributes( ParsedContainer *aContainer )
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
    case JAVA_IS_ATTRIBUTE:
      anAttr = new ParsedAttribute();
      fillInParsedVariable( anAttr );
      if( !anAttr->name().isEmpty() )
        aContainer->addAttribute( anAttr );
      else
        delete anAttr;
      break;
    case JAVA_IS_METHOD:
      parseMethodImpl( aContainer );
      break;
    case JAVA_IS_MULTI_ATTRIBUTE:
      fillInMultipleVariable( aContainer );
      break;
    case JAVA_IS_OTHER:
      emptyStack();
      break;
  }

  declaredAccess = PIE_PACKAGE;
  return;
}

/*--------------------------------- JavaClassParser::parseGenericLexem()
 * parseGenericLexem()
 *   Take care of generic lexem.
 *
 * Parameters:
 *   aContainer  Container to store parsed items in.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseGenericLexem(  ParsedContainer *aContainer )
{
  REQUIRE( "Valid container", aContainer != NULL );

  switch( lexem )
  {
    case JAVANATIVE:
      break;
    case JAVAABSTRACT:
      break;
    case JAVAFINAL:
      break;
    case JAVASYNCHRONIZED:
      break;
    case JAVATRANSIENT:
      break;
    case JAVAVOLATILE:
      break;
    case JAVASTRICTFP:
      break;
    case JAVATHROW:
      kdDebug(9013) << "Found throw statement." << endl;
      break;
  }
}

/*--------------------------------- JavaClassParser::parseTopLevelLexem()
 * parseTopLevelLexem()
 *   Take care of lexem in a top-level context.
 *
 * Parameters:
 *   scope      The current scope.
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseTopLevelLexem( ParsedScopeContainer *scope )
{
  REQUIRE( "Valid scope", scope != NULL );

  ParsedClass *aClass;
  ParsedClass *parentClass;
  ParsedScopeContainer *parentScope;
  QString key;

  switch( lexem )
  {
    case JAVASTATIC:
      isStatic = true;
      break;
    case JAVAPUBLIC:
      declaredAccess = PIE_PUBLIC;
      break;
    case JAVAPROTECTED:
      declaredAccess = PIE_PROTECTED;
      break;
    case JAVAPRIVATE:
      declaredAccess = PIE_PRIVATE;
      break;
    case JAVAINTERFACE:
	  if (declaredAccess == PIE_PACKAGE) {
        declaredAccess = PIE_PUBLIC;
      }
    case JAVACLASS:
      aClass = parseClassHeader();

      if( aClass != NULL )
      {
      	// Build up the 'declared in scope' path for the class. But
      	// don't put the class into the scope hierarchy yet, in case
      	// it is already in the store (when 'aClass' will need to be
      	// deleted, and the existing parsed class used instead).
        QString savedClassPath = QString( aClass->declaredInScope() );
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

        kdDebug(9013) << "Storing class with path: " << aClass->path() << endl;

        // Check if class is in the global store, add it if missing
        bool inStore = store->hasClass( aClass->path() );
        if (inStore) {
  	      ParsedClass *	parsedClassRef = store->getClassByName( aClass->path() );
  	      parsedClassRef->setDeclaredOnLine( aClass->declaredOnLine() );
  	      parsedClassRef->setDeclaredInFile( aClass->declaredInFile() );
  	      delete aClass;
  	      aClass = parsedClassRef;
        }
            
        // Restore the 'declared in scope' path, so that 'aClass'
        // can be given the correct parent in the scope hierarchy
        aClass->setDeclaredInScope( savedClassPath );
        QString scopePath = scope->path();

        if( aClass->declaredInScope().isEmpty() )
        {
          aClass->setDeclaredInScope(scopePath);
          scope->addClass( aClass );
        }
        else
        {
          // Get the parent class;
          parentClass = store->getClassByName( scopePath );

          // If we didn't find a parent class, try to find a namespace.
          if( parentClass == NULL )
          {
            parentScope = store->getScopeByName( scopePath );

            if( parentScope != NULL )
              parentScope->addClass( aClass );
            // ### else globalScope->addClass( aClass );
          }
          else
            parentClass->addClass( aClass );
        }

        if (!inStore)
            store->addClass( aClass );

        // Check for inheritance
        if ( lexem == JAVAEXTENDS ) {
          parseClassInheritance( aClass );
        }

        if ( lexem == JAVAIMPLEMENTS ) {
          parseClassInheritance( aClass );
        }

        parseClass( aClass );
      } else {
      	// Not a class or interface, and can't be a method or field declaration
      	// outside the scope of a class declaration
      }
      break;
    case '{': // Skip implementation blocks
      skipBlock();
      break;
    case ID:
      parseMethodAttributes( scope );
      break;
    default:
      break;
  }
}

/*------------------------------------- JavaClassParser::parseToplevel()
 * parseToplevel()
 *   Parse and add all toplevel definitions i.e classes, global
 *   functions and variables.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseToplevel()
{
  // Ok... Here we go.
  lexem = -1;

  getNextLexem();

  // Loop until we're out of lexem.
  while( lexem != 0 )
  {
    declStart = getLineno();

    if( isGenericLexem() )
      parseGenericLexem( &store->globalContainer );
    else
      parseTopLevelLexem( &store->globalContainer );

  	kapp->processEvents(500);
    getNextLexem();
  }
}

/*--------------------------------------------- JavaClassParser::reset()
 * reset()
 *   Reset the variables in the class.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::reset()
{
  lexem = -1;
  declaredAccess = PIE_GLOBAL;
  isStatic=false;
  declStart = -1;
  comment_start = -1;
  comment_end = -1;
}

/*----------------------------------------- JavaClassParser::parseFile()
 * parseFile()
 *   Parse one file and add its' results to the internal structures.
 *
 * Parameters:
 *   file          The file to parse.
 *
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::parseFile( ifstream &file )
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

/*--------------------------------------------- JavaClassParser::parse()
 * parse()
 *   Parse a file and add found items to the store.
 *
 * Parameters:
 *   file           Path of the file.
 *
 * Returns:
 *   bool           Was the parsing successful.
 *-----------------------------------------------------------------*/
bool JavaClassParser::parse( const QString &file )
{
  ifstream f( QFile::encodeName(file) );
  currentFile = file;

  // Remove all items with references to this file.
  store->removeWithReferences( file );

  kdDebug(9013) << "Parsing Java file " << file << endl;

  // Parse the file.
  parseFile( f );

  return true;
}





/*-------------------------------------------- JavaClassParser::wipeout()
 * wipeout()
 *   Remove all parsed items and reset the state.
 *
 * Parameters:
 *   -
 * Returns:
 *   -
 *-----------------------------------------------------------------*/
void JavaClassParser::wipeout()
{
  store->wipeout();

  reset();
}
