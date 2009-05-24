// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "qmakeparser.h"


#include "qmakelexer.h"
#include <kdebug.h>
#include <QtCore/QString>

namespace QMake
{

void Parser::tokenize( const QString& contents )
{
    m_contents = contents;
    QMake::Lexer lexer( this, contents );
    int kind = Parser::Token_EOF;

    do
    {
        kind = lexer.nextTokenKind();

        if ( !kind ) // when the lexer returns 0, the end of file is reached
            kind = Parser::Token_EOF;

        Parser::Token &t = this->tokenStream->next();
        t.kind = kind;
        if ( t.kind == Parser::Token_EOF )
        {
            t.begin = -1;
            t.end = -1;
        }
        else
        {
            t.begin = lexer.tokenBegin();
            t.end = lexer.tokenEnd();
        }

        if ( m_debug )
        {
            kDebug(9024) << kind << "(" << t.begin << "," << t.end << ")::" << tokenText(t.begin, t.end);
        }

    }
    while ( kind != Parser::Token_EOF );

    this->yylex(); // produce the look ahead token
}

QString Parser::tokenText( qint64 begin, qint64 end ) const
{
    return m_contents.mid((int)begin, (int)end-begin+1);
}

void Parser::reportProblem( Parser::ProblemType type, const QString& message )
{
    if (type == Error)
        kDebug(9024) << "** ERROR:" << message;
    else if (type == Warning)
        kDebug(9024) << "** WARNING:" << message;
    else if (type == Info)
        kDebug(9024) << "** Info:" << message;
}


// custom error recovery
void Parser::expectedToken(int /*expected*/, qint64 /*where*/, const QString& name)
{
    reportProblem(
        Parser::Error,
        QString("Expected token \"%1\"").arg(name));
}

void Parser::expectedSymbol(int /*expected_symbol*/, const QString& name)
{
    qint64 line;
    qint64 col;
    size_t index = tokenStream->index()-1;
    Token &token = tokenStream->token(index);
    kDebug(9024) << "token starts at:" << token.begin;
    kDebug(9024) << "index is:" << index;
    tokenStream->startPosition(index, &line, &col);
    QString tokenValue = tokenText(token.begin, token.end);
    reportProblem(
        Parser::Error,
        QString("Expected symbol \"%1\" (current token: \"%2\" [%3] at line: %4 col: %5)")
        .arg(name)
        .arg(token.kind != 0 ? tokenValue : "EOF")
        .arg(token.kind)
        .arg(line)
        .arg(col));
}

void Parser::setDebug( bool debug )
{
    m_debug = debug;
}

} // end of namespace QMake


