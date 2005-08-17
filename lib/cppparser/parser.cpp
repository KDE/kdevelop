/* This file is part of KDevelop
    Copyright (C) 2002,2003,2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// c++ support
#include "parser.h"
#include "tokens.h"
#include "lexer.h"
#include "errors.h"
#include "control.h"

#include <cassert>
#include <cstdlib>

#define ADVANCE(tk, descr) \
{ \
  if (token_stream.lookAhead() != tk) { \
      reportError("unexpected token"); \
      return false; \
  } \
  token_stream.nextToken(); \
}

#define ADVANCE_NR(tk, descr) \
  do { \
    if (token_stream.lookAhead() != tk) { \
      reportError("unexpected token"); \
    } \
    else \
        token_stream.nextToken(); \
  } while (0)

#define CHECK(tk, descr) \
  do { \
    if (token_stream.lookAhead() != tk) { \
        return false; \
    } \
    token_stream.nextToken(); \
  } while (0)

#define MATCH(tk, descr) \
  do { \
    if (token_stream.lookAhead() != tk) { \
        reportError(Errors::SyntaxError); \
        return false; \
    } \
  } while (0)

#define UPDATE_POS(_node, start, end) \
  do { \
      (_node)->setPosition(start, end); \
  } while (0)

#define AST_FROM_TOKEN(node, tk) \
    AST *node = CreateNode<AST>(m_pool); \
    UPDATE_POS(node, (tk), (tk)+1);

#define DUMP_AST(node) \
  do { \
    fprintf(stderr, "\n=================================================\n"); \
    for (int i=node->startToken(); i<node->endToken(); ++i) \
       fprintf(stderr, "%s", token_stream.tokenText(i).c_str()); \
    fprintf(stderr, "\n=================================================\n"); \
  } while (0)

//@todo remove me
enum
{
    OBJC_CLASS,
    OBJC_PROTOCOL,
    OBJC_ALIAS
};

Parser::Parser(Control *c)
    : control(c), lexer(token_stream, location_table, line_table, control)
{
    m_maxProblems = 5;
    objcp = false;
    m_no_errors = false;
}

Parser::~Parser()
{
}

TranslationUnitAST *Parser::parse(const char *contents, std::size_t size, pool *p)
{
    m_no_errors = false;
    m_pool = p;
    lexer.tokenize(contents, size);
    TranslationUnitAST *ast = 0;
    parseTranslationUnit(ast);
    return ast;
}

bool Parser::reportError(const Error& err)
{
    if (m_no_errors)
        return false;

    int tok = token_stream.cursor();

    std::string filename;
    int line, column;
    lexer.positionAt(token_stream.position(tok), &line, &column, &filename);
    fprintf(stderr, "%s:%d: unexpected token '%s'\n",
            filename.c_str(), line, token_stream.currentTokenText().c_str());

    // ### exit(EXIT_FAILURE);

#if 0
    if (m_problems < m_maxProblems) {
        ++m_problems;
        int line=0, col=0;
        std::string fileName;
        token_stream.getTokenStartPosition(token_stream.cursor(), &line, &col, &fileName);

        std::string s(token_stream.currentTokenText(), 0, 30);
        if (s.empty())
            s = ("<eof>");

        if (fileName.empty())
            fileName = m_file->fileName;

        //        m_driver->addProblem(m_driver->currentFileName(), Problem(err.text.arg(s), line, col));
        fprintf(stderr, "%s: error %s at line %d column %d\n",
                fileName.c_str(),
                err.text.c_str(), line, col);
    }
#endif
    return true;
}

bool Parser::reportError(const std::string& msg)
{
    if (m_no_errors)
        return false;

    int tok = token_stream.cursor();

    std::string filename;
    int line, column;
    lexer.positionAt(token_stream.position(tok), &line, &column, &filename);
    fprintf(stderr, "%s:%d: unexpected token '%s'\n",
            filename.c_str(), line, token_stream.currentTokenText().c_str());

    // ### exit(EXIT_FAILURE);

#if 0
    if (m_problems < m_maxProblems) {
        ++m_problems;
        int line=0, col=0;
        std::string fileName;
        token_stream.getTokenStartPosition(token_stream.cursor(), &line, &col, &fileName);

        if (fileName.empty())
            fileName = m_file->fileName;

        //        m_driver->addProblem(m_driver->currentFileName(), Problem(msg, line, col));
        fprintf(stderr, "%s: error %s at line %d column %d\n",
                fileName.c_str(),
                msg.c_str(), line, col);
    }
#endif
    return true;
}

void Parser::syntaxError()
{
    if (m_no_errors)
        return;

    int tok = token_stream.cursor();

    std::string filename;
    int line, column;
    lexer.positionAt(token_stream.position(tok), &line, &column, &filename);
    fprintf(stderr, "%s:%d: unexpected token '%s'\n",
            filename.c_str(), line, token_stream.currentTokenText().c_str());

    exit(EXIT_FAILURE);

#if 0
    if (m_problems < m_maxProblems) {
        ++m_problems;
        int line=0, col=0;
        std::string fileName;
        token_stream.getTokenStartPosition(token_stream.cursor(), &line, &col, &fileName);

        if (fileName.empty())
            fileName = m_file->fileName;

        //        m_driver->addProblem(m_driver->currentFileName(), Problem(msg, line, col));
        fprintf(stderr, "%s: syntax error at line %d column %d\n",
                fileName.c_str(), line, col);
    }
#endif
}

bool Parser::skipUntil(int token)
{
    while (token_stream.lookAhead()) {
        if (token_stream.lookAhead() == token)
            return true;

        token_stream.nextToken();
    }

    return false;
}

bool Parser::skipUntilDeclaration()
{
    while (token_stream.lookAhead()) {

        switch(token_stream.lookAhead()) {
        case ';':
        case '~':
        case Token_scope:
        case Token_identifier:
        case Token_operator:
        case Token_char:
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
        case Token_extern:
        case Token_namespace:
        case Token_using:
        case Token_typedef:
        case Token_asm:
        case Token_template:
        case Token_export:

        case Token_const:       // cv
        case Token_volatile:    // cv

        case Token_public:
        case Token_protected:
        case Token_private:
        case Token_signals:      // Qt
        case Token_slots:        // Qt
              return true;

        default:
            token_stream.nextToken();
        }
    }

    return false;
}

bool Parser::skipUntilStatement()
{
    while (token_stream.lookAhead()) {
        switch(token_stream.lookAhead()) {
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
                case Token_class:
                case Token_struct:
                case Token_union:
                case Token_enum:
                case Token_scope:
                case Token_template:
                case Token_using:
                    return true;

            default:
                  token_stream.nextToken();
        }
    }

    return false;
}

bool Parser::skip(int l, int r)
{
    int count = 0;
    while (token_stream.lookAhead()) {
        int tk = token_stream.lookAhead();

        if (tk == l)
            ++count;
        else if (tk == r)
            --count;
        else if (l != '{' && (tk == '{' || tk == '}' || tk == ';'))
            return false;

        if (count == 0)
            return true;

        token_stream.nextToken();
    }

    return false;
}

bool Parser::skipCommaExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *expr = 0;
    if (!skipExpression(expr))
        return false;

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();

        if (!skipExpression(expr)) {
            reportError(("expression expected"));
            return false;
        }
    }

    AbstractExpressionAST *ast = CreateNode<AbstractExpressionAST>(m_pool);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::skipExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();
    int count = 0;

    while (token_stream.lookAhead()) {
        int tk = token_stream.lookAhead();

        switch(tk) {
        case '(':
        case '[':
        case '{':
            ++count;
            token_stream.nextToken();
            break;

        case ']':
        case ')':
        case '}':
            if (count == 0) {
                AbstractExpressionAST *ast = CreateNode<AbstractExpressionAST>(m_pool);
                UPDATE_POS(ast, start, token_stream.cursor());
                node = ast;
                return true;
            }
            --count;
            token_stream.nextToken();
            break;

        case Token_struct:
        case Token_union:
        case Token_class: {
            int c = token_stream.cursor();
            TypeSpecifierAST *spec = 0;
            if (!parseClassSpecifier(spec))
                token_stream.rewind(c + 1);
        }
        break;

        case ',':
        case ';':
        case Token_case:
        case Token_default:
        case Token_if:
        case Token_while:
        case Token_do:
        case Token_for:
        case Token_break:
        case Token_continue:
        case Token_return:
        case Token_goto:
        {
            if ((tk == ',' || tk == ';') && count > 0) {
                token_stream.nextToken();
                break;
            }

            AbstractExpressionAST *ast = CreateNode<AbstractExpressionAST>(m_pool);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
        }
        return true;

        default:
            token_stream.nextToken();
        }
    }

    return false;
}

bool Parser::parseName(NameAST *&node, bool acceptTemplateId)
{
    std::size_t start = token_stream.cursor();

    AST *winDeclSpec = 0;
    parseWinDeclSpec(winDeclSpec);

    NameAST *ast = CreateNode<NameAST>(m_pool);

    if (token_stream.lookAhead() == Token_scope) {
        ast->setGlobal(true);
        token_stream.nextToken();
    }

    std::size_t idx = token_stream.cursor();

    while (true) {
        ClassOrNamespaceNameAST *n = 0;
        if (!parseUnqualifiedName(n))
            return false;

        if (token_stream.lookAhead() == Token_scope) {
            token_stream.nextToken();
            ast->addClassOrNamespaceName(n);
            if (token_stream.lookAhead() == Token_template)
                token_stream.nextToken(); /// skip optional template     #### @todo CHECK
        } else {
            assert(n != 0);
            if (!acceptTemplateId) {
                token_stream.rewind(n->startToken());
                parseUnqualifiedName(n, false);
            }
            ast->setUnqualifiedName(n);
            break;
        }
    }

    if (idx == token_stream.cursor())
        return false;

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseTranslationUnit(TranslationUnitAST *&node)
{
    std::size_t start = token_stream.cursor();

    m_problems = 0;
    TranslationUnitAST *tun = CreateNode<TranslationUnitAST>(m_pool);
    node = tun;
    while (token_stream.lookAhead()) {
        DeclarationAST *def = 0;
        std::size_t startDecl = token_stream.cursor();
        if (!parseDeclaration(def)) {
            // error recovery
            if (startDecl == token_stream.cursor())
                token_stream.nextToken(); // skip at least one token
            skipUntilDeclaration();
        }
        node->addDeclaration(def);
    }

    UPDATE_POS(node, start, token_stream.cursor());

    return m_problems == 0;
}

bool Parser::parseDeclaration(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    switch(token_stream.lookAhead()) {

    case ';':
        token_stream.nextToken();
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

    default:
        {
            // token_stream.rewind(start);

            if (objcp && parseObjcDef(node))
                return true;

            token_stream.rewind(start);

            AST *cv = 0;
            parseCvQualify(cv);

            AST *storageSpec = 0;
            parseStorageClassSpecifier(storageSpec);

            if (!cv)
                parseCvQualify(cv);

            TypeSpecifierAST *spec = 0;
            if (parseEnumSpecifier(spec) || parseClassSpecifier(spec)) {
                spec->setCvQualify(cv);

                AST *cv2 = 0;
                parseCvQualify(cv2);
                spec->setCv2Qualify(cv2);

                InitDeclaratorListAST *declarators = 0;
                parseInitDeclaratorList(declarators);
                ADVANCE(';', ";");

                SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(m_pool);
                ast->setStorageSpecifier(storageSpec);
                ast->setTypeSpec(spec);
                ast->setInitDeclaratorList(declarators);
                UPDATE_POS(ast, start, token_stream.cursor());
                node = ast;

                return true;
            }

            token_stream.rewind(start);
            return parseDeclarationInternal(node);
        }

    } // end switch
}

bool Parser::parseLinkageSpecification(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_extern) {
        return false;
    }
    token_stream.nextToken();

    LinkageSpecificationAST *ast = CreateNode<LinkageSpecificationAST>(m_pool);

    int startExternType = token_stream.cursor();
    if (token_stream.lookAhead() == Token_string_literal) {
        token_stream.nextToken();
        AST *externType = CreateNode<AST>(m_pool);
        UPDATE_POS(externType, startExternType, token_stream.cursor());

        ast->setExternType(externType);
    }

    if (token_stream.lookAhead() == '{') {
        LinkageBodyAST *linkageBody = 0;
        parseLinkageBody(linkageBody);
        ast->setLinkageBody(linkageBody);
    } else {
        DeclarationAST *decl = 0;
        if (!parseDeclaration(decl)) {
            reportError(("Declaration syntax error"));
        }
        ast->setDeclaration(decl);
    }

    UPDATE_POS(ast, start, token_stream.cursor());

    node = ast;

    return true;
}

bool Parser::parseLinkageBody(LinkageBodyAST *&node)
{

    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != '{') {
        return false;
    }
    token_stream.nextToken();

    LinkageBodyAST *lba = CreateNode<LinkageBodyAST>(m_pool);
    node = lba;

    while (token_stream.lookAhead()) {
        int tk = token_stream.lookAhead();

        if (tk == '}')
            break;

        DeclarationAST *def = 0;
        std::size_t startDecl = token_stream.cursor();
        if (parseDeclaration(def)) {
            node->addDeclaration(def);
        } else {
            // error recovery
            if (startDecl == token_stream.cursor())
                token_stream.nextToken(); // skip at least one token
            skipUntilDeclaration();
        }
    }

    if (token_stream.lookAhead() != '}') {
        reportError(("} expected"));
    } else
        token_stream.nextToken();

    UPDATE_POS(node, start, token_stream.cursor());
    return true;
}

bool Parser::parseNamespace(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_namespace) {
        return false;
    }
    token_stream.nextToken();

    int startNamespaceName = token_stream.cursor();
    if (token_stream.lookAhead() == Token_identifier) {
        token_stream.nextToken();
    }
    AST *namespaceName = CreateNode<AST>(m_pool);
    UPDATE_POS(namespaceName, startNamespaceName, token_stream.cursor());

    if (token_stream.lookAhead() == '=') {
        // namespace alias
        token_stream.nextToken();

        NameAST *name = 0;
        if (parseName(name)) {
            ADVANCE(';', ";");

            NamespaceAliasAST *ast = CreateNode<NamespaceAliasAST>(m_pool);
            ast->setNamespaceName(namespaceName);
            ast->setAliasName(name);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
            return true;
        } else {
            reportError(("namespace expected"));
            return false;
        }
    } else if (token_stream.lookAhead() != '{') {
        reportError(("{ expected"));
        return false;
    }

    NamespaceAST *ast = CreateNode<NamespaceAST>(m_pool);
    ast->setNamespaceName(namespaceName);

    LinkageBodyAST *linkageBody = 0;
    parseLinkageBody(linkageBody);

    ast->setLinkageBody(linkageBody);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseUsing(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_using) {
        return false;
    }
    token_stream.nextToken();

    if (token_stream.lookAhead() == Token_namespace) {
        if (!parseUsingDirective(node)) {
            return false;
        }
        UPDATE_POS(node, start, token_stream.cursor());
        return true;
    }

    UsingAST *ast = CreateNode<UsingAST>(m_pool);

    int startTypeName = token_stream.cursor();
    if (token_stream.lookAhead() == Token_typename) {
        token_stream.nextToken();
        AST *tn = CreateNode<AST>(m_pool);
        UPDATE_POS(tn, startTypeName, token_stream.cursor());
        ast->setTypeName(tn);
    }

    NameAST *name = 0;
    if (!parseName(name))
        return false;

    ast->setName(name);

    ADVANCE(';', ";");

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseUsingDirective(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_namespace) {
        return false;
    }
    token_stream.nextToken();

    NameAST *name = 0;
    if (!parseName(name)) {
        reportError(("Namespace name expected"));
        return false;
    }

    ADVANCE(';', ";");

    UsingDirectiveAST *ast = CreateNode<UsingDirectiveAST>(m_pool);
    ast->setName(name);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}


bool Parser::parseOperatorFunctionId(AST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_operator) {
        return false;
    }
    token_stream.nextToken();

    AST *op = 0;
    if (parseOperator(op)) {
        AST *asn = CreateNode<AST>(m_pool);
        node = asn;
        UPDATE_POS(node, start, token_stream.cursor());
        return true;
    } else {
        // parse cast operator
        AST *cv = 0;
        parseCvQualify(cv);

        TypeSpecifierAST *spec = 0;
        if (!parseSimpleTypeSpecifier(spec)) {
            syntaxError();
            return false;
        }
        spec->setCvQualify(cv);

        AST *cv2 = 0;
        parseCvQualify(cv2);
        spec->setCv2Qualify(cv2);

        AST *ptrOp = 0;
        while (parsePtrOperator(ptrOp))
              ;

        AST *asn = CreateNode<AST>(m_pool);
        node = asn;
        UPDATE_POS(node, start, token_stream.cursor());
        return true;
    }
}

bool Parser::parseTemplateArgumentList(TemplateArgumentListAST *&node, bool reportError)
{
    std::size_t start = token_stream.cursor();

    TemplateArgumentListAST *ast = CreateNode<TemplateArgumentListAST>(m_pool);

    AST *templArg = 0;
    if (!parseTemplateArgument(templArg))
        return false;
    ast->addArgument(templArg);

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();

        if (!parseTemplateArgument(templArg)) {
            if (reportError) {
               syntaxError();
               break;
            } else
               return false;
        }
        ast->addArgument(templArg);
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseTypedef(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_typedef) {
        return false;
    }
    token_stream.nextToken();

    TypeSpecifierAST *spec = 0;
    if (!parseTypeSpecifierOrClassSpec(spec)) {
        reportError(("Need a type specifier to declare"));
        return false;
    }

    InitDeclaratorListAST *declarators = 0;
    if (!parseInitDeclaratorList(declarators)) {
        //reportError(("Need an identifier to declare"));
        //return false;
    }

    ADVANCE(';', ";");

    TypedefAST *ast = CreateNode<TypedefAST>(m_pool);
    ast->setTypeSpec(spec);
    ast->setInitDeclaratorList(declarators);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseAsmDefinition(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    ADVANCE(Token_asm, "asm");

    AST *cv = 0;
    parseCvQualify(cv);

    skip('(', ')');
    token_stream.nextToken();
    ADVANCE(';', ";");

    DeclarationAST *ast = CreateNode<DeclarationAST>(m_pool);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseTemplateDeclaration(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    AST *exp = 0;

    int startExport = token_stream.cursor();
    if (token_stream.lookAhead() == Token_export) {
        token_stream.nextToken();
        AST *n = CreateNode<AST>(m_pool);
        UPDATE_POS(n, startExport, token_stream.cursor());
        exp = n;
    }

    if (token_stream.lookAhead() != Token_template) {
        return false;
    }
    token_stream.nextToken();

    TemplateParameterListAST *params = 0;
    if (token_stream.lookAhead() == '<') {
        token_stream.nextToken();
        parseTemplateParameterList(params);

        ADVANCE('>', ">");
    }

    DeclarationAST *def = 0;
    if (!parseDeclaration(def)) {
        reportError(("expected a declaration"));
    }

    TemplateDeclarationAST *ast = CreateNode<TemplateDeclarationAST>(m_pool);
    ast->setExported(exp);
    ast->setTemplateParameterList(params);
    ast->setDeclaration(def);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseOperator(AST *&/*node*/)
{
    std::string text = token_stream.currentTokenText();

    switch(token_stream.lookAhead()) {
    case Token_new:
    case Token_delete:
        token_stream.nextToken();
        if (token_stream.lookAhead() == '[' && token_stream.lookAhead(1) == ']') {
            token_stream.nextToken();
            token_stream.nextToken();
            text += "[]";
        }
        return true;

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
    case Token_assign:
    case Token_shift:
    case Token_eq:
    case Token_not_eq:
    case Token_leq:
    case Token_geq:
    case Token_and:
    case Token_or:
    case Token_incr:
    case Token_decr:
    case Token_ptrmem:
    case Token_arrow:
        token_stream.nextToken();
        return true;

    default:
        if (token_stream.lookAhead() == '(' && token_stream.lookAhead(1) == ')') {
            token_stream.nextToken();
            token_stream.nextToken();
            return true;
        } else if (token_stream.lookAhead() == '[' && token_stream.lookAhead(1) == ']') {
            token_stream.nextToken();
            token_stream.nextToken();
            return true;
        }
    }

    return false;
}

