/***************************************************************************
                          ClassParser.h  -  description
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

#ifndef _CLASSPARSER_H_INCLUDED
#define _CLASSPARSER_H_INCLUDED

#include <qstring.h>
#include <qlist.h>
#include <qstack.h>
#include <FlexLexer.h>
#include <fstream>
#include "tokenizer.h"
#include "ClassStore.h"

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

  CClassParser();
  ~CClassParser();

private: // Private classes

  /** This class represents one lexem, which is a type and a text.
      It is used to store lexem in a stack. */
  class CParsedLexem
  {
  public: // Constructor & destructor
    
    CParsedLexem( int aType, const char *aText) { type = aType; text = aText; }
    ~CParsedLexem() {}
    
  public: // Public attributes
    
    /** The lexem text. */
    QString text;
    
    /** Type of lexem. */
    int type;
  };

public: // Public attributes

  /** Store for the classes. */
  CClassStore store;

public: // Public Methods

  /** 
   * Parse the file and store the parsed classes found. 
   *
   * @param file Name of the file to parse.
   *
   * @return If the parsing was successful.
   */
  bool parse( const char *file = NULL );

  /** Remove all parsed classes */
  void wipeout();

  /** Output this object as text on stdout */
  void out()     { store.out(); }

//  void getDependentFiles( QStrList& fileList, QStrList& dependentList)
//    { store.getDependentFiles( fileList, dependentList); }

  void removeWithReferences( const char*  aFile )
      { store.removeWithReferences( aFile ); }

private: // Private attributes

  /** flag that helps checkClassDecl() to recognize the lexem */
  bool m_isParsingClassDeclaration;

  /** Lexer used to fetch lexical patterns */
  yyFlexLexer *lexer;

  /** Current lexem */
  int lexem;

  /** The name of the parsed file. */
  QString currentFile;
  
  /** The scope in which the attributes and metods being defined. */
  PIExport declaredScope;

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
  bool commentInRange( CParsedItem *aItem );

  /** Parse all structure declarations.
   * @param aStruct The struct that holds the declarations.
   */
  void parseStructDeclarations( CParsedStruct *aStruct);

  /** Parse a structure using header declaration from stack.
   * @param aContainer Container to store the parsed struct in.
   */
  void fillInParsedStruct( CParsedContainer *aContainer );

  /** Parse a structure. 
   * @param aContainer Container to store the parsed struct in.
   */
  void parseStruct( CParsedContainer *aContainer );

  /** Parse an enumeration. */
  void parseEnum();

  /** Parse an union. */
  void parseUnion();

  /** Parse a namespace. */
  void parseNamespace( CParsedScopeContainer *scope );

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
  void fillInParsedVariableHead( CParsedAttribute *anAttr );

  /** Initialize a attribute using the arguments on the stack. */
  void fillInParsedVariable( CParsedAttribute *anAttr );

  /** Take what's on the stack and return as a list of variables. 
   *   Works for variable declarations like int foo, bar, baz.... */
  void fillInMultipleVariable( CParsedContainer *aContainer );

  /** Parse a variable declaration. */
  CParsedAttribute *parseVariable();

  /** Parse and add all arguments to a function. */
  void parseFunctionArgs( CParsedMethod *method );

  /** Initialize a method using the arguments on the stack. */
  void fillInParsedMethod(CParsedMethod *aMethod, bool isOperator=false);

  /** Parse a method declaration. */
  CParsedMethod *parseMethodDeclaration();

  /** Parse a method implementation. */
  void parseMethodImpl(bool isOperator,CParsedContainer *scope);
//  void parseMethodImpl(bool isOperator);

  /** Push lexems on the stack until we find something we know and 
   *   return what we found. */
  int checkClassDecl();

  /** Parse the inheritance clause of a class declaration. */
  void parseClassInheritance( CParsedClass *aClass );

  /** Parse a class header, i.e find out classname and possible parents. */
  CParsedClass *parseClassHeader();

  /** Handle lexem that are specific of a class. 
   * @param aClass Class to store items in.
   * @return Tells if we should stop parsing.
   */
  bool parseClassLexem( CParsedClass *aClass );
  
  /** Parse a class declaration. 
   * @return The parsed class or NULL if it was no class.
   */
  CParsedClass *parseClass(CParsedClass * aClass);

  /** Tells if the current lexem is generic and needs no special
   * handling depending on the current scope.
   * @return Is this a generic lexem?
   */
  bool isGenericLexem();

  /** Parse all methods and attributes and add them to the container. */
  void parseMethodAttributes( CParsedContainer *aContainer );

  /** Take care of generic lexem.
   * @param aContainer Container to store parsed items in.
   */
  void parseGenericLexem( CParsedContainer *aContainer );

  /** Take care of lexem in a top-level context. */
  void parseTopLevelLexem( CParsedScopeContainer *aContainer );

  /** Parse toplevel statements */
  void parseToplevel();

  /** Parse a file.
   * @param file Stream to parse from.
   */
  void parseFile( std::ifstream &file );

  /** Reset the internal variables */
  void reset();
};

#endif
