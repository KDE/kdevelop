/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef PARSER_H
#define PARSER_H

#include "config.h"

#include "ast.h"
#include "lexer.h"
#include "commentparser.h"

#include <QtCore/QQueue>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <cppparserexport.h>
#include "commentformatter.h"

class TokenStream;
class Control;

/**
The Parser.
LL(k) parser for c++ code.
*/
class KDEVCPPPARSER_EXPORT Parser
{
public:
  Parser(Control *control);
  ~Parser();

  /**Parses the @p contents of the buffer of given @p size using the
  memory pool @p p to store tokens found.

  Calls lexer to tokenize all contents buffer, skips the first token
  (because the lexer provides Token_EOF as the first token,
  creates and fills the AST and returns translation unit or 0
  if nothing was parsed.

  @sa pool for more information about the memory pool used.*/
  TranslationUnitAST *parse(ParseSession* session);
  /**

   * Same as parse, except that it parses the content as a compound statement.
   * This is useful for parsing independent expression-strings that appear for
   * example within a function.
   *
   * parse(..) expects the content to be a valid translation-unit,
   * while parseStatement can parse strings like "{a.b = 5;}" as if they
   * appeared within a function.
   *
   * Note: The input must be a valid compound-statement, which means it must
   * start at least with "{", and end with ";}"
   **/
  StatementAST *parseStatement(ParseSession* session);

  /**
   * Nearly the same as parseStatement, except that it parses the content as a type-id if possible,
   * if that fails, as an expression. This is perfect for parsing template-arguements.
   * The input can be as simple as "A" or "A::a+2"
   * @param forceExpression If this is true, the text will not be considered to be a type-id, only an expression.
   *                                         With this parameter, unlike during normal parsing, primary expressions may
   *                                         have template parameters even without a function-call behind. That allows evaluating only partial expressions.
   * */
  AST *parseTypeOrExpression(ParseSession* session, bool forceExpression = false);
  /**@return the problem count.*/
  int problemCount() const { return _M_problem_count; }

