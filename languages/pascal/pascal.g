//
//
// Pascal Grammar
//
// Adapted from,
// Pascal User Manual And Report (Second Edition-1978)
// Kathleen Jensen - Niklaus Wirth
//
// By
//
// Hakki Dogusan dogusanh@tr-net.net.tr
//
// Then significantly enhanced by Piet Schoutteten
// with some guidance by Terence Parr.  Piet added tree
// construction, and some tree walkers.
//
//
// Turbo Pascal, Free Pascal and Delphi pascal extensions
// by Alexander Dymo <cloudtemple@mksat.net>
//
//
// Adapted from,
// Free Pascal: Reference Guide 1.9 April 2002
//
//
// KDevelop 3.0 support by Alexander Dymo <cloudtemple@mksat.net>
//

header "pre_include_hpp" {
        #include "problemreporter.h"
        #include "PascalAST.hpp"

        #include <qlistview.h>
        #include <kdebug.h>

        #define SET_POSITION(ast,t)\
        { \
                RefPascalAST(ast)->setLine( t->getLine() );\
                RefPascalAST(ast)->setColumn( t->getColumn() ); \
        }
}

options {
        language="Cpp";
}

class PascalParser extends Parser;
options {
  k = 2;                           // two token lookahead
  exportVocab=Pascal;              // Call its vocabulary "Pascal"
  codeGenMakeSwitchThreshold = 2;  // Some optimizations
  codeGenBitsetTestThreshold = 3;
  defaultErrorHandler = true;     // Generate parser error handlers
  buildAST = true;
  ASTLabelType = "RefPascalAST";
}

/* Define imaginary tokens used to organize tree
 *
 * One of the principles here is that any time you have a list of
 * stuff, you usually want to treat it like one thing (a list) a some
 * point in the grammar.  You want trees to have a fixed number of children
 * as much as possible.  For example, the definition of a procedure should
 * be something like #(PROCEDURE ID #(ARGDECLS ARG1 ARG2...)) not
 * #(PROCEDURE ID ARG1 ARG2 ... ) since this is harder to parse and
 * harder to manipulate.  Same is true for statement lists (BLOCK) etc...
 */
tokens {
	BLOCK; 		// list of statements
	IDLIST;		// list of identifiers; e.g., #(PROGRAM #(IDLIST ID ID...))
	ELIST;		// expression list for proc args etc...
	FUNC_CALL;
	PROC_CALL;
	SCALARTYPE; // IDLIST that is really a scalar type like (Mon,Tue,Wed)
	TYPELIST;	// list of types such as for array declarations
	VARIANT_TAG;// for CASEs in a RECORD
	VARIANT_TAG_NO_ID;// for CASEs in a RECORD (no id, just a type)
	VARIANT_CASE;// a case of the variant
	CONSTLIST;	// List of constants
	FIELDLIST;	// list of fields in a record
	ARGDECLS;	// overall group of declarations of args for proc/func.
	VARDECL;	// declaration of a variable
	ARGDECL;	// declaration of a parameter
	ARGLIST;	// list of actual arguments (expressions)
	TYPEDECL;	// declaration of a type
	FIELD;		// the root a RECORD field
}

// Define some methods and variables to use in the generated parser.
{
private:
        unsigned int m_numberOfErrors;
        ProblemReporter* m_problemReporter;

public:
        void resetErrors()                              { m_numberOfErrors = 0; }
        unsigned int numberOfErrors() const             { return m_numberOfErrors; }
        void setProblemReporter( ProblemReporter* r )   { m_problemReporter = r; }

        void reportError( const ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex ){
                m_problemReporter->reportError( ex.getMessage().c_str(),
                                                ex.getFilename().c_str(),
                                                ex.getLine(),
                                                ex.getColumn() );
                ++m_numberOfErrors;
        }

        void reportError( const ANTLR_USE_NAMESPACE(std)string& errorMessage ){
                m_problemReporter->reportError( errorMessage.c_str(),
                                                getFilename().c_str(),
                                                LT(1)->getLine(),
                                                LT(1)->getColumn() );
                ++m_numberOfErrors;
        }

        void reportMessage( const ANTLR_USE_NAMESPACE(std)string& message ){
                m_problemReporter->reportMessage( message.c_str(),
                                                getFilename().c_str(),
                                                LT(1)->getLine(),
                                                LT(1)->getColumn() );
        }
}

compilationUnit
    : program
    | library
    | unit
    ;