bool Parser::parseCvQualify(AST *&node)
{
    std::size_t start = token_stream.cursor();

    AST *ast = CreateNode<AST>(m_pool);

    int n = 0;
    while (token_stream.lookAhead()) {
        int tk = token_stream.lookAhead();
        if (tk == Token_const || tk == Token_volatile) {
            ++n;
            int startWord = token_stream.cursor();
            token_stream.nextToken();
            AST *word = CreateNode<AST>(m_pool);
            UPDATE_POS(word, startWord, token_stream.cursor());
            word->setParent(ast);
        } else
            break;
    }

    if (n == 0)
        return false;


    UPDATE_POS(ast, start, token_stream.cursor());

    node = ast;
    return true;
}

bool Parser::parseSimpleTypeSpecifier(TypeSpecifierAST *&node, bool onlyIntegral)
{
    std::size_t start = token_stream.cursor();
    bool isIntegral = false;
    bool done = false;

    while (!done) {
        switch(token_stream.lookAhead()) {
            case Token_char:
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
                isIntegral = true;
                token_stream.nextToken();
                break;

            default:
                done = true;
        }
    }

    TypeSpecifierAST *ast = CreateNode<TypeSpecifierAST>(m_pool);
    if (isIntegral) {
        ClassOrNamespaceNameAST *cl = CreateNode<ClassOrNamespaceNameAST>(m_pool);

        AST *n = CreateNode<AST>(m_pool);
        UPDATE_POS(n, start, token_stream.cursor());
        cl->setName(n);
        UPDATE_POS(cl, start, token_stream.cursor());

        NameAST *name = CreateNode<NameAST>(m_pool);
        name->setUnqualifiedName(cl);
        UPDATE_POS(name, start, token_stream.cursor());
        ast->setName(name);
    } else if (token_stream.lookAhead() == Token___typeof) {
        token_stream.nextToken();
        if (token_stream.lookAhead() == '(') {
            token_stream.nextToken();
            int idx = token_stream.cursor();
            TypeIdAST *typeId = 0;
            parseTypeId(typeId);
            if (token_stream.lookAhead() != ')') {
                token_stream.rewind(idx);
                AbstractExpressionAST *e = 0;
                parseUnaryExpression(e);
            }
            ADVANCE(')', ")");
        } else {
            AbstractExpressionAST *e = 0;
            parseUnaryExpression(e);
        }
    } else if (onlyIntegral) {
        token_stream.rewind(start);
        return false;
    } else {
        NameAST *name = 0;
        if (!parseName(name, true)) {
            token_stream.rewind(start);
            return false;
        }
        ast->setName(name);
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parsePtrOperator(AST *&node)
{
    std::size_t start = token_stream.cursor();
    int tok = token_stream.lookAhead();
    AST *memPtr = 0;

    switch (tok) {
        case '&':
        case '*':
            token_stream.nextToken();
            break;

        case Token_scope:
        case Token_identifier:
            if (!parsePtrToMember(memPtr)) {
                token_stream.rewind(start);
                return false;
            }
            break;

        default:
            return false;
    }

    AST *cv = 0;
    parseCvQualify(cv);

    AST *ast = CreateNode<AST>(m_pool);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}


bool Parser::parseTemplateArgument(AST *&node)
{
    std::size_t start = token_stream.cursor();

    TypeIdAST *typeId = 0;
    if (parseTypeId(typeId)) {
        if (token_stream.lookAhead() == ',' || token_stream.lookAhead() == '>') {
            node = typeId;
            return true;
        }
    }

    token_stream.rewind(start);
    AbstractExpressionAST *expr = 0;
    if (!parseLogicalOrExpression(expr, true)) {
        return false;
    }
    node = expr;

    return true;
}

bool Parser::parseTypeSpecifier(TypeSpecifierAST *&spec)
{
    AST *cv = 0;
    parseCvQualify(cv);

    TypeSpecifierAST *ast = 0;
    if (parseElaboratedTypeSpecifier(ast) || parseSimpleTypeSpecifier(ast)) {
        spec = ast;
        spec->setCvQualify(cv);

        AST *cv2 = 0;
        parseCvQualify(cv2);
        spec->setCv2Qualify(cv2);

        return true;
    }

    return false;
}

bool Parser::parseDeclarator(DeclaratorAST *&node)
{
    std::size_t start = token_stream.cursor();

    DeclaratorAST *ast = CreateNode<DeclaratorAST>(m_pool);

    DeclaratorAST *decl = 0;
    NameAST *declId = 0;

    AST *ptrOp = 0;
    while (parsePtrOperator(ptrOp)) {
        ast->addPtrOp(ptrOp);
    }

    if (token_stream.lookAhead() == '(') {
        token_stream.nextToken();

        if (!parseDeclarator(decl)) {
            return false;
        }
        ast->setSubDeclarator(decl);

        if (token_stream.lookAhead() != ')') {
            return false;
        }
        token_stream.nextToken();
    } else {
        if (token_stream.lookAhead() == ':') {
             // unnamed bitfield
        } else if (parseDeclaratorId(declId)) {
            ast->setDeclaratorId(declId);
        } else {
            token_stream.rewind(start);
            return false;
        }

        if (token_stream.lookAhead() == ':') {
            token_stream.nextToken();
            AbstractExpressionAST *expr = 0;
            if (!parseConstantExpression(expr)) {
                reportError(("Constant expression expected"));
            }
            goto update_pos;
        }
    }

    {
        bool isVector = true;

        while (token_stream.lookAhead() == '[') {
            int startArray = token_stream.cursor();
            token_stream.nextToken();
            AbstractExpressionAST *expr = 0;
            parseCommaExpression(expr);

            ADVANCE(']', "]");
            AST *array = CreateNode<AST>(m_pool);
            UPDATE_POS(array, startArray, token_stream.cursor());
            ast->addArrayDimension(array);
            isVector = true;
        }

        bool skipParen = false;
        if (token_stream.lookAhead() == Token_identifier
                && token_stream.lookAhead(1) == '('
                && token_stream.lookAhead(2) == '(') {
            token_stream.nextToken();
            token_stream.nextToken();
            skipParen = true;
        }

        int tok = token_stream.lookAhead();
        if (ast->subDeclarator() && !(isVector || tok == '(' || tok == ',' || tok == ';' || tok == '=')) {
            token_stream.rewind(start);
            return false;
        }

        int index = token_stream.cursor();
        if (token_stream.lookAhead() == '(') {
            token_stream.nextToken();

            ParameterDeclarationClauseAST *params = 0;
            if (!parseParameterDeclarationClause(params)) {
                token_stream.rewind(index);
                goto update_pos;
            }
            ast->setParameterDeclarationClause(params);

            if (token_stream.lookAhead() != ')') {
                token_stream.rewind(index);
                goto update_pos;
            }

            token_stream.nextToken();  // skip ')'

            AST *constant = 0;
            parseCvQualify(constant);
            ast->setConstant(constant);

            AST *except = 0;
            if (parseExceptionSpecification(except)) {
                ast->setExceptionSpecification(except);
            }

            if (token_stream.lookAhead() == Token___attribute__) {
                token_stream.nextToken();

                ADVANCE('(', "(");

                AbstractExpressionAST *expr = 0;
                parseExpression(expr);

                if (token_stream.lookAhead() != ')') {
                    reportError(("')' expected"));
                } else {
                    token_stream.nextToken();
                }
            }
        }

        if (skipParen) {
            if (token_stream.lookAhead() != ')') {
                reportError(("')' expected"));
            } else
                token_stream.nextToken();
        }
    }

update_pos:
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseAbstractDeclarator(DeclaratorAST *&node)
{
    std::size_t start = token_stream.cursor();

    DeclaratorAST *ast = CreateNode<DeclaratorAST>(m_pool);
    DeclaratorAST *decl = 0;

    AST *ptrOp = 0;
    while (parsePtrOperator(ptrOp)) {
        ast->addPtrOp(ptrOp);
    }

    int index = token_stream.cursor();
    if (token_stream.lookAhead() == '(') {
        token_stream.nextToken();

        if (!parseAbstractDeclarator(decl)) {
            token_stream.rewind(index);
            goto label1;
        }

        ast->setSubDeclarator(decl);

        if (token_stream.lookAhead() != ')'){
            token_stream.rewind(start);
            return false;
        }
        token_stream.nextToken();
    } else if (token_stream.lookAhead() == ':') {
        token_stream.nextToken();
        AbstractExpressionAST *expr = 0;
        if (!parseConstantExpression(expr)) {
            reportError(("Constant expression expected"));
        }
        goto update_pos;
    }

label1:
    {
        bool isVector = true;

        while (token_stream.lookAhead() == '[') {
            int startArray = token_stream.cursor();
            token_stream.nextToken();
            AbstractExpressionAST *expr = 0;
            parseCommaExpression(expr);

            ADVANCE(']', "]");
            AST *array = CreateNode<AST>(m_pool);
            UPDATE_POS(array, startArray, token_stream.cursor());
            ast->addArrayDimension(array);
            isVector = true;
        }

        int tok = token_stream.lookAhead();
        if (ast->subDeclarator() && !(isVector || tok == '(' || tok == ',' || tok == ';' || tok == '=')) {
            token_stream.rewind(start);
            return false;
        }

        int index = token_stream.cursor();
        if (token_stream.lookAhead() == '(') {
            token_stream.nextToken();

            ParameterDeclarationClauseAST *params = 0;
            if (!parseParameterDeclarationClause(params)) {
                token_stream.rewind(index);
                goto update_pos;
            }
            ast->setParameterDeclarationClause(params);

            if (token_stream.lookAhead() != ')') {
                token_stream.rewind(index);
                goto update_pos;
            }

            token_stream.nextToken();  // skip ')'

            AST *constant = 0;
            parseCvQualify(constant);
            ast->setConstant(constant);

            AST *except = 0;
            if (parseExceptionSpecification(except)) {
                ast->setExceptionSpecification(except);
            }
        }
    }

update_pos:
    if (token_stream.cursor() == start)
        return false;

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseEnumSpecifier(TypeSpecifierAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_enum) {
        return false;
    }

    token_stream.nextToken();

    NameAST *name = 0;
    parseName(name);

    if (token_stream.lookAhead() != '{') {
        token_stream.rewind(start);
        return false;
    }
    token_stream.nextToken();

    EnumSpecifierAST *ast = CreateNode<EnumSpecifierAST>(m_pool);
    ast->setName(name);

    EnumeratorAST *enumerator = 0;
    if (parseEnumerator(enumerator)) {
        ast->addEnumerator(enumerator);

        while (token_stream.lookAhead() == ',') {
            token_stream.nextToken();

            if (!parseEnumerator(enumerator)) {
                //reportError(("Enumerator expected"));
                break;
            }

            ast->addEnumerator(enumerator);
        }
    }

    if (token_stream.lookAhead() != '}')
        reportError(("} missing"));
    else
        token_stream.nextToken();

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseTemplateParameterList(TemplateParameterListAST *&node)
{
    std::size_t start = token_stream.cursor();

    TemplateParameterListAST *ast = CreateNode<TemplateParameterListAST>(m_pool);

    TemplateParameterAST *param = 0;
    if (!parseTemplateParameter(param)) {
        return false;
    }
    ast->addTemplateParameter(param);

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();

        if (!parseTemplateParameter(param)) {
            syntaxError();
            break;
        } else {
            ast->addTemplateParameter(param);
        }
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseTemplateParameter(TemplateParameterAST *&node)
{
    std::size_t start = token_stream.cursor();
    TemplateParameterAST *ast = CreateNode<TemplateParameterAST>(m_pool);

    TypeParameterAST *typeParameter = 0;
    ParameterDeclarationAST *param = 0;

    int tk = token_stream.lookAhead();

    if ((tk == Token_class || tk == Token_typename || tk == Token_template) && parseTypeParameter(typeParameter)) {
        ast->setTypeParameter(typeParameter);
        goto ok;
    }

    if (!parseParameterDeclaration(param))
        return false;
    ast->setTypeValueParameter(param);

ok:
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseTypeParameter(TypeParameterAST *&node)
{
    std::size_t start = token_stream.cursor();
    TypeParameterAST *ast = CreateNode<TypeParameterAST>(m_pool);

    AST_FROM_TOKEN(kind, token_stream.cursor());
    ast->setKind(kind);

    switch(token_stream.lookAhead()) {

    case Token_class:
    case Token_typename:
        {
            token_stream.nextToken(); // skip class

            // parse optional name
            NameAST *name = 0;
            if(parseName(name, true)){
                ast->setName(name);

                if (token_stream.lookAhead() == '='){
                    token_stream.nextToken();

                    TypeIdAST *typeId = 0;
                    if(!parseTypeId(typeId)){
                        //syntaxError();
                        token_stream.rewind(start);
                        return false;
                    }
                    ast->setTypeId(typeId);
                } else if (!(token_stream.lookAhead() == ',' || token_stream.lookAhead() == '>')) {
                    token_stream.rewind(start);
                    return false;
                }
            }
        }
        break;

    case Token_template:
        {
            token_stream.nextToken(); // skip template
            ADVANCE('<', '<');

            TemplateParameterListAST *params = 0;
            if (!parseTemplateParameterList(params)) {
                return false;
            }
            ast->setTemplateParameterList(params);

            ADVANCE('>', ">");

            if (token_stream.lookAhead() == Token_class)
                token_stream.nextToken();

            // parse optional name
            NameAST *name = 0;
            if (parseName(name, true)) {
                ast->setName(name);
                if (token_stream.lookAhead() == '=') {
                    token_stream.nextToken();

                    TypeIdAST *typeId = 0;
                    if (!parseTypeId(typeId)) {
                        syntaxError();
                        return false;
                    }
                    ast->setTypeId(typeId);
                }
            }

            if (token_stream.lookAhead() == '=') {
                token_stream.nextToken();

                NameAST *templ_name = 0;
                parseName(templ_name, true);
            }
        }
        break;

    default:
        return false;

    } // end switch


    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseStorageClassSpecifier(AST *&node)
{
    std::size_t start = token_stream.cursor();
    AST *ast = CreateNode<AST>(m_pool);

    while (token_stream.lookAhead()) {
        int tk = token_stream.lookAhead();
        if (tk == Token_friend || tk == Token_auto || tk == Token_register || tk == Token_static ||
                tk == Token_extern || tk == Token_mutable) {
            int startNode = token_stream.cursor();
            token_stream.nextToken();

            AST *n = CreateNode<AST>(m_pool);
            UPDATE_POS(n, startNode, token_stream.cursor());
            n->setParent(ast);
        } else
            break;
    }

    if (!ast->children()) // ### check if the length is 0?
        return false;

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseFunctionSpecifier(AST *&node)
{
    std::size_t start = token_stream.cursor();
    AST *ast = CreateNode<AST>(m_pool);

    while (token_stream.lookAhead()) {
        int tk = token_stream.lookAhead();
        if (tk == Token_inline || tk == Token_virtual || tk == Token_explicit) {
            int startNode = token_stream.cursor();
            token_stream.nextToken();

            AST *n = CreateNode<AST>(m_pool);
            UPDATE_POS(n, startNode, token_stream.cursor());
            n->setParent(ast);
        } else {
            break;
    }
    }

    if (!ast->children()) // ### check if the length is 0
       return false;

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseTypeId(TypeIdAST *&node)
{
    /// @todo implement the AST for typeId
    std::size_t start = token_stream.cursor();

    TypeSpecifierAST *spec = 0;
    if (!parseTypeSpecifier(spec)) {
        token_stream.rewind(start);
        return false;
    }

    DeclaratorAST *decl = 0;
    parseAbstractDeclarator(decl);

    TypeIdAST *ast = CreateNode<TypeIdAST>(m_pool);
    ast->setTypeSpecifier(spec);
    ast->setDeclarator(decl);

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseInitDeclaratorList(InitDeclaratorListAST *&node)
{
    std::size_t start = token_stream.cursor();

    InitDeclaratorListAST *ast = CreateNode<InitDeclaratorListAST>(m_pool);
    InitDeclaratorAST *decl = 0;

    if (!parseInitDeclarator(decl)) {
        return false;
    }
    ast->addInitDeclarator(decl);

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();

        if (!parseInitDeclarator(decl)) {
            syntaxError();
            break;
        }
        ast->addInitDeclarator(decl);
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseParameterDeclarationClause(ParameterDeclarationClauseAST *&node)
{
    std::size_t start = token_stream.cursor();

    ParameterDeclarationClauseAST *ast = CreateNode<ParameterDeclarationClauseAST>(m_pool);

    ParameterDeclarationListAST *params = 0;
    if (!parseParameterDeclarationList(params)) {

        if (token_stream.lookAhead() == ')')
            goto good;

        if (token_stream.lookAhead() == Token_ellipsis && token_stream.lookAhead(1) == ')') {
            AST_FROM_TOKEN(ellipsis, token_stream.cursor());
            ast->setEllipsis(ellipsis);
            token_stream.nextToken();
            goto good;
        }
        return false;
    }

    if (token_stream.lookAhead() == Token_ellipsis) {
        AST_FROM_TOKEN(ellipsis, token_stream.cursor());
        ast->setEllipsis(ellipsis);
        token_stream.nextToken();
    }

good:
    ast->setParameterDeclarationList(params);

    /// @todo add ellipsis
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseParameterDeclarationList(ParameterDeclarationListAST *&node)
{
    std::size_t start = token_stream.cursor();

    ParameterDeclarationListAST *ast = CreateNode<ParameterDeclarationListAST>(m_pool);

    ParameterDeclarationAST *param = 0;
    if (!parseParameterDeclaration(param)) {
        token_stream.rewind(start);
        return false;
    }
    ast->addParameter(param);

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();

        if (token_stream.lookAhead() == Token_ellipsis)
            break;

        if (!parseParameterDeclaration(param)) {
            token_stream.rewind(start);
            return false;
        }
        ast->addParameter(param);
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseParameterDeclaration(ParameterDeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    AST *storage = 0;
    parseStorageClassSpecifier(storage);

    // parse decl spec
    TypeSpecifierAST *spec = 0;
    if (!parseTypeSpecifier(spec)) {
        token_stream.rewind(start);
        return false;
    }

    int index = token_stream.cursor();

    DeclaratorAST *decl = 0;
    if (!parseDeclarator(decl)) {
        token_stream.rewind(index);

        // try with abstract declarator
        parseAbstractDeclarator(decl);
    }

    AbstractExpressionAST *expr = 0;
    if (token_stream.lookAhead() == '=') {
        token_stream.nextToken();
        if (!parseLogicalOrExpression(expr,true)) {
            //reportError(("Expression expected"));
        }
    }

    ParameterDeclarationAST *ast = CreateNode<ParameterDeclarationAST>(m_pool);
    ast->setTypeSpec(spec);
    ast->setDeclarator(decl);
    ast->setExpression(expr);

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseClassSpecifier(TypeSpecifierAST *&node)
{
    std::size_t start = token_stream.cursor();

    AST *classKey = 0;
    int classKeyStart = token_stream.cursor();

    int kind = token_stream.lookAhead();
    if (kind == Token_class || kind == Token_struct || kind == Token_union) {
        AST *asn = CreateNode<AST>(m_pool);
        classKey = asn;
        token_stream.nextToken();
        UPDATE_POS(classKey, classKeyStart, token_stream.cursor());
    } else {
        return false;
    }

    AST *winDeclSpec = 0;
    parseWinDeclSpec(winDeclSpec);

    while (token_stream.lookAhead() == Token_identifier && token_stream.lookAhead(1) == Token_identifier)
            token_stream.nextToken();

    NameAST *name = 0;
    parseName(name, true);

    BaseClauseAST *bases = 0;
    if (token_stream.lookAhead() == ':') {
        if (!parseBaseClause(bases)) {
            skipUntil('{');
        }
    }

    if (token_stream.lookAhead() != '{') {
        token_stream.rewind(start);
        return false;
    }

    ADVANCE('{', '{');

    ClassSpecifierAST *ast = CreateNode<ClassSpecifierAST>(m_pool);
    ast->setWinDeclSpec(winDeclSpec);
    ast->setClassKey(classKey);
    ast->setName(name);
    ast->setBaseClause(bases);

    while (token_stream.lookAhead()) {
        if (token_stream.lookAhead() == '}')
            break;

        DeclarationAST *memSpec = 0;
        std::size_t startDecl = token_stream.cursor();
        if (!parseMemberSpecification(memSpec)) {
            if (startDecl == token_stream.cursor())
                token_stream.nextToken(); // skip at least one token
            skipUntilDeclaration();
        } else
            ast->addDeclaration(memSpec);
    }

    if (token_stream.lookAhead() != '}') {
        reportError(("} missing"));
    } else
        token_stream.nextToken();

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseAccessSpecifier(AST *&node)
{
    std::size_t start = token_stream.cursor();

    switch(token_stream.lookAhead()) {
    case Token_public:
    case Token_protected:
    case Token_private: {
        AST *asn = CreateNode<AST>(m_pool);
        node = asn;
        token_stream.nextToken();
        UPDATE_POS(node, start, token_stream.cursor());
        return true;
        }
    }

    return false;
}

bool Parser::parseMemberSpecification(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    AST *access = 0;

    if (token_stream.lookAhead() == ';') {
        token_stream.nextToken();
        return true;
    } else if (token_stream.lookAhead() == Token_Q_OBJECT || token_stream.lookAhead() == Token_K_DCOP) {
        token_stream.nextToken();
        return true;
    } else if (token_stream.lookAhead() == Token_signals
            || token_stream.lookAhead() == Token_k_dcop
            || token_stream.lookAhead() == Token_k_dcop_signals) {
        AccessDeclarationAST *ast = CreateNode<AccessDeclarationAST>(m_pool);
        token_stream.nextToken();
        AST *n = CreateNode<AST>(m_pool);
        UPDATE_POS(n, start, token_stream.cursor());
        ast->addAccess(n);
        ADVANCE(':', ":");
        UPDATE_POS(ast, start, token_stream.cursor());
        node = ast;
        return true;
    } else if (parseTypedef(node)) {
        return true;
    } else if (parseUsing(node)) {
        return true;
    } else if (parseTemplateDeclaration(node)) {
        return true;
    } else if (parseAccessSpecifier(access)) {
        AccessDeclarationAST *ast = CreateNode<AccessDeclarationAST>(m_pool);
        ast->addAccess(access);

        int startSlot = token_stream.cursor();
        if (token_stream.lookAhead() == Token_slots) {
            token_stream.nextToken();
            AST *sl = CreateNode<AST>(m_pool);
            UPDATE_POS(sl, startSlot, token_stream.cursor());
            ast->addAccess(sl);
        }
        ADVANCE(':', ":");
        UPDATE_POS(ast, start, token_stream.cursor());
        node = ast;
        return true;
    }

    token_stream.rewind(start);

    AST *cv = 0;
    parseCvQualify(cv);

    AST *storageSpec = 0;
    parseStorageClassSpecifier(storageSpec);

    if (!cv)
        parseCvQualify(cv);

    TypeSpecifierAST *spec = 0;
    if (parseEnumSpecifier(spec) || parseClassSpecifier(spec)) {
        spec->setCvQualify(cv);

        AST *cv2 = 0;
        parseCvQualify(cv2);
        spec->setCv2Qualify(cv2);

            InitDeclaratorListAST *declarators = 0;
        parseInitDeclaratorList(declarators);
        ADVANCE(';', ";");

        SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(m_pool);
        ast->setTypeSpec(spec);
        ast->setInitDeclaratorList(declarators);
        UPDATE_POS(ast, start, token_stream.cursor());
        node = ast;

        return true;
    }

    token_stream.rewind(start);
    return parseDeclarationInternal(node);
}

bool Parser::parseCtorInitializer(AST *&/*node*/)
{
    if (token_stream.lookAhead() != ':') {
        return false;
    }
    token_stream.nextToken();

    AST *inits = 0;
    if (!parseMemInitializerList(inits)) {
        reportError(("Member initializers expected"));
    }

    return true;
}

bool Parser::parseElaboratedTypeSpecifier(TypeSpecifierAST *&node)
{
    std::size_t start = token_stream.cursor();

    int tk = token_stream.lookAhead();
    if (tk == Token_class  ||
        tk == Token_struct ||
        tk == Token_union  ||
        tk == Token_enum   ||
        tk == Token_typename)
    {
        AST *kind = CreateNode<AST>(m_pool);
        token_stream.nextToken();
        UPDATE_POS(kind, start, token_stream.cursor());

        NameAST *name = 0;

        if (parseName(name, true)) {
            ElaboratedTypeSpecifierAST *ast = CreateNode<ElaboratedTypeSpecifierAST>(m_pool);
            ast->setKind(kind);
            ast->setName(name);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;

            return true;
        }
    }

    token_stream.rewind(start);
    return false;
}

bool Parser::parseDeclaratorId(NameAST *&node)
{
    return parseName(node, true);
}

bool Parser::parseExceptionSpecification(AST *&node)
{
    if (token_stream.lookAhead() != Token_throw) {
        return false;
    }
    token_stream.nextToken();

    ADVANCE('(', "(");
    if (token_stream.lookAhead() == Token_ellipsis) {
        // extension found in MSVC++ 7.x headers
        std::size_t start = token_stream.cursor();
        AST *ast = CreateNode<AST>(m_pool);
        AST_FROM_TOKEN(ellipsis, token_stream.cursor());
        ellipsis->setParent(ast);
        token_stream.nextToken();
        UPDATE_POS(ast, start, token_stream.cursor());
        node = ast;
    } else {
        parseTypeIdList(node);
    }
    ADVANCE(')', ")");

    return true;
}

bool Parser::parseEnumerator(EnumeratorAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_identifier) {
        return false;
    }
    token_stream.nextToken();

    EnumeratorAST *ena = CreateNode<EnumeratorAST>(m_pool);
    node = ena;

    AST *id = CreateNode<AST>(m_pool);
    UPDATE_POS(id, start, token_stream.cursor());
    node->setId(id);

    if (token_stream.lookAhead() == '=') {
        token_stream.nextToken();

        AbstractExpressionAST *expr = 0;
        if (!parseConstantExpression(expr)) {
            reportError(("Constant expression expected"));
        }
        node->setExpression(expr);
    }

    UPDATE_POS(node, start, token_stream.cursor());

    return true;
}

bool Parser::parseInitDeclarator(InitDeclaratorAST *&node)
{
    std::size_t start = token_stream.cursor();

    DeclaratorAST *decl = 0;
    AST *init = 0;
    if (!parseDeclarator(decl)) {
        return false;
    }

    if (token_stream.lookAhead(0) == Token_asm) { // ### todo
        token_stream.nextToken();
        skip('(', ')');
        token_stream.nextToken();
    }

    parseInitializer(init);

    InitDeclaratorAST *ast = CreateNode<InitDeclaratorAST>(m_pool);
    ast->setDeclarator(decl);
    ast->setInitializer(init);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseBaseClause(BaseClauseAST *&node)
{
    std::size_t start = token_stream.cursor();
    if (token_stream.lookAhead() != ':') {
        return false;
    }
    token_stream.nextToken();

    BaseClauseAST *bca = CreateNode<BaseClauseAST>(m_pool);

    BaseSpecifierAST *baseSpec = 0;
    if (parseBaseSpecifier(baseSpec)) {
        bca->addBaseSpecifier(baseSpec);

        while (token_stream.lookAhead() == ',') {
            token_stream.nextToken();

            if (!parseBaseSpecifier(baseSpec)) {
                reportError(("Base class specifier expected"));
                return false;
            }
            bca->addBaseSpecifier(baseSpec);
        }
    } else
        return false;

    UPDATE_POS(bca, start, token_stream.cursor());
    node = bca;

    return true;
}

bool Parser::parseInitializer(AST *&node)
{
    if (token_stream.lookAhead() == '=') {
        token_stream.nextToken();

        if (!parseInitializerClause(node)) {
            reportError(("Initializer clause expected"));
            return false;
        }
        return true;
    } else if (token_stream.lookAhead() == '(') {
        token_stream.nextToken();
        AbstractExpressionAST *expr = 0;
        parseCommaExpression(expr);
        CHECK(')', ")");
        node = expr;
        return true;
    }

    return false;
}

bool Parser::parseMemInitializerList(AST *&/*node*/)
{
    AST *init = 0;
    if (!parseMemInitializer(init)) {
        return false;
    }

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();

        if (!parseMemInitializer(init)) {
            break;
        }
    }

    return true;
}

bool Parser::parseMemInitializer(AST *&/*node*/)
{
    NameAST *initId = 0;
    if (!parseMemInitializerId(initId)) {
        reportError(("Identifier expected"));
        return false;
    }
    ADVANCE('(', '(');
    AbstractExpressionAST *expr = 0;
    parseCommaExpression(expr);
    ADVANCE(')', ')');

    return true;
}

bool Parser::parseTypeIdList(AST *&node)
{
    std::size_t start = token_stream.cursor();

    TypeIdAST *typeId = 0;
    if (!parseTypeId(typeId)) {
        return false;
    }

    AST *ast = CreateNode<AST>(m_pool);
    typeId->setParent(ast);

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();
        if (parseTypeId(typeId)) {
            typeId->setParent(ast);
        } else {
            reportError(("Type id expected"));
            break;
        }
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseBaseSpecifier(BaseSpecifierAST *&node)
{
    std::size_t start = token_stream.cursor();
    BaseSpecifierAST *ast = CreateNode<BaseSpecifierAST>(m_pool);

    AST *access = 0;
    if (token_stream.lookAhead() == Token_virtual) {
        AST_FROM_TOKEN(virt, token_stream.cursor());
        ast->setIsVirtual(virt);

        token_stream.nextToken();

        parseAccessSpecifier(access);
    } else {
        parseAccessSpecifier(access);

        if (token_stream.lookAhead() == Token_virtual) {
            AST_FROM_TOKEN(virt, token_stream.cursor());
            ast->setIsVirtual(virt);
            token_stream.nextToken();
        }
    }

    NameAST *name = 0;
    if (!parseName(name, true)) {
        reportError(("Class name expected"));
    }

    ast->setAccess(access);
    ast->setName(name);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}


bool Parser::parseInitializerClause(AST *&node)
{
    if (token_stream.lookAhead() == '{') {
        if (!skip('{','}')) {
            reportError(("} missing"));
        } else
            token_stream.nextToken();
    } else {
        AbstractExpressionAST *expr = 0;
        if (!parseAssignmentExpression(expr)) {
            //reportError(("Expression expected"));
        }
        node = expr;
    }

    return true;
}

bool Parser::parseMemInitializerId(NameAST *&node)
{
    return parseName(node, true);
}

bool Parser::parsePtrToMember(AST *&/*node*/)     /// ### create the AST node
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() == Token_scope)
        token_stream.nextToken();

    ClassOrNamespaceNameAST *name = 0;
    while (token_stream.lookAhead() == Token_identifier) {

        if (!parseUnqualifiedName(name))
            break;

        if (token_stream.lookAhead() == Token_scope
                && token_stream.lookAhead(1) == '*') {
            token_stream.nextToken();
            token_stream.nextToken();
            return true;
        }

        if (token_stream.lookAhead() == Token_scope)
            token_stream.nextToken();
    }

    token_stream.rewind(start);
    return false;
}

bool Parser::parseUnqualifiedName(ClassOrNamespaceNameAST *&node, bool parseTemplateId)
{
    std::size_t start = token_stream.cursor();
    bool isDestructor = false;

    ClassOrNamespaceNameAST *ast = CreateNode<ClassOrNamespaceNameAST>(m_pool);

    if (token_stream.lookAhead() == Token_identifier) {
        int startName = token_stream.cursor();
        AST *n = CreateNode<AST>(m_pool);
        token_stream.nextToken();
        UPDATE_POS(n, startName, token_stream.cursor());
        ast->setName(n);
    } else if (token_stream.lookAhead() == '~' && token_stream.lookAhead(1) == Token_identifier) {
        int startName = token_stream.cursor();
        AST *n = CreateNode<AST>(m_pool);
        token_stream.nextToken(); // skip ~
        token_stream.nextToken(); // skip classname
        UPDATE_POS(n, startName, token_stream.cursor());
        ast->setName(n);
        isDestructor = true;
    } else if (token_stream.lookAhead() == Token_operator) {
        AST *n = 0;
        if (!parseOperatorFunctionId(n))
            return false;
        ast->setName(n);
    } else {
        return false;
    }

    if (parseTemplateId && !isDestructor) {

        int index = token_stream.cursor();

        if (token_stream.lookAhead() == '<') {
            token_stream.nextToken();

            // optional template arguments
            TemplateArgumentListAST *args = 0;
            parseTemplateArgumentList(args);

            if (token_stream.lookAhead() != '>') {
                token_stream.rewind(index);
            } else {
                token_stream.nextToken();
                ast->setTemplateArgumentList(args);
            }
        }
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseStringLiteral(AST *&node)
{
    if (token_stream.lookAhead() != Token_string_literal)
        return false;

    std::size_t start = token_stream.cursor();

    while (token_stream.lookAhead() == Token_string_literal)
        token_stream.nextToken();

    AST *ast = CreateNode<AST>(m_pool);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::skipExpressionStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *expr = 0;
    skipCommaExpression(expr);

    ADVANCE(';', ";");

    ExpressionStatementAST *ast = CreateNode<ExpressionStatementAST>(m_pool);
    ast->setExpression(expr);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseExpressionStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *expr = 0;
    parseCommaExpression(expr);

    ADVANCE(';', ";");

    ExpressionStatementAST *ast = CreateNode<ExpressionStatementAST>(m_pool);
    ast->setExpression(expr);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    switch(token_stream.lookAhead()) {

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
        token_stream.nextToken();
        ADVANCE(';', ";");
        return true;

    case Token_goto:
        token_stream.nextToken();
        ADVANCE(Token_identifier, "identifier");
        ADVANCE(';', ";");
        return true;

    case Token_return:
    {
        token_stream.nextToken();
        AbstractExpressionAST *expr = 0;
        parseCommaExpression(expr);

        ADVANCE(';', ";");

        ReturnStatementAST *ast = CreateNode<ReturnStatementAST>(m_pool);
        ast->setExpression(expr);
        UPDATE_POS(ast, start, token_stream.cursor());
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

bool Parser::parseExpressionOrDeclarationStatement(StatementAST *&node)
{
    bool sv_no_errors = m_no_errors;
    m_no_errors = true;

    std::size_t start = token_stream.cursor();

    StatementAST *decl_ast = 0;
    bool maybe_amb = parseDeclarationStatement(decl_ast);
    maybe_amb &= token_stream.kind(token_stream.cursor() - 1) == ';';

    std::size_t end = token_stream.cursor();

    token_stream.rewind(start);
    StatementAST *expr_ast = 0;
    maybe_amb &= parseExpressionStatement(expr_ast);
    maybe_amb &= token_stream.kind(token_stream.cursor() - 1) == ';';

    if (maybe_amb) {
        assert(decl_ast != 0 && expr_ast != 0);
        ExpressionOrDeclarationStatementAST *ast = CreateNode<ExpressionOrDeclarationStatementAST>(m_pool);
        ast->setDeclaration(decl_ast);
        ast->setExpression(expr_ast);
        UPDATE_POS(ast, start, token_stream.cursor());
        node = ast;
    } else {
        token_stream.rewind(std::max(end, token_stream.cursor()));

        node = decl_ast;
        if (!node)
            node = expr_ast;
    }

    m_no_errors = sv_no_errors;

    if (!node)
        syntaxError();

    return node != 0;
}

bool Parser::parseCondition(ConditionAST *&node, bool initRequired)
{
    std::size_t start = token_stream.cursor();

    ConditionAST *ast = CreateNode<ConditionAST>(m_pool);
    TypeSpecifierAST *spec = 0;

    if (parseTypeSpecifier(spec)) {
        ast->setTypeSpec(spec);

        DeclaratorAST *decl = 0;
        std::size_t declarator_start = token_stream.cursor();

        if (!parseDeclarator(decl)) {
            token_stream.rewind(declarator_start);
            if (!initRequired && !parseAbstractDeclarator(decl))
                decl = 0;
        }

        if (decl && (!initRequired || token_stream.lookAhead() == '=')) {
            ast->setDeclarator(decl);

            if (token_stream.lookAhead() == '=') {
                token_stream.nextToken();

                AbstractExpressionAST *expr = 0;
                if (parseExpression(expr))
                    ast->setExpression(expr);
            }

            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;

            return true;
        }
    }

    token_stream.rewind(start);

    AbstractExpressionAST *expr = 0;
    if (!parseCommaExpression(expr)) {
        return false;
    }

    ast->setExpression(expr);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}


bool Parser::parseWhileStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    ADVANCE(Token_while, "while");
    ADVANCE('(' , "(");

    ConditionAST *cond = 0;
    if (!parseCondition(cond)) {
        reportError(("condition expected"));
        return false;
    }
    ADVANCE(')', ")");

    StatementAST *body = 0;
    if (!parseStatement(body)) {
        reportError(("statement expected"));
        return false;
    }

    WhileStatementAST *ast = CreateNode<WhileStatementAST>(m_pool);
    ast->setCondition(cond);
    ast->setStatement(body);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseDoStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    ADVANCE(Token_do, "do");

    StatementAST *body = 0;
    if (!parseStatement(body)) {
        reportError(("statement expected"));
        //return false;
    }

    ADVANCE_NR(Token_while, "while");
    ADVANCE_NR('(' , "(");

    AbstractExpressionAST *expr = 0;
    if (!parseCommaExpression(expr)) {
        reportError(("expression expected"));
        //return false;
    }

    ADVANCE_NR(')', ")");
    ADVANCE_NR(';', ";");

    DoStatementAST *ast = CreateNode<DoStatementAST>(m_pool);
    ast->setStatement(body);
    //ast->setCondition(condition);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseForStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    ADVANCE(Token_for, "for");
    ADVANCE('(', "(");

    StatementAST *init = 0;
    if (!parseForInitStatement(init)) {
        reportError(("for initialization expected"));
        return false;
    }

    ConditionAST *cond = 0;
    parseCondition(cond);
    ADVANCE(';', ";");

    AbstractExpressionAST *expr = 0;
    parseCommaExpression(expr);
    ADVANCE(')', ")");

    StatementAST *body = 0;
    if (!parseStatement(body))
        return false;

    ForStatementAST *ast = CreateNode<ForStatementAST>(m_pool);
    ast->setInitStatement(init);
    ast->setCondition(cond);
    // ast->setExpression(expression);
    ast->setStatement(body);
    UPDATE_POS(ast, start, token_stream.cursor());
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
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != '{') {
        return false;
    }
    token_stream.nextToken();

    StatementListAST *ast = CreateNode<StatementListAST>(m_pool);

    while (token_stream.lookAhead()) {
        if (token_stream.lookAhead() == '}')
            break;

        StatementAST *stmt = 0;
        std::size_t startStmt = token_stream.cursor();
        if (!parseStatement(stmt)) {
            if (startStmt == token_stream.cursor())
                token_stream.nextToken();
            skipUntilStatement();
        } else {
            ast->addStatement(stmt);
        }
    }

    if (token_stream.lookAhead() != '}') {
        reportError(("} expected"));
    } else {
        token_stream.nextToken();
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseIfStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    ADVANCE(Token_if, "if");

    ADVANCE('(' , "(");

    IfStatementAST *ast = CreateNode<IfStatementAST>(m_pool);

    ConditionAST *cond = 0;
    if (!parseCondition(cond)) {
        reportError(("condition expected"));
        return false;
    }
    ADVANCE(')', ")");

    StatementAST *stmt = 0;
    if (!parseStatement(stmt)) {
        reportError(("statement expected"));
        return false;
    }

    ast->setCondition(cond);
    ast->setStatement(stmt);

    if (token_stream.lookAhead() == Token_else) {
        token_stream.nextToken();
        StatementAST *elseStmt = 0;
        if (!parseStatement(elseStmt)) {
            reportError(("statement expected"));
            return false;
        }
        ast->setElseStatement(elseStmt);
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseSwitchStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();
    ADVANCE(Token_switch, "switch");

    ADVANCE('(' , "(");

    ConditionAST *cond = 0;
    if (!parseCondition(cond)) {
        reportError(("condition expected"));
        return false;
    }
    ADVANCE(')', ")");

    StatementAST *stmt = 0;
    if (!parseCompoundStatement(stmt)) {
        syntaxError();
        return false;
    }

    SwitchStatementAST *ast = CreateNode<SwitchStatementAST>(m_pool);
    ast->setCondition(cond);
    ast->setStatement(stmt);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseLabeledStatement(StatementAST *&node)
{
    switch(token_stream.lookAhead()) {
    case Token_identifier:
    case Token_default:
        if (token_stream.lookAhead(1) == ':') {
            token_stream.nextToken();
            token_stream.nextToken();

            StatementAST *stmt = 0;
            if (parseStatement(stmt)) {
                node = stmt;
                return true;
            }
        }
        break;

    case Token_case:
    {
        token_stream.nextToken();
        AbstractExpressionAST *expr = 0;
        if (!parseConstantExpression(expr)) {
            reportError(("expression expected"));
        } else if (token_stream.lookAhead() == Token_ellipsis) {
            token_stream.nextToken();

            AbstractExpressionAST *expr2 = 0;
            if (!parseConstantExpression(expr2)) {
                reportError(("expression expected"));
            }
        }
        ADVANCE(':', ":");

        StatementAST *stmt = 0;
        if (parseStatement(stmt)) {
            node = stmt;
            return true;
        }
    }
    break;

    }

    return false;
}

bool Parser::parseBlockDeclaration(DeclarationAST *&node)
{
    switch(token_stream.lookAhead()) {
    case Token_typedef:
        return parseTypedef(node);
    case Token_using:
        return parseUsing(node);
    case Token_asm:
        return parseAsmDefinition(node);
    case Token_namespace:
        return parseNamespaceAliasDefinition(node);
    }

    std::size_t start = token_stream.cursor();

    AST *cv = 0;
    parseCvQualify(cv);

    AST *storageSpec = 0;
    parseStorageClassSpecifier(storageSpec);

    if (!cv)
        parseCvQualify(cv);

    TypeSpecifierAST *spec = 0;
    if (!parseTypeSpecifierOrClassSpec(spec)) { // replace with simpleTypeSpecifier?!?!
        token_stream.rewind(start);
        return false;
    }
    spec->setCvQualify(cv);

    AST *cv2 = 0;
    parseCvQualify(cv2);
    spec->setCv2Qualify(cv2);

    InitDeclaratorListAST *declarators = 0;
    parseInitDeclaratorList(declarators);

    if (token_stream.lookAhead() != ';') {
        token_stream.rewind(start);
        return false;
    }
    token_stream.nextToken();

    SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(m_pool);
    ast->setTypeSpec(spec);
    ast->setInitDeclaratorList(declarators);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseNamespaceAliasDefinition(DeclarationAST *&/*node*/)
{
    if (token_stream.lookAhead() != Token_namespace) {
        return false;
    }
    token_stream.nextToken();

    ADVANCE(Token_identifier,  "identifier");
    ADVANCE('=', "=");

    NameAST *name = 0;
    if (!parseName(name)) {
        reportError(("Namespace name expected"));
    }

    ADVANCE(';', ";");

    return true;

}

bool Parser::parseDeclarationStatement(StatementAST *&node)
{
    std::size_t start = token_stream.cursor();

    DeclarationAST *decl = 0;
    if (!parseBlockDeclaration(decl))
        return false;

    DeclarationStatementAST *ast = CreateNode<DeclarationStatementAST>(m_pool);
    ast->setDeclaration(decl);
    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

bool Parser::parseDeclarationInternal(DeclarationAST *&node)
{
    std::size_t start = token_stream.cursor();

    // that is for the case '__declspec(dllexport) int ...' or
    // '__declspec(dllexport) inline int ...', etc.
    AST *winDeclSpec = 0;
    parseWinDeclSpec(winDeclSpec);

    AST *funSpec = 0;
    bool hasFunSpec = parseFunctionSpecifier(funSpec);

    AST *cv = 0;
    parseCvQualify(cv);

    AST *storageSpec = 0;
    bool hasStorageSpec = parseStorageClassSpecifier(storageSpec);

    if (hasStorageSpec && !hasFunSpec)
        hasFunSpec = parseFunctionSpecifier(funSpec);

    // that is for the case 'friend __declspec(dllexport) ....'
    AST *winDeclSpec2 = 0;
    parseWinDeclSpec(winDeclSpec2);

    if (!cv)
        parseCvQualify(cv);

    int index = token_stream.cursor();
    NameAST *name = 0;
    if (parseName(name, true) && token_stream.lookAhead() == '(') {
        // no type specifier, maybe a constructor or a cast operator??

        token_stream.rewind(index);

        InitDeclaratorAST *declarator = 0;
        if (parseInitDeclarator(declarator)) {
            switch(token_stream.lookAhead()) {
            case ';':
                {
                    token_stream.nextToken();

                    InitDeclaratorListAST *declarators = CreateNode<InitDeclaratorListAST>(m_pool);

                    // update declarators position
                    if (declarator)
                        declarators->setPosition(declarator->startToken(), declarator->endToken());
                    declarators->addInitDeclarator(declarator);

                    SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(m_pool);
                    ast->setInitDeclaratorList(declarators);
                    node = ast;
                    UPDATE_POS(node, start, token_stream.cursor());
                    return true;

                }
                break;

            case ':':
                {
                    AST *ctorInit = 0;
                    StatementListAST *funBody = 0;
                    if (parseCtorInitializer(ctorInit) && parseFunctionBody(funBody)) {
                        FunctionDefinitionAST *ast = CreateNode<FunctionDefinitionAST>(m_pool);
                        ast->setStorageSpecifier(storageSpec);
                        ast->setFunctionSpecifier(funSpec);
                        ast->setInitDeclarator(declarator);
                        ast->setFunctionBody(funBody);
                        node = ast;
                        UPDATE_POS(node, start, token_stream.cursor());
                        return true;
                    }
                }
                break;

            case '{':
                {
                    StatementListAST *funBody = 0;
                    if (parseFunctionBody(funBody)) {
                        FunctionDefinitionAST *ast = CreateNode<FunctionDefinitionAST>(m_pool);
                        ast->setStorageSpecifier(storageSpec);
                        ast->setFunctionSpecifier(funSpec);
                        ast->setInitDeclarator(declarator);
                        ast->setFunctionBody(funBody);
                        node = ast;
                        UPDATE_POS(node, start, token_stream.cursor());
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
    }

start_decl:
    token_stream.rewind(index);

    if (token_stream.lookAhead() == Token_const && token_stream.lookAhead(1) == Token_identifier && token_stream.lookAhead(2) == '=') {
        // constant definition
        token_stream.nextToken();
        InitDeclaratorListAST *declarators = 0;
        if (parseInitDeclaratorList(declarators)) {
            ADVANCE(';', ";");
            DeclarationAST *ast = CreateNode<DeclarationAST>(m_pool);
            node = ast;
            UPDATE_POS(node, start, token_stream.cursor());
            return true;
        }
        syntaxError();
        return false;
    }

    TypeSpecifierAST *spec = 0;
    if (parseTypeSpecifier(spec)) {
        if (!hasFunSpec)
            parseFunctionSpecifier(funSpec);         // e.g. "void inline"
        spec->setCvQualify(cv);

        InitDeclaratorListAST *declarators = 0;

        InitDeclaratorAST *decl = 0;
        int startDeclarator = token_stream.cursor();
        bool maybeFunctionDefinition = false;

        if (token_stream.lookAhead() != ';') {
            if (parseInitDeclarator(decl) && token_stream.lookAhead() == '{') {
                // function definition
                maybeFunctionDefinition = true;
            } else {
                token_stream.rewind(startDeclarator);
                if (!parseInitDeclaratorList(declarators)) {
                    syntaxError();
                    return false;
                }
            }
        }

        switch(token_stream.lookAhead()) {
        case ';':
            {
                token_stream.nextToken();
                SimpleDeclarationAST *ast = CreateNode<SimpleDeclarationAST>(m_pool);
                ast->setStorageSpecifier(storageSpec);
                ast->setFunctionSpecifier(funSpec);
                ast->setTypeSpec(spec);
                ast->setWinDeclSpec(winDeclSpec);
                ast->setInitDeclaratorList(declarators);
                node = ast;
                UPDATE_POS(node, start, token_stream.cursor());
            }
            return true;

        case '{':
            {
                if (!maybeFunctionDefinition) {
                    syntaxError();
                    return false;
                }
                StatementListAST *funBody = 0;
                if (parseFunctionBody(funBody)) {
                    FunctionDefinitionAST *ast = CreateNode<FunctionDefinitionAST>(m_pool);
                    ast->setWinDeclSpec(winDeclSpec);
                    ast->setStorageSpecifier(storageSpec);
                    ast->setFunctionSpecifier(funSpec);
                    ast->setTypeSpec(spec);
                    ast->setInitDeclarator(decl);
                    ast->setFunctionBody(funBody);
                    node = ast;
                    UPDATE_POS(node, start, token_stream.cursor());
                    return true;
                }
            }
            break;

        }
    }

    syntaxError();
    return false;
}

bool Parser::skipFunctionBody(StatementListAST *&node)
{
    if (token_stream.lookAhead() != '{') {
        return false;
    }

    skip('{', '}');

    if (token_stream.lookAhead() != '}') {
        reportError(("} expected"));
    } else
        token_stream.nextToken();

    node = 0;
    return true;
}

bool Parser::parseFunctionBody(StatementListAST *&node)
{
    if (control->skipFunctionBody())
        return skipFunctionBody(node);

    std::size_t start = token_stream.cursor();
    if (token_stream.lookAhead() != '{') {
        return false;
    }
    token_stream.nextToken();

    StatementListAST *ast = CreateNode<StatementListAST>(m_pool);

    while (token_stream.lookAhead()) {
        if (token_stream.lookAhead() == '}')
            break;

        StatementAST *stmt = 0;
        std::size_t startStmt = token_stream.cursor();
        if (!parseStatement(stmt)) {
            syntaxError();
            if (startStmt == token_stream.cursor())
                token_stream.nextToken();
            skipUntilStatement();
        } else
            ast->addStatement(stmt);
    }

    if (token_stream.lookAhead() != '}') {
        reportError(("} expected"));
    } else
        token_stream.nextToken();

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
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
    if (token_stream.lookAhead() != Token_try) {
        return false;
    }
    token_stream.nextToken();

    StatementAST *stmt = 0;
    if (!parseCompoundStatement(stmt)) {
        syntaxError();
        return false;
    }

    if (token_stream.lookAhead() != Token_catch) {
        reportError(("catch expected"));
        return false;
    }

    while (token_stream.lookAhead() == Token_catch) {
        token_stream.nextToken();
        ADVANCE('(', "(");
        ConditionAST *cond = 0;
        if (token_stream.lookAhead() == Token_ellipsis) {
            token_stream.nextToken();
        } else if (!parseCondition(cond, false)) {
            reportError(("condition expected"));
            return false;
        }
        ADVANCE(')', ")");

        StatementAST *body = 0;
        if (!parseCompoundStatement(body)) {
            syntaxError();
            return false;
        }
    }

    node = stmt;
    return true;
}

bool Parser::parsePrimaryExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = CreateExpression<NodeType_PrimaryExpression>(m_pool);

    switch(token_stream.lookAhead()) {
        case Token_string_literal:
        {
            AST *lit = 0;
            parseStringLiteral(lit);
            if (lit)
                lit->setParent(ast);
        }
        break;

        case Token_number_literal:
        case Token_char_literal:
        case Token_true:
        case Token_false:
        case Token_this:
            {
                AST_FROM_TOKEN(opNode, token_stream.cursor());
                opNode->setParent(ast);
                token_stream.nextToken();
            }
            break;

        case '(':
            {
                token_stream.nextToken();

                if (token_stream.lookAhead() == '{') {
                    StatementAST *stmt = 0;
                    if (!parseCompoundStatement(stmt))
                        return false;
                    if (stmt)
                        stmt->setParent(ast);
                } else {
                    AbstractExpressionAST *expr = 0;
                    if (!parseExpression(expr)) {
                        return false;
                    }
                    if (expr)
                        expr->setParent(ast);
                }
                CHECK(')', ")");
            }
            break;

        default:
            {
/*  ### reenable me
                TypeSpecifierAST *typeSpec = 0;
                if (parseSimpleTypeSpecifier(typeSpec) && token_stream.lookAhead() == '(') {
                    assert (0);
                    token_stream.nextToken();
                    AbstractExpressionAST *expr = 0;
                    parseCommaExpression(expr);
                    CHECK(')', ")");
                    break;

                    if (typeSpec)
                        typeSpec->setParent(ast);

                    if (expr)
                        expr->setParent(ast);
                }

                token_stream.rewind(start);
*/

                NameAST *name = 0;
                if (!parseName(name))
                    return false;

                name->setParent(ast);

                break;
            }
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
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
bool Parser::parsePostfixExpressionInternal(AbstractExpressionAST *postfixExpr, AbstractExpressionAST *&node)
{
    assert (postfixExpr);

    std::size_t start = token_stream.cursor();

    switch (token_stream.lookAhead()) {
        case '[':
        {
            token_stream.nextToken();
            AbstractExpressionAST *expr = 0;
            parseExpression(expr);
            CHECK(']', "]");

            SubscriptingAST *ast = CreateNode<SubscriptingAST>(m_pool);
            ast->setExpression(postfixExpr);
            ast->setSubscript(expr);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
        }
        return true;

        case '(':
        {
            token_stream.nextToken();
            AbstractExpressionAST *expr = 0;
            parseExpression(expr);
            CHECK(')', ")");

            FunctionCallAST *ast = CreateNode<FunctionCallAST>(m_pool);
            ast->setExpression(postfixExpr);
            ast->setArguments(expr);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
        }
        return true;

        case '.':
        case Token_arrow:
        {
            AST_FROM_TOKEN(op, token_stream.cursor());

            token_stream.nextToken();
            bool templ = (token_stream.lookAhead() == Token_template);
            if (templ)
                token_stream.nextToken();

            NameAST *name = 0;
            if (!parseName(name, templ))
                return false;

            ClassMemberAccessAST *ast = CreateNode<ClassMemberAccessAST>(m_pool);
            ast->setOp(op);
            ast->setExpression(postfixExpr);
            ast->setName(name);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
        }
        return true;

        case Token_incr:
        case Token_decr:
        {
            AST_FROM_TOKEN(op, token_stream.cursor());
            token_stream.nextToken();

            IncrDecrAST *ast = CreateNode<IncrDecrAST>(m_pool);
            ast->setExpression(postfixExpr);
            ast->setOp(op);
            UPDATE_POS(ast, start, token_stream.cursor());
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
bool Parser::parsePostfixExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    switch (token_stream.lookAhead()) {
        case Token_dynamic_cast:
        case Token_static_cast:
        case Token_reinterpret_cast:
        case Token_const_cast:
        {
            AST_FROM_TOKEN(castOp, token_stream.cursor());

            token_stream.nextToken();
            CHECK('<', "<");
            TypeIdAST *typeId = 0;
            parseTypeId(typeId);
            CHECK('>', ">");

            CHECK('(', ")");
            AbstractExpressionAST *expr = 0;
            parseCommaExpression(expr);
            CHECK(')', ")");

            CppCastExpressionAST *tmp = CreateNode<CppCastExpressionAST>(m_pool);
            tmp->setCastOp(castOp);
            tmp->setTypeId(typeId);
            tmp->setExpression(expr);

            AbstractExpressionAST *ast = tmp;
            AbstractExpressionAST *e = 0;
            while (parsePostfixExpressionInternal(ast, e)) {
                ast = e;
            }

            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
        }
        return true;

        case Token_typename:
        {
            token_stream.nextToken();

            NameAST* name = 0;
            if (!parseName(name, true))
                return false;

            CHECK('(', "(");
            AbstractExpressionAST *expr = 0;
            parseCommaExpression(expr);
            CHECK(')', ")");

            // ### AST
            TypeIdentificationAST *ast = CreateNode<TypeIdentificationAST>(m_pool);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
        }
        return true;

        case Token_typeid:
        {
            token_stream.nextToken();

            CHECK('(', "(");
            TypeIdAST *typeId = 0;
            parseTypeId(typeId);
            CHECK(')', ")");

            TypeIdentificationAST *ast = CreateNode<TypeIdentificationAST>(m_pool);
            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
        }
        return true;

        default:
            break;
    }

    std::size_t saved_pos = token_stream.cursor();

    TypeSpecifierAST *typeSpec = 0;
    AbstractExpressionAST *expr = 0;

    // let's try to parse a type
    NameAST *name = 0;
    if (parseName(name, true)) {
        assert(name->unqualifiedName() != 0);
        bool has_template_args = name->unqualifiedName()->templateArgumentList() != 0;
        if (has_template_args && token_stream.lookAhead() == '(') {
            AbstractExpressionAST *cast_expr = 0;
            if (parseCastExpression(cast_expr) && cast_expr->nodeType() == NodeType_CastExpression) {
                token_stream.rewind(saved_pos);
                parsePrimaryExpression(expr);
                goto L_no_rewind;
            }
        }
    }

    token_stream.rewind(saved_pos);

L_no_rewind:
    if (!expr && parseSimpleTypeSpecifier(typeSpec) && token_stream.lookAhead() == '(') {
        token_stream.nextToken(); // skip '('
        parseCommaExpression(expr);
        CHECK(')', ")");
    } else if (expr) {
        typeSpec = 0;
    } else {
        typeSpec = 0;
        token_stream.rewind(start);

        if (!parsePrimaryExpression(expr))
            return false;
    }

    AbstractExpressionAST *ast = CreateExpression<NodeType_PostfixExpression>(m_pool);
    if (typeSpec)
        typeSpec->setParent(ast);

    if (expr)
        expr->setParent(ast);

    AbstractExpressionAST *e = 0;
    while (parsePostfixExpressionInternal(ast, e)) {
        ast = e;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseUnaryExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    switch(token_stream.lookAhead()) {
        case Token_incr:
        case Token_decr:
        case '*':
        case '&':
        case '+':
        case '-':
        case '!':
        case '~':
        {
            AST_FROM_TOKEN(opNode, token_stream.cursor());

            token_stream.nextToken();
            AbstractExpressionAST *expr = 0;
            if (!parseCastExpression(expr))
                return false;

            AbstractExpressionAST *ast = CreateExpression<NodeType_UnaryExpression>(m_pool);

            opNode->setParent(ast);
            if (expr)
                expr->setParent(ast);

            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
            return true;
        }

        case Token_sizeof:
        {
            AbstractExpressionAST *ast = CreateExpression<NodeType_UnaryExpression>(m_pool);

            AST_FROM_TOKEN(opNode, token_stream.cursor());
            opNode->setParent(ast);

            token_stream.nextToken();
            int index = token_stream.cursor();
            if (token_stream.lookAhead() == '(') {
                token_stream.nextToken();
                TypeIdAST *typeId = 0;
                if (parseTypeId(typeId) && token_stream.lookAhead() == ')') {
                    if (typeId)
                        typeId->setParent(ast);

                    token_stream.nextToken();

                    UPDATE_POS(ast, start, token_stream.cursor());
                    node = ast;
                    return true;
                }
                token_stream.rewind(index);
            }
            AbstractExpressionAST *expr = 0;
            if (!parseUnaryExpression(expr))
                return false;

            UPDATE_POS(ast, start, token_stream.cursor());
            node = ast;
            return true;
        }

        default:
            break;
    }

    int token = token_stream.lookAhead(0);

    if (token == Token_new || (token == Token_scope && token_stream.lookAhead(1) == Token_new))
        return parseNewExpression(node);

    if (token == Token_delete || (token == Token_scope && token_stream.lookAhead(1) == Token_delete))
        return parseDeleteExpression(node);

    return parsePostfixExpression(node);
}

bool Parser::parseNewExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = CreateExpression<NodeType_NewExpression>(m_pool);

    if (token_stream.lookAhead() == Token_scope && token_stream.lookAhead(1) == Token_new) {
        AST_FROM_TOKEN(scopeNode, token_stream.cursor());
        scopeNode->setParent(ast);
        token_stream.nextToken();
    }

    AST_FROM_TOKEN(newNode, token_stream.cursor());
    newNode->setParent(ast);

    CHECK(Token_new, "new");

    if (token_stream.lookAhead() == '(') {
        token_stream.nextToken();
        AbstractExpressionAST *expr = 0;
        parseCommaExpression(expr);
        if (expr)
            expr->setParent(ast);
        CHECK(')', ")");
    }

    if (token_stream.lookAhead() == '(') {
        token_stream.nextToken();
        TypeIdAST *typeId = 0;
        parseTypeId(typeId);
        if (typeId)
            typeId->setParent(ast);
        CHECK(')', ")");
    } else {
        AbstractExpressionAST *typeId = 0;
        parseNewTypeId(typeId);
        if (typeId)
            typeId->setParent(ast);
    }

    AbstractExpressionAST *init = 0;
    parseNewInitializer(init);
    if (init)
        init->setParent(ast);

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseNewTypeId(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    TypeSpecifierAST *typeSpec = 0;
    if (!parseTypeSpecifier(typeSpec))
        return false;

    AbstractExpressionAST *ast = CreateExpression<NodeType_NewTypeId>(m_pool);

    if (typeSpec)
        typeSpec->setParent(ast);

    AbstractExpressionAST *declarator = 0;
    parseNewDeclarator(declarator);
    if (declarator)
        declarator->setParent(ast);

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseNewDeclarator(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = CreateExpression<NodeType_NewInitializer>(m_pool);

    AST *ptrOp = 0;
    if (parsePtrOperator(ptrOp)) {
        if (ptrOp)
            ptrOp->setParent(ast);

        AbstractExpressionAST *declarator = 0;
        parseNewDeclarator(declarator);

        if (declarator)
            declarator->setParent(ast);
    }

    while (token_stream.lookAhead() == '[') {
        token_stream.nextToken();
        AbstractExpressionAST *expr = 0;
        parseExpression(expr);
        ADVANCE(']', "]");

        if (expr)
            expr->setParent(ast);
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseNewInitializer(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != '(')
        return false;

    AbstractExpressionAST *ast = CreateExpression<NodeType_NewInitializer>(m_pool);

    token_stream.nextToken();
    AbstractExpressionAST *expr = 0;
    parseCommaExpression(expr);

    if (expr)
        expr->setParent(ast);

    CHECK(')', ")");

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseDeleteExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = CreateExpression<NodeType_DeleteExpression>(m_pool);

    if (token_stream.lookAhead() == Token_scope && token_stream.lookAhead(1) == Token_delete) {
        AST_FROM_TOKEN(scopeNode, token_stream.cursor());
        scopeNode->setParent(ast);
        token_stream.nextToken();
    }

    AST_FROM_TOKEN(deleteNode, token_stream.cursor());
    deleteNode->setParent(ast);

    CHECK(Token_delete, "delete");

    if (token_stream.lookAhead() == '[') {
        int beg = token_stream.cursor();
        token_stream.nextToken();
        CHECK(']', "]");

        AST *n = CreateNode<AST>(m_pool);
        UPDATE_POS(n, beg, token_stream.cursor());
        n->setParent(ast);
    }

    AbstractExpressionAST *expr = 0;
    if (!parseCastExpression(expr))
        return false;

    if (expr)
        expr->setParent(ast);

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseCastExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() == '(') {
        AbstractExpressionAST *ast = CreateExpression<NodeType_CastExpression>(m_pool);

        token_stream.nextToken();
        TypeIdAST *typeId = 0;
        if (parseTypeId(typeId)) {

            if (typeId)
                typeId->setParent(ast);

            if (token_stream.lookAhead() == ')') {
                token_stream.nextToken();

                AbstractExpressionAST *expr = 0;
                if (parseCastExpression(expr)) {
                    if (expr)
                        expr->setParent(ast);

                    UPDATE_POS(ast, start, token_stream.cursor());
                    node = ast;
                    return true;
                }
            }
        }
    }

    token_stream.rewind(start);
    return parseUnaryExpression(node);
}

bool Parser::parsePmExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseCastExpression(ast) || !ast) // ### fixme
        return false;

    while (token_stream.lookAhead() == Token_ptrmem) {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseCastExpression(rightExpr))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseMultiplicativeExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parsePmExpression(ast))
        return false;

    while (token_stream.lookAhead() == '*' || token_stream.lookAhead() == '/' || token_stream.lookAhead() == '%') {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parsePmExpression(rightExpr))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}


bool Parser::parseAdditiveExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseMultiplicativeExpression(ast))
        return false;

    while (token_stream.lookAhead() == '+' || token_stream.lookAhead() == '-') {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseMultiplicativeExpression(rightExpr))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseShiftExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseAdditiveExpression(ast))
        return false;

    while (token_stream.lookAhead() == Token_shift) {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseAdditiveExpression(rightExpr))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseRelationalExpression(AbstractExpressionAST *&node, bool templArgs)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseShiftExpression(ast))
        return false;

    while (token_stream.lookAhead() == '<' || (token_stream.lookAhead() == '>' && !templArgs) ||
           token_stream.lookAhead() == Token_leq || token_stream.lookAhead() == Token_geq) {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseShiftExpression(rightExpr))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseEqualityExpression(AbstractExpressionAST *&node, bool templArgs)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseRelationalExpression(ast, templArgs))
        return false;

    while (token_stream.lookAhead() == Token_eq || token_stream.lookAhead() == Token_not_eq) {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseRelationalExpression(rightExpr, templArgs))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseAndExpression(AbstractExpressionAST *&node, bool templArgs)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseEqualityExpression(ast, templArgs))
        return false;

    while (token_stream.lookAhead() == '&') {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseEqualityExpression(rightExpr, templArgs))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseExclusiveOrExpression(AbstractExpressionAST *&node, bool templArgs)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseAndExpression(ast, templArgs))
        return false;

    while (token_stream.lookAhead() == '^') {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseAndExpression(rightExpr, templArgs))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseInclusiveOrExpression(AbstractExpressionAST *&node, bool templArgs)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseExclusiveOrExpression(ast, templArgs))
        return false;

    while (token_stream.lookAhead() == '|') {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseExclusiveOrExpression(rightExpr, templArgs))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseLogicalAndExpression(AbstractExpressionAST *&node, bool templArgs)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseInclusiveOrExpression(ast, templArgs))
        return false;

    while (token_stream.lookAhead() == Token_and) {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseInclusiveOrExpression(rightExpr, templArgs))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseLogicalOrExpression(AbstractExpressionAST *&node, bool templArgs)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseLogicalAndExpression(ast, templArgs))
        return false;

    while (token_stream.lookAhead() == Token_or) {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseLogicalAndExpression(rightExpr, templArgs))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseConditionalExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseLogicalOrExpression(ast))
        return false;

    if (token_stream.lookAhead() == '?') {
        token_stream.nextToken();

        AbstractExpressionAST *leftExpr = 0;
        if (!parseExpression(leftExpr))
            return false;

        CHECK(':', ":");

        AbstractExpressionAST *rightExpr = 0;
        if (!parseAssignmentExpression(rightExpr))
            return false;

        ConditionalExpressionAST *tmp = CreateNode<ConditionalExpressionAST>(m_pool);
        tmp->setCondition(ast);
        tmp->setLeftExpression(leftExpr);
        tmp->setRightExpression(rightExpr);
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseAssignmentExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (token_stream.lookAhead() == Token_throw && !parseThrowExpression(ast))
        return false;
    else if (!parseConditionalExpression(ast))
        return false;

    while (token_stream.lookAhead() == Token_assign || token_stream.lookAhead() == '=') {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseConditionalExpression(rightExpr))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseConstantExpression(AbstractExpressionAST *&node)
{
    return parseConditionalExpression(node);
}

bool Parser::parseExpression(AbstractExpressionAST *&node)
{
    return parseCommaExpression(node);
}

bool Parser::parseCommaExpression(AbstractExpressionAST *&node)
{
    std::size_t start = token_stream.cursor();

    AbstractExpressionAST *ast = 0;
    if (!parseAssignmentExpression(ast))
        return false;

    while (token_stream.lookAhead() == ',') {
        int startOp = token_stream.cursor();
        AST_FROM_TOKEN(op, startOp);
        token_stream.nextToken();

        AbstractExpressionAST *rightExpr = 0;
        if (!parseAssignmentExpression(rightExpr))
            return false;

        BinaryExpressionAST *tmp = CreateNode<BinaryExpressionAST>(m_pool);
        tmp->setOp(op);
        tmp->setLeftExpression(ast);
        tmp->setRightExpression(rightExpr);
        UPDATE_POS(tmp, startOp, token_stream.cursor());
        ast = tmp;
    }

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;
    return true;
}

bool Parser::parseThrowExpression(AbstractExpressionAST *&node)
{
    if (token_stream.lookAhead() != Token_throw)
        return false;

    std::size_t start = token_stream.cursor();

    AST_FROM_TOKEN(throwNode, token_stream.cursor());
    CHECK(Token_throw, "throw");
    AbstractExpressionAST *expr = 0;
    if (!parseAssignmentExpression(expr))
        return false;

    AbstractExpressionAST *ast = CreateExpression<NodeType_ThrowExpression>(m_pool);
    throwNode->setParent(ast);
    if (expr)
        expr->setParent(ast);

    UPDATE_POS(ast, start, token_stream.cursor());
    node = ast;

    return true;
}

// ### Objective C++
bool Parser::parseIvarDeclList(AST *& /*node*/)
{
    return false;
}

bool Parser::parseIvarDecls(AST *& /*node*/)
{
    return false;
}

bool Parser::parseIvarDecl(AST *& /*node*/)
{
    return false;
}

bool Parser::parseIvars(AST *& /*node*/)
{
    return false;
}

bool Parser::parseIvarDeclarator(AST *& /*node*/)
{
    return false;
}

bool Parser::parseMethodDecl(AST *& /*node*/)
{
    return false;
}

bool Parser::parseUnarySelector(AST *& /*node*/)
{
    return false;
}

bool Parser::parseKeywordSelector(AST *& /*node*/)
{
    return false;
}

bool Parser::parseSelector(AST *& /*node*/)
{
    return false;
}

bool Parser::parseKeywordDecl(AST *& /*node*/)
{
    return false;
}

bool Parser::parseReceiver(AST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcMessageExpr(AST *& /*node*/)
{
    return false;
}

bool Parser::parseMessageArgs(AST *& /*node*/)
{
    return false;
}

bool Parser::parseKeywordExpr(AST *& /*node*/)
{
    return false;
}

bool Parser::parseKeywordArgList(AST *& /*node*/)
{
    return false;
}

bool Parser::parseKeywordArg(AST *& /*node*/)
{
    return false;
}

bool Parser::parseReservedWord(AST *& /*node*/)
{
    return false;
}

bool Parser::parseMyParms(AST *& /*node*/)
{
    return false;
}

bool Parser::parseMyParm(AST *& /*node*/)
{
    return false;
}

bool Parser::parseOptParmList(AST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcSelectorExpr(AST *& /*node*/)
{
    return false;
}

bool Parser::parseSelectorArg(AST *& /*node*/)
{
    return false;
}

bool Parser::parseKeywordNameList(AST *& /*node*/)
{
    return false;
}

bool Parser::parseKeywordName(AST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcEncodeExpr(AST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcString(AST *& /*node*/)
{
    return false;
}

bool Parser::parseProtocolRefs(AST *& /*node*/)
{
    return false;
}

bool Parser::parseIdentifierList(AST *& node)
{
    std::size_t start = token_stream.cursor();

    if (token_stream.lookAhead() != Token_identifier)
        return false;

    AST *ast = CreateNode<AST>(m_pool);

    AST_FROM_TOKEN(tk, token_stream.cursor());
    tk->setParent(ast);
    token_stream.nextToken();

    while (token_stream.lookAhead() == ',') {
        token_stream.nextToken();
        if (token_stream.lookAhead() == Token_identifier) {
            AST_FROM_TOKEN(tk, token_stream.cursor());
            tk->setParent(ast);
            token_stream.nextToken();
        }
        ADVANCE(Token_identifier, "identifier");
    }

    node = ast;
    UPDATE_POS(node, start, token_stream.cursor());
    return true;
}

bool Parser::parseIdentifierColon(AST *& /*node*/)
{
    if (token_stream.lookAhead() == Token_identifier && token_stream.lookAhead(1) == ':') {
        token_stream.nextToken();
        token_stream.nextToken();
        return true;
    } // ### else if PTYPENAME -> return true ;

    return false;
}

bool Parser::parseObjcProtocolExpr(AST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcOpenBracketExpr(AST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcCloseBracket(AST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcDef(DeclarationAST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcClassDef(DeclarationAST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcClassDecl(DeclarationAST *& /*node*/)
{
    ADVANCE(OBJC_CLASS, "@class");

    AST *idList = 0;
    parseIdentifierList(idList);
    ADVANCE(';', ";");

    return true;
}

bool Parser::parseObjcProtocolDecl(DeclarationAST *& /*node*/)
{
    ADVANCE(OBJC_PROTOCOL, "@protocol");

    AST *idList = 0;
    parseIdentifierList(idList);
    ADVANCE(';', ";");

    return true;
}

bool Parser::parseObjcAliasDecl(DeclarationAST *& /*node*/)
{
    ADVANCE(OBJC_ALIAS, "@alias");

    AST *idList = 0;
    parseIdentifierList(idList);
    ADVANCE(';', ";");

    return true;
}

bool Parser::parseObjcProtocolDef(DeclarationAST *& /*node*/)
{
    return false;
}

bool Parser::parseObjcMethodDef(DeclarationAST *& /*node*/)
{
    return false;
}

bool Parser::parseWinDeclSpec(AST *& node)
{
    if (token_stream.lookAhead() == Token_identifier
            && token_stream.lookAhead(1) == '('
            && token_stream.currentTokenText() == "__declspec") {
        std::size_t start = token_stream.cursor();
        token_stream.nextToken();
        token_stream.nextToken(); // skip '('

        parseIdentifierList(node);
        ADVANCE(')', ")");

        UPDATE_POS(node, start, token_stream.cursor());
        return true;
    }

    return false;
}

void Parser::advance()
{
    token_stream.nextToken();
}

