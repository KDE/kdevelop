// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef QMAKE_H_INCLUDED
#define QMAKE_H_INCLUDED

#include "qmakeast.h"
#include <kdev-pg-memory-pool.h>
#include <kdev-pg-allocator.h>
#include <kdev-pg-token-stream.h>

namespace QMake
{

class  Parser
{
public:
    typedef KDevPG::TokenStream::Token Token;
    KDevPG::TokenStream *tokenStream;
    int yytoken;

    inline Token LA(qint64 k = 1) const
    {
        return tokenStream->token(tokenStream->index() - 1 + k - 1);
    }
    inline int yylex()
    {
        return (yytoken = tokenStream->nextToken());
    }
    inline void rewind(qint64 index)
    {
        tokenStream->rewind(index);
        yylex();
    }

// token stream
    void setTokenStream(KDevPG::TokenStream *s)
    {
        tokenStream = s;
    }

// error handling
    void expectedSymbol(int kind, const QString& name);
    void expectedToken(int kind, qint64 token, const QString& name);

    bool mBlockErrors;
    inline bool blockErrors(bool block)
    {
        bool previous = mBlockErrors;
        mBlockErrors = block;
        return previous;
    }

// memory pool
    typedef KDevPG::MemoryPool memoryPoolType;

    KDevPG::MemoryPool *memoryPool;
    void setMemoryPool(KDevPG::MemoryPool *p)
    {
        memoryPool = p;
    }
    template <class T>
    inline T *create()
    {
        T *node = new (memoryPool->allocate(sizeof(T))) T();
        node->kind = T::KIND;
        return node;
    }

    enum TokenType
    {
        Token_COLON = 1000,
        Token_COMMA = 1001,
        Token_CONT = 1002,
        Token_EOF = 1003,
        Token_EQUAL = 1004,
        Token_EXCLAM = 1005,
        Token_IDENTIFIER = 1006,
        Token_INVALID = 1007,
        Token_LBRACE = 1008,
        Token_LPAREN = 1009,
        Token_MINUSEQ = 1010,
        Token_NEWLINE = 1011,
        Token_OR = 1012,
        Token_PLUSEQ = 1013,
        Token_RBRACE = 1014,
        Token_RPAREN = 1015,
        Token_STAREQ = 1016,
        Token_TILDEEQ = 1017,
        Token_VALUE = 1018,
        TokenTypeSize
    }; // TokenType

// user defined declarations:
public:

    /**
     * Transform the raw input into tokens.
     * When this method returns, the parser's token stream has been filled
     * and any parse_*() method can be called.
     */
    void tokenize( const QString& contents );

    enum ProblemType
    {
        Error,
        Warning,
        Info
    };
    void reportProblem( Parser::ProblemType type, const QString& message );

    QString tokenText(qint64 begin, qint64 end) const;

    void setDebug( bool debug );


private:

    QString m_contents;
    bool m_debug;


public:
    Parser()
    {
        memoryPool = 0;
        tokenStream = 0;
        yytoken = Token_EOF;
        mBlockErrors = false;
    }

    virtual ~Parser() {}

    bool parseArgumentList(ArgumentListAst **yynode);
    bool parseFunctionArguments(FunctionArgumentsAst **yynode);
    bool parseItem(ItemAst **yynode);
    bool parseOp(OpAst **yynode);
    bool parseOrOperator(OrOperatorAst **yynode);
    bool parseProject(ProjectAst **yynode);
    bool parseScope(ScopeAst **yynode);
    bool parseScopeBody(ScopeBodyAst **yynode);
    bool parseStatement(StatementAst **yynode);
    bool parseValue(ValueAst **yynode);
    bool parseValueList(ValueListAst **yynode);
    bool parseVariableAssignment(VariableAssignmentAst **yynode);
};

} // end of namespace QMake

#endif