program
    : programHeading
      (usesClause)?
      block
      DOT!
    ;

library
    : LIBRARY^ identifier SEMI!
      (usesClause)?
      libraryBlock
      exportsClause
      END! DOT!
    ;

libraryBlock
    : declarationPart (statementPart)?
    ;

exportsClause
    : EXPORTS^
      exportsList
    ;

exportsList
    : exportsEntry ( COMMA! exportsEntry )* (SEMI!)?
    ;

exportsEntry
    : identifier ("index" integerConstant)? ("name" stringConstant)?
    ;

usesClause
    : USES^ identifierList SEMI!
    ;

unit
    : UNIT^ identifier SEMI!
      interfacePart
      implementationPart
      ( (initializationPart (finalizationPart)?) | realizationPart )?
      END!
    ;

interfacePart
    : INTERFACE^
      (usesClause)?
    ( constantDeclarationPart
    | typeDeclarationPart
    | procedureHeadersPart
    )*
    ;

implementationPart
    : IMPLEMENTATION^
      (usesClause)?
      declarationPart
    ;

realizationPart
    : BEGIN^ statement ( SEMI! statement )*
    ;

programHeading
    : PROGRAM^ identifier (LPAREN! identifierList RPAREN!)? SEMI!
    ;

initializationPart
    : INITIALIZATION^
      statement ( SEMI! statement )*
    ;

finalizationPart
    : FINALIZATION^
      statement ( SEMI! statement )*
    ;

block
    : declarationPart statementPart
    ;

declarationPart
    : (labelDeclarationPart
    | constantDeclarationPart
    | resourcestringDeclarationPart
    | typeDeclarationPart
    | variableDeclarationPart
    | procedureAndFunctionDeclarationPart
      )*
    ;

labelDeclarationPart
    : LABEL^ label ( COMMA! label )* SEMI!
    ;

constantDeclarationPart
    : CONST^ ( constantDeclaration | typedConstantDeclaration )+
    ;

resourcestringDeclarationPart
    : RESOURCESTRING^ ( stringConstantDeclaration )*
    ;

stringConstantDeclaration
    : identifier EQUAL! string
    ;

typeDeclarationPart
    : TYPE^ ( typeDeclaration )+
    ;

variableDeclarationPart
    : VAR^ variableDeclaration ( SEMI! variableDeclaration )* SEMI!
    ;