namespace QMake
{

bool Parser::parseArgumentList(ArgumentListAst **yynode)
{
    *yynode = create<ArgumentListAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_VALUE
        || yytoken == Token_CONT || yytoken == Token_EOF
        || yytoken == Token_RPAREN)
    {
        if (yytoken == Token_VALUE
            || yytoken == Token_CONT)
        {
            if (yytoken == Token_VALUE)
            {
                ValueAst *__node_0 = 0;
                if (!parseValue(&__node_0))
                {
                    expectedSymbol(AstNode::ValueKind, "value");
                    return false;
                }
                (*yynode)->argsSequence = snoc((*yynode)->argsSequence, __node_0, memoryPool);

            }
            else if (yytoken == Token_CONT)
            {
                if (yytoken != Token_CONT)
                {
                    expectedToken(yytoken, Token_CONT, "cont");
                    return false;
                }
                yylex();

                if (yytoken != Token_NEWLINE)
                {
                    expectedToken(yytoken, Token_NEWLINE, "newline");
                    return false;
                }
                yylex();

            }
            else
            {
                return false;
            }
            while (yytoken == Token_CONT
                   || yytoken == Token_COMMA)
            {
                if (yytoken == Token_COMMA)
                {
                    if (yytoken != Token_COMMA)
                    {
                        expectedToken(yytoken, Token_COMMA, "comma");
                        return false;
                    }
                    yylex();

                }
                else if (yytoken == Token_CONT)
                {
                    if (yytoken != Token_CONT)
                    {
                        expectedToken(yytoken, Token_CONT, "cont");
                        return false;
                    }
                    yylex();

                    if (yytoken != Token_NEWLINE)
                    {
                        expectedToken(yytoken, Token_NEWLINE, "newline");
                        return false;
                    }
                    yylex();

                }
                else
                {
                    return false;
                }
                ValueAst *__node_1 = 0;
                if (!parseValue(&__node_1))
                {
                    expectedSymbol(AstNode::ValueKind, "value");
                    return false;
                }
                (*yynode)->argsSequence = snoc((*yynode)->argsSequence, __node_1, memoryPool);

            }
        }
        else if (true /*epsilon*/)
        {
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseFunctionArguments(FunctionArgumentsAst **yynode)
{
    *yynode = create<FunctionArgumentsAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_LPAREN)
    {
        if (yytoken != Token_LPAREN)
        {
            expectedToken(yytoken, Token_LPAREN, "lparen");
            return false;
        }
        yylex();

        ArgumentListAst *__node_2 = 0;
        if (!parseArgumentList(&__node_2))
        {
            expectedSymbol(AstNode::ArgumentListKind, "argumentList");
            return false;
        }
        (*yynode)->args = __node_2;

        if (yytoken != Token_RPAREN)
        {
            expectedToken(yytoken, Token_RPAREN, "rparen");
            return false;
        }
        yylex();

    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseItem(ItemAst **yynode)
{
    *yynode = create<ItemAst>();

    (*yynode)->startToken = tokenStream->index() - 1;
    (*yynode)->id = -1;

    if (yytoken == Token_IDENTIFIER)
    {
        if (yytoken != Token_IDENTIFIER)
        {
            expectedToken(yytoken, Token_IDENTIFIER, "identifier");
            return false;
        }
        (*yynode)->id = tokenStream->index() - 1;
        yylex();

        if (yytoken == Token_LPAREN)
        {
            FunctionArgumentsAst *__node_3 = 0;
            if (!parseFunctionArguments(&__node_3))
            {
                expectedSymbol(AstNode::FunctionArgumentsKind, "functionArguments");
                return false;
            }
            (*yynode)->functionArguments = __node_3;

        }
        else if (true /*epsilon*/)
        {
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseOp(OpAst **yynode)
{
    *yynode = create<OpAst>();

    (*yynode)->startToken = tokenStream->index() - 1;
    (*yynode)->optoken = -1;
    (*yynode)->optoken = -1;
    (*yynode)->optoken = -1;
    (*yynode)->optoken = -1;
    (*yynode)->optoken = -1;

    if (yytoken == Token_MINUSEQ
        || yytoken == Token_EQUAL
        || yytoken == Token_TILDEEQ
        || yytoken == Token_PLUSEQ
        || yytoken == Token_STAREQ)
    {
        if (yytoken == Token_PLUSEQ)
        {
            if (yytoken != Token_PLUSEQ)
            {
                expectedToken(yytoken, Token_PLUSEQ, "pluseq");
                return false;
            }
            (*yynode)->optoken = tokenStream->index() - 1;
            yylex();

        }
        else if (yytoken == Token_MINUSEQ)
        {
            if (yytoken != Token_MINUSEQ)
            {
                expectedToken(yytoken, Token_MINUSEQ, "minuseq");
                return false;
            }
            (*yynode)->optoken = tokenStream->index() - 1;
            yylex();

        }
        else if (yytoken == Token_STAREQ)
        {
            if (yytoken != Token_STAREQ)
            {
                expectedToken(yytoken, Token_STAREQ, "stareq");
                return false;
            }
            (*yynode)->optoken = tokenStream->index() - 1;
            yylex();

        }
        else if (yytoken == Token_EQUAL)
        {
            if (yytoken != Token_EQUAL)
            {
                expectedToken(yytoken, Token_EQUAL, "equal");
                return false;
            }
            (*yynode)->optoken = tokenStream->index() - 1;
            yylex();

        }
        else if (yytoken == Token_TILDEEQ)
        {
            if (yytoken != Token_TILDEEQ)
            {
                expectedToken(yytoken, Token_TILDEEQ, "tildeeq");
                return false;
            }
            (*yynode)->optoken = tokenStream->index() - 1;
            yylex();

        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseOrOperator(OrOperatorAst **yynode)
{
    *yynode = create<OrOperatorAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_OR)
    {
        do
        {
            if (yytoken != Token_OR)
            {
                expectedToken(yytoken, Token_OR, "or");
                return false;
            }
            yylex();

            ItemAst *__node_4 = 0;
            if (!parseItem(&__node_4))
            {
                expectedSymbol(AstNode::ItemKind, "item");
                return false;
            }
            (*yynode)->itemSequence = snoc((*yynode)->itemSequence, __node_4, memoryPool);

        }
        while (yytoken == Token_OR);
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseProject(ProjectAst **yynode)
{
    *yynode = create<ProjectAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_NEWLINE
        || yytoken == Token_EXCLAM
        || yytoken == Token_IDENTIFIER || yytoken == Token_EOF)
    {
        while (yytoken == Token_NEWLINE
               || yytoken == Token_EXCLAM
               || yytoken == Token_IDENTIFIER)
        {
            StatementAst *__node_5 = 0;
            if (!parseStatement(&__node_5))
            {
                expectedSymbol(AstNode::StatementKind, "statement");
                return false;
            }
            (*yynode)->statementsSequence = snoc((*yynode)->statementsSequence, __node_5, memoryPool);

        }
        if (Token_EOF != yytoken)
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseScope(ScopeAst **yynode)
{
    *yynode = create<ScopeAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_COLON
        || yytoken == Token_LPAREN
        || yytoken == Token_OR
        || yytoken == Token_LBRACE)
    {
        if (yytoken == Token_LPAREN)
        {
            FunctionArgumentsAst *__node_6 = 0;
            if (!parseFunctionArguments(&__node_6))
            {
                expectedSymbol(AstNode::FunctionArgumentsKind, "functionArguments");
                return false;
            }
            (*yynode)->functionArguments = __node_6;

            if (yytoken == Token_COLON
                || yytoken == Token_LBRACE)
            {
                ScopeBodyAst *__node_7 = 0;
                if (!parseScopeBody(&__node_7))
                {
                    expectedSymbol(AstNode::ScopeBodyKind, "scopeBody");
                    return false;
                }
                (*yynode)->scopeBody = __node_7;

            }
            else if (yytoken == Token_OR)
            {
                OrOperatorAst *__node_8 = 0;
                if (!parseOrOperator(&__node_8))
                {
                    expectedSymbol(AstNode::OrOperatorKind, "orOperator");
                    return false;
                }
                (*yynode)->orOperator = __node_8;

                ScopeBodyAst *__node_9 = 0;
                if (!parseScopeBody(&__node_9))
                {
                    expectedSymbol(AstNode::ScopeBodyKind, "scopeBody");
                    return false;
                }
                (*yynode)->scopeBody = __node_9;

            }
            else if (true /*epsilon*/)
            {
            }
            else
            {
                return false;
            }
        }
        else if (yytoken == Token_COLON
                 || yytoken == Token_OR
                 || yytoken == Token_LBRACE)
        {
            if (yytoken == Token_OR)
            {
                OrOperatorAst *__node_10 = 0;
                if (!parseOrOperator(&__node_10))
                {
                    expectedSymbol(AstNode::OrOperatorKind, "orOperator");
                    return false;
                }
                (*yynode)->orOperator = __node_10;

            }
            else if (true /*epsilon*/)
            {
            }
            else
            {
                return false;
            }
            ScopeBodyAst *__node_11 = 0;
            if (!parseScopeBody(&__node_11))
            {
                expectedSymbol(AstNode::ScopeBodyKind, "scopeBody");
                return false;
            }
            (*yynode)->scopeBody = __node_11;

        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseScopeBody(ScopeBodyAst **yynode)
{
    *yynode = create<ScopeBodyAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_COLON
        || yytoken == Token_LBRACE)
    {
        if (yytoken == Token_LBRACE)
        {
            if (yytoken != Token_LBRACE)
            {
                expectedToken(yytoken, Token_LBRACE, "lbrace");
                return false;
            }
            yylex();

            if (yytoken == Token_NEWLINE)
            {
                if (yytoken != Token_NEWLINE)
                {
                    expectedToken(yytoken, Token_NEWLINE, "newline");
                    return false;
                }
                yylex();

            }
            else if (true /*epsilon*/)
            {
            }
            else
            {
                return false;
            }
            while (yytoken == Token_NEWLINE
                   || yytoken == Token_EXCLAM
                   || yytoken == Token_IDENTIFIER)
            {
                StatementAst *__node_12 = 0;
                if (!parseStatement(&__node_12))
                {
                    expectedSymbol(AstNode::StatementKind, "statement");
                    return false;
                }
                (*yynode)->statementsSequence = snoc((*yynode)->statementsSequence, __node_12, memoryPool);

            }
            if (yytoken != Token_RBRACE)
            {
                expectedToken(yytoken, Token_RBRACE, "rbrace");
                return false;
            }
            yylex();

        }
        else if (yytoken == Token_COLON)
        {
            if (yytoken != Token_COLON)
            {
                expectedToken(yytoken, Token_COLON, "colon");
                return false;
            }
            yylex();

            StatementAst *__node_13 = 0;
            if (!parseStatement(&__node_13))
            {
                expectedSymbol(AstNode::StatementKind, "statement");
                return false;
            }
            (*yynode)->statementsSequence = snoc((*yynode)->statementsSequence, __node_13, memoryPool);

        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseStatement(StatementAst **yynode)
{
    *yynode = create<StatementAst>();

    (*yynode)->startToken = tokenStream->index() - 1;
    (*yynode)->id = -1;
    (*yynode)->id = -1;

    if (yytoken == Token_NEWLINE
        || yytoken == Token_EXCLAM
        || yytoken == Token_IDENTIFIER)
    {
        if (yytoken == Token_IDENTIFIER)
        {
            if (yytoken != Token_IDENTIFIER)
            {
                expectedToken(yytoken, Token_IDENTIFIER, "identifier");
                return false;
            }
            (*yynode)->id = tokenStream->index() - 1;
            yylex();

            if (yytoken == Token_MINUSEQ
                || yytoken == Token_EQUAL
                || yytoken == Token_TILDEEQ
                || yytoken == Token_PLUSEQ
                || yytoken == Token_STAREQ)
            {
                VariableAssignmentAst *__node_14 = 0;
                if (!parseVariableAssignment(&__node_14))
                {
                    expectedSymbol(AstNode::VariableAssignmentKind, "variableAssignment");
                    return false;
                }
                (*yynode)->var = __node_14;

            }
            else if (yytoken == Token_COLON
                     || yytoken == Token_LPAREN
                     || yytoken == Token_OR
                     || yytoken == Token_LBRACE)
            {
                ScopeAst *__node_15 = 0;
                if (!parseScope(&__node_15))
                {
                    expectedSymbol(AstNode::ScopeKind, "scope");
                    return false;
                }
                (*yynode)->scope = __node_15;

            }
            else
            {
                return false;
            }

            (*yynode)->isNewline = false;
            (*yynode)->isExclam = false;
        }
        else if (yytoken == Token_EXCLAM)
        {
            if (yytoken != Token_EXCLAM)
            {
                expectedToken(yytoken, Token_EXCLAM, "exclam");
                return false;
            }
            yylex();

            if (yytoken != Token_IDENTIFIER)
            {
                expectedToken(yytoken, Token_IDENTIFIER, "identifier");
                return false;
            }
            (*yynode)->id = tokenStream->index() - 1;
            yylex();

            ScopeAst *__node_16 = 0;
            if (!parseScope(&__node_16))
            {
                expectedSymbol(AstNode::ScopeKind, "scope");
                return false;
            }
            (*yynode)->scope = __node_16;


            (*yynode)->isNewline = false;
            (*yynode)->isExclam = true;
        }
        else if (yytoken == Token_NEWLINE)
        {
            if (yytoken != Token_NEWLINE)
            {
                expectedToken(yytoken, Token_NEWLINE, "newline");
                return false;
            }
            yylex();


            (*yynode)->isNewline = true;
            (*yynode)->isExclam = false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseValue(ValueAst **yynode)
{
    *yynode = create<ValueAst>();

    (*yynode)->startToken = tokenStream->index() - 1;
    (*yynode)->value = -1;

    if (yytoken == Token_VALUE)
    {
        if (yytoken != Token_VALUE)
        {
            expectedToken(yytoken, Token_VALUE, "value");
            return false;
        }
        (*yynode)->value = tokenStream->index() - 1;
        yylex();

    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseValueList(ValueListAst **yynode)
{
    *yynode = create<ValueListAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_VALUE
        || yytoken == Token_CONT)
    {
        do
        {
            if (yytoken == Token_VALUE)
            {
                ValueAst *__node_17 = 0;
                if (!parseValue(&__node_17))
                {
                    expectedSymbol(AstNode::ValueKind, "value");
                    return false;
                }
                (*yynode)->listSequence = snoc((*yynode)->listSequence, __node_17, memoryPool);

            }
            else if (yytoken == Token_CONT)
            {
                if (yytoken != Token_CONT)
                {
                    expectedToken(yytoken, Token_CONT, "cont");
                    return false;
                }
                yylex();

                if (yytoken != Token_NEWLINE)
                {
                    expectedToken(yytoken, Token_NEWLINE, "newline");
                    return false;
                }
                yylex();

            }
            else
            {
                return false;
            }
        }
        while (yytoken == Token_VALUE
               || yytoken == Token_CONT);
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}

bool Parser::parseVariableAssignment(VariableAssignmentAst **yynode)
{
    *yynode = create<VariableAssignmentAst>();

    (*yynode)->startToken = tokenStream->index() - 1;

    if (yytoken == Token_MINUSEQ
        || yytoken == Token_EQUAL
        || yytoken == Token_TILDEEQ
        || yytoken == Token_PLUSEQ
        || yytoken == Token_STAREQ)
    {
        OpAst *__node_18 = 0;
        if (!parseOp(&__node_18))
        {
            expectedSymbol(AstNode::OpKind, "op");
            return false;
        }
        (*yynode)->op = __node_18;

        if (yytoken == Token_VALUE
            || yytoken == Token_CONT)
        {
            ValueListAst *__node_19 = 0;
            if (!parseValueList(&__node_19))
            {
                expectedSymbol(AstNode::ValueListKind, "valueList");
                return false;
            }
            (*yynode)->values = __node_19;

            if (yytoken == Token_NEWLINE)
            {
                if (yytoken != Token_NEWLINE)
                {
                    expectedToken(yytoken, Token_NEWLINE, "newline");
                    return false;
                }
                yylex();

            }
            else if (true /*epsilon*/)
            {
            }
            else
            {
                return false;
            }
        }
        else if (yytoken == Token_NEWLINE)
        {
            if (yytoken != Token_NEWLINE)
            {
                expectedToken(yytoken, Token_NEWLINE, "newline");
                return false;
            }
            yylex();

        }
        else if (true /*epsilon*/)
        {
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    (*yynode)->endToken = tokenStream->index() - 1;

    return true;
}


} // end of namespace QMake

