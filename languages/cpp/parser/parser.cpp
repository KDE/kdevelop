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

//krazy:excludeall=cpp

// c++ support
#include "parser.h"
#include "tokens.h"
#include "lexer.h"
#include "control.h"
#include "parsesession.h"
#include "commentformatter.h"
#include "memorypool.h"
#include "debug.h"

#include <cstdlib>
#include <iostream>
#include "rpp/chartools.h"

Q_LOGGING_CATEGORY(CPPPARSER, "kdevelop.languages.cpp.parser")

#define ADVANCE(tk, descr) \
{ \
  if (session->token_stream->lookAhead() != tk) { \
      tokenRequiredError(tk); \
      return false; \
  } \
  advance(); \
}

#define ADVANCE_NR(tk, descr) \
  do { \
    if (session->token_stream->lookAhead() != tk) { \
      tokenRequiredError(tk); \
    } \
    else \
        advance(); \
  } while (0)

#define CHECK(tk) \
  do { \
    if (session->token_stream->lookAhead() != tk) { \
        return false; \
    } \
    advance(); \
  } while (0)

#define UPDATE_POS(_node, start, end) \
  do { \
      (_node)->start_token = start; \
      (_node)->end_token = end; \
  } while (0)

template <class _Tp>
inline _Tp *CreateNode(MemoryPool *memory_pool)
{
  _Tp *node = memory_pool->allocate<_Tp>();
  node->kind = _Tp::__node_kind;
  return node;
}


void Parser::addComment( CommentAST* ast, const Comment& comment ) {
  if( comment ) {
/*    qCDebug(CPPPARSER) << "Adding but leaving comment" << session->token_stream->token(comment.token()).symbol();*/
    ast->comments = snoc(ast->comments, comment.token(), session->mempool);
  }
}

void Parser::moveComments( CommentAST* ast ) {
  while( m_commentStore.hasComment() ) {
    uint token = m_commentStore.takeFirstComment().token();

/*    qCDebug(CPPPARSER) << "Moving comment" << session->token_stream->token(token).symbol();*/

    ast->comments = snoc(ast->comments, token, session->mempool);
  }
}

Parser::Parser(Control *c)
  : control(c), lexer(control)
  , _M_problem_count(0)
  , _M_max_problem_count(5)
  , session(0)
  , _M_hold_errors(false)
  , _M_last_valid_token(0)
  , _M_last_parsed_comment(0)
  , _M_hadMismatchingCompoundTokens(false)
  , m_primaryExpressionWithTemplateParamsNeedsFunctionCall(true)
{
}

Parser::~Parser()
{
}

void Parser::rewind(uint position) {
  session->token_stream->rewind(position);

  //Search the previous valid token

  _M_last_valid_token = position > 0 ? position-1 : position;

  while( _M_last_valid_token > 0 && session->token_stream->kind(_M_last_valid_token) == Token_comment )
    --_M_last_valid_token;
}

void Parser::advance( bool skipComment ) {
  uint t = session->token_stream->lookAhead();
  if( session->token_stream->cursor() > 0 && t == Token_EOF ) {
    return;
  }
  if(  t != Token_comment )
    _M_last_valid_token = session->token_stream->cursor();

  session->token_stream->nextToken();

  if( session->token_stream->lookAhead() == Token_comment ) {
    if( skipComment ) {
      processComment();
      advance();
    }
  }
}

Comment Parser::comment() {
    return m_commentStore.latestComment();
}

void Parser::preparseLineComments( int tokenNumber ) {
  const Token& token( (*session->token_stream)[tokenNumber] );
  KDevelop::CursorInRevision tokenPosition = KDevelop::CursorInRevision::invalid();

  for( int a = 0; a < 40; a++ ) {
      if( !session->token_stream->lookAhead(a) ) break;
      if( session->token_stream->lookAhead(a) == Token_comment ) {
        //Make sure the token's line is before the searched token's line
        const Token& commentToken( (*session->token_stream)[session->token_stream->cursor() + a] );

        if( !tokenPosition.isValid() ) //Get the token line. Only on-demand, because it's not cheap.
          tokenPosition = session->positionAt(token.position);

        KDevelop::CursorInRevision commentPosition = session->positionAt( commentToken.position );

        if( commentPosition.line < tokenPosition.line ) {
            continue;
        } else if( commentPosition.line == tokenPosition.line ) {
            processComment( a );
        } else {
            //Too far
            break;
        }
      }
  }
}

int Parser::lineFromTokenNumber( uint tokenNumber ) const {
  const Token& token( (*session->token_stream)[tokenNumber] );
  return session->positionAt( token.position ).line;
}


void Parser::processComment( int offset, int line ) {
  uint tokenNumber = session->token_stream->cursor() + offset;

  if(_M_last_parsed_comment >= tokenNumber)
    return; //The comment was already parsed. May happen because of pre-parsing

  _M_last_parsed_comment = tokenNumber;

  const Token& commentToken( (*session->token_stream)[tokenNumber] );
  Q_ASSERT(commentToken.kind == Token_comment);
  if( line == -1 ) {
    KDevelop::CursorInRevision position = session->positionAt( commentToken.position );
    line = position.line;
  }

  session->m_commentFormatter.extractToDos( tokenNumber, session, control );
  m_commentStore.addComment( Comment( session->token_stream->cursor() + offset, line ) );

}

void Parser::clearComment( ) {
  m_commentStore.clear();
}

TranslationUnitAST *Parser::parse(ParseSession* _session)
{
  clear();
  session = _session;

  if (!session->token_stream)
    session->token_stream = new TokenStream(session);

  lexer.tokenize(session);
  advance(); // skip the first token

  TranslationUnitAST *ast = 0;
  parseTranslationUnit(ast);
  return ast;
}

StatementAST *Parser::parseStatement(ParseSession* _session)
{
  clear();
  session = _session;

  if (!session->token_stream)
    session->token_stream = new TokenStream(session);

  lexer.tokenize(session);
  advance(); // skip the first token

  StatementAST *ast = 0;
  parseCompoundStatement(ast);
  return ast;
}

AST *Parser::parseTypeOrExpression(ParseSession* _session, bool forceExpression)
{
  clear();
  session = _session;

  if (!session->token_stream)
    session->token_stream = new TokenStream(session);

  lexer.tokenize(session);
  advance(); // skip the first token

  TypeIdAST *ast = 0;
  if (!forceExpression)
    parseTypeId(ast);
  if(!ast) {
    m_primaryExpressionWithTemplateParamsNeedsFunctionCall = false;
    ExpressionAST* ast = 0;
    parseExpression(ast);
    return ast;
  }

  return ast;
}

void Parser::fixupInitializerFromParameter(InitDeclaratorAST* node, ParseSession* _session)
{
  clear();
  session = _session;

  Q_ASSERT(session->token_stream);

  Q_ASSERT(!node->initializer);
  Q_ASSERT(node->declarator);
  Q_ASSERT(node->declarator->parameter_declaration_clause);

  // include the '(', which is not included in the parameter_declaration_clause, thus: -1
  rewind(node->declarator->parameter_declaration_clause->start_token - 1);

  InitializerAST* initializer = 0;
  if (!parseInitializer(initializer)) {
    // note: this can happen when we encounter unresolved types, since then
    // DeclarationBuilder::checkParameterDeclarationClause will think this is an initializer
    // we handle this gracefully and just exit
    return;
  }

  // fixup AST
  node->initializer = initializer;
  // note: this is not leaking, as the AST is managed by a memory pool
  node->declarator->parameter_declaration_clause = 0;
}


void Parser::clear()
{
  _M_hold_errors = false;
  _M_problem_count = 0;
  _M_hadMismatchingCompoundTokens = false;
  m_tokenMarkers.clear();
}


void Parser::addTokenMarkers(uint tokenNumber, Parser::TokenMarkers markers)
{
  QHash<uint, TokenMarkers>::iterator it = m_tokenMarkers.find(tokenNumber);
  if(it != m_tokenMarkers.end())
    it.value() = (TokenMarkers)(it.value() | markers);
  else
    m_tokenMarkers.insert(tokenNumber, markers);
}

Parser::TokenMarkers Parser::tokenMarkers(uint tokenNumber) const
{
  return m_tokenMarkers.value(tokenNumber, None);
}

bool Parser::parseWinDeclSpec(WinDeclSpecAST *&node)
{
  if (session->token_stream->lookAhead() != Token_identifier)
    return false;

  uint start = session->token_stream->cursor();

  const uint tokenIndex = session->token_stream->symbolIndex(session->token_stream->cursor());
  static const KDevelop::IndexedString declSpecString("__declspec");
  if (declSpecString.index() != tokenIndex)
    return false;

  uint specifier = session->token_stream->cursor();

  advance();
  if (session->token_stream->lookAhead() != '(')
    return false;

  advance();
  if (session->token_stream->lookAhead() != Token_identifier)
    return false;
  uint modifier = session->token_stream->cursor();

  advance();
  if (session->token_stream->lookAhead() != ')')
    return false;

  advance();

  node = CreateNode<WinDeclSpecAST>(session->mempool);
  node->specifier = specifier;
  node->modifier = modifier;

  UPDATE_POS(node, start, _M_last_valid_token+1);

  return true;
}

void Parser::tokenRequiredError(int token)
{
  QString err;

  err += "Expected token ";
  err += '\'';
  err += token_name(token);
  err += "\' after \'";
  err += token_name(session->token_stream->lookAhead(-1));
  err += "\' found \'";
  err += token_name(session->token_stream->lookAhead());
  err += '\'';

  if(token == '}' || token == '{')
    _M_hadMismatchingCompoundTokens = true;

  reportError(err);
}

void Parser::syntaxError()
{
  uint cursor = session->token_stream->cursor();
  uint kind = session->token_stream->lookAhead();

  if (m_syntaxErrorTokens.contains(cursor))
      return; // syntax error at this point has already been reported

  m_syntaxErrorTokens.insert(cursor);

  QString err;

  if (kind == Token_EOF)
    err += "Unexpected end of file";
  else
  {
    err += "Unexpected token ";
    err += '\'';
    err += token_name(kind);
    err += '\'';
  }

  reportError(err);
}

void Parser::reportPendingErrors()
{
  bool hold = holdErrors(false);

  uint start = session->token_stream->cursor();
 while (m_pendingErrors.count() > 0)
 {
   PendingError error = m_pendingErrors.dequeue();
    session->token_stream->rewind(error.cursor);
    reportError(error.message);
 }
  rewind(start);

  holdErrors(hold);
}

void Parser::reportError(const QString& msg, KDevelop::ProblemData::Severity severity)
{
  if (!_M_hold_errors && _M_problem_count < _M_max_problem_count)
    {
      ++_M_problem_count;

      QString fileName;

      uint tok = session->token_stream->cursor();
      KDevelop::CursorInRevision position = session->positionAt(session->token_stream->position(tok));

      KDevelop::ProblemPointer p(new KDevelop::Problem);
      p->setFinalLocation(KDevelop::DocumentRange(session->url(), KTextEditor::Range(position.castToSimpleCursor(), 0)));
      p->setDescription(msg);
      p->setSource(KDevelop::ProblemData::Parser);
      p->setSeverity(severity);

      control->reportProblem(p);
    }
  else if (_M_hold_errors)
  {
    PendingError pending;
    pending.message = msg;
    pending.cursor = session->token_stream->cursor();
    m_pendingErrors.enqueue(pending);
  }
}

bool Parser::skipUntil(int token)
{
  clearComment();

  while (session->token_stream->lookAhead())
    {
      if (session->token_stream->lookAhead() == token)
        return true;

      advance();
    }

  return false;
}

bool Parser::skipUntilDeclaration()
{
  while (session->token_stream->lookAhead())
    {

      switch(session->token_stream->lookAhead())
        {
        case ';':
        case '~':
        case Token_scope:
        case Token_identifier:
        case Token_inline:
        case Token_operator:
        case Token_char:
        case Token_char16_t:
        case Token_char32_t:
        case Token_wchar_t:
        case Token_bool:
        case Token_short:
        case Token_int:
        case Token_long:
        case Token_signed:
        case Token_unsigned:
        case Token_float:
        case Token_double:
        case Token_void:
        case Token_auto:
        case Token_extern:
        case Token_namespace:
        case Token_using:
        case Token_typedef:
        case Token_asm:
        case Token_template:
        case Token_export:
        case Token_thread_local:
        case Token_static_assert:
        case Token_constexpr:

        case Token_const:       // cv
        case Token_volatile:    // cv

        case Token_public:
        case Token_protected:
        case Token_private:
        case Token___qt_signals__:      // Qt
        case Token___qt_slots__:        // Qt
          return true;
        case '}':
          return false;

        default:
          advance();
        }
    }

  return false;
}

bool Parser::skipUntilStatement()
{
  while (session->token_stream->lookAhead())
    {
      switch(session->token_stream->lookAhead())
        {
        case ';':
        case '{':
        case '}':
        case Token_const:
        case Token_volatile:
        case Token_identifier:
        case Token_case:
        case Token_default:
        case Token_if:
        case Token_switch:
        case Token_while:
        case Token_do:
        case Token_for:
        case Token_break:
        case Token_continue:
        case Token_return:
        case Token_goto:
        case Token_try:
        case Token_catch:
        case Token_throw:
        case Token_char:
        case Token_char16_t:
        case Token_char32_t:
        case Token_wchar_t:
        case Token_bool:
        case Token_short:
        case Token_int:
        case Token_long:
        case Token_signed:
        case Token_unsigned:
        case Token_float:
        case Token_double:
        case Token_void:
        case Token_auto:
        case Token_class:
        case Token_struct:
        case Token_union:
        case Token_enum:
        case Token_scope:
        case Token_template:
        case Token_using:
        case Token_static_assert:
        case Token_constexpr:
          return true;

        default:
          advance();
        }
    }

  return false;
}

bool Parser::skip(int l, int r)
{
  int count = 0;
  while (session->token_stream->lookAhead())
    {
      int tk = session->token_stream->lookAhead();

      if (tk == l)
        ++count;
      else if (tk == r)
        --count;
      else if (l != '{' && (tk == '{' || tk == '}' || tk == ';'))
        return false;

      if (count == 0)
        return true;

      advance();
    }

  return false;
}

