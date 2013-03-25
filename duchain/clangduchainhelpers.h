#ifndef CLANGDUCHAINHELPERS_H
#define CLANGDUCHAINHELPERS_H

#include <language/editor/rangeinrevision.h>
#include <language/duchain/identifier.h>
#include <clang/Basic/SourceManager.h>
#include <clang/AST/TypeLoc.h>
#include <clang/AST/Stmt.h>
#include <clang/Lex/Preprocessor.h>

namespace clang
{
    class Decl;
    class NamedDecl;
    class DeclRefExpr;
}

namespace KDevelop
{
    class Declaration;
    class FunctionDeclaration;
    class DUContext;
}

struct AST
{
    AST() : context(0) {};
    virtual ~AST() {}
    clang::SourceLocation start;
    clang::SourceLocation end;
    KDevelop::DUContext *context;
};

struct NameAST : public AST
{
    NameAST() : AST() {}
    NameAST(clang::NamedDecl* d, clang::Preprocessor &p) : AST()
    {
        start = d->getLocation();
        end = p.getLocForEndOfToken(start);
    }
};

struct TagAST : public AST
{
    TagAST() : AST() {}
    TagAST(clang::TagDecl *d) : AST()
    {
        start = d->getInnerLocStart();
        end = d->getRBraceLoc();
    }
};

struct CompoundStmtAST : public AST
{
    CompoundStmtAST() : AST() {}
    CompoundStmtAST(clang::CompoundStmt *s)
    {
        start = s->getLBracLoc();
        end = s->getRBracLoc();
    }
};

inline KDevelop::CursorInRevision locCursor(clang::SourceLocation loc, const clang::SourceManager& sm)
{
    return KDevelop::CursorInRevision(
        sm.getPresumedLineNumber(loc) - 1,
        sm.getPresumedColumnNumber(loc) - 1
    );
}
KDevelop::RangeInRevision typeRange(clang::TypeLoc typeLoc, const clang::SourceManager& sm);
KDevelop::RangeInRevision declRefRange(clang::DeclRefExpr* declRefExpr);
KDevelop::QualifiedIdentifier declQualifiedIdentifier(clang::NamedDecl* decl);
int declContextType(clang::Decl* decl);

#endif //CLANGDUCHAINHELPERS_H