/***************************************************************************
                          classparser.h  -  description
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

#ifndef _CLASSPARSER_H_
#define _CLASSPARSER_H_

#include <qstring.h>
#include <qlist.h>
#include <qstack.h>
#include <FlexLexer.h>
#include <fstream.h>
#include "tokenizer.h"

#include "classstore.h"


/** This class handles the parsing and generation of all C++ and C
 * constructions. The classparser has a store in which all elements that
 * have been parsed is stored. The external interface to this class is
 * very simple. Just call parse() and you're off.
 *
 * @author Jonas Nordin
 * @short Handles parsing of C++ and C expressions.
 */
class CClassParser
{
public: // Constructor & Destructor

  CClassParser(ClassStore *classstore);
  ~CClassParser();

private: // Private classes

  /** This class represents one lexem, which is a type and a text.
      It is used to store lexem in a stack. */
  class CParsedLexem
  {
  public: // Constructor & destructor
    
    CParsedLexem( int aType, const QString &aText) { type = aType; text = aText; }
    ~CParsedLexem() {}
    
  public: // Public attributes
    
    /** The lexem text. */
    QString text;
    
    /** Type of lexem. */
    int type;
  };

public: // Public Methods

  /** 
   * Parse the file and store the parsed classes found. 
   *
   * @param file Name of the file to parse.
   *
   * @return If the parsing was successful.
   */
  bool parse( const QString &file );

  /** Remove all parsed classes */
  void wipeout();

  /** Output this object as text on stdout */
  void out()     { store->out(); }
  /** get a list of all classes */
  ClassStore * getClassStore();

//  void getDependentFiles( QStrList& fileList, QStrList& dependentList)
//    { store.getDependentFiles( fileList, dependentList); }

private: // Private attributes

  /** Store for the classes. */
  ClassStore *store;

  /** Lexer used to fetch lexical patterns */
  yyFlexLexer *lexer;

  /** Current lexem */
  int lexem;

  /** The name of the parsed file. */
  QString currentFile;
  
  /** The scope in which the attributes and metods being defined. */
  PIAccess declaredAccess;

  /** Stack of lexems currently parsed. */
  QStack<CParsedLexem> lexemStack;
 
  /** Type of method. 0 = NORMAL else QTSIGNAL/QTSLOT. */
  int methodType;

  /** Tells if this is an static declaration. */
  bool isStatic;

  /** Start of a declaration that has been pushed on the stack. */
  int declStart;

private: // Private methods

  /** Get the next lexem from the lexer. */
  void getNextLexem()        { lexem = lexer->yylex(); }

  /** Fetch the current text from the lexer and return it. 
   * @return The text of the current lexem.
   */
  const char *getText()      { return lexer->YYText(); }
  
  /** Fetch the current linenumber from the lexer and return it. 
   * @return The current linenumber.
   */
  int getLineno()            { return lexer->lineno() - 1;  }
  //int getLineno()            { debug("%d",lexer->lineno());return lexer->lineno() - 1;  }

  /** Remove all elements from the stack. */
  void emptyStack();

  /** Tells if the last parsed comment is in range to be a comment
   * for the current parsed item. */
  bool commentInRange( ParsedItem *aItem );

  /** Parse all structure declarations.
   * @param aStruct The struct that holds the declarations.
   */
  void parseStructDeclarations( ParsedStruct *aStruct);

  /** Parse a structure using header declaration from stack.
   * @param aContainer Container to store the parsed struct in.
   */
  void fillInParsedStruct( ParsedContainer *aContainer );

  /** Parse a structure. 
   * @param aContainer Container to store the parsed struct in.
   */
  void parseStruct( ParsedContainer *aContainer );

  /** Parse an enumeration. */
  void parseEnum();
  
  /** try to parse an enum right */
  void parseEnum( ParsedContainer* aContainer );

  /** Parse an union. */
  void parseUnion();