  /**
   * Fixup an InitDeclaratorAST @p node, which was misinterpreted to contain a
   * parameter-declaration-clause while that is actually an initializer.
   *
   * This function will create a proper initializer and assign it to @p node
   * and unsets the association to the parameter-declaration-clause.
   */
  void fixupInitializerFromParameter(InitDeclaratorAST* node, ParseSession* session);

private:
  /**Convenience method to report problems. Constructs the problem
  using the information about the current line and column in the buffer
  that is being parsed. Then stores the problem in the control object.*/
  void reportError(const QString& msg, KDevelop::ProblemData::Severity severity = KDevelop::ProblemData::Error);
  /**Reports a syntax error about unexpected token. The token
  reported is LA (look-ahead) from the stream.*/
  void syntaxError();
  /**Reports a syntax error about required token when LA is wrong.*/
  void tokenRequiredError(int expected);

public:
  bool parseAbstractDeclarator(DeclaratorAST *&node);
  bool parseAccessSpecifier(DeclarationAST *&node);
  bool parseAdditiveExpression(ExpressionAST *&node);
  bool parseAndExpression(ExpressionAST *&node, bool templArgs = false);
  bool parseAsmDefinition(DeclarationAST *&node);
  bool parseAssignmentExpression(ExpressionAST *&node);
  bool parseBaseClause(BaseClauseAST *&node);
  bool parseBaseSpecifier(BaseSpecifierAST *&node);
  bool parseBlockDeclaration(DeclarationAST *&node);
  bool parseBracedInitList(ExpressionAST *&node);
  bool parseCastExpression(ExpressionAST *&node);
  bool parseClassSpecifier(TypeSpecifierAST *&node);
  bool parseSignalSlotExpression(ExpressionAST *&node);
  bool parseQProperty(DeclarationAST *&node);
  bool parseCommaExpression(ExpressionAST *&node);
  bool parseCompoundStatement(StatementAST *&node);
  bool parseCondition(ConditionAST *&node, bool initRequired = true);
  bool parseConditionalExpression(ExpressionAST *&node, bool templArgs = false);
  bool parseConstantExpression(ExpressionAST *&node);
  bool parseCtorInitializer(CtorInitializerAST *&node);
  bool parseCvQualify(const ListNode<uint> *&node);
  bool parseRefQualifier(uint& ref_qualifier);
  bool parseDeclaration(DeclarationAST *&node);
  bool parseDeclarationInternal(DeclarationAST *&node);
  bool parseFunctionDefinitionInternal(DeclarationAST *&node, uint start, WinDeclSpecAST* winDeclSpec,
                                       const ListNode<uint>* storageSpec, const ListNode<uint>* funSpec,
                                       TypeSpecifierAST* spec);
  bool parseDeclarationStatement(StatementAST *&node);
  bool parseDeclarator(DeclaratorAST *&node, bool allowBitfield = true);
  bool parseDeleteExpression(ExpressionAST *&node);
  bool parseDoStatement(StatementAST *&node);
  bool parseElaboratedTypeSpecifier(TypeSpecifierAST *&node);
  bool parseEnumSpecifier(TypeSpecifierAST *&node);
  bool parseEnumerator(EnumeratorAST *&node);
  bool parseEqualityExpression(ExpressionAST *&node,
			       bool templArgs = false);
  bool parseExceptionSpecification(ExceptionSpecificationAST *&node);
  bool parseExclusiveOrExpression(ExpressionAST *&node,
				  bool templArgs = false);
  bool parseExpression(ExpressionAST *&node);
  bool parseExpressionList(ExpressionAST *&node);
  bool parseExpressionOrDeclarationStatement(StatementAST *&node);
  bool parseExpressionStatement(StatementAST *&node);
  bool parseForInitStatement(StatementAST *&node);
  bool parseForStatement(StatementAST *&node);
  bool parseRangeBasedFor(ForRangeDeclarationAst *&node);
  bool parseFunctionBody(StatementAST *&node);
  bool parseFunctionSpecifier(const ListNode<uint> *&node);
  bool parseIfStatement(StatementAST *&node);
  bool parseInclusiveOrExpression(ExpressionAST *&node,
				  bool templArgs = false);
  bool parseInitDeclarator(InitDeclaratorAST *&node);
  bool parseInitDeclaratorList(const ListNode<InitDeclaratorAST*> *&node);
  bool parseInitializer(InitializerAST *&node);
  bool parseDesignatedInitializer(InitializerClauseAST *&node);
  bool parseInitializerClause(InitializerClauseAST *&node);
  bool parseInitializerList(InitializerListAST *&node);
  bool parseJumpStatement(StatementAST *&node);
  bool parseLabeledStatement(StatementAST *&node);
  bool parseLinkageBody(LinkageBodyAST *&node);
  bool parseLinkageSpecification(DeclarationAST *&node);
  bool parseLogicalAndExpression(ExpressionAST *&node,
				 bool templArgs = false);
  bool parseLogicalOrExpression(ExpressionAST *&node,
				bool templArgs = false);
  bool parseMemInitializer(MemInitializerAST *&node);
  bool parseMemInitializerList(const ListNode<MemInitializerAST*> *&node);
  bool parseMemberSpecification(DeclarationAST *&node);
  bool parseMultiplicativeExpression(ExpressionAST *&node);
  bool parseStaticAssert(DeclarationAST *&node);
  bool parseTrailingReturnType(TrailingReturnTypeAST *&node);
  
