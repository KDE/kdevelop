#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include <language/duchain/builders/abstractcontextbuilder.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <QHash>
#include "duchainexport.h"
#include "clangduchainhelpers.h"

class ParseSession;
typedef KDevelop::AbstractContextBuilder<AST, NameAST> ContextBuilderBase;
class KDEVCLANGDUCHAIN_EXPORT ContextBuilder 
: public ContextBuilderBase, public clang::RecursiveASTVisitor<ContextBuilder>
{
public:
    ContextBuilder();

    virtual KDevelop::RangeInRevision editorFindRange(AST* fromNode, AST* toNode);
    virtual KDevelop::RangeInRevision editorFindRangeForContext(AST* fromNode, AST* toNode);
    virtual KDevelop::QualifiedIdentifier identifierForNode(NameAST* node);

    virtual void setContextOnNode(AST* node, KDevelop::DUContext* context);
    virtual KDevelop::DUContext* contextFromNode(AST* node);

    virtual KDevelop::TopDUContext* newTopContext(const KDevelop::RangeInRevision& range,
                                                  KDevelop::ParsingEnvironmentFile* file = 0);
    virtual void startVisiting(AST* node);
    
protected:
    ParseSession *m_session;
    //These aren't virtual functions from RecursiveASTVisitor, but are rather
    //are called by the RecursiveASTVisitor and so need to be available for
    //inheritors of the ContextBuilder
    virtual bool VisitDeclRefExpr(clang::DeclRefExpr*) = 0;
    virtual bool TraverseDecl(clang::Decl* decl);
    virtual bool VisitFunctionTypeLoc(clang::FunctionTypeLoc TL);
    virtual bool VisitTypeLoc(clang::TypeLoc TL) = 0;
    virtual bool VisitCompoundStmt(clang::CompoundStmt *stmt);

    friend class RecursiveASTVisitor<ContextBuilder>;
};

#endif // CONTEXTBUILDER_H