bool Parser::parseName(NameAST*& node, ParseNameAcceptTemplate acceptTemplateId)
{
  uint start = session->token_stream->cursor();

  WinDeclSpecAST *winDeclSpec = 0;
  parseWinDeclSpec(winDeclSpec);

  NameAST *ast = 0;
  bool setGlobal = false;

  if (session->token_stream->lookAhead() == Token_scope)
    {
      setGlobal = true;
      advance();
    }

  uint idx = session->token_stream->cursor();

  while (true)
    {
      UnqualifiedNameAST *n = 0;
      if (!parseUnqualifiedName(n)) {
        return false;
      }

      if (!ast)
        ast = CreateNode<NameAST>(session->mempool);

      if (session->token_stream->lookAhead() == Token_scope &&
        //ptr-to-member
        session->token_stream->lookAhead(1) != '*')
        {

          advance();

          ast->qualified_names
            = snoc(ast->qualified_names, n, session->mempool);

          if (session->token_stream->lookAhead() == Token_template)
            {
              /// skip optional template     #### @todo CHECK
              advance();
            }
        }
      else
        {
          Q_ASSERT(n != 0);

          if (acceptTemplateId == DontAcceptTemplate ||
            //Eventually only accept template parameters as primary expression if the expression is followed by a function call
            //or by a braced init list
            (acceptTemplateId == EventuallyAcceptTemplate && n->template_arguments
              && session->token_stream->lookAhead() != '('
              && session->token_stream->lookAhead() != '{'
              && m_primaryExpressionWithTemplateParamsNeedsFunctionCall))
            {
              rewind(n->start_token);
              parseUnqualifiedName(n, false);
            }

          ast->unqualified_name = n;
          break;
        }
    }

  if (idx == session->token_stream->cursor())
    return false;
  if (setGlobal)
    ast->global = true;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTranslationUnit(TranslationUnitAST *&node)
{
  clear();

/*  qCDebug(CPPPARSER) << "tokens:";
  for(uint a = 0; a < session->token_stream->size(); ++a)
    qCDebug(CPPPARSER) << token_name(session->token_stream->token(a).kind) << session->token_stream->token(a).symbolString();*/

  uint start = session->token_stream->cursor();
  TranslationUnitAST *ast = CreateNode<TranslationUnitAST>(session->mempool);
  session->setTopAstNode(ast);

  if( m_commentStore.hasComment() )
    addComment(ast, m_commentStore.takeFirstComment());

  while (session->token_stream->lookAhead())
    {
      uint startDecl = session->token_stream->cursor();

      DeclarationAST *declaration = 0;
      if (parseDeclaration(declaration))
        {
          ast->declarations =
            snoc(ast->declarations, declaration, session->mempool);
        }
      else
        {
          // error recovery
          if (startDecl == session->token_stream->cursor())
            {
              // skip at least one token
              advance();
            }

          skipUntilDeclaration();
        }
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  ast->hadMissingCompoundTokens = _M_hadMismatchingCompoundTokens;

  return true;
}

bool Parser::parseDeclaration(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  switch(session->token_stream->lookAhead())
    {
    case ';':
      advance();
      return true;

    case Token_extern:
      return parseLinkageSpecification(node);

    case Token_namespace:
      return parseNamespace(node);

    case Token_using:
      return parseUsing(node);

    case Token_typedef:
      return parseTypedef(node);

    case Token_asm:
      return parseAsmDefinition(node);

    case Token_template:
    case Token_export:
      return parseTemplateDeclaration(node);

    case Token_static_assert:
      return parseStaticAssert(node);

    case Token_inline:
      if (session->token_stream->lookAhead(+1) == Token_namespace) {
        return parseNamespace(node);
      }
      // else fallthrough
    default:
      {
        const ListNode<uint> *cv = 0;
        parseCvQualify(cv);

        const ListNode<uint> *storageSpec = 0;
        parseStorageClassSpecifier(storageSpec);

        parseCvQualify(cv);

        Comment mcomment = comment();
        clearComment();

        TypeSpecifierAST *spec = 0;
        if (parseEnumSpecifier(spec)
            || parseClassSpecifier(spec))
          {
            parseCvQualify(cv);

            spec->cv = cv;

            const ListNode<InitDeclaratorAST*> *declarators = 0;
            parseInitDeclaratorList(declarators);
            ADVANCE(';', ";");

            SimpleDeclarationAST *ast =
              CreateNode<SimpleDeclarationAST>(session->mempool);
            ast->storage_specifiers = storageSpec;
            ast->type_specifier = spec;
            ast->init_declarators = declarators;
            UPDATE_POS(ast, start, _M_last_valid_token+1);
            node = ast;

            if( mcomment )
              addComment(ast, mcomment);

            preparseLineComments(ast->end_token-1);

            if( m_commentStore.hasComment() )
              addComment( ast, m_commentStore.takeCommentInRange( lineFromTokenNumber( --ast->end_token ) ) );

            return true;
          } else {
            rewind(start);
            if( parseDeclarationInternal(node) ) {
              //Add the comments to the declaration
              if( mcomment )
                addComment(node, mcomment);

              preparseLineComments(node->end_token-1);

              if( m_commentStore.hasComment() )
                addComment( node, m_commentStore.takeCommentInRange( lineFromTokenNumber( --node->end_token ) ) );

              return true;
            }
          }
      }
    } // end switch

    return false;
}

bool Parser::parseLinkageSpecification(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_extern);

  LinkageSpecificationAST *ast = CreateNode<LinkageSpecificationAST>(session->mempool);

  if (session->token_stream->lookAhead() == Token_string_literal)
    {
      ast->extern_type = session->token_stream->cursor();
      advance();
    }

  if (session->token_stream->lookAhead() == '{')
    {
      parseLinkageBody(ast->linkage_body);
    }
  else if (!parseDeclaration(ast->declaration))
    {
      reportError(("Declaration syntax error"));
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseLinkageBody(LinkageBodyAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK('{');

  LinkageBodyAST *ast = CreateNode<LinkageBodyAST>(session->mempool);

  while (session->token_stream->lookAhead())
    {
      int tk = session->token_stream->lookAhead();

      if (tk == '}')
        break;

      uint startDecl = session->token_stream->cursor();

      DeclarationAST *declaration = 0;
      if (parseDeclaration(declaration))
        {
          ast->declarations = snoc(ast->declarations, declaration, session->mempool);
        }
      else
        {
          // error recovery
          if (startDecl == session->token_stream->cursor())
            {
              // skip at least one token
              advance();
            }

          skipUntilDeclaration();
        }
    }

  clearComment();

  if (session->token_stream->lookAhead() != '}') {
    reportError(("} expected"));
    _M_hadMismatchingCompoundTokens = true;
  } else
    advance();

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseNamespace(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  bool inlined = false;
  if (session->token_stream->lookAhead() == Token_inline)
    {
      inlined = true;
      advance();
    }

  CHECK(Token_namespace);

  uint namespace_name = 0;
  if (session->token_stream->lookAhead() == Token_identifier)
    {
      namespace_name = session->token_stream->cursor();
      advance();
    }

  if (session->token_stream->lookAhead() == '=')
    {
      // namespace alias
      advance();

      NameAST *name = 0;
      if (parseName(name))
        {
          ADVANCE(';', ";");

          NamespaceAliasDefinitionAST *ast
            = CreateNode<NamespaceAliasDefinitionAST>(session->mempool);
          ast->namespace_name = namespace_name;
          ast->alias_name = name;
          UPDATE_POS(ast, start, _M_last_valid_token+1);

          node = ast;
          return true;
        }
      else
        {
          reportError(("Namespace expected"));
          return false;
        }
    }
  else if (session->token_stream->lookAhead() != '{')
    {
      reportError(("{ expected"));
      _M_hadMismatchingCompoundTokens = true;
      return false;
    }

  NamespaceAST *ast = CreateNode<NamespaceAST>(session->mempool);
  ast->namespace_name = namespace_name;
  ast->inlined = inlined;
  parseLinkageBody(ast->linkage_body);

  UPDATE_POS(ast, start, ast->linkage_body->end_token);
  node = ast;

  return true;
}

bool Parser::parseUsing(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_using);

  if (session->token_stream->lookAhead() == Token_namespace)
    return parseUsingDirective(node);

  uint type_name = 0;
  NameAST* name = 0;

  if (session->token_stream->lookAhead() == Token_typename)
    {
      type_name = session->token_stream->cursor();
      advance();
    }

  if (!parseName(name))
    return false;

  DeclarationAST* ast = 0;

  if (type_name || session->token_stream->lookAhead() == ';') {
    ADVANCE(';', ";");
    UsingAST *usingAst = CreateNode<UsingAST>(session->mempool);
    usingAst->type_name = type_name;
    usingAst->name = name;
    ast = usingAst;
  } else {
    ADVANCE('=', "=");
    TypeIdAST* type_id = 0;
    if (!parseTypeId(type_id)) {
      return false;
    }
    ADVANCE(';', ";");
    AliasDeclarationAST *aliasAST = CreateNode<AliasDeclarationAST>(session->mempool);
    aliasAST->name = name;
    aliasAST->type_id = type_id;
    ast = aliasAST;
  }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseUsingDirective(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_namespace);

  NameAST *name = 0;
  if (!parseName(name))
    {
      reportError(("Namespace name expected"));
      return false;
    }

  ADVANCE(';', ";");

  UsingDirectiveAST *ast = CreateNode<UsingDirectiveAST>(session->mempool);
  ast->name = name;
  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}


bool Parser::parseOperatorFunctionId(OperatorFunctionIdAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_operator);

  OperatorFunctionIdAST *ast = CreateNode<OperatorFunctionIdAST>(session->mempool);
  if (!parseOperator(ast->op))
    {
      ast->op = 0;

      // parse cast operator
      const ListNode<uint> *cv = 0;
      parseCvQualify(cv);

      if (!parseSimpleTypeSpecifier(ast->type_specifier))
        {
          syntaxError();
          return false;
        }

      parseCvQualify(cv);
      ast->type_specifier->cv = cv;

      PtrOperatorAST *ptr_op = 0;
      while (parsePtrOperator(ptr_op))
        ast->ptr_ops = snoc(ast->ptr_ops, ptr_op, session->mempool);
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;
  return true;
}

bool Parser::parseTemplateArgumentList(const ListNode<TemplateArgumentAST*> *&node,
                                       bool reportError)
{
  TemplateArgumentAST *templArg = 0;
  if (!parseTemplateArgument(templArg))
    return false;

  node = snoc(node, templArg, session->mempool);

  while (session->token_stream->lookAhead() == ',')
    {
      advance();

      if (!parseTemplateArgument(templArg))
        {
          if (reportError)
            {
              syntaxError();
              break;
            }

          node = 0;
          return false;
        }

      node = snoc(node, templArg, session->mempool);
    }

  return true;
}

bool Parser::parseTypedef(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();
  Comment mcomment = comment();

  CHECK(Token_typedef);

  TypeSpecifierAST *spec = 0;
  if (!parseTypeSpecifierOrClassSpec(spec))
    {
      reportError(("Need a type specifier to declare"));
      return false;
    }

  const ListNode<InitDeclaratorAST*> *declarators = 0;
  if (!parseInitDeclaratorList(declarators))
    {
      //reportError(("Need an identifier to declare"));
      //return false;
    }
  clearComment();

  TypedefAST *ast = CreateNode<TypedefAST>(session->mempool);

  if( mcomment )
      addComment( ast, mcomment );

  ADVANCE(';', ";");

  ast->type_specifier = spec;
  ast->init_declarators = declarators;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  preparseLineComments( ast->end_token-1 );

  if( m_commentStore.hasComment() )
    addComment( ast, m_commentStore.takeCommentInRange( lineFromTokenNumber( --ast->end_token ) ) );

  return true;
}

bool Parser::parseAsmDefinition(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  ADVANCE(Token_asm, "asm");

  const ListNode<uint> *cv = 0;
  parseCvQualify(cv);

  skip('(', ')');
  advance();
  ADVANCE(';', ";");

  AsmDefinitionAST *ast = CreateNode<AsmDefinitionAST>(session->mempool);
  ast->cv = cv;
  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTemplateDeclaration(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  uint exported = 0;
  if (session->token_stream->lookAhead() == Token_export)
    {
      exported = session->token_stream->cursor();
      advance();
    }

  CHECK(Token_template);

  const ListNode<TemplateParameterAST*> *params = 0;
  if (session->token_stream->lookAhead() == '<')
    {
      advance();
      parseTemplateParameterList(params);

      ADVANCE('>', ">");
    }

  DeclarationAST *declaration = 0;
  if (!parseDeclaration(declaration))
    {
      reportError(("Expected a declaration"));
    }

  TemplateDeclarationAST *ast = CreateNode<TemplateDeclarationAST>(session->mempool);
  ast->exported = exported;
  ast->template_parameters = params;
  ast->declaration = declaration;

  UPDATE_POS(ast, start, declaration ? declaration->end_token : _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseOperator(OperatorAST *&node)
{
  uint start = session->token_stream->cursor();

  OperatorAST *ast = CreateNode<OperatorAST>(session->mempool);

  switch(session->token_stream->lookAhead())
    {
    case Token_new:
    case Token_delete:
      {
        ast->op = session->token_stream->cursor();
        advance();

        if (session->token_stream->lookAhead() == '['
            && session->token_stream->lookAhead(1) == ']')
          {
            ast->open = session->token_stream->cursor();
            advance();

            ast->close = session->token_stream->cursor();
            advance();
          }
      }
      break;

    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '^':
    case '&':
    case '|':
    case '~':
    case '!':
    case '=':
    case '<':
    case '>':
    case ',':
    case Token_leftshift:
    case Token_rightshift:
    case Token_eq:
    case Token_not:
    case Token_not_eq:
    case Token_and_eq:
    case Token_or_eq:
    case Token_xor_eq:
    case Token_leftshift_eq:
    case Token_rightshift_eq:
    case Token_div_eq:
    case Token_star_eq:
    case Token_plus_eq:
    case Token_minus_eq:
    case Token_remainder_eq:
    case Token_leq:
    case Token_geq:
    case Token_and:
    case Token_or:
    case Token_incr:
    case Token_decr:
    case Token_ptrmem:
    case Token_arrow:
      ast->op = session->token_stream->cursor();
      advance();
      break;

    default:
      if (session->token_stream->lookAhead() == '('
          && session->token_stream->lookAhead(1) == ')')
        {
          ast->op = ast->open = session->token_stream->cursor();
          advance();
          ast->close = session->token_stream->cursor();
          advance();
        }
      else if (session->token_stream->lookAhead() == '['
               && session->token_stream->lookAhead(1) == ']')
        {
          ast->op = ast->open = session->token_stream->cursor();
          advance();
          ast->close = session->token_stream->cursor();
          advance();
        }
      else
        {
          return false;
        }
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseCvQualify(const ListNode<uint> *&node)
{
  uint start = session->token_stream->cursor();

  quint16 tk;
  while (0 != (tk = session->token_stream->lookAhead())
         && (tk == Token_const || tk == Token_volatile))
    {
      node = snoc(node, session->token_stream->cursor(), session->mempool);
      advance();
    }

  return start != session->token_stream->cursor();
}

bool Parser::parseRefQualifier(uint& ref_qualifier)
{
  const quint16 tk = session->token_stream->lookAhead();
  if (tk == '&' || tk == Token_bitand || tk == Token_and) {
    advance();
    ref_qualifier = session->token_stream->cursor();
    return true;
  }
  return false;
}

bool Parser::parseSimpleTypeSpecifier(TypeSpecifierAST *&node,
                                      bool onlyIntegral)
{
  uint start = session->token_stream->cursor();
  bool isIntegral = false;
  bool done = false;

  const ListNode<uint> *integrals = 0;

  while (!done)
    {
      switch(session->token_stream->lookAhead())
        {
        case Token_char:
        case Token_char16_t:
        case Token_char32_t:
        case Token_wchar_t:
        case Token_bool:
        case Token_short:
        case Token_int:
        case Token_long:
        case Token_signed:
        case Token_unsigned:
        case Token_float:
        case Token_double:
        case Token_void:
        case Token_auto:
          integrals = snoc(integrals, session->token_stream->cursor(), session->mempool);
          isIntegral = true;
          advance();
          break;

        default:
          done = true;
        }
    }

  SimpleTypeSpecifierAST *ast = 0;
  if (isIntegral)
    {
      ast = CreateNode<SimpleTypeSpecifierAST>(session->mempool);
      ast->integrals = integrals;
    }
  else if (session->token_stream->lookAhead() == Token___typeof)
    {
      ast = CreateNode<SimpleTypeSpecifierAST>(session->mempool);
      ast->isTypeof = true;
      advance();

      if (session->token_stream->lookAhead() == '(')
        {
          advance();

          uint saved = session->token_stream->cursor();
          if (!parseTypeId(ast->type_id) || session->token_stream->lookAhead() != ')')
            {
              ast->type_id = 0;
              ast->integrals = 0;
              rewind(saved);
              parseCommaExpression(ast->expression);
            }
          ADVANCE(')', ")");
        }
      else
        {
          parseUnaryExpression(ast->expression);
        }
    }
  else if (session->token_stream->lookAhead() == Token_decltype)
    {
      ast = CreateNode<SimpleTypeSpecifierAST>(session->mempool);
      ast->isDecltype = true;
      advance();
      ADVANCE('(', "(");
      parseExpression(ast->expression);
      ADVANCE(')', ")");
    }
  else if (onlyIntegral)
    {
      rewind(start);
      return false;
    }
  else
    {
      NameAST *name = 0;
      if (!parseName(name, AcceptTemplate))
        {
          rewind(start);
          return false;
        }
      ast = CreateNode<SimpleTypeSpecifierAST>(session->mempool);
      ast->name = name;
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parsePtrOperator(PtrOperatorAST *&node)
{
  int tk = session->token_stream->lookAhead();

  if (tk != '&' && tk != '*' && tk != Token_and
      && tk != Token_scope && tk != Token_identifier)
    {
      return false;
    }

  uint start = session->token_stream->cursor();

  PtrOperatorAST *ast = CreateNode<PtrOperatorAST>(session->mempool);
  switch (session->token_stream->lookAhead())
    {
    case '&':
    case '*':
    case Token_and:
      ast->op = session->token_stream->cursor();
      advance();
      break;

    case Token_scope:
    case Token_identifier:
      {
        if (!parsePtrToMember(ast->mem_ptr))
          {
            rewind(start);
            return false;
          }
      }
      break;

    default:
      Q_ASSERT(0);
      break;
    }

  parseCvQualify(ast->cv);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTemplateArgument(TemplateArgumentAST *&node)
{
  uint start = session->token_stream->cursor();

  TypeIdAST *typeId = 0;
  ExpressionAST *expr = 0;

  if (!parseTypeId(typeId) ||
       (session->token_stream->lookAhead() != ',' && session->token_stream->lookAhead() != '>' && session->token_stream->lookAhead() != ')'
         && session->token_stream->lookAhead() != Token_rightshift && session->token_stream->lookAhead() != Token_ellipsis ))
  {
    rewind(start);

    if (!parsePrimaryExpression(expr) ||
         (session->token_stream->lookAhead() != ',' && session->token_stream->lookAhead() != '>' && session->token_stream->lookAhead() != ')'
         && session->token_stream->lookAhead() != Token_rightshift && session->token_stream->lookAhead() != Token_ellipsis ))
    {
      rewind(start);

      if (!parseConditionalExpression(expr, true))
        return false;
    }
  }

  bool isVariadic = false;
  if (session->token_stream->lookAhead() == Token_ellipsis) {
    isVariadic = true;
    advance();
  }

  TemplateArgumentAST *ast = CreateNode<TemplateArgumentAST>(session->mempool);
  ast->type_id = typeId;
  ast->expression = expr;
  ast->isVariadic = isVariadic;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTypeSpecifier(TypeSpecifierAST *&node)
{
  ///TODO: Proper parsing of typename, this is some kind a hack, typename will be eaten, wrong typename-usages will be accepted, e.g. outside of templates. Please have a look at the C++-language-specification, if you want to fix it.
  if(session->token_stream->lookAhead() == Token_typename)
    session->token_stream->nextToken();
  ///</hack> ;)

  uint start = session->token_stream->cursor();

  const ListNode<uint> *cv = 0;
  parseCvQualify(cv);

  TypeSpecifierAST *ast = 0;
  if (!parseElaboratedTypeSpecifier(ast) && !parseSimpleTypeSpecifier(ast))
    {
      rewind(start);
      return false;
    }

  parseCvQualify(cv);
  ast->cv = cv;
  UPDATE_POS(ast, start, _M_last_valid_token+1);

  node = ast;

  return true;
}

bool Parser::parseDeclarator(DeclaratorAST*& node, bool allowBitfield)
{
  uint start = session->token_stream->cursor();

  DeclaratorAST *ast = CreateNode<DeclaratorAST>(session->mempool);
  DeclaratorAST *decl = 0;
  NameAST *declId = 0;

  PtrOperatorAST *ptrOp = 0;
  while (parsePtrOperator(ptrOp))
    {
      ast->ptr_ops = snoc(ast->ptr_ops, ptrOp, session->mempool);
    }

  if (session->token_stream->lookAhead() == '(')
    {
      advance();

      if (!parseDeclarator(decl))
        return false;

      ast->sub_declarator = decl;

      CHECK(')');
    }
  else
    {
      // bitfield can be unnamed
      if (!allowBitfield || session->token_stream->lookAhead() != ':')
        {
          if (session->token_stream->lookAhead() == Token_ellipsis)
            {
              advance();
              ast->isVariadic = true;
            }
          if (parseName(declId, AcceptTemplate))
            {
              ast->id = declId;
            }
          else
            {
              rewind(start);
              return false;
            }
        }

      if (allowBitfield && session->token_stream->lookAhead() == ':')
        {
          advance();

          if (!parseConstantExpression(ast->bit_expression))
            {
              reportError(("Constant expression expected"));
            }
          goto update_pos;
        }
    }

  {
    bool isVector = false;

    while (session->token_stream->lookAhead() == '[')
      {
        advance();

        ExpressionAST *expr = 0;
        parseCommaExpression(expr);

        ADVANCE(']', "]");

        ast->array_dimensions = snoc(ast->array_dimensions, expr, session->mempool);
        isVector = true;
      }

    bool skipParen = false;
    if (session->token_stream->lookAhead() == Token_identifier
        && session->token_stream->lookAhead(1) == '('
        && session->token_stream->lookAhead(2) == '(')
      {
        advance();
        advance();
        skipParen = true;
      }

    int tok = session->token_stream->lookAhead();
    if (ast->sub_declarator
        && !(isVector || tok == '(' || tok == ','
             || tok == ';' || tok == '='))
      {
        rewind(start);
        return false;
      }

    uint index = session->token_stream->cursor();
    if (session->token_stream->lookAhead() == '(')
      {
        advance();
        ///@todo Sometimes something like (test()) is parsed as a parameter declaration clause, although it cannot be one.
        ParameterDeclarationClauseAST *params = 0;
        if (!parseParameterDeclarationClause(params))
          {
            rewind(index);
            goto update_pos;
          }

        ast->parameter_declaration_clause = params;

        if (session->token_stream->lookAhead() != ')')
          {
            rewind(index);
            goto update_pos;
          }

        advance();  // skip ')'

        parseCvQualify(ast->fun_cv);
        parseRefQualifier(ast->ref_qualifier);
        parseExceptionSpecification(ast->exception_spec);

        // trailing return type support
        if (session->token_stream->lookAhead() == Token_arrow)
          {
            parseTrailingReturnType(ast->trailing_return_type);
          }

        parseMemberVirtSpecifier(ast->virt_specifiers);
      }

    if (skipParen)
      {
        if (session->token_stream->lookAhead() != ')')
          {
            reportError(("')' expected"));
          }
        else
          advance();
      }
  }

 update_pos:
  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseAbstractDeclarator(DeclaratorAST *&node)
{
  uint start = session->token_stream->cursor();

  DeclaratorAST *ast = CreateNode<DeclaratorAST>(session->mempool);
  DeclaratorAST *decl = 0;

  PtrOperatorAST *ptrOp = 0;
  while (parsePtrOperator(ptrOp))
    {
      ast->ptr_ops = snoc(ast->ptr_ops, ptrOp, session->mempool);
    }

  int index = session->token_stream->cursor();
  if (session->token_stream->lookAhead() == '(')
    {
      advance();

      if (!parseAbstractDeclarator(decl))
        {
          rewind(index);
          goto label1;
        }

      ast->sub_declarator = decl;

      if (session->token_stream->lookAhead() != ')')
        {
          rewind(start);
          return false;
        }
      advance();
    }
  else if (session->token_stream->lookAhead() == ':')
    {
      advance();
      if (!parseConstantExpression(ast->bit_expression))
        {
          ast->bit_expression = 0;
          reportError(("Constant expression expected"));
        }
      goto update_pos;
    }
  else if (session->token_stream->lookAhead() == Token_ellipsis)
    {
      advance();
      ast->isVariadic = true;
      goto update_pos;
    }

 label1:
  {
    bool isVector = false;

    while (session->token_stream->lookAhead() == '[')
      {
        advance();

        ExpressionAST *expr = 0;
        parseCommaExpression(expr);

        ADVANCE(']', "]");

        ast->array_dimensions = snoc(ast->array_dimensions, expr, session->mempool);
        isVector = true;
      }

    int tok = session->token_stream->lookAhead();
    if (ast->sub_declarator
        && !(isVector || tok == '(' || tok == ','
             || tok == ';' || tok == '='))
      {
        rewind(start);
        return false;
      }

    int index = session->token_stream->cursor();
    if (session->token_stream->lookAhead() == '(')
      {
        advance();

        ParameterDeclarationClauseAST *params = 0;
        if (!parseParameterDeclarationClause(params))
          {
            rewind(index);
            goto update_pos;
          }

        ast->parameter_declaration_clause = params;

        if (session->token_stream->lookAhead() != ')')
          {
            rewind(index);
            goto update_pos;
          }

        advance();  // skip ')'

        parseCvQualify(ast->fun_cv);
        parseExceptionSpecification(ast->exception_spec);
      }
  }

 update_pos:
  if (session->token_stream->cursor() == start)
    return false;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseEnumSpecifier(TypeSpecifierAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_enum);

  bool isClass = session->token_stream->lookAhead()==Token_class || session->token_stream->lookAhead()==Token_struct;
  if(isClass) {
    advance();
  }

  NameAST *name = 0;
  parseName(name);

  TypeSpecifierAST *type = 0;
  if(session->token_stream->lookAhead() == ':') {
    advance();
    if(!parseTypeSpecifier(type)) {
      rewind(start);
      return false;
    }
  }

  EnumSpecifierAST *ast = CreateNode<EnumSpecifierAST>(session->mempool);
  ast->name = name;
  ast->type = type;
  ast->isClass = isClass;

  if (session->token_stream->lookAhead() == '{')
    {
      advance();

      ast->isOpaque = false;

      EnumeratorAST *enumerator = 0;
      if (parseEnumerator(enumerator))
        {

          ast->enumerators = snoc(ast->enumerators, enumerator, session->mempool);

          while (session->token_stream->lookAhead() == ',')
            {
              advance();

              if (!parseEnumerator(enumerator))
                {
                  break;
                }

              ast->enumerators = snoc(ast->enumerators, enumerator, session->mempool);
            }
        }

      clearComment();

      ADVANCE_NR('}', "}");
    }
  else if (session->token_stream->lookAhead() == ';')
    {
      ast->isOpaque = true;
    }
  else
    {
      rewind(start);
      return false;
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTemplateParameterList(const ListNode<TemplateParameterAST*> *&node)
{
  TemplateParameterAST *param = 0;
  if (!parseTemplateParameter(param))
    return false;

  node = snoc(node, param, session->mempool);

  while (session->token_stream->lookAhead() == ',')
    {
      advance();

      if (!parseTemplateParameter(param))
        {
          syntaxError();
          break;
        }
      else
        {
          node = snoc(node, param, session->mempool);
        }
    }

  return true;
}

bool Parser::parseTemplateParameter(TemplateParameterAST *&node)
{
  uint start = session->token_stream->cursor();
  TemplateParameterAST *ast = CreateNode<TemplateParameterAST>(session->mempool);

  int tk = session->token_stream->lookAhead();

  if ((tk == Token_class || tk == Token_typename || tk == Token_template)
      && parseTypeParameter(ast->type_parameter))
    {
      // nothing to do
    }
  else if (!parseParameterDeclaration(ast->parameter_declaration))
    return false;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTypeParameter(TypeParameterAST *&node)
{
  uint start = session->token_stream->cursor();

  TypeParameterAST *ast = CreateNode<TypeParameterAST>(session->mempool);
  ast->type = start;

  switch(session->token_stream->lookAhead())
    {
    case Token_class:
    case Token_typename:
      {
        advance(); // skip class

        if(session->token_stream->lookAhead() == Token_ellipsis)
          {
            advance();
            ast->isVariadic = true;
          }

        // parse optional name
        parseName(ast->name, AcceptTemplate);

        if (session->token_stream->lookAhead() == '=')
          {
            advance();

            if(!parseTypeId(ast->type_id))
              {
                //syntaxError();
                rewind(start);
                return false;
              }
          }
        else if (session->token_stream->lookAhead() != ','
                  && session->token_stream->lookAhead() != '>') ///TODO: right-shift also OK? see spec 14.2/3
          {
            rewind(start);
            return false;
          }
      }
      break;

    case Token_template:
      {
        advance(); // skip template
        ADVANCE('<', "<");

        if (!parseTemplateParameterList(ast->template_parameters))
          return false;

        ADVANCE('>', ">");

        // TODO add to AST
        if (session->token_stream->lookAhead() == Token_class)
          advance();

        if (session->token_stream->lookAhead() == Token_ellipsis) {
          advance();
          ast->isVariadic = true;
        }

        // parse optional name
        if (parseName(ast->name, AcceptTemplate))
          {
            if (session->token_stream->lookAhead() == '=')
              {
                advance();

                if (!parseTypeId(ast->type_id))
                  {
                    syntaxError();
                    return false;
                  }
              }
          }

        if (session->token_stream->lookAhead() == '=')
          {
            advance();

            parseName(ast->template_name, AcceptTemplate);
          }
      }
      break;

    default:
      return false;

    } // end switch


  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;
  return true;
}

///TODO: the spec has a decl-specifier, friend and constexpr e.g. are no storage class specifiers!
bool Parser::parseStorageClassSpecifier(const ListNode<uint> *&node)
{
  uint start = session->token_stream->cursor();

  int tk;
  while (0 != (tk = session->token_stream->lookAhead())
         && (tk == Token_friend
             || tk == Token_register || tk == Token_static
             || tk == Token_extern || tk == Token_mutable || tk == Token_thread_local
             || tk == Token_constexpr))
    {
      node = snoc(node, session->token_stream->cursor(), session->mempool);
      advance();
    }

  return start != session->token_stream->cursor();
}

bool Parser::parseFunctionSpecifier(const ListNode<uint> *&node)
{
  uint start = session->token_stream->cursor();

  int tk;
  while (0 != (tk = session->token_stream->lookAhead())
         && (tk == Token_inline || tk == Token_virtual
             || tk == Token_explicit))
    {
      node = snoc(node, session->token_stream->cursor(), session->mempool);
      advance();
    }

  return start != session->token_stream->cursor();
}

bool Parser::parseTypeId(TypeIdAST *&node)
{
  uint start = session->token_stream->cursor();

  TypeSpecifierAST *spec = 0;
  if (!parseTypeSpecifier(spec))
    {
      rewind(start);
      return false;
    }

  DeclaratorAST *decl = 0;
  parseAbstractDeclarator(decl);

  TypeIdAST *ast = CreateNode<TypeIdAST>(session->mempool);
  ast->type_specifier = spec;
  ast->declarator = decl;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseInitDeclaratorList(const ListNode<InitDeclaratorAST*> *&node)
{
  InitDeclaratorAST *decl = 0;
  if (!parseInitDeclarator(decl))
    return false;

  node = snoc(node, decl, session->mempool);

  while (session->token_stream->lookAhead() == ',')
    {
      advance();

      if (!parseInitDeclarator(decl))
        {
          syntaxError();
          break;
        }
      node = snoc(node, decl, session->mempool);
    }

  return true;
}

bool Parser::parseParameterDeclarationClause(ParameterDeclarationClauseAST *&node)
{
  uint start = session->token_stream->cursor();

  ParameterDeclarationClauseAST *ast
    = CreateNode<ParameterDeclarationClauseAST>(session->mempool);

  if (!parseParameterDeclarationList(ast->parameter_declarations))
    {
      if (session->token_stream->lookAhead() == ')')
        goto good;

      if (session->token_stream->lookAhead() == Token_ellipsis
          && session->token_stream->lookAhead(1) == ')')
        {
          ast->ellipsis = session->token_stream->cursor();
          goto good;
        }

      return false;
    }

 good:

  if (session->token_stream->lookAhead() == Token_ellipsis)
    {
      ast->ellipsis = session->token_stream->cursor();
      advance();
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseParameterDeclarationList(const ListNode<ParameterDeclarationAST*> *&node)
{
  uint start = session->token_stream->cursor();

  ParameterDeclarationAST *param = 0;
  if (!parseParameterDeclaration(param))
    {
      rewind(start);
      return false;
    }

  node = snoc(node, param, session->mempool);

  while (session->token_stream->lookAhead() == ',')
    {
      advance();

      if (!parseParameterDeclaration(param))
        {
          rewind(start);
          return false;
        }
      node = snoc(node, param, session->mempool);
    }

  return true;
}

bool Parser::parseParameterDeclaration(ParameterDeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  const ListNode<uint> *storage = 0;
  parseStorageClassSpecifier(storage);

  // parse decl spec
  TypeSpecifierAST *spec = 0;
  if (!parseTypeSpecifier(spec))
    {
      rewind(start);
      return false;
    }

  int index = session->token_stream->cursor();

  DeclaratorAST *decl = 0;
  if (!parseDeclarator(decl))
    {
      rewind(index);

      // try with abstract declarator
      parseAbstractDeclarator(decl);
    }

  ExpressionAST *expr = 0;
  if (session->token_stream->lookAhead() == '=')
    {
      advance();
      //FIXME: this should actually be an InitializerClause as per http://www.nongnu.org/hcb/#parameter-declaration
      parseLogicalOrExpression(expr, true) || parseBracedInitList(expr);
    }

  if( session->token_stream->lookAhead() != ',' && session->token_stream->lookAhead() != ')' && session->token_stream->lookAhead() != '>' )
  {
    //Not a valid parameter declaration
    rewind(start);
    return false;
  }

  ParameterDeclarationAST *ast = CreateNode<ParameterDeclarationAST>(session->mempool);
  ast->type_specifier = spec;
  ast->declarator = decl;
  ast->expression = expr;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseClassSpecifier(TypeSpecifierAST *&node)
{
  uint start = session->token_stream->cursor();

  int kind = session->token_stream->lookAhead();
  if (kind != Token_class && kind != Token_struct && kind != Token_union)
    return false;

  uint class_key = session->token_stream->cursor();
  advance();

  WinDeclSpecAST *winDeclSpec = 0;
  parseWinDeclSpec(winDeclSpec);

  while (session->token_stream->lookAhead() == Token_identifier
         && session->token_stream->lookAhead(1) == Token_identifier)
    {
      advance();
    }

  NameAST *name = 0;
  parseName(name, AcceptTemplate);

  const ListNode<uint> *virt_specifiers = 0;
  parseClassVirtSpecifier(virt_specifiers);

  BaseClauseAST *bases = 0;
  if (session->token_stream->lookAhead() == ':')
    {
      if (!parseBaseClause(bases))
        {
          skipUntil('{');
        }
    }

  if (session->token_stream->lookAhead() != '{')
    {
      rewind(start);
      return false;
    }

  ADVANCE('{', "{");

  ClassSpecifierAST *ast = CreateNode<ClassSpecifierAST>(session->mempool);
  ast->win_decl_specifiers = winDeclSpec;
  ast->class_key = class_key;
  ast->name = name;
  ast->base_clause = bases;

  while (session->token_stream->lookAhead())
    {
      if (session->token_stream->lookAhead() == '}')
        break;

      uint startDecl = session->token_stream->cursor();

      DeclarationAST *memSpec = 0;
      if (!parseMemberSpecification(memSpec))
        {
          if (startDecl == session->token_stream->cursor())
            advance(); // skip at least one token
          skipUntilDeclaration();
        }
      else
        ast->member_specs = snoc(ast->member_specs, memSpec, session->mempool);
    }

  clearComment();

  ADVANCE_NR('}', "}");

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseAccessSpecifier(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  const ListNode<uint> *specs = 0;

  bool done = false;
  while (!done)
    {
      switch(session->token_stream->lookAhead())
        {
        case Token___qt_signals__:
        case Token___qt_slots__:
        case Token_k_dcop:
        case Token_k_dcop_signals:
        case Token_public:
        case Token_protected:
        case Token_private:
          specs = snoc(specs, session->token_stream->cursor(), session->mempool);
          advance();
          break;

        default:
          done = true;
          break;
        }
    }

  if (!specs)
    return false;

  ADVANCE(':', ":");

  AccessSpecifierAST *ast = CreateNode<AccessSpecifierAST>(session->mempool);
  ast->specs = specs;
  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseMemberSpecification(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  if (session->token_stream->lookAhead() == ';')
    {
      advance();
      return true;
    }
  else if (session->token_stream->lookAhead() == Token_Q_OBJECT || session->token_stream->lookAhead() == Token_K_DCOP)
    {
      advance();
      return true;
    }
  else if (parseTypedef(node))
    {
      return true;
    }
  else if (parseUsing(node))
    {
      return true;
    }
  else if (parseTemplateDeclaration(node))
    {
      return true;
    }
  else if (parseAccessSpecifier(node))
    {
      return true;
    }
  else if (parseQProperty(node))
    {
      return true;
    }
  else if (parseStaticAssert(node))
    {
      return true;
    }

  rewind(start);

  const ListNode<uint> *cv = 0;
  parseCvQualify(cv);

  const ListNode<uint> *storageSpec = 0;
  parseStorageClassSpecifier(storageSpec);

  parseCvQualify(cv);

  Comment mcomment = comment();
  clearComment();

  TypeSpecifierAST *spec = 0;
  if (parseEnumSpecifier(spec) || parseClassSpecifier(spec))
    {
      parseCvQualify(cv);
      spec->cv = cv;

      const ListNode<InitDeclaratorAST*> *declarators = 0;
      parseInitDeclaratorList(declarators);
      ADVANCE(';', ";");

      SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(session->mempool);
      ast->storage_specifiers = storageSpec;
      ast->type_specifier = spec;
      ast->init_declarators = declarators;
      UPDATE_POS(ast, start, _M_last_valid_token+1);

      if( mcomment )
        addComment(ast,mcomment);

      preparseLineComments(ast->end_token-1);

      if( m_commentStore.hasComment() )
        addComment( ast, m_commentStore.takeCommentInRange( lineFromTokenNumber( --ast->end_token ) ) );

      node = ast;


      return true;
    } else {
      rewind(start);

      if( parseDeclarationInternal(node) ) {
        //Add the comments to the declaration
        if( mcomment )
          addComment(node, mcomment);

        preparseLineComments(node->end_token-1);

        if( m_commentStore.hasComment() )
          addComment( node, m_commentStore.takeCommentInRange( lineFromTokenNumber( --node->end_token ) ) );

        return true;
      }
    }
    return false;
}

bool Parser::parseCtorInitializer(CtorInitializerAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(':');

  CtorInitializerAST *ast = CreateNode<CtorInitializerAST>(session->mempool);
  ast->colon = start;

  if (!parseMemInitializerList(ast->member_initializers))
    {
      reportError(("Member initializers expected"));
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseElaboratedTypeSpecifier(TypeSpecifierAST *&node)
{
  uint start = session->token_stream->cursor();

  int tk = session->token_stream->lookAhead();
  if (tk == Token_class  ||
      tk == Token_struct ||
      tk == Token_union  ||
      tk == Token_enum   ||
      tk == Token_typename)
    {
      uint type = session->token_stream->cursor();
      advance();

      NameAST *name = 0;
      if (parseName(name, AcceptTemplate))
        {
          ElaboratedTypeSpecifierAST *ast
            = CreateNode<ElaboratedTypeSpecifierAST>(session->mempool);

          ast->type = type;
          ast->name = name;

          UPDATE_POS(ast, start, _M_last_valid_token+1);
          node = ast;

          return true;
        }
    }

  rewind(start);
  return false;
}

bool Parser::parseExceptionSpecification(ExceptionSpecificationAST *&node)
{
  uint start = session->token_stream->cursor();

  if (session->token_stream->lookAhead() == Token_throw)
    {
      advance();

      ADVANCE('(', "(");

      ExceptionSpecificationAST *ast
        = CreateNode<ExceptionSpecificationAST>(session->mempool);

      if (session->token_stream->lookAhead() == Token_ellipsis)
        {
          ast->ellipsis = session->token_stream->cursor();
          advance();
        }

      parseTypeIdList(ast->type_ids);

      if (!ast->ellipsis && session->token_stream->lookAhead() == Token_ellipsis)
        {
          ast->ellipsis = session->token_stream->cursor();
          advance();
        }


      ADVANCE(')', ")");

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;

      return true;

    }

  else if (session->token_stream->lookAhead() == Token_noexcept)
    {
      ExceptionSpecificationAST *ast
        = CreateNode<ExceptionSpecificationAST>(session->mempool);

      ast->no_except = session->token_stream->cursor();
      advance();

      if (session->token_stream->lookAhead() == '(')
        {
          advance();
          parseExpression(ast->noexcept_expression);
          CHECK(')');
        }

      UPDATE_POS (ast, start, _M_last_valid_token+1);
      node = ast;

      return true;
    }

  return false;
}

bool Parser::parseEnumerator(EnumeratorAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_identifier);
  uint id = start;

  EnumeratorAST *ast = CreateNode<EnumeratorAST>(session->mempool);
  ast->id = id;


  if (session->token_stream->lookAhead() == '=')
    {
      advance();
      if (!parseConstantExpression(ast->expression))
        {
          reportError(("Constant expression expected"));
        }
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  moveComments(node);

  preparseLineComments( ast->end_token-1 );

  if( m_commentStore.hasComment() )
    addComment( node, m_commentStore.takeCommentInRange( lineFromTokenNumber(--ast->end_token) ) );

  return true;
}

bool Parser::parseInitDeclarator(InitDeclaratorAST *&node)
{
  uint start = session->token_stream->cursor();

  DeclaratorAST *decl = 0;
  if (!parseDeclarator(decl))
    {
      return false;
    }

  if (session->token_stream->lookAhead(0) == Token_asm)
    {
      advance();
      skip('(', ')');
      advance();
    }

  InitializerAST *init = 0;
  parseInitializer(init);

  InitDeclaratorAST *ast = CreateNode<InitDeclaratorAST>(session->mempool);
  ast->declarator = decl;
  ast->initializer = init;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseBaseClause(BaseClauseAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(':');

  BaseSpecifierAST *baseSpec = 0;
  if (!parseBaseSpecifier(baseSpec))
    return false;

  BaseClauseAST *ast = CreateNode<BaseClauseAST>(session->mempool);
  ast->base_specifiers = snoc(ast->base_specifiers, baseSpec, session->mempool);

  while (session->token_stream->lookAhead() == ',')
    {
      advance();

      if (!parseBaseSpecifier(baseSpec))
        {
          reportError(("Base class specifier expected"));
          break;
        }
      ast->base_specifiers = snoc(ast->base_specifiers, baseSpec, session->mempool);
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseInitializer(InitializerAST *&node)
{
  uint start = session->token_stream->cursor();

  int tk = session->token_stream->lookAhead();

  InitializerAST *ast = CreateNode<InitializerAST>(session->mempool);

  if (tk == '=')
    {
      advance();

      if (!parseInitializerClause(ast->initializer_clause))
        {
          rewind(start);
          return false;
        }
    }
  else if (tk == '(')
    {
      advance();
      parseExpressionList(ast->expression);
      CHECK(')');
    }
  else if (!parseBracedInitList(ast->expression))
    {
      rewind(start);
      return false;
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseMemInitializerList(const ListNode<MemInitializerAST*> *&node)
{
  MemInitializerAST *init = 0;

  if (!parseMemInitializer(init))
    return false;

  node = snoc(node, init, session->mempool);

  while (session->token_stream->lookAhead() == ',')
    {
      advance();

      if (!parseMemInitializer(init))
        break;

      node = snoc(node, init, session->mempool);
    }

  return true;
}

bool Parser::parseMemInitializer(MemInitializerAST *&node)
{
  uint start = session->token_stream->cursor();

  NameAST *initId = 0;
  if (!parseName(initId, AcceptTemplate))
    {
      reportError(("Identifier expected"));
      return false;
    }

  ExpressionAST *expr = 0;
  bool expressionIsVariadic = false;

  if (session->token_stream->lookAhead() == '(')
    {
      advance();
      parseExpressionList(expr);
      if (session->token_stream->lookAhead() == Token_ellipsis)
        {
          advance();
          expressionIsVariadic = true;
        }
      ADVANCE(')', ")");
    }
  else
    {
      parseBracedInitList(expr);
    }

  bool initializerIsVariadic = false;
  if (session->token_stream->lookAhead() == Token_ellipsis)
    {
      advance();
      initializerIsVariadic = true;
    }

  MemInitializerAST *ast = CreateNode<MemInitializerAST>(session->mempool);
  ast->initializer_id = initId;
  ast->expression = expr;
  ast->initializerIsVariadic = initializerIsVariadic;
  ast->expressionIsVariadic = expressionIsVariadic;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTypeIdList(const ListNode<TypeIdAST*> *&node)
{
  TypeIdAST *typeId = 0;
  if (!parseTypeId(typeId))
    return false;

  node = snoc(node, typeId, session->mempool);

  while (session->token_stream->lookAhead() == ',')
    {
      advance();
      if (parseTypeId(typeId))
        {
          node = snoc(node, typeId, session->mempool);
        }
      else
        {
          reportError(("Type id expected"));
          break;
        }
    }

  return true;
}

bool Parser::parseBaseSpecifier(BaseSpecifierAST *&node)
{
  uint start = session->token_stream->cursor();

  BaseSpecifierAST *ast = CreateNode<BaseSpecifierAST>(session->mempool);

  if (session->token_stream->lookAhead() == Token_virtual)
    {
      ast->virt = session->token_stream->cursor();
      advance();

      int tk = session->token_stream->lookAhead();
      if (tk == Token_public || tk == Token_protected
          || tk == Token_private)
        {
          ast->access_specifier = session->token_stream->cursor();
          advance();
        }
    }
  else
    {
      int tk = session->token_stream->lookAhead();
      if (tk == Token_public || tk == Token_protected
          || tk == Token_private)
        {
          ast->access_specifier = session->token_stream->cursor();
          advance();
        }

      if (session->token_stream->lookAhead() == Token_virtual)
        {
          ast->virt = session->token_stream->cursor();
          advance();
        }
    }

  if (!parseName(ast->name, AcceptTemplate))
    reportError(("Class name expected"));

  if (session->token_stream->lookAhead() == Token_ellipsis)
    {
      advance();
      ast->isVariadic = true;
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseBracedInitList(ExpressionAST*& node)
{
  uint start = session->token_stream->cursor();

  CHECK('{');
  InitializerListAST *list = 0;
  parseInitializerList(list);

  CHECK('}');

  BracedInitListAST *ast = CreateNode<BracedInitListAST>(session->mempool);
  ast->list = list;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseInitializerList(InitializerListAST *&node)
{
  uint start = session->token_stream->cursor();

  const ListNode<InitializerClauseAST*> *clauses = 0;
  do
    {
      if (clauses)
        {
          advance(); // skip ',' separator between clauses

          if (session->token_stream->lookAhead() == '}')
            {
              // see https://bugs.kde.org/show_bug.cgi?id=233328
              // and grammar spec on braced-init-list
              // init lists may have a trailing comma
              break;
            }
        }

      InitializerClauseAST *init_clause = 0;
      if (!parseInitializerClause(init_clause) && !parseDesignatedInitializer(init_clause))
        {
          return false;
        }
      clauses = snoc(clauses,init_clause,session->mempool);
    } while (session->token_stream->lookAhead() == ',');

  InitializerListAST *list = CreateNode<InitializerListAST>(session->mempool);
  list->clauses = clauses;

  if (session->token_stream->lookAhead() == Token_ellipsis)
    {
      advance();
      list->isVariadic = true;
    }

  UPDATE_POS(list, start, _M_last_valid_token+1);
  node = list;

  return true;
}

/**
 * Parse C99 designated initializers, e.g:
 * struct foo_t foo = {
 *   .has_cake = true,
 *   .nb_candles = 12,
 * };
 *
 * int bar[10] = {
 *   [1] = 15,
 *   [9] = 25,
 * };
 */
bool Parser::parseDesignatedInitializer(InitializerClauseAST *&ast)
{
  uint start = session->token_stream->cursor();
  uint memberOp = session->token_stream->cursor();

  NameAST *member = 0;
  ExpressionAST *index = 0;
  const ListNode<ExpressionAST*> *indexes = 0;
  if (session->token_stream->lookAhead() == '.')
    {
    // Designated member
    advance();
    if (!parseName(member, Parser::DontAcceptTemplate))
      {
        rewind(start);
        return false;
      }
    }
  else
    {
    do
      {
        // Designated array index
        if (session->token_stream->lookAhead() != '[' || !parsePostfixExpressionInternal(index))
          {
            rewind(start);
            return false;
          }

        indexes = snoc(indexes, index, session->mempool);
      } while (session->token_stream->lookAhead() == '[');
    }

  uint cm_end = session->token_stream->cursor();

  uint asop = session->token_stream->cursor();
  if (session->token_stream->lookAhead() != '=')
    {
      rewind(start);
      return false;
    }
  advance();

  ExpressionAST *rightExpr = 0;
  if (!parseConditionalExpression(rightExpr) && !parseBracedInitList(rightExpr))
    {
      rewind(start);
      return false;
    }

  BinaryExpressionAST *binaryExpr = CreateNode<BinaryExpressionAST>(session->mempool);
  binaryExpr->op = asop;
  binaryExpr->right_expression = rightExpr;

  if (member) {
    ClassMemberAccessAST *memberAst = CreateNode<ClassMemberAccessAST>(session->mempool);
    memberAst->op = memberOp;
    memberAst->name = member;
    UPDATE_POS(memberAst, start, cm_end);
    binaryExpr->left_expression = memberAst;
  } else {
    PostfixExpressionAST *sub = CreateNode<PostfixExpressionAST>(session->mempool);
    sub->sub_expressions = indexes;
    UPDATE_POS(sub, start, cm_end);
    binaryExpr->left_expression = sub;
  }

  UPDATE_POS(binaryExpr, start, _M_last_valid_token+1);

  ast = CreateNode<InitializerClauseAST>(session->mempool);
  ast->expression = binaryExpr;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  return true;
}

bool Parser::parseInitializerClause(InitializerClauseAST *&node)
{
  uint start = session->token_stream->cursor();

  ExpressionAST *expr = 0;
  // assignment or braced-init-list
  parseAssignmentExpression(expr) || parseBracedInitList(expr);

  if (!expr) {
    rewind(start);
    return false;
  }

  InitializerClauseAST *ast = CreateNode<InitializerClauseAST>(session->mempool);
  ast->expression = expr;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parsePtrToMember(PtrToMemberAST *&node)
{
  uint start = session->token_stream->cursor();

  TypeSpecifierAST* type_ast = 0;

  if(parseTypeSpecifier(type_ast)){
      if (session->token_stream->lookAhead() == Token_scope
          && session->token_stream->lookAhead(1) == '*')
        {
          advance();
          advance();

          PtrToMemberAST *ast = CreateNode<PtrToMemberAST>(session->mempool);
      ast->class_type=type_ast;
          UPDATE_POS(ast, start, _M_last_valid_token+1);
          node = ast;
          return true;
        }
    }

  rewind(start);
  return false;
}

bool Parser::parseUnqualifiedName(UnqualifiedNameAST *&node,
                                  bool parseTemplateId)
{
  uint start = session->token_stream->cursor();

  bool tilde = false;
  uint id = 0;
  bool ellipsis = false;
  OperatorFunctionIdAST *operator_id = 0;

  if (session->token_stream->lookAhead() == Token_identifier
       // identifier with special meaning
      || session->token_stream->lookAhead() == Token_override
      || session->token_stream->lookAhead() == Token_final)
    {
      id = session->token_stream->cursor();
      advance();
    }
  else if (session->token_stream->lookAhead() == '~'
           && session->token_stream->lookAhead(1) == Token_identifier)
    {
      tilde = true;
      advance(); // skip ~

      id = session->token_stream->cursor();
      advance(); // skip classname
    }
  else if (session->token_stream->lookAhead() == Token_operator)
    {
      if (!parseOperatorFunctionId(operator_id))
        return false;
    }
  else if (session->token_stream->lookAhead() == Token_ellipsis)
    {
      ellipsis = true;
      advance();
    }
  else
    {
      return false;
    }

  UnqualifiedNameAST *ast = CreateNode<UnqualifiedNameAST>(session->mempool);
  ast->tilde = tilde;
  ast->id = id;
  ast->ellipsis = ellipsis;
  ast->operator_id = operator_id;

  if (parseTemplateId)
    {
      uint index = session->token_stream->cursor();

      if (session->token_stream->lookAhead() == '<' && !(tokenMarkers(index) & IsNoTemplateArgumentList))
        {
          advance();

          // optional template arguments
          parseTemplateArgumentList(ast->template_arguments);

          if (session->token_stream->lookAhead() == '>')
            {
              advance();
            }
          else if (session->token_stream->lookAhead() == Token_rightshift)
            {
              session->token_stream->splitRightShift(session->token_stream->cursor());
              advance();
            }
          else
            {
              addTokenMarkers(index, IsNoTemplateArgumentList);
              ast->template_arguments = 0;
              rewind(index);
            }
        }
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseStringLiteral(StringLiteralAST *&node)
{
  uint start = session->token_stream->cursor();

  if (session->token_stream->lookAhead() != Token_string_literal)
    return false;

  StringLiteralAST *ast = CreateNode<StringLiteralAST>(session->mempool);

  while (session->token_stream->lookAhead() == Token_string_literal)
    {
      ast->literals = snoc(ast->literals, session->token_stream->cursor(), session->mempool);
      advance();
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseExpressionStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  ExpressionAST *expr = 0;
  parseCommaExpression(expr);

  ADVANCE(';', ";");

  ExpressionStatementAST *ast = CreateNode<ExpressionStatementAST>(session->mempool);
  ast->expression = expr;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;
  return true;
}

bool Parser::parseJumpStatement(StatementAST *&node)
{
  uint op = session->token_stream->cursor();
  uint kind = session->token_stream->lookAhead();
  uint identifier = 0;

  if (kind != Token_break && kind != Token_continue && kind != Token_goto)
      return false;

  advance();
  if (kind == Token_goto)
    {
      ADVANCE(Token_identifier,"label");
      identifier = op+1;
    }
  ADVANCE(';',";");

  JumpStatementAST* ast = CreateNode<JumpStatementAST>(session->mempool);
  ast->op = op;
  ast->identifier = identifier;

  UPDATE_POS(ast,ast->op,_M_last_valid_token+1);
  node = ast;
  return true;
}

bool Parser::parseStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  switch(session->token_stream->lookAhead())
    {
    case Token_while:
      return parseWhileStatement(node);

    case Token_do:
      return parseDoStatement(node);

    case Token_for:
      return parseForStatement(node);

    case Token_if:
      return parseIfStatement(node);

    case Token_switch:
      return parseSwitchStatement(node);

    case Token_try:
      return parseTryBlockStatement(node);

    case Token_case:
    case Token_default:
      return parseLabeledStatement(node);

    case Token_break:
    case Token_continue:
    case Token_goto:
      return parseJumpStatement(node);

    case Token_return:
      {
        ///TODO: cleanup and put this into parseJumpStatement to follow the spec
        advance();
        ExpressionAST *expr = 0;
        parseCommaExpression(expr) || parseBracedInitList(expr);

        ADVANCE(';', ";");

        ReturnStatementAST *ast = CreateNode<ReturnStatementAST>(session->mempool);
        ast->expression = expr;

        UPDATE_POS(ast, start, _M_last_valid_token+1);

        node = ast;
      }
      return true;

    case '{':
      return parseCompoundStatement(node);

    case Token_identifier:
      if (parseLabeledStatement(node))
        return true;
      break;
    }

  return parseExpressionOrDeclarationStatement(node);
}

//since advance includes comments, this checks for both, ; or ; //
bool isValidExprOrDeclEnd(ParseSession* session) {
  int prev = session->token_stream->kind(session->token_stream->cursor() - 1);
  if (prev == Token_comment) {
    prev = session->token_stream->kind(session->token_stream->cursor() - 2);
  }
  return prev == ';';
}

bool Parser::parseExpressionOrDeclarationStatement(StatementAST *&node)
{
  // hold any errors while the expression/declaration ambiguity is resolved
  // for this statement
  bool hold = holdErrors(true);

  uint start = session->token_stream->cursor();

  ///@todo solve -1 thing
  StatementAST *decl_ast = 0;
  bool maybe_amb = parseDeclarationStatement(decl_ast);
  maybe_amb &= isValidExprOrDeclEnd(session);
  maybe_amb &= isValidExprOrDeclEnd(session);

  // if parsing as a declaration succeeded, then any pending errors are genuine.
  // Otherwise this is not a declaration so ignore the errors.
  if (decl_ast)
      reportPendingErrors();
  else
      m_pendingErrors.clear();

  uint end = session->token_stream->cursor();

  rewind(start);
  StatementAST *expr_ast = 0;
  maybe_amb &= parseExpressionStatement(expr_ast);
  maybe_amb &= isValidExprOrDeclEnd(session);

  // if parsing as an expression succeeded, then any pending errors are genuine.
  // Otherwise this is not an expression so ignore the errors.
  if (expr_ast)
      reportPendingErrors();
  else
      m_pendingErrors.clear();

  if (maybe_amb)
    {
      Q_ASSERT(decl_ast != 0 && expr_ast != 0);
      ExpressionOrDeclarationStatementAST *ast
        = CreateNode<ExpressionOrDeclarationStatementAST>(session->mempool);
      ast->declaration = decl_ast;
      ast->expression = expr_ast;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }
  else
    {
      rewind(std::max(end, session->token_stream->cursor()));

      node = decl_ast;
      if (!node)
        node = expr_ast;
    }

  holdErrors(hold);

  if (!node)
    syntaxError();

  return node != 0;
}

bool Parser::parseCondition(ConditionAST *&node, bool initRequired)
{
  uint start = session->token_stream->cursor();

  ConditionAST *ast = CreateNode<ConditionAST>(session->mempool);
  TypeSpecifierAST *spec = 0;

  if (parseTypeSpecifier(spec))
    {
      ast->type_specifier = spec;

      uint declarator_start = session->token_stream->cursor();

      DeclaratorAST *decl = 0;
      if (!parseDeclarator(decl))
        {
          rewind(declarator_start);
          if (!initRequired && !parseAbstractDeclarator(decl))
            decl = 0;
        }

      if (decl)
        {
          ExpressionAST* expression = 0;
          if (session->token_stream->lookAhead() == '=')
            {
              advance();

              parseExpression(expression);
            }
          else
            {
              parseBracedInitList(expression);
            }

          if (expression || !initRequired)
            {
              ast->declarator = decl;
              ast->expression = expression;

              UPDATE_POS(ast, start, _M_last_valid_token+1);
              node = ast;
              return true;
            }
        }
    }

  ast->type_specifier = 0;

  rewind(start);

  if (!parseCommaExpression(ast->expression)) {
    return false;
  }
  Q_ASSERT(ast->expression);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}


bool Parser::parseWhileStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  ADVANCE(Token_while, "while");
  ADVANCE('(' , "(");

  ConditionAST *cond = 0;
  if (!parseCondition(cond))
    {
      reportError("Condition expected");
      return false;
    }
  ADVANCE(')', ")");

  StatementAST *body = 0;
  if (!parseStatement(body))
    {
      reportError("Statement expected");
      return false;
    }

  WhileStatementAST *ast = CreateNode<WhileStatementAST>(session->mempool);
  ast->condition = cond;
  ast->statement = body;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseDoStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  ADVANCE(Token_do, "do");

  StatementAST *body = 0;
  if (!parseStatement(body))
    {
      reportError(("Statement expected"));
      //return false;
    }

  ADVANCE_NR(Token_while, "while");
  ADVANCE_NR('(' , "(");

  ExpressionAST *expr = 0;
  if (!parseCommaExpression(expr))
    {
      reportError(("Expression expected"));
      //return false;
    }

  ADVANCE_NR(')', ")");
  ADVANCE_NR(';', ";");

  DoStatementAST *ast = CreateNode<DoStatementAST>(session->mempool);
  ast->statement = body;
  ast->expression = expr;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseForStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  ADVANCE(Token_for, "for");
  ADVANCE('(', "(");

  ForRangeDeclarationAst *rangeDecl = 0;
  StatementAST *init = 0;
  ConditionAST *cond = 0;

  if (!parseRangeBasedFor(rangeDecl))
    {
      if (!parseForInitStatement(init))
        {
          reportError(("'for' initialization expected"));
          return false;
        }

      parseCondition(cond);
      ADVANCE(';', ";");
    }

  ExpressionAST *expr = 0;
  parseCommaExpression(expr);
  ADVANCE(')', ")");

  StatementAST *body = 0;
  if (!parseStatement(body))
    return false;

  ForStatementAST *ast = CreateNode<ForStatementAST>(session->mempool);
  ast->init_statement = init;
  ast->range_declaration = rangeDecl;
  ast->condition = cond;
  ast->expression = expr;
  ast->statement = body;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseRangeBasedFor(ForRangeDeclarationAst *&node)
{
  Comment mcomment = comment();
  clearComment();

  uint start = session->token_stream->cursor();

  const ListNode<uint> *cv = 0;
  parseCvQualify(cv);

  const ListNode<uint> *storageSpec = 0;
  parseStorageClassSpecifier(storageSpec);

  parseCvQualify(cv);

  TypeSpecifierAST *spec = 0;

  // auto support: right now it is part of the storage spec, put it back
  if (storageSpec && session->token_stream->kind(storageSpec->toBack()->element) == Token_auto) {
    rewind(storageSpec->toBack()->element);
  }

  if (!parseTypeSpecifier(spec))
    {
      rewind(start);
      return false;
    }

  parseCvQualify(cv);
  spec->cv = cv;

  DeclaratorAST *declarator = 0;
  if (!parseDeclarator(declarator, false /* no bitfield allowed */))
    {
      rewind(start);
      return false;
    }

  if (session->token_stream->lookAhead() != ':')
    {
      rewind(start);
      return false;
    }
  advance();

  ForRangeDeclarationAst *ast = CreateNode<ForRangeDeclarationAst>(session->mempool);
  ast->type_specifier = spec;
  ast->storage_specifiers = storageSpec;
  ast->declarator = declarator;

  if(mcomment)
    addComment(ast, mcomment);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseForInitStatement(StatementAST *&node)
{
  if (parseDeclarationStatement(node))
    return true;

  return parseExpressionStatement(node);
}

bool Parser::parseCompoundStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK('{');

  CompoundStatementAST *ast = CreateNode<CompoundStatementAST>(session->mempool);
  while (session->token_stream->lookAhead())
    {
      if (session->token_stream->lookAhead() == '}')
        break;

      uint startStmt = session->token_stream->cursor();

      StatementAST *stmt = 0;
      if (!parseStatement(stmt))
        {
          if (startStmt == session->token_stream->cursor())
            advance();

          skipUntilStatement();
        }
      else
        {
          ast->statements = snoc(ast->statements, stmt, session->mempool);
        }
    }

  clearComment();
  ADVANCE_NR('}', "}");


  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseIfStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  ADVANCE(Token_if, "if");

  ADVANCE('(' , "(");
  IfStatementAST *ast = CreateNode<IfStatementAST>(session->mempool);

  ConditionAST *cond = 0;
  if (!parseCondition(cond))
    {
      reportError(("Condition expected"));
      return false;
    }

  ADVANCE(')', ")");

  StatementAST *stmt = 0;
  if (!parseStatement(stmt))
    {
      reportError(("Statement expected"));
      return false;
    }

  ast->condition = cond;
  ast->statement = stmt;

  if (session->token_stream->lookAhead() == Token_else)
    {
      advance();

      if (!parseStatement(ast->else_statement))
        {
          reportError(("Statement expected"));
          return false;
        }
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseSwitchStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();
  ADVANCE(Token_switch, "switch");

  ADVANCE('(' , "(");

  ConditionAST *cond = 0;
  if (!parseCondition(cond))
    {
      reportError(("Condition expected"));
      return false;
    }
  ADVANCE(')', ")");

  StatementAST *stmt = 0;
  if (!parseStatement(stmt))
  {
    reportError(("Statement expected"));
    return false;
  }

  SwitchStatementAST *ast = CreateNode<SwitchStatementAST>(session->mempool);
  ast->condition = cond;
  ast->statement = stmt;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseLabeledStatement(StatementAST *&node)
{
  switch(session->token_stream->lookAhead())
    {
    case Token_identifier:
    case Token_default:
      {
        uint start = session->token_stream->cursor();
        if (session->token_stream->lookAhead(1) == ':')
          {
            advance();
            advance();

            StatementAST *stmt = 0;
            if (parseStatement(stmt))
              {
                LabeledStatementAST* ast = CreateNode<LabeledStatementAST>(session->mempool);
                ast->label = start;
                ast->statement = stmt;
                UPDATE_POS(ast, start, _M_last_valid_token+1);
                node = ast;
                return true;
              }
          }
          break;
      }

    case Token_case:
      {
        uint start = session->token_stream->cursor();

        advance();
        ExpressionAST *expr = 0;
        if (!parseConstantExpression(expr))
          {
            reportError(("Expression expected"));
          }
        else if (session->token_stream->lookAhead() == Token_ellipsis)
          {
            advance();

            if (!parseConstantExpression(expr))
              {
                reportError(("Expression expected"));
              }
          }
        ADVANCE(':', ":");

        LabeledStatementAST* ast = CreateNode<LabeledStatementAST>(session->mempool);
        ast->label = start;
        ast->expression = expr;

        parseStatement(ast->statement);

        if(ast->expression || ast->statement) {
          UPDATE_POS(ast, start, _M_last_valid_token+1);
          node = ast;
          return true;
        }
      }
      break;

    }

  return false;
}

///TODO: decl-specifier-seq is currently not used at all,
///      can lead to problems for certain orders of specifiers
bool Parser::parseBlockDeclaration(DeclarationAST *&node)
{
  switch(session->token_stream->lookAhead())
    {
    case Token_typedef:
      return parseTypedef(node);
    case Token_using:
      return parseUsing(node);
    case Token_asm:
      return parseAsmDefinition(node);
    case Token_namespace:
      return parseNamespaceAliasDefinition(node);
    case Token_static_assert:
      return parseStaticAssert(node);
    }

  Comment mcomment = comment();
  clearComment();

  uint start = session->token_stream->cursor();

  const ListNode<uint> *cv = 0;
  parseCvQualify(cv);

  const ListNode<uint> *storageSpec = 0;
  parseStorageClassSpecifier(storageSpec);

  parseCvQualify(cv);

  TypeSpecifierAST *spec = 0;
  // auto support: right now it is part of the storage spec, put it back
  if (storageSpec && session->token_stream->kind(storageSpec->toBack()->element) == Token_auto) {
    rewind(storageSpec->toBack()->element);
  }
  if (!parseTypeSpecifierOrClassSpec(spec))
    { // replace with simpleTypeSpecifier?!?!
      rewind(start);
      return false;
    }

  if (!storageSpec) // see also https://bugs.kde.org/show_bug.cgi?id=253827
    parseStorageClassSpecifier(storageSpec);

  parseCvQualify(cv);
  spec->cv = cv;

  const ListNode<InitDeclaratorAST*> *declarators = 0;
  parseInitDeclaratorList(declarators);

  if (session->token_stream->lookAhead() != ';')
    {
      rewind(start);
      return false;
    }
  advance();

  SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(session->mempool);
  ast->storage_specifiers = storageSpec;
  ast->type_specifier = spec;
  ast->init_declarators = declarators;

  if(mcomment)
    addComment(ast, mcomment);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseStaticAssert(DeclarationAST*& node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_static_assert);

  ADVANCE('(', ")");

  StaticAssertAST *ast
    = CreateNode<StaticAssertAST>(session->mempool);

  if (!parseConstantExpression(ast->expression))
    {
      reportError("Constant expression expected");
    }

  ADVANCE(',', ",");

  if (!parseStringLiteral(ast->string))
    {
      reportError("String literal expected");
    }

  ADVANCE(')', ")");
  ADVANCE(';', ";");

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseNamespaceAliasDefinition(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_namespace);

  NamespaceAliasDefinitionAST *ast
    = CreateNode<NamespaceAliasDefinitionAST>(session->mempool);

  uint pos = session->token_stream->cursor();
  ADVANCE(Token_identifier,  "identifier");
  ast->namespace_name = pos;

  ADVANCE('=', "=");

  if (!parseName(ast->alias_name))
    {
      reportError(("Namespace name expected"));
    }

  ADVANCE(';', ";");

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseDeclarationStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  DeclarationAST *decl = 0;
  if (!parseBlockDeclaration(decl))
    return false;

  DeclarationStatementAST *ast = CreateNode<DeclarationStatementAST>(session->mempool);
  ast->declaration = decl;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseDeclarationInternal(DeclarationAST *&node)
{
  uint start = session->token_stream->cursor();

  // that is for the case '__declspec(dllexport) int ...' or
  // '__declspec(dllexport) inline int ...', etc.
  WinDeclSpecAST *winDeclSpec = 0;
  parseWinDeclSpec(winDeclSpec);

  const ListNode<uint> *funSpec = 0;
  bool hasFunSpec = parseFunctionSpecifier(funSpec);

  const ListNode<uint> *cv = 0;
  parseCvQualify(cv);

  const ListNode<uint> *storageSpec = 0;
  bool hasStorageSpec = parseStorageClassSpecifier(storageSpec);

  if (hasStorageSpec && !hasFunSpec)
    hasFunSpec = parseFunctionSpecifier(funSpec);

  // that is for the case 'friend __declspec(dllexport) ....'
  parseWinDeclSpec(winDeclSpec);

  if (!cv)
    parseCvQualify(cv);

  int index = session->token_stream->cursor();

  NameAST *name = 0;
  if (parseName(name, AcceptTemplate) && session->token_stream->lookAhead() == '(')
    {
      // no type specifier, maybe a constructor or a cast operator??

      rewind(index);

      int startDeclarator = session->token_stream->cursor();
      DeclaratorAST *declarator = 0;
      if (parseDeclarator(declarator) && declarator->parameter_declaration_clause)
        {
          switch(session->token_stream->lookAhead())
            {
            case '=':
              {
                advance();
                if (session->token_stream->lookAhead() == Token_delete ||
                    session->token_stream->lookAhead() == Token_default)
                  {
                    FunctionDefinitionAST::DefaultDeleted defaultDeleted = (session->token_stream->lookAhead() == Token_delete)
                        ? FunctionDefinitionAST::Deleted : FunctionDefinitionAST::Default;
                    advance();
                    CHECK(';');
                    // defaulted or deleted functions are definitions
                    FunctionDefinitionAST *ast
                      = CreateNode<FunctionDefinitionAST>(session->mempool);
                    ast->storage_specifiers = storageSpec;
                    ast->function_specifiers = funSpec;
                    ast->declarator = declarator;
                    ast->defaultDeleted = defaultDeleted;

                    UPDATE_POS(ast, start, _M_last_valid_token+1);
                    node = ast;

                    return true;
                  }
              }

            case ':':
              {
                CtorInitializerAST *ctorInit = 0;
                StatementAST *funBody = 0;

                if (parseCtorInitializer(ctorInit)
                    && parseFunctionBody(funBody))
                  {
                    FunctionDefinitionAST *ast
                      = CreateNode<FunctionDefinitionAST>(session->mempool);

                    ast->storage_specifiers = storageSpec;
                    ast->function_specifiers = funSpec;
                    ast->declarator = declarator;
                    ast->function_body = funBody;
                    ast->constructor_initializers = ctorInit;

                    UPDATE_POS(ast, start, _M_last_valid_token+1);
                    node = ast;

                    return true;
                  }
              }
              break;

            case Token_try:
            case '{':
              {
                StatementAST *funBody = 0;
                if (parseFunctionBody(funBody))
                  {
                    FunctionDefinitionAST *ast
                      = CreateNode<FunctionDefinitionAST>(session->mempool);

                    ast->storage_specifiers = storageSpec;
                    ast->function_specifiers = funSpec;
                    ast->declarator = declarator;
                    ast->function_body = funBody;

                    UPDATE_POS(ast, start, _M_last_valid_token+1);
                    node = ast;

                    return true;
                  }
              }
              break;

            case '(':
            case '[':
              // ops!! it seems a declarator
              goto start_decl;
              break;
            }

        }

        // else expect something else
        rewind(startDeclarator);

        // check for simple-declaration
        const ListNode<InitDeclaratorAST*> *declarators = 0;
        parseInitDeclaratorList(declarators);

        if (session->token_stream->lookAhead() == ';')
          {
            advance();
            // simple-declaration
            SimpleDeclarationAST *ast
              = CreateNode<SimpleDeclarationAST>(session->mempool);

            ast->storage_specifiers = storageSpec;
            ast->function_specifiers = funSpec;
            ast->win_decl_specifiers = winDeclSpec;
            ast->init_declarators = declarators;

            UPDATE_POS(ast, start, _M_last_valid_token+1);
            node = ast;
            return true;
          }
    }

 start_decl:
  rewind(index);

  if (session->token_stream->lookAhead() == Token_const
      && session->token_stream->lookAhead(1) == Token_identifier
      && session->token_stream->lookAhead(2) == '=')
    {
      // constant definition
      advance(); // skip const

      const ListNode<InitDeclaratorAST*> *declarators = 0;
      if (!parseInitDeclaratorList(declarators))
        {
          syntaxError();
          return false;
        }

      ADVANCE(';', ";");

      SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(session->mempool);
      ast->init_declarators = declarators;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;

      return true;
    }

  TypeSpecifierAST *spec = 0;
  // auto support: right now it is part of the storage spec, put it back
  if (hasStorageSpec && session->token_stream->kind(storageSpec->toBack()->element) == Token_auto) {
    rewind(storageSpec->toBack()->element);
  }
  if (parseTypeSpecifier(spec))
    {
      Q_ASSERT(spec != 0);

      if (!hasFunSpec)
        parseFunctionSpecifier(funSpec);         // e.g. "void inline"

      if (cv)
        spec->cv = cv;

      int startDeclarator = session->token_stream->cursor();
      if (parseFunctionDefinitionInternal(node, start, winDeclSpec, storageSpec, funSpec, spec))
        {
          return true;
        }
      // else simple-declaration
      rewind(startDeclarator);
      const ListNode<InitDeclaratorAST*> *declarators = 0;
      parseInitDeclaratorList(declarators);

      if (session->token_stream->lookAhead() == ';')
        {
          advance();
          // simple-declaration
          SimpleDeclarationAST *ast
            = CreateNode<SimpleDeclarationAST>(session->mempool);

          ast->storage_specifiers = storageSpec;
          ast->function_specifiers = funSpec;
          ast->type_specifier = spec;
          ast->win_decl_specifiers = winDeclSpec;
          ast->init_declarators = declarators;

          UPDATE_POS(ast, start, _M_last_valid_token+1);
          node = ast;
          return true;
        }
    }

  syntaxError();
  return false;
}

bool Parser::parseFunctionDefinitionInternal(DeclarationAST*& node, uint start,
                                              WinDeclSpecAST* winDeclSpec, const ListNode<uint>* storageSpec,
                                              const ListNode<uint>* funSpec, TypeSpecifierAST* spec)
{
  DeclaratorAST* declarator = 0;
  StatementAST *funBody = 0;
  FunctionDefinitionAST::DefaultDeleted defaultDeleted = FunctionDefinitionAST::NotDefaultOrDeleted;
  if (!parseDeclarator(declarator))
    {
      return false;
    }

  if (!declarator->parameter_declaration_clause)
    {
      // cannot be a function definition without a param decl clause
      rewind(start);
      return false;
    }

  if (session->token_stream->lookAhead() == '=' &&
        (session->token_stream->lookAhead(1) == Token_delete
        || session->token_stream->lookAhead(1) == Token_default) &&
      session->token_stream->lookAhead(2) == ';')
    {
      advance();
      if (session->token_stream->lookAhead() == Token_delete) {
        defaultDeleted = FunctionDefinitionAST::Deleted;
      } else {
        defaultDeleted = FunctionDefinitionAST::Default;
      }
      advance();
      advance();
    }
  else if (!parseFunctionBody(funBody))
    {
      return false;
    }

  FunctionDefinitionAST *ast = CreateNode<FunctionDefinitionAST>(session->mempool);

  ast->win_decl_specifiers = winDeclSpec;
  ast->storage_specifiers = storageSpec;
  ast->function_specifiers = funSpec;
  ast->type_specifier = spec;
  ast->declarator = declarator;
  ast->function_body = funBody;
  ast->defaultDeleted = defaultDeleted;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;
  return true;
}


bool Parser::parseFunctionBody(StatementAST *&node)
{
  if (session->token_stream->lookAhead() == Token_try)
    return parseTryBlockStatement(node);

  return parseCompoundStatement(node);
}

bool Parser::parseTypeSpecifierOrClassSpec(TypeSpecifierAST *&node)
{
  if (parseClassSpecifier(node))
    return true;
  else if (parseEnumSpecifier(node))
    return true;
  else if (parseTypeSpecifier(node))
    return true;

  return false;
}

bool Parser::parseTryBlockStatement(StatementAST *&node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_try);

  TryBlockStatementAST *ast = CreateNode<TryBlockStatementAST>(session->mempool);

  StatementAST *stmt = 0;
  if (!parseCompoundStatement(stmt))
    {
      syntaxError();
      return false;
    }
  ast->try_block = stmt;

  if (session->token_stream->lookAhead() != Token_catch)
    {
      reportError(("'catch' expected after try block"));
      return false;
    }

  while (session->token_stream->lookAhead() == Token_catch)
    {
      uint catchStart = session->token_stream->cursor();

      advance();
      ADVANCE('(', "(");
      ConditionAST *cond = 0;
      if (session->token_stream->lookAhead() == Token_ellipsis)
        {
          advance();
        }
      else if(session->token_stream->lookAhead() == ')') {
        //Do nothing, this is equivalent to ellipsis
      } else if (!parseCondition(cond, false))
        {
          reportError(("condition expected"));
          return false;
        }
      ADVANCE(')', ")");

      StatementAST *body = 0;
      if (!parseCompoundStatement(body))
        {
          syntaxError();
          return false;
        }

      CatchStatementAST *catch_ast = CreateNode<CatchStatementAST>(session->mempool);
      catch_ast->condition = cond;
      catch_ast->statement = body;
      UPDATE_POS(catch_ast, catchStart, _M_last_valid_token+1);

      ast->catch_blocks = snoc(ast->catch_blocks, catch_ast, session->mempool);
    }

  node = ast;
  UPDATE_POS(ast, start, _M_last_valid_token+1);
  return true;
}

bool Parser::parsePrimaryExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  switch(session->token_stream->lookAhead())
    {
    case Token_string_literal:
      {
      PrimaryExpressionAST *ast = 0;
      ast = CreateNode<PrimaryExpressionAST>(session->mempool);
      parseStringLiteral(ast->literal);
      ast->type = PrimaryExpressionAST::Literal;
      node = ast;
      }
      break;

    case Token_number_literal:
    case Token_char_literal:
    case Token_true:
    case Token_false:
    case Token_this:
    case Token_nullptr:
      {
      PrimaryExpressionAST *ast = 0;
      ast = CreateNode<PrimaryExpressionAST>(session->mempool);
      ast->token = session->token_stream->cursor();
      advance();
      ast->type = PrimaryExpressionAST::Token;
      node = ast;
      }
      break;

    case '(':
      {
      PrimaryExpressionAST *ast = 0;
      advance();

      if (session->token_stream->lookAhead() == '{')
        {
          StatementAST *expressionStatement = 0;
          if (!parseCompoundStatement(expressionStatement))
            return false;
          ast = CreateNode<PrimaryExpressionAST>(session->mempool);
          ast->expression_statement = expressionStatement;
          ast->type = PrimaryExpressionAST::Statement;
        }
      else
        {
          ExpressionAST *expression = 0;
          if (!parseExpression(expression))
            return false;
          ast = CreateNode<PrimaryExpressionAST>(session->mempool);
          ast->sub_expression = expression;
          ast->type = PrimaryExpressionAST::SubExpression;
        }

      CHECK(')');
      node = ast;
      }
      break;

    default:
      NameAST *name = 0;
      if (parseName(name, EventuallyAcceptTemplate))
        {
          PrimaryExpressionAST *ast = 0;
          ast = CreateNode<PrimaryExpressionAST>(session->mempool);
          ast->name = name;
          ast->type = PrimaryExpressionAST::Name;
          node = ast;
        }
      else if(!parseLambdaExpression(node))
        {
          return false;
        }

      break;
    }

  UPDATE_POS(node, start, _M_last_valid_token+1);

  return true;
}


/*
  postfix-expression-internal:
  [ expression ]
  ( expression-list [opt] )
  (.|->) template [opt] id-expression
  (.|->) pseudo-destructor-name
  ++
  --
*/
bool Parser::parsePostfixExpressionInternal(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  switch (session->token_stream->lookAhead())
    {
    case '[':
      {
        advance();
        ExpressionAST *expr = 0;
        parseExpression(expr) || parseBracedInitList(expr);
        CHECK(']');

        SubscriptExpressionAST *ast
          = CreateNode<SubscriptExpressionAST>(session->mempool);

        ast->subscript = expr;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    case '(':
      {
        advance();
        ExpressionAST *expr = 0;
        parseExpressionList(expr);
        ///TODO: is this the right place? can't find anything in the last public spec file...
        bool isVariadic = false;
        if (session->token_stream->lookAhead() == Token_ellipsis)
          {
            advance();
            isVariadic = true;
          }
        CHECK(')');

        FunctionCallAST *ast = CreateNode<FunctionCallAST>(session->mempool);
        ast->arguments = expr;
        ast->isVariadic = isVariadic;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    case '{':
      {
        ExpressionAST *expr = 0;
        if (parseBracedInitList(expr))
          {
            UPDATE_POS(expr, start, _M_last_valid_token+1);
            node = expr;
            return true;
          }
        return false;
      }
    case '.':
    case Token_arrow:
      {
        uint op = session->token_stream->cursor();
        advance();

        if (session->token_stream->lookAhead() == Token_template)
            advance();

        NameAST *name = 0;
        if (!parseName(name, EventuallyAcceptTemplate))
          return false;

        ClassMemberAccessAST *ast = CreateNode<ClassMemberAccessAST>(session->mempool);
        ast->op = op;
        ast->name = name;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    case Token_incr:
    case Token_decr:
      {
        uint op = session->token_stream->cursor();
        advance();

        IncrDecrExpressionAST *ast = CreateNode<IncrDecrExpressionAST>(session->mempool);
        ast->op = op;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    default:
      return false;
    }
}

/*
  postfix-expression:
  simple-type-specifier ( expression-list [opt] )
  primary-expression postfix-expression-internal*
*/
bool Parser::parsePostfixExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  switch (session->token_stream->lookAhead())
    {
    case Token_dynamic_cast:
    case Token_static_cast:
    case Token_reinterpret_cast:
    case Token_const_cast:
      {
        uint castOp = session->token_stream->cursor();
        advance();

        CHECK('<');
        TypeIdAST *typeId = 0;
        parseTypeId(typeId);
        CHECK('>');

        CHECK('(');
        ExpressionAST *expr = 0;
        parseCommaExpression(expr);
        CHECK(')');

        CppCastExpressionAST *ast = CreateNode<CppCastExpressionAST>(session->mempool);
        ast->op = castOp;
        ast->type_id = typeId;
        ast->expression = expr;

        ExpressionAST *e = 0;
        while (parsePostfixExpressionInternal(e))
          {
            ast->sub_expressions = snoc(ast->sub_expressions, e, session->mempool);
          }

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    case Token_typename:
      {
        uint token = session->token_stream->cursor();
        advance();

        NameAST* name = 0;
        if (!parseName(name, AcceptTemplate))
          return false;

        CHECK('(');
        ExpressionAST *expr = 0;
        parseExpressionList(expr);
        CHECK(')');

        TypeIdentificationAST *ast = CreateNode<TypeIdentificationAST>(session->mempool);
        ast->typename_token = token;
        ast->name = name;
        ast->expression = expr;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    case Token_typeid:
      {
        advance();

        CHECK('(');
        TypeIdAST *typeId = 0;
        ExpressionAST *expression = 0;
        // either typeid or expression expected
        parseTypeId(typeId) || parseExpression(expression);
        CHECK(')');

        TypeIDOperatorAST *ast = CreateNode<TypeIDOperatorAST>(session->mempool);
        ast->expression = expression;
        ast->typeId = typeId;

        ExpressionAST *e = 0;
        while (parsePostfixExpressionInternal(e))
          {
            ast->sub_expressions = snoc(ast->sub_expressions, e, session->mempool);
          }

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    default:
      break;
    }

  TypeSpecifierAST *typeSpec = 0;
  ExpressionAST *expr = 0;

  bool expectPrimary = true;
  if (parseSimpleTypeSpecifier(typeSpec,true))
    {
      if (session->token_stream->lookAhead() == '(')
        {
          advance(); // skip '('
          parseExpressionList(expr);
          CHECK(')');
          expectPrimary = false;
        }
      else if (parseBracedInitList(expr))
        {
          expectPrimary = false;
        }
    }

  if (expectPrimary)
    {
      typeSpec = 0;
      rewind(start);

      if (!parsePrimaryExpression(expr)) {
        return false;
      }
    }

  const ListNode<ExpressionAST*> *sub_expressions = 0;

  if (typeSpec && expr)
  {
    // If we have a type-specifier, make the expression a sub-expression
    sub_expressions = snoc(sub_expressions, expr, session->mempool);
    expr = 0;
  }

  ExpressionAST *sub_expression = 0;
  while (parsePostfixExpressionInternal(sub_expression))
    {
      sub_expressions = snoc(sub_expressions, sub_expression, session->mempool);
    }

  node = expr;
  if (sub_expressions || !node)
    {
      PostfixExpressionAST *ast = CreateNode<PostfixExpressionAST>(session->mempool);
      ast->type_specifier = typeSpec;
      ast->expression = expr;
      ast->sub_expressions = sub_expressions;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseUnaryExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  switch(session->token_stream->lookAhead())
    {
    case Token_incr:
    case Token_decr:
    case '*':
    case '&':
    case '+':
    case '-':
    case '!':
    case Token_not:
    case '~':
    case Token_compl:
      {
        uint op = session->token_stream->cursor();
        advance();

        ExpressionAST *expr = 0;
        if (!parseCastExpression(expr))
          return false;

        UnaryExpressionAST *ast = CreateNode<UnaryExpressionAST>(session->mempool);
        ast->op = op;
        ast->expression = expr;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
      }
      return true;

    case Token_sizeof:
      {
        uint sizeof_token = session->token_stream->cursor();
        advance();
        bool isVariadic = false;
        if (session->token_stream->lookAhead() == Token_ellipsis) {
          isVariadic = true;
          advance();
        }

        SizeofExpressionAST *ast = CreateNode<SizeofExpressionAST>(session->mempool);
        ast->sizeof_token = sizeof_token;
        ast->isVariadic = isVariadic;

        uint index = session->token_stream->cursor();
        if (session->token_stream->lookAhead() == '(')
          {
            advance();
            if (parseTypeId(ast->type_id) && session->token_stream->lookAhead() == ')')
              {
                advance(); // skip )

                UPDATE_POS(ast, start, _M_last_valid_token+1);
                node = ast;
                return true;
              }

            ast->type_id = 0;
            rewind(index);
          }

        if (!parseUnaryExpression(ast->expression))
          return false;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
        return true;
      }

    case Token_noexcept: // same as generic case, but parentheses are not optional
      {
        uint op = session->token_stream->cursor();
        advance();

        if (session->token_stream->lookAhead() != '(')
          {
            tokenRequiredError ('(');
            return false;
          }

        ExpressionAST *expr = 0;
        if (!parseUnaryExpression(expr))
          return false;

        UnaryExpressionAST *ast = CreateNode<UnaryExpressionAST>(session->mempool);
        ast->op = op;
        ast->expression = expr;

        UPDATE_POS(ast, start, _M_last_valid_token+1);
        node = ast;
        return true;
      }

    default:
      break;
    }

  int token = session->token_stream->lookAhead();

  if (token == Token_new
      || (token == Token_scope && session->token_stream->lookAhead(1) == Token_new))
    return parseNewExpression(node);

  if (token == Token_delete
      || (token == Token_scope && session->token_stream->lookAhead(1) == Token_delete))
    return parseDeleteExpression(node);

  return parsePostfixExpression(node);
}

bool Parser::parseNewExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  NewExpressionAST *ast = CreateNode<NewExpressionAST>(session->mempool);

  if (session->token_stream->lookAhead() == Token_scope
      && session->token_stream->lookAhead(1) == Token_new)
    {
      ast->scope_token = session->token_stream->cursor();
      advance();
    }

  uint pos = session->token_stream->cursor();
  CHECK(Token_new);
  ast->new_token = pos;

  if (session->token_stream->lookAhead() == '(')
    {
      // new-placement
      advance();
      parseExpressionList(ast->expression);
      CHECK(')');
    }

  if (session->token_stream->lookAhead() == '(')
    {
      advance();
      parseTypeId(ast->type_id);
      CHECK(')');
    }
  else
    {
      parseNewTypeId(ast->new_type_id);
    }

  parseNewInitializer(ast->new_initializer);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseNewTypeId(NewTypeIdAST *&node)
{
  uint start = session->token_stream->cursor();

  TypeSpecifierAST *typeSpec = 0;
  if (!parseTypeSpecifier(typeSpec))
    return false;

  NewTypeIdAST *ast = CreateNode<NewTypeIdAST>(session->mempool);
  ast->type_specifier = typeSpec;

  parseNewDeclarator(ast->new_declarator);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseNewDeclarator(NewDeclaratorAST *&node)
{
  uint start = session->token_stream->cursor();

  NewDeclaratorAST *ast = CreateNode<NewDeclaratorAST>(session->mempool);
  PtrOperatorAST *ptrOp = 0;
  if (parsePtrOperator(ptrOp))
    {
      ast->ptr_op = ptrOp;
      parseNewDeclarator(ast->sub_declarator);
    }

  while (session->token_stream->lookAhead() == '[')
    {
      advance();
      ExpressionAST *expr = 0;
      parseExpression(expr);
      ast->expressions = snoc(ast->expressions, expr, session->mempool);
      ADVANCE(']', "]");
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseNewInitializer(NewInitializerAST *&node)
{
  uint start = session->token_stream->cursor();

  ExpressionAST *expr = 0;
  if (session->token_stream->lookAhead() == '(')
    {
      advance();

      parseExpressionList(expr);
      CHECK(')');
    }
  else if (!parseBracedInitList(expr))
    {
      rewind(start);
      return false;
    }

  NewInitializerAST *ast = CreateNode<NewInitializerAST>(session->mempool);
  ast->expression = expr;
  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseDeleteExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  DeleteExpressionAST *ast = CreateNode<DeleteExpressionAST>(session->mempool);

  if (session->token_stream->lookAhead() == Token_scope
      && session->token_stream->lookAhead(1) == Token_delete)
    {
      ast->scope_token = session->token_stream->cursor();
      advance();
    }

  uint pos = session->token_stream->cursor();
  CHECK(Token_delete);
  ast->delete_token =  pos;

  if (session->token_stream->lookAhead() == '[')
    {
      ast->lbracket_token = session->token_stream->cursor();
      advance();
      uint pos = session->token_stream->cursor();
      CHECK(']');
      ast->rbracket_token = pos;
    }

  if (!parseCastExpression(ast->expression))
    return false;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseCastExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  if (session->token_stream->lookAhead() == '(')
    {
      advance();


      TypeIdAST *typeIdAst = 0;
      if (parseTypeId(typeIdAst))
        {
          if (session->token_stream->lookAhead() == ')')
            {
              advance();

              ExpressionAST *castExpressionAst = 0;
              if (parseCastExpression(castExpressionAst))
                {
                  CastExpressionAST *ast = CreateNode<CastExpressionAST>(session->mempool);
                  ast->type_id = typeIdAst;
                  ast->expression = castExpressionAst;

                  UPDATE_POS(ast, start, _M_last_valid_token+1);
                  node = ast;

                  return true;
                }
            }
        }
    }

  rewind(start);
  return parseUnaryExpression(node);
}

bool Parser::parsePmExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  if (!parseCastExpression(node) || !node) // ### fixme
    return false;

  while (session->token_stream->lookAhead() == Token_ptrmem)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseCastExpression(rightExpr))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseMultiplicativeExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  if (!parsePmExpression(node))
    return false;

  while (session->token_stream->lookAhead() == '*'
         || session->token_stream->lookAhead() == '/'
         || session->token_stream->lookAhead() == '%')
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parsePmExpression(rightExpr))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}


bool Parser::parseAdditiveExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  if (!parseMultiplicativeExpression(node))
    return false;

  while (session->token_stream->lookAhead() == '+' || session->token_stream->lookAhead() == '-')
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseMultiplicativeExpression(rightExpr))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseShiftExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  if (!parseAdditiveExpression(node))
    return false;

  while (session->token_stream->lookAhead() == Token_rightshift || session->token_stream->lookAhead() == Token_leftshift)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseAdditiveExpression(rightExpr))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseRelationalExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseShiftExpression(node))
    return false;

  while (session->token_stream->lookAhead() == '<'
         || (session->token_stream->lookAhead() == '>' && !templArgs)
         || session->token_stream->lookAhead() == Token_leq
         || session->token_stream->lookAhead() == Token_geq)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseShiftExpression(rightExpr))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseEqualityExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseRelationalExpression(node, templArgs))
    return false;

  while (session->token_stream->lookAhead() == Token_eq
         || session->token_stream->lookAhead() == Token_not_eq
         || session->token_stream->lookAhead() == Token_xor_eq
         || session->token_stream->lookAhead() == Token_or_eq
         || session->token_stream->lookAhead() == Token_and_eq)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseRelationalExpression(rightExpr, templArgs))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseAndExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseEqualityExpression(node, templArgs))
    return false;

  while (session->token_stream->lookAhead() == '&' || session->token_stream->lookAhead() == Token_bitand)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseEqualityExpression(rightExpr, templArgs))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseExclusiveOrExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseAndExpression(node, templArgs))
    return false;

  while (session->token_stream->lookAhead() == '^' || session->token_stream->lookAhead() == Token_xor)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseAndExpression(rightExpr, templArgs))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseInclusiveOrExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseExclusiveOrExpression(node, templArgs))
    return false;

  while (session->token_stream->lookAhead() == '|' || session->token_stream->lookAhead() == Token_bitor)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseExclusiveOrExpression(rightExpr, templArgs))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseLogicalAndExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseInclusiveOrExpression(node, templArgs))
    return false;

  while (session->token_stream->lookAhead() == Token_and)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseInclusiveOrExpression(rightExpr, templArgs))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseLogicalOrExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseLogicalAndExpression(node, templArgs))
    return false;

  while (session->token_stream->lookAhead() == Token_or)
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseLogicalAndExpression(rightExpr, templArgs))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseConditionalExpression(ExpressionAST *&node, bool templArgs)
{
  uint start = session->token_stream->cursor();

  if (!parseLogicalOrExpression(node, templArgs))
    return false;

  if (session->token_stream->lookAhead() == '?')
    {
      advance();

      ExpressionAST *leftExpr = 0;
      if (!parseExpression(leftExpr)) {
        //NOTE: allow ommitting operand, for compatibility with gcc, see also:
        // http://gcc.gnu.org/onlinedocs/gcc-2.95.3/gcc_4.html#SEC70
        // https://bugs.kde.org/show_bug.cgi?id=292357
        reportError("ISO C++ does not allow ?: with omitted middle operand", KDevelop::ProblemData::Warning);
      }

      CHECK(':');

      ExpressionAST *rightExpr = 0;
      if (!parseAssignmentExpression(rightExpr))
        return false;

      ConditionalExpressionAST *ast
        = CreateNode<ConditionalExpressionAST>(session->mempool);

      ast->condition = node;
      ast->left_expression = leftExpr;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

//note: this does not really follow the grammar,
//      esp. the braced-init-list support might need to be
//      reworked, since the grammar says:
//      logical-or-expression assignment-operator initializer-clause
bool Parser::parseAssignmentExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  if(parseSignalSlotExpression(node))
    return true;
    //Parsed a signal/slot expression, fine

  if (session->token_stream->lookAhead() == Token_throw && !parseThrowExpression(node))
    return false;
  else if (!parseConditionalExpression(node))
    return false;

  while ( token_is_assignment(session->token_stream->lookAhead()) )
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseConditionalExpression(rightExpr) &&
          !parseSignalSlotExpression(rightExpr) &&
          !parseBracedInitList(rightExpr))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseConstantExpression(ExpressionAST *&node)
{
  return parseConditionalExpression(node);
}

bool Parser::parseExpression(ExpressionAST *&node)
{
  return parseCommaExpression(node);
}

bool Parser::parseExpressionList(ExpressionAST*& node)
{
  InitializerListAST* listNode = 0;
  const bool success = parseInitializerList(listNode);
  node = listNode;
  return success;
}

bool Parser::parseSignalSlotExpression(ExpressionAST *&node) {
  if(session->token_stream->lookAhead() == Token___qt_signal__ ||
     session->token_stream->lookAhead() == Token___qt_slot__) {
    uint start = session->token_stream->cursor();

    if(session->token_stream->lookAhead() == Token___qt_signal__)
      CHECK(Token___qt_signal__);
    else
      CHECK(Token___qt_slot__);

    CHECK('(');

    SignalSlotExpressionAST *ast = CreateNode<SignalSlotExpressionAST>(session->mempool);
    parseUnqualifiedName(ast->name, false);
    CHECK('(');

    if(ast->name)
      parseTemplateArgumentList(ast->name->template_arguments);

    CHECK(')');

    if(ast->name)
      ast->name->end_token = _M_last_valid_token+1;

    CHECK(')');

    UPDATE_POS(ast, start, _M_last_valid_token+1);
    node = ast;

    return true;
  }else{
    return false;
  }
}

bool Parser::parseQProperty(DeclarationAST *&node)
{
  if(session->token_stream->lookAhead() == Token___qt_property__) {
    uint start = session->token_stream->cursor();
    QPropertyDeclarationAST *ast = CreateNode<QPropertyDeclarationAST>(session->mempool);

    ast->member = 0;
    ast->getter = 0;
    ast->setter = 0;
    ast->resetter = 0;
    ast->notifier = 0;
    ast->designableMethod = 0;
    ast->scriptableMethod = 0;

    CHECK(Token___qt_property__);
    CHECK('(');

    if(!parseTypeSpecifier(ast->type))
      return false;

    PtrOperatorAST *ptrOp = 0;
    while (parsePtrOperator(ptrOp))
      ast->ptr_ops = snoc(ast->ptr_ops, ptrOp, session->mempool);

    if(!parseName(ast->name))
      return false;

    static KDevelop::IndexedString memberStr("MEMBER");
    static KDevelop::IndexedString readStr("READ");
    static KDevelop::IndexedString writeStr("WRITE");
    static KDevelop::IndexedString resetStr("RESET");
    static KDevelop::IndexedString notifyStr("NOTIFY");
    static KDevelop::IndexedString revisionStr("REVISION");
    static KDevelop::IndexedString designableStr("DESIGNABLE");
    static KDevelop::IndexedString scriptableStr("SCRIPTABLE");
    static KDevelop::IndexedString storedStr("STORED");
    static KDevelop::IndexedString userStr("USER");
    static KDevelop::IndexedString constantStr("CONSTANT");
    static KDevelop::IndexedString finalStr("FINAL");

    while(session->token_stream->lookAhead() != ')') {
      const KDevelop::IndexedString propertyField = session->token_stream->symbol(session->token_stream->cursor());
      if (propertyField == memberStr) {
        advance(); // skip MEMBER
        if (!parseName(ast->member))
          return false;
      } else if(propertyField == readStr) {
        advance(); // skip READ
        if(!parseName(ast->getter))
          return false;
      }else if(propertyField == writeStr){
        advance(); // skip WRITE
        if(!parseName(ast->setter))
          return false;
      }else if(propertyField == resetStr){
        advance(); // skip RESET
        if(!parseName(ast->resetter))
          return false;
      }else if(propertyField == notifyStr){
        advance(); // skip NOTIFY
        if(!parseName(ast->notifier))
          return false;
      } else if (propertyField == revisionStr) {
        advance(); // skip REVISION
        if (session->token_stream->lookAhead() == Token_number_literal) {
          advance();
        } else {
          return false;
        }
      } else if(propertyField == designableStr){
        advance(); // skip DESIGNABLE
        if(session->token_stream->lookAhead() == Token_true){
          advance(); // skip 'true'
        }else if(session->token_stream->lookAhead() == Token_false){
          advance(); // skip 'false'
        }else{
          if(!parseName(ast->designableMethod))
            return false;
        }
      }else if(propertyField == scriptableStr){
        advance(); // skip SCRIPTABLE
        if(session->token_stream->lookAhead() == Token_true){
          advance(); // skip 'true'
        }else if(session->token_stream->lookAhead() == Token_false){
          advance(); // skip 'false'
        }else{
          if(!parseName(ast->scriptableMethod))
            return false;
        }
      }else if(propertyField == storedStr){
        advance(); // skip STORED
        if(session->token_stream->lookAhead() == Token_true){
          advance(); // skip 'true'
        }else if(session->token_stream->lookAhead() == Token_false){
          advance(); // skip 'false'
        }else{
          return false;
        }
      }else if(propertyField == userStr){
        advance(); // skip USER
        if(session->token_stream->lookAhead() == Token_true){
          advance(); // skip 'true'
        }else if(session->token_stream->lookAhead() == Token_false){
          advance(); // skip 'false'
        }else{
          return false;
        }
      }else if(propertyField == constantStr){
        advance(); // skip CONSTANT
      }else if(propertyField == finalStr){
        advance(); // skip FINAL
      }else{
        return false;
      }
    }

    CHECK(')');

    UPDATE_POS(ast, start, _M_last_valid_token+1);
    node = ast;

    return true;
  }else{
    return false;
  }
}

bool Parser::parseCommaExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  if (!parseAssignmentExpression(node))
    return false;

  while (session->token_stream->lookAhead() == ',')
    {
      uint op = session->token_stream->cursor();
      advance();

      ExpressionAST *rightExpr = 0;
      if (!parseAssignmentExpression(rightExpr))
        return false;

      BinaryExpressionAST *ast = CreateNode<BinaryExpressionAST>(session->mempool);
      ast->op = op;
      ast->left_expression = node;
      ast->right_expression = rightExpr;

      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
    }

  return true;
}

bool Parser::parseThrowExpression(ExpressionAST *&node)
{
  uint start = session->token_stream->cursor();

  uint pos = session->token_stream->cursor();

  CHECK(Token_throw);

  ThrowExpressionAST *ast = CreateNode<ThrowExpressionAST>(session->mempool);
  ast->throw_token = pos;

  parseAssignmentExpression(ast->expression);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseTrailingReturnType(TrailingReturnTypeAST*& node)
{
  uint start = session->token_stream->cursor();

  CHECK(Token_arrow);

  TrailingReturnTypeAST *ast = CreateNode<TrailingReturnTypeAST>(session->mempool);

  // trailing-type-specifier-seq
  TypeSpecifierAST* type = 0;
  while (parseTypeSpecifier(type))
    {
      ast->type_specifier = snoc(ast->type_specifier, type, session->mempool);
    }

  // abstract-declarator_opt
  parseAbstractDeclarator(ast->abstractDeclarator);

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;

  return true;
}

bool Parser::parseLambdaExpression(ExpressionAST*& node)
{
  uint start = session->token_stream->cursor();

  // parse lambda-capture
  CHECK('[');
  uint defaultCapture = 0;
  const ListNode<LambdaCaptureAST*>* captures = 0;
  // capture-default
  if ( (session->token_stream->lookAhead() == '&'
        || session->token_stream->lookAhead() == '=')
      && (session->token_stream->lookAhead(+1) == ']'
        || session->token_stream->lookAhead(+1) == ','))
    {
      defaultCapture = session->token_stream->lookAhead();
      advance();
      if (session->token_stream->lookAhead() == ',')
        advance();
    }
  // capture-list
  while(session->token_stream->lookAhead() && session->token_stream->lookAhead() != ']')
    {
      LambdaCaptureAST* capture = 0;
      if (!parseLambdaCapture(capture)) {
        break;
      }
      captures = snoc(captures, capture, session->mempool);
      if (session->token_stream->lookAhead() == ',')
        advance();
      else
        break;
    }
  CHECK(']');

  LambdaDeclaratorAST* declarator = 0;
  // optional
  parseLambdaDeclarator(declarator);

  StatementAST* compound;
  if (!parseCompoundStatement(compound))
    {
      reportError("Compound statement expected");
      rewind(start);
      return false;
    }

  LambdaExpressionAST* ast = CreateNode<LambdaExpressionAST>(session->mempool);
  ast->capture_list = captures;
  ast->compound = compound;
  ast->declarator = declarator;
  ast->default_capture = defaultCapture;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;
  return true;
}

bool Parser::parseLambdaCapture(LambdaCaptureAST*& node)
{
  uint start = session->token_stream->cursor();

  LambdaCaptureAST* ast = CreateNode<LambdaCaptureAST>(session->mempool);

  if (session->token_stream->lookAhead() == Token_this)
    {
      advance();
      ast->isThis = true;
      UPDATE_POS(ast, start, _M_last_valid_token+1);
      node = ast;
      return true;
    }

  if (session->token_stream->lookAhead() == '&')
    {
      ast->isRef = true;
      advance();
    }

  if (!parseName(ast->identifier)) {
    rewind(start);
    return false;
  }

  if (session->token_stream->lookAhead() == Token_ellipsis)
    {
      advance();
      ast->isVariadic = true;
    }

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;
  return true;
}

bool Parser::parseLambdaDeclarator(LambdaDeclaratorAST*& node)
{
  uint start = session->token_stream->cursor();

  CHECK('(');
  ParameterDeclarationClauseAST* params = 0;
  parseParameterDeclarationClause(params);
  CHECK(')');

  ///TODO: attribute-specifier

  bool isMutable = false;
  if (session->token_stream->lookAhead() == Token_mutable)
    {
      isMutable = true;
      advance();
    }

  ExceptionSpecificationAST* exception_spec = 0;
  parseExceptionSpecification(exception_spec);

  TrailingReturnTypeAST* trailing_return_type = 0;
  parseTrailingReturnType(trailing_return_type);

  LambdaDeclaratorAST* ast = CreateNode<LambdaDeclaratorAST>(session->mempool);
  ast->parameter_declaration_clause = params;
  ast->isMutable = isMutable;
  ast->exception_spec = exception_spec;
  ast->trailing_return_type = trailing_return_type;

  UPDATE_POS(ast, start, _M_last_valid_token+1);
  node = ast;
  return true;
}

bool Parser::parseMemberVirtSpecifier(const ListNode< uint >*& node)
{
  uint start = session->token_stream->cursor();

  int tk;
  while (0 != (tk = session->token_stream->lookAhead())
         && (tk == Token_override || tk == Token_final || tk == Token_new))
    {
      node = snoc(node, session->token_stream->cursor(), session->mempool);
      advance();
    }

  return start != session->token_stream->cursor();
}

bool Parser::parseClassVirtSpecifier(const ListNode< uint >*& node)
{
  uint start = session->token_stream->cursor();

  int tk;
  while (0 != (tk = session->token_stream->lookAhead())
         && (tk == Token_final || tk == Token_explicit))
    {
      node = snoc(node, session->token_stream->cursor(), session->mempool);
      advance();
    }

  return start != session->token_stream->cursor();
}

bool Parser::holdErrors(bool hold)
{
  bool current = _M_hold_errors;
  _M_hold_errors = hold;
  return current;
}

QString Parser::stringForNode(AST* node) const
{
  if (!node) {
    return "<invalid node>";
  }

  return session->stringForNode(node);
}