  enum ParseNameAcceptTemplate {
    DontAcceptTemplate = 0,
    AcceptTemplate = 1,
    //If this is given, template-parameters are only accepted if the name is followed by a function call
    //or by a braced init list
    EventuallyAcceptTemplate = 2
  };
  bool parseName(NameAST *&node, ParseNameAcceptTemplate acceptTemplateId = DontAcceptTemplate);
  bool parseNamespace(DeclarationAST *&node);
  bool parseNamespaceAliasDefinition(DeclarationAST *&node);
  bool parseNewDeclarator(NewDeclaratorAST *&node);
  bool parseNewExpression(ExpressionAST *&node);
  bool parseNewInitializer(NewInitializerAST *&node);
  bool parseNewTypeId(NewTypeIdAST *&node);
  bool parseOperator(OperatorAST *&node);
  bool parseOperatorFunctionId(OperatorFunctionIdAST *&node);
  bool parseParameterDeclaration(ParameterDeclarationAST *&node);
  bool parseParameterDeclarationClause(ParameterDeclarationClauseAST *&node);
  bool parseParameterDeclarationList(const ListNode<ParameterDeclarationAST*> *&node);
  bool parsePmExpression(ExpressionAST *&node);
  bool parsePostfixExpression(ExpressionAST *&node);
  bool parsePostfixExpressionInternal(ExpressionAST *&node);
  bool parsePrimaryExpression(ExpressionAST *&node);
  bool parsePtrOperator(PtrOperatorAST *&node);
  bool parsePtrToMember(PtrToMemberAST *&node);
  bool parseRelationalExpression(ExpressionAST *&node,
				 bool templArgs = false);
  bool parseShiftExpression(ExpressionAST *&node);
  bool parseSimpleTypeSpecifier(TypeSpecifierAST *&node,
				bool onlyIntegral = false);
  bool parseStatement(StatementAST *&node);
  bool parseStorageClassSpecifier(const ListNode<uint> *&node);
  bool parseStringLiteral(StringLiteralAST *&node);
  bool parseSwitchStatement(StatementAST *&node);
  bool parseTemplateArgument(TemplateArgumentAST *&node);
  bool parseTemplateArgumentList(const ListNode<TemplateArgumentAST*> *&node,
				 bool reportError = true);
  bool parseTemplateDeclaration(DeclarationAST *&node);
  bool parseTemplateParameter(TemplateParameterAST *&node);
  bool parseTemplateParameterList(const ListNode<TemplateParameterAST*> *&node);
  bool parseThrowExpression(ExpressionAST *&node);
  bool parseTranslationUnit(TranslationUnitAST *&node);
  bool parseTryBlockStatement(StatementAST *&node);
  bool parseTypeId(TypeIdAST *&node);
  bool parseTypeIdList(const ListNode<TypeIdAST*> *&node);
  bool parseTypeParameter(TypeParameterAST *&node);
  bool parseTypeSpecifier(TypeSpecifierAST *&node);
  bool parseTypeSpecifierOrClassSpec(TypeSpecifierAST *&node);
  bool parseTypedef(DeclarationAST *&node);
  bool parseUnaryExpression(ExpressionAST *&node);
  bool parseUnqualifiedName(UnqualifiedNameAST *&node,
			    bool parseTemplateId = true);
  bool parseUsing(DeclarationAST *&node);
  bool parseUsingDirective(DeclarationAST *&node);
  bool parseWhileStatement(StatementAST *&node);
  bool parseWinDeclSpec(WinDeclSpecAST *&node);
  bool parseLambdaExpression(ExpressionAST *&node);
  bool parseLambdaCapture(LambdaCaptureAST *&node);
  bool parseLambdaDeclarator(LambdaDeclaratorAST *&node);
  bool parseMemberVirtSpecifier(const ListNode<uint> *&node);
  bool parseClassVirtSpecifier(const ListNode<uint> *&node);

  bool skipUntil(int token);
  bool skipUntilDeclaration();
  bool skipUntilStatement();
  bool skip(int l, int r);

  void addComment( CommentAST* ast, const Comment& comment );
  //Moves all currently available comments to the given AST, removing them from the comment-store
  void moveComments( CommentAST* ast );

  void advance(bool skipComment = true);
  void rewind(uint position);
  // private:
  TokenStream* token_stream;

// private:
  Control *control;
  Lexer lexer;
private:
  
  enum TokenMarkers {
    None = 0,
    IsNoTemplateArgumentList = 1
  };
  
  TokenMarkers tokenMarkers(uint tokenNumber) const;
  void addTokenMarkers(uint tokenNumber, TokenMarkers markers);


  int lineFromTokenNumber( uint tokenNumber ) const;

  void clear();
  
  ///parses all comments until the end of the line
  Comment comment();
  ///Preparses comments in the same line as given token-number
  void preparseLineComments( int tokenNumber );
  void processComment( int offset = 0, int line = -1 );
  void clearComment( );

  bool holdErrors(bool hold);
  void reportPendingErrors();

  Comment m_currentComment;
  CommentStore m_commentStore;
 
  QHash<uint, TokenMarkers> m_tokenMarkers;
  int _M_problem_count;
  int _M_max_problem_count;
  ParseSession* session;
  bool _M_hold_errors;
  uint _M_last_valid_token; //Last encountered token that was not a comment
  uint _M_last_parsed_comment;
  
  bool _M_hadMismatchingCompoundTokens;
  bool m_primaryExpressionWithTemplateParamsNeedsFunctionCall;

  // keeps track of tokens where a syntax error has been found
  // so that the same error is not reported twice for a token
  QSet<uint> m_syntaxErrorTokens;

  // when _M_hold_errors is true, reported errors are held in m_pendingErrors
  // rather than being reported to the Control immediately.
  //
  // this is used, for example, when parsing ambiguous statements.
  struct PendingError
  {
   QString message;
   uint cursor;
  };
  QQueue<PendingError> m_pendingErrors;

  ///return string representation of @p node for debugging
  QString stringForNode(AST* node) const;

private:
  Parser(const Parser& source);
  void operator = (const Parser& source);

};

#endif