  /** Parse a namespace. */
  void parseNamespace( ParsedScopeContainer *scope );

  /** Skip a throw() statement. */
  void skipThrowStatement();

  /** Skip all lexems between '{' and '}'. */
  void skipBlock();

  /** Skip a template declaration. */
  void parseTemplate();

  /** Create a type using the arguments on the stack. */
  void fillInParsedType(QString &type);

  /** Tells if the current lexem is the end of a variable declaration. */
  bool isEndOfVarDecl();

  /** Initialize a attribute, except the type, using the arguments on
   *   the stack. */
  void fillInParsedVariableHead( ParsedAttribute *anAttr );

  /** Initialize a attribute using the arguments on the stack. */
  void fillInParsedVariable( ParsedAttribute *anAttr );

  /** Take what's on the stack and return as a list of variables. 
   *   Works for variable declarations like int foo, bar, baz.... */
  void fillInMultipleVariable( ParsedContainer *aContainer );

  /** Parse a variable declaration. */
  ParsedAttribute *parseVariable();

  /** Parse and add all arguments to a function. */
  void parseFunctionArgs( ParsedMethod *method );

  /** Initialize a method using the arguments on the stack. */
  void fillInParsedMethod(ParsedMethod *aMethod, bool isOperator=false);

  /** Parse a method declaration. */
  ParsedMethod *parseMethodDeclaration();

  /** Parse a method implementation. */
  void parseMethodImpl(bool isOperator,ParsedContainer *scope);
//  void parseMethodImpl(bool isOperator);

   /** Initialize an Objective-C method */
   void fillInParsedObjcMethod( ParsedMethod *aMethod );

  /** Push lexems on the stack until we find something we know and
   *   return what we found. */
  int checkClassDecl();

  /** Parse the inheritance clause of a class declaration. */
  void parseClassInheritance( ParsedClass *aClass );

  /** Parse a class header, i.e find out classname and possible parents. */
  ParsedClass *parseClassHeader();

  /** Handle lexem that are specific of a class. 
   * @param aClass Class to store items in.
   * @return Tells if we should stop parsing.
   */
  bool parseClassLexem( ParsedClass *aClass );
  
  /** Parse a class declaration. 
   * @return The parsed class or NULL if it was no class.
   */
  ParsedClass *parseClass(ParsedClass * aClass);

   /** Parse an Objective-C category.
    * @return The (fake!) parsed class - ie <classname>(<category name>).
    */
   ParsedClass *parseObjcCategory( QString &aClassName );

   /** Parse an Objective-C class implementation.
    * @return The parsed class or NULL if it was no class.
    */
   ParsedClass *parseObjcImplementation();

   /** Handle lexem that are specific of an Objective-C class.
    * @param aClass Class to store items in.
    * @return Tells if we should stop parsing.
    */
   bool parseObjcClassLexem( ParsedClass *aClass );

   /** Parse an Objective-C class header, i.e find out classname and possible parent. */
   ParsedClass *parseObjcClassHeader();

   /** Parse an Objective-C class declaration.
    * @return The parsed class or NULL if it was no class.
    */
   ParsedClass *parseObjcClass();

  /** Tells if the current lexem is generic and needs no special
   * handling depending on the current scope.
   * @return Is this a generic lexem?
   */
  bool isGenericLexem();

  /** Parse all methods and attributes and add them to the container. */
  void parseMethodAttributes( ParsedContainer *aContainer );

  /** Take care of generic lexem.
   * @param aContainer Container to store parsed items in.
   */
  void parseGenericLexem( ParsedContainer *aContainer );

  /** Take care of lexem in a top-level context. */
  void parseTopLevelLexem( ParsedScopeContainer *aContainer );

  /** Parse toplevel statements */
  void parseToplevel();

  /** Parse a file.
   * @param file Stream to parse from.
   */
  void parseFile( ifstream &file );

  /** Reset the internal variables */
  void reset();
};

#endif
