/********************************************************************
* Name    : Definition of tha classparser.                          *
* ------------------------------------------------------------------*
* File    : ClassParser.h                                           *
* Author  : Jonas Nordin(jonas.nordin@cenacle.se)                   *
* Date    : Mon Mar 15 13:14:44 CET 1999                            *
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
* Macros:                                                           *
*                                                                   *
*                                                                   *
*                                                                   *
* ------------------------------------------------------------------*
* Types:                                                            *
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
#ifndef _CLASSPARSER_H_INCLUDED
#define _CLASSPARSER_H_INCLUDED

#include <qstring.h>
#include <qlist.h>
#include <FlexLexer.h>
#include <fstream.h>
#include "tokenizer.h"
#include "ClassStore.h"

class CClassParser
{
public: // Constructor & Destructor

  CClassParser();
  ~CClassParser();

public: // Public attributes

  /** Store for the classes. */
  CClassStore store;

public: // Public Methods

  /** Parse the two files and return the parsed classes found. */
  bool parse( const char *file = NULL );

  /** Remove all parsed classes */
  void wipeout();

  /** Output this object as text on stdout */
  void out()     { store.out(); }

private: // Private attributes

  /** Lexer used to fetch lexical patterns */
  yyFlexLexer *lexer;

  /** Current lexem */
  int lexem;

  /** Previous lexem */
  int prevLexem;

  /** The class we're currently parsing(in .h file) */
  CParsedClass *currentClass;

  /** We're currently in a class definition */
  bool isDefiningClass;

  /** Declares the scope of the attributes and metods being defined */
  int declaredScope;

  /** Type of method. -1 = NORMAL else PUBLIC/PROTECTED/PRIVATE */
  int methodType;

  /** The current scopedepth(== 0 for global declarations) */
  int scopedepth;

  /** The name of the parsed file. */
  QString currentFile;
  
private: // Private methods

  /** Get the next lexem from the lexer. */
  void getNextLexem()        { lexem = lexer->yylex(); }

  /** Fetch the current text from the lexer and return it. */
  const char *getText()      { return lexer->YYText(); }
  
  /** Fetch the current linenumber from the lexer and return it. */
  int getLineno()            { return lexer->lineno() - 1; }

  /** Parse a classdefinition i.e name and possible inheritances. */
  void parseClassHeader();
  
  /** Parse a typedeclaration. */
  void parseType( QString *aStr );
  
  /** Parse the arguments of a function. */
  void parseFunctionArgs( CParsedMethod *method );

  /** Check if virtual/static. */
  void parseModifiers( CParsedMethod *method );

  /** Add a parsed declaration. */
  void addDeclaration( CParsedMethod *method, bool isAttr, bool isImpl );

  /** Parse a global declaration(attribute/method/function/variable). */
  void parseDeclaration();

  /** Parse the relation between a signal and a slot. */
  void parseSignalSlotMap();

  /** Parse the relation between a signal and a text. */
  void parseSignalTextMap();

  /** Parse toplevel statements */
  void parseToplevel();

  /** Parse a file */
  void parseFile( ifstream &file );

  /** Reset the internal variables */
  void reset();

};

#endif
