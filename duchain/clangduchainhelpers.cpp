#include "clangduchainhelpers.h"
#include <language/duchain/ducontext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclCXX.h>

using namespace llvm;
//Don't use KDevelop or clang namespaces, as there are a lot of similar looking decls and possible name clashes

KDevelop::RangeInRevision rangeFromLocations(clang::SourceLocation loc1, clang::SourceLocation loc2,
                                             const clang::SourceManager& sourceManager)
{
    return KDevelop::RangeInRevision(
        sourceManager.getPresumedLineNumber(loc1) - 1,
        sourceManager.getPresumedColumnNumber(loc1) - 1,
        sourceManager.getPresumedLineNumber(loc2) - 1,
        sourceManager.getPresumedColumnNumber(loc2) - 1
    );
}

KDevelop::CursorInRevision cursorFromLocation(clang::SourceLocation loc, const clang::SourceManager &sm)
{
    return KDevelop::CursorInRevision(
        sm.getPresumedLineNumber(loc) - 1,
        sm.getPresumedColumnNumber(loc) - 1
    );
}
KDevelop::RangeInRevision typeRange(clang::TypeLoc typeLoc, const clang::SourceManager& sm)
{
    if (clang::CXXRecordDecl *recDecl = typeLoc.getType()->getAsCXXRecordDecl())
    {
        int nameLen = recDecl->getName().size();
        return rangeFromLocations(typeLoc.getBeginLoc(), typeLoc.getBeginLoc().getLocWithOffset(nameLen), sm);
    }
    return KDevelop::RangeInRevision();
}
KDevelop::RangeInRevision declRefRange(clang::DeclRefExpr* declRefExpr)
{
    clang::NamedDecl *namedDecl = dynamic_cast<clang::NamedDecl*>(declRefExpr->getDecl());
    clang::SourceManager &manager = namedDecl->getASTContext().getSourceManager();
    clang::SourceLocation endLoc = declRefExpr->getLocation().getLocWithOffset(namedDecl->getName().size());
    return rangeFromLocations(declRefExpr->getLocation(), endLoc, manager);
}
KDevelop::QualifiedIdentifier declQualifiedIdentifier(clang::NamedDecl* decl)
{
    std::string str = decl->getQualifiedNameAsString();
    return KDevelop::QualifiedIdentifier(QString(str.c_str()));
}

int declContextType(clang::Decl *decl)
{
    if (!dynamic_cast<clang::DeclContext*>(decl))
        return -1;
    switch(decl->getKind())
    {
    case clang::Decl::CXXRecord:
        if (static_cast<clang::CXXRecordDecl*>(decl)->isThisDeclarationADefinition())
            return KDevelop::DUContext::Class;
    default:
        return - 1;
    }
}