variableDeclaration
    : identifierList c:COLON^ {#c->setType(VARDECL);} type
    ;

procedureAndFunctionDeclarationPart
    : procedureAndFunctionDeclaration
    ;

procedureAndFunctionDeclaration
    : procedureDeclaration
    | functionDeclaration
    | constructorDeclaration
    | destructorDeclaration
    ;

statementPart
    : compoundStatement
    ;

procedureDeclaration
    : procedureHeader subroutineBlock SEMI!
    ;

procedureHeadersPart
    : procedureHeader | functionHeader // ) SEMI! ( callModifiers SEMI! )
    ;

procedureHeader
    : PROCEDURE^ ( identifier | qualifiedMethodIdentifier )
      (formalParameterList)? SEMI! (modifiers SEMI!)*
    ;

qualifiedMethodIdentifier
    : identifier COLON! COLON! identifier
    ;

subroutineBlock
    : block
    | externalDirective
//    | asmBlock
    | FORWARD!
    ;

functionDeclaration
    : functionHeader subroutineBlock SEMI!
    ;

functionHeader
    : (FUNCTION^ identifier COLON! type SEMI!)=> FUNCTION^ identifier COLON! type SEMI! (modifiers SEMI!)*
    | (FUNCTION^ identifier COLON! COLON! identifier COLON! type SEMI!)=> FUNCTION^ qualifiedMethodIdentifier COLON! type SEMI! (modifiers SEMI!)*
    | (FUNCTION^ identifier COLON! COLON! identifier LPAREN!)=> FUNCTION^ qualifiedMethodIdentifier formalParameterList COLON! type SEMI! (modifiers SEMI!)*
    | FUNCTION^ identifier formalParameterList COLON! type SEMI! (modifiers SEMI!)*
    ;

functionHeaderEnding
    : (COLON! type SEMI!)=> COLON! type SEMI! (modifiers SEMI!)*
    | formalParameterList COLON! type SEMI! (modifiers SEMI!)*
    ;

formalParameterList
    : LPAREN! parameterDeclaration ( SEMI! parameterDeclaration )* RPAREN!
    ;

parameterDeclaration
    : valueParameter
    | variableParameter
    | constantParameter
    ;

valueParameter
    : (identifierList COLON! ARRAY! OF!)=> identifierList COLON! ARRAY! OF! type
    | identifierList COLON! type
    ;

variableParameter
    : VAR identifierList ( untypedParameterPart )?
    ;

untypedParameterPart
    : (COLON! ARRAY! OF! type)=> COLON! ARRAY! OF! type
    | COLON! type
    ;

constantParameter
    : CONST identifierList ( untypedParameterPart )?
    ;

externalDirective
    : EXTERNAL^ ( stringConstant ( ("name" stringConstant) | ("index" integerConstant) )? )?
    ;

/*asmBlock
    : ASSEMBLER^ SEMI! declarationPart asmStatement
    ;
*/
modifiers
    : PUBLIC! | (ALIAS! stringConstant) | INTERRUPT! | callModifiers | EXPORT!
    ;

callModifiers
    : REGISTER! | PASCAL! | CDECL! | STDCALL! | POPSTACK! | SAVEREGISTERS! | INLINE! | SAFECALL! | NEAR! | FAR!
    ;

constantDeclaration
//    : ( identifier EQUAL! expression SEMI! ) ( identifier EQUAL! expression SEMI! )*
    :  identifier EQUAL! expression SEMI!
    ;

typedConstantDeclaration
//    : ( identifier COLON! type EQUAL! typedConstant SEMI! )+
    : identifier COLON! type EQUAL! typedConstant SEMI!
    ;

//FIXME: is there a better way to handle this instead of simply forcing the rules
typedConstant
    : (constant)=> constant
//    | addressConstant
    | (LPAREN! identifier COLON!)=> recordConstant
    | (arrayConstant)=> arrayConstant
    | proceduralConstant
    ;

arrayConstant
    : LPAREN! ( constant | arrayConstant ) ( COMMA! ( constant | arrayConstant ) )* RPAREN!
    ;

recordConstant
    : LPAREN! ( identifier COLON! constant ) ( SEMI! ( identifier COLON! constant ) )* RPAREN!
    ;

addressConstant
    : NUM_INT
    ;

proceduralConstant
    : expression
    ;

typeDeclaration
    : identifier EQUAL! type SEMI!
    ;

type
//    : simpleType | subrangeType | enumeratedType | stringType | structuredType | pointerType | proceduralType | typeIdentifier
    : simpleType | subrangeTypeOrTypeIdentifier | enumeratedType | stringType | structuredType | pointerType | proceduralType
    ;

simpleType
    : ordinalType | realType
    ;

ordinalType
    : INTEGER! | SHORTINT! | SMALLINT! | LONGINT! | INT64! | BYTE! | WORD! | CARDINAL! | QWORD!
    | BOOLEAN! | BYTEBOOL! | LONGBOOL! | CHAR!
    ;

subrangeTypeOrTypeIdentifier
    : constant (DOTDOT! constant)?
    ;

typeIdentifier
    : identifier
    ;

subrangeType
    : constant DOTDOT! constant
    ;

enumeratedType
    : (LPAREN! identifier ASSIGN!)=> LPAREN! assignedEnumList RPAREN!
    | LPAREN! identifierList RPAREN!
    ;

assignedEnumList
    : (identifier ASSIGN! expression) ( COMMA! (identifier ASSIGN! expression) )*
    ;

realType
    : REAL! | SINGLE! | DOUBLE! | EXTENDED! | COMP!
    ;

stringType
    : STRING^ ( LBRACK! unsignedInteger RBRACK! )?
    ;

structuredType
    : (PACKED!)? ( arrayType | recordType | objectType | classType | setType | fileType )
    ;

arrayType
    : ARRAY^ LBRACK! arrayIndexType ( COMMA! arrayIndexType )* RBRACK! OF! type
    ;

arrayIndexType
    : ordinalType
    | (expression DOTDOT!)=> arraySubrangeType
    | enumeratedType
    ;

arraySubrangeType
    : expression DOTDOT! expression
    ;

recordType
    : RECORD^ (fieldList)* END!
    ;

fieldList
    : fixedField | variantPart
    ;

fixedField
    : identifierList COLON! type SEMI!
    ;

variantPart
    : CASE^ (identifier COLON!)? identifier OF! variant ( SEMI! variant )*
    ;

variant
    : (constant COMMA!)+ COLON! LPAREN! (fieldList)* RPAREN!
    ;

setType
    : SET^ OF! ordinalType
    ;

fileType
    : FILE^ OF! type
    ;

pointerType
    : POINTER^ typeIdentifier
    ;

proceduralType
    : (proceduralTypePart1 SEMI!)=> proceduralTypePart1 SEMI! callModifiers
    | proceduralTypePart1
    ;

proceduralTypePart1
    : ( functionHeader | procedureHeader ) (OF! OBJECT!)?
    ;

objectType
    : OBJECT^ (heritage)? (componentList | objectVisibilitySpecifier ) END!
    ;

heritage
    : LPAREN! identifier RPAREN!
    ;

componentList
    : ( (fieldDefinition)+ )? ( (methodDefinition)+ )?
    ;

fieldDefinition
    : identifierList COLON! type SEMI!
    ;

methodDefinition
    : ( functionHeader | procedureHeader | constructorHeader | destructorHeader ) SEMI! methodDirectives
    ;

methodDirectives
    : ( VIRTUAL! SEMI! (ABSTRACT! SEMI!)? )? (callModifiers SEMI!)?
    ;

objectVisibilitySpecifier
    : PRIVATE! | PROTECTED! | PUBLIC!
    ;

constructorDeclaration
    : constructorHeader SEMI! subroutineBlock
    ;

destructorDeclaration
    : destructorHeader SEMI! subroutineBlock
    ;

constructorHeader
    : CONSTRUCTOR^ ( identifier | qualifiedMethodIdentifier ) formalParameterList
    ;

destructorHeader
    : DESTRUCTOR^ ( identifier | qualifiedMethodIdentifier ) formalParameterList
    ;

classType
    : CLASS^ (heritage)? (classComponentList | classVisibilitySpecifier ) END!
    ;

classComponentList
    : ( (fieldDefinition)+ )? ( ( (classMethodDefinition | propertyDefinition) )+ )?
    ;

classMethodDefinition
    : ( ( (CLASS!)? (functionHeader | procedureHeader) ) | constructorHeader | destructorHeader ) SEMI! classMethodDirectives
    ;

classMethodDirectives
    : ( directiveVariants SEMI! )? (callModifiers SEMI!)?
    ;

directiveVariants
    : ( VIRTUAL! (ABSTRACT! SEMI!)? )
    | OVERRIDE!
    | (MESSAGE! (integerConstant | stringConstant))
    ;

classVisibilitySpecifier
    : PRIVATE! | PROTECTED! | PUBLIC! | PUBLISHED!
    ;

propertyDefinition
    : PROPERTY^ identifier (propertyInterface)? propertySpecifiers
    ;

propertyInterface
    : (propertyParameterList)? COLON! typeIdentifier ("index" integerConstant)?
    ;

propertyParameterList
    : LBRACK! parameterDeclaration (SEMI! parameterDeclaration)* RBRACK!
    ;

propertySpecifiers
    : (readSpecifier)? (writeSpecifier)? (defaultSpecifier)?
    ;

readSpecifier
    : "read" fieldOrMethod
    ;

writeSpecifier
    : "write" fieldOrMethod
    ;

defaultSpecifier
    : ( DEFAULT (constant)? )
    | "nodefault"
    ;

fieldOrMethod
    : identifier
    ;

expression
    : simpleExpression ( expressionSign simpleExpression )?
    ;

expressionSign
//    : STAR! | LE! | GE! | LTH! | GT! | NOT_EQUAL! | IN! | IS!
    : LE! | GE! | LTH! | GT! | NOT_EQUAL! | IN! | IS! | EQUAL!
    ;

simpleExpression
    : term ( ( PLUS! | MINUS! | OR! | XOR! ) term )*
    ;

term
    : factor ( (STAR! | SLASH! | DIV! | MOD! | AND! | SHL! | SHR!) factor )*
    ;

//TODO: distinguish between identifiers, typecasts and function calls -> semantic predicate
factor
    : ( LPAREN! expression RPAREN! )
//    | (qualifiedMethodIdentifier2 LBRACK!)=> qualifiedMethodIdentifier2 LBRACK! arrayIndexType ( COMMA! arrayIndexType )* RBRACK!
    | identifierOrValueTypecastOrFunctionCall
//    | identifier
//    | functionCall
    | unsignedConstant
    | ( NOT! factor )
    | ( (PLUS! | MINUS!) factor )
    | setConstructor
//    | valueTypecast
    | addressFactor
    | TRUE
    | FALSE
    | identifier LBRACK! expression ( COMMA! expression )* RBRACK!
    ;

//FIXME: is this correct?
identifierOrValueTypecastOrFunctionCall
    : (identifier LPAREN! expression COMMA!)=> identifier LPAREN! expressions RPAREN!
    | (identifier LPAREN! expression RPAREN!)=> identifier LPAREN! expression RPAREN!
//    | (qualifiedMethodIdentifier2 LPAREN! expression COMMA!)=> qualifiedMethodIdentifier2 LPAREN! expressions RPAREN!
//    | (identifier DOT)=> qualifiedMethodIdentifier2
    | identifier
    ;

/*qualifiedMethodIdentifier2
    : identifier DOT identifier
    ;
*/
//( functionIdentifier | methodDesignator | qualifiedMethodDesignator | variableReference )
functionCall
    : identifier (actualParameterList)?
    ;

actualParameterList
    : LPAREN! ( expressions )? RPAREN!
    ;

expressions
    : expression ( COMMA! expression )*
    ;

setConstructor
    : LBRACK! ( setGroup ( COMMA! setGroup )* )? RBRACK!
    ;

setGroup
    : expression ( DOT! DOT! expression )?
    ;

valueTypecast
    : typeIdentifier LPAREN! expression RPAREN!
    ;

//( variableReference | procedureIdentifier | functionIdentifier | qualifiedMethodIdentifier )
addressFactor
    : AT! identifier
    ;

statement
//    : (label COLON!)? (simpleStatement | structuredStatement | asmStatement)
    : (label COLON!)? (simpleStatement | structuredStatement)
    ;

simpleStatement
    : assignmentStatement | procedureStatement | gotoStatement | raiseStatement
    ;

assignmentStatement
    : identifierOrArrayIdentifier assignmentOperator expression
    ;

identifierOrArrayIdentifier
    : identifier
//    | (qualifiedMethodIdentifier LBRACK!)=> qualifiedMethodIdentifier LBRACK! arrayIndexType ( COMMA! arrayIndexType )* RBRACK!
//    | qualifiedMethodIdentifier
    | identifier LBRACK! expression ( COMMA! expression )* RBRACK!
    ;

assignmentOperator
    : ASSIGN! | PLUSEQ | MINUSEQ | STAREQ | SLASHQE
    ;

procedureStatement
    : identifier (actualParameterList)?
    ;

gotoStatement
    : GOTO! label
    ;

structuredStatement
    : compoundStatement | repetitiveStatement | conditionalStatement | exceptionStatement | withStatement
    ;

conditionalStatement
    : ifStatement | caseStatement
    ;

repetitiveStatement
    : forStatement | repeatStatement | whileStatement
    ;

compoundStatement
    : BEGIN! END!
    | BEGIN! (SEMI!)+ END!
    | BEGIN! statement (SEMI! (statement)?)* END!
    ;

ifStatement
    : IF^ expression THEN! statement
      (
        // CONFLICT: the old "dangling-else" problem...
        //           ANTLR generates proper code matching
        //           as soon as possible.  Hush warning.
        options {
            generateAmbigWarnings=false;
        }
        : ELSE! statement
      )?
    ;

caseStatement
    : CASE^ expression OF!
        caseListElement ( SEMI! caseListElement )*
      ( SEMI! ELSE! statement ( SEMI! statement )* )?
      END!
    ;

caseListElement
    : constList COLON^ statement
    ;

constList
    : constant ( COMMA! constant )*
      {#constList = #([CONSTLIST],#constList);}
    ;

whileStatement
    : WHILE^ expression DO! statement
    ;

repeatStatement
    : REPEAT^ statement ( SEMI! (statement)? )* UNTIL! expression
    ;

forStatement
    : FOR^ identifier ASSIGN! forList DO! statement
    ;

forList
    : initialValue (TO^ | DOWNTO^) finalValue
    ;

initialValue
    : expression
    ;

finalValue
    : expression
    ;

withStatement
    : WITH^ recordVariableList DO! statement
    ;

recordVariableList
    : variable ( COMMA! variable )*
    ;

/** A variable is an id with a suffix and can look like:
 *  id
 *  id[expr,...]
 *  id.id
 *  id.id[expr,...]
 *  id^
 *  id^.id
 *  id^.id[expr,...]
 *  ...
 *
 *  LL has a really hard time with this construct as it's naturally
 *  left-recursive.  We have to turn into a simple loop rather than
 *  recursive loop, hence, the suffixes.  I keep in the same rule
 *  for easy tree construction.
 */
variable
    : ( AT^ identifier // AT is root of identifier; then other op becomes root
      | identifier
      )
      ( LBRACK^ expression ( COMMA! expression)* RBRACK!
      | LBRACK2^ expression ( COMMA! expression)* RBRACK2!
      | DOT^ identifier
      | POINTER^
      )*
    ;

/*asmStatement
    : ASM^ assemblerCode END! (registerList)?
    ;

registerList
    : LBRACK! stringConstant ( COMMA! stringConstant )*
    ;

assemblerCode
    : (.)*
    ;
*/
operatorDefinition
    : OPERATOR^ ( assignmentOperatorDefinition | arithmeticOperatorDefinition | comparisonOperatorDefinition )
      identifier COLON! type SEMI! subroutineBlock
    ;

assignmentOperatorDefinition
    : ASSIGN! LPAREN! valueParameter RPAREN!
    ;

arithmeticOperatorDefinition
    : ( PLUS! | MINUS! | STAR! | SLASH! | (STAR! STAR!) ) LPAREN! formalParameterList RPAREN!
    ;

comparisonOperatorDefinition
    : ( EQUAL! | LE! | GE! | GT! | LTH! ) LPAREN! formalParameterList RPAREN!
    ;

raiseStatement
    : RAISE^ ( functionCall (AT! addressConstant)? )?
    ;

exceptionStatement
    : tryStatement
    ;

tryStatement
    : TRY^ (statements)? exceptOrFinallyPart END!
    ;

exceptOrFinallyPart
    : EXCEPT! (exceptionHandlers)?
    | FINALLY! (statements)?
    ;

statements
    : statement ( SEMI! statement )*
    ;

exceptionHandlers
    : statements | exceptionHandler ( SEMI! exceptionHandler )* ( ELSE! statements )?
    ;

exceptionHandler
    : ON! (identifier COLON!)? identifier DO! statement
    ;

identifierList
    : identifier ( COMMA! identifier )*
      {#identifierList = #(#[IDLIST],#identifierList);}
    ;

label
    : unsignedInteger
    ;

unsignedInteger
    : NUM_INT
    ;

integerConstant
    : unsignedInteger
    |! s:sign n:unsignedInteger { #integerConstant=#(s,n); }
    ;

stringConstant
    : string | constantChr
    ;

sign
    : PLUS | MINUS
    ;

string
    : STRING_LITERAL
    ;

constantChr
    : CHR^ LPAREN! unsignedInteger RPAREN!
    ;

constant
    : unsignedNumber
    |! s:sign n:unsignedNumber { #constant=#(s,n); }
    | identifier
    |! s2:sign id:identifier { #constant=#(s2,id); }
    | string
    | constantChr
    ;

unsignedConstant
    : unsignedNumber | constantChr | string | NIL!
    ;

unsignedNumber
    : unsignedInteger
    | unsignedReal
    ;

unsignedReal
    : NUM_REAL
    ;

identifier
    : IDENT
    ;


//----------------------------------------------------------------------------
// The Pascal scanner
//----------------------------------------------------------------------------
class PascalLexer extends Lexer;

options {
  charVocabulary = '\0'..'\377';
  exportVocab = Pascal;   // call the vocabulary "Pascal"
  testLiterals = false;   // don't automatically test for literals
  k = 4;                  // four characters of lookahead
  caseSensitive = false;
  caseSensitiveLiterals = false;
  defaultErrorHandler=false;
}

tokens {
  ABSOLUTE         = "absolute"        ;
  ABSTRACT         = "abstract"        ;
  ALIAS            = "alias"           ;
  AND              = "and"             ;
  ARRAY            = "array"           ;
  AS               = "as"              ;
  ASM              = "asm"             ;
  ASSEMBLER        = "assembler"       ;
  BEGIN            = "begin"           ;
  BREAK            = "break"           ;
  BOOLEAN          = "boolean"         ;
  BYTE             = "byte"            ;
  CARDINAL         = "cardinal"        ;
  CASE             = "case"            ;
  CDECL            = "cdecl"           ;
  CHAR             = "char"            ;
  CHR              = "chr"             ;
  CLASS            = "class"           ;
  COMP             = "comp"            ;
  CONST            = "const"           ;
  CONSTRUCTOR      = "constructor"     ;
  CONTINUE         = "continue"        ;
  DEFAULT          = "default"         ;
  DESTRUCTOR       = "destructor"      ;
  DISPOSE          = "dispose"         ;
  DIV              = "div"             ;
  DO               = "do"              ;
  DOUBLE           = "double"          ;
  DOWNTO           = "downto"          ;
  ELSE             = "else"            ;
  END              = "end"             ;
  EXCEPT           = "except"          ;
  EXPORT           = "export"          ;
  EXPORTS          = "exports"         ;
  EXTENDED         = "extended"        ;
  EXTERNAL         = "external"        ;
  EXIT             = "exit"            ;
  FALSE            = "false"           ;
  FILE             = "file"            ;
  FINALLY          = "finally"         ;
  FAR              = "far"             ;
  FOR              = "for"             ;
  FORWARD          = "forward"         ;
  FUNCTION         = "function"        ;
  GOTO             = "goto"            ;
  IF               = "if"              ;
  IN               = "in"              ;
//  INDEX            = "index"           ;
  IS               = "is"              ;
  INHERITED        = "inherited"       ;
  INLINE           = "inline"          ;
  INT64            = "int64"           ;
  INTEGER          = "integer"         ;
  LABEL            = "label"           ;
  LIBRARY          = "library"         ;
  LONGINT          = "longint"         ;
  MOD              = "mod"             ;
//  NAME             = "name"            ;
  NEAR             = "near"            ;
  NEW              = "new"             ;
  NIL              = "nil"             ;
  NOT              = "not"             ;
  OBJECT           = "object"          ;
  OF               = "of"              ;
  ON               = "on"              ;
  OPERATOR         = "operator"        ;
  OR               = "or"              ;
  OVERRIDE         = "override"        ;
  PACKED           = "packed"          ;
  PASCAL           = "pascal"          ;
  POPSTACK         = "popstack"        ;
  PRIVATE          = "private"         ;
  PROCEDURE        = "procedure"       ;
  PROTECTED        = "protected"       ;
  PROGRAM          = "program"         ;
  PROPERTY         = "property"        ;
  PUBLIC           = "public"          ;
  PUBLISHED        = "published"       ;
  QWORD            = "qword"           ;
  RAISE            = "raise"           ;
  REAL             = "real"            ;
  RECORD           = "record"          ;
  REGISTER         = "register"        ;
  REPEAT           = "repeat"          ;
  SAFECALL         = "safecall"        ;
  SAVEREGISTERS    = "saveregisters"   ;
  SELF             = "self"            ;
  SET              = "set"             ;
  SHORTINT         = "shortint"        ;
  SHR              = "shr"             ;
  SHL              = "shl"             ;
  SINGLE           = "single"          ;
  SMALLINT         = "smallint"        ;
  STDCALL          = "stdcall"         ;
  THEN             = "then"            ;
  TO               = "to"              ;
  TRUE             = "true"            ;
  TRY              = "try"             ;
  TYPE             = "type"            ;
  UNTIL            = "until"           ;
  VAR              = "var"             ;
  VIRTUAL          = "virtual"         ;
  WHILE            = "while"           ;
  WITH             = "with"            ;
  WORD             = "word"            ;
  XOR              = "xor"             ;
  METHOD                               ;
  ADDSUBOR                             ;
  ASSIGNEQUAL                          ;
  SIGN                                 ;
  FUNC                                 ;
  NODE_NOT_EMIT                        ;
  MYASTVAR                             ;
  LF                                   ;
  UNIT             = "unit"            ;
  INTERFACE        = "interface"       ;
  USES             = "uses"            ;
  STRING           = "string"          ;
  IMPLEMENTATION   = "implementation"  ;
  FINALIZATION     = "finalization"    ;
  INITIALIZATION   = "initialization"  ;
  RESOURCESTRING   = "resourcestring"  ;
//pspsps ???
}

{
private:
        ProblemReporter* m_problemReporter;
        unsigned int m_numberOfErrors;

public:
        void resetErrors()                              { m_numberOfErrors = 0; }
        unsigned int numberOfErrors() const             { return m_numberOfErrors; }
        void setProblemReporter( ProblemReporter* r )   { m_problemReporter = r; }

        virtual void reportError( const ANTLR_USE_NAMESPACE(antlr)RecognitionException& ex ){
                m_problemReporter->reportError( ex.getMessage().c_str(),
                                                ex.getFilename().c_str(),
                                                ex.getLine(),
                                                ex.getColumn() );
                ++m_numberOfErrors;
        }

        virtual void reportError( const ANTLR_USE_NAMESPACE(std)string& errorMessage ){
                m_problemReporter->reportError( errorMessage.c_str(),
                                                getFilename().c_str(),
                                                getLine(), getColumn() );
                ++m_numberOfErrors;
        }

        virtual void reportWarning( const ANTLR_USE_NAMESPACE(std)string& warnMessage ){
                m_problemReporter->reportWarning( warnMessage.c_str(),
                                                getFilename().c_str(),
                                                getLine(), getColumn() );
        }
}

//----------------------------------------------------------------------------
// OPERATORS
//----------------------------------------------------------------------------
PLUS            : '+'   ;
MINUS           : '-'   ;
STAR            : '*'   ;
SLASH           : '/'   ;
ASSIGN          : ":="  ;
COMMA           : ','   ;
SEMI            : ';'   ;
COLON           : ':'   ;
EQUAL           : '='   ;
NOT_EQUAL       : "<>"  ;
LTH             : '<'   ;
LE              : "<="  ;
GE              : ">="  ;
GT              : '>'   ;
LPAREN          : '('   ;
RPAREN          : ')'   ;
LBRACK          : '['   ; // line_tab[line]
LBRACK2         : "(."  ; // line_tab(.line.)
RBRACK          : ']'   ;
RBRACK2         : ".)"  ;
POINTER         : '^'   ;
AT              : '@'   ;
DOT             : '.' ('.' {$setType(DOTDOT);})?  ;
LCURLY          : "{" ;
RCURLY          : "}" ;
PLUSEQ          : "+=" ;
MINUSEQ         : "-=" ;
STAREQ          : "*=" ;
SLASHQE         : "/=" ;


// Whitespace -- ignored
WS      : ( ' '
		|	'\t'
		|	'\f'
		// handle newlines
		|	(	"\r\n"  // Evil DOS
			|	'\r'    // Macintosh
			|	'\n'    // Unix (the right way)
			)
			{ newline(); }
		)
		  { _ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP; }
	;


COMMENT_1
        : "(*"
		   ( options { generateAmbigWarnings=false; }
		   :	{ LA(2) != ')' }? '*'
		   |	'\r' '\n'		{newline();}
		   |	'\r'			{newline();}
		   |	'\n'			{newline();}
           |   ~('*' | '\n' | '\r')
		   )*
          "*)"
		{$setType(ANTLR_USE_NAMESPACE(antlr)Token::SKIP);}
	;

COMMENT_2
        :  '{'
		    ( options {generateAmbigWarnings=false;}
            :   '\r' '\n'       {newline();}
		    |	'\r'			{newline();}
		    |	'\n'			{newline();}
            |   ~('}' | '\n' | '\r')
		    )*
           '}'
		{$setType(ANTLR_USE_NAMESPACE(antlr)Token::SKIP);}
	;

COMMENT_3
    : "//" (~'\n')* '\n'
    {$setType(ANTLR_USE_NAMESPACE(antlr)Token::SKIP);}
    ;

// an identifier.  Note that testLiterals is set to true!  This means
// that after we match the rule, we look in the literals table to see
// if it's a literal or really an identifer
IDENT
	options {testLiterals=true;}
	:	('a'..'z') ('a'..'z'|'0'..'9'|'_')*   //pspsps
	;

// string literals
STRING_LITERAL
	: '\'' ("\'\'" | ~('\''))* '\''   //pspsps   * in stead of + because of e.g. ''
	;

/** a numeric literal.  Form is (from Wirth)
 *  digits
 *  digits . digits
 *  digits . digits exponent
 *  digits exponent
 */
NUM_INT
	:	('0'..'9')+ // everything starts with a digit sequence
		(	(	{(LA(2)!='.')&&(LA(2)!=')')}?				// force k=2; avoid ".."
//PSPSPS example ARRAY (.1..99.) OF char; // after .. thinks it's a NUM_REAL
				'.' {$setType(NUM_REAL);}	// dot means we are float
				('0'..'9')+ (EXPONENT)?
			)?
		|	EXPONENT {$setType(NUM_REAL);}	// 'E' means we are float
		)
	;

// a couple protected methods to assist in matching floating point numbers
protected
EXPONENT
	:	('e') ('+'|'-')? ('0'..'9')+
	;
