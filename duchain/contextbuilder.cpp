#include "contextbuilder.h"

#include "parsesession.h"
#include "clangduchainhelpers.h"
#include <clang/AST/ASTContext.h>

using namespace KDevelop;

ContextBuilder::ContextBuilder()
: ContextBuilderBase()
{
}

RangeInRevision ContextBuilder::editorFindRange(AST* fromNode, AST* toNode)
{
    Q_ASSERT(fromNode && fromNode == toNode);
    return KDevelop::RangeInRevision(locCursor(fromNode->start, m_session->sourceManager()),
                                     locCursor(fromNode->end, m_session->sourceManager()));
}

RangeInRevision ContextBuilder::editorFindRangeForContext(AST* fromNode, AST* toNode)
{
    Q_ASSERT(fromNode && fromNode == toNode);
    return KDevelop::RangeInRevision(locCursor(fromNode->start, m_session->sourceManager()),
                                     locCursor(fromNode->end, m_session->sourceManager()));
}

QualifiedIdentifier ContextBuilder::identifierForNode(NameAST* node)
{
    return declQualifiedIdentifier(node->named());
}

void ContextBuilder::setContextOnNode(AST* node, DUContext* context)
{
    node->context = context;
}

DUContext* ContextBuilder::contextFromNode(AST* node)
{
    return node->context;
}

TopDUContext* ContextBuilder::newTopContext(const RangeInRevision& range, ParsingEnvironmentFile* file)
{
    //TODO: If this assert doesn't get triggered, this function can be removed
    //Q_ASSERT(file);
    if (!file) {
        file = new ParsingEnvironmentFile(m_session->url());
        /// identify environment files from this language plugin
        file->setLanguage(m_session->languageString());
    }
    return ContextBuilderBase::newTopContext(range, file);
}

void ContextBuilder::startVisiting(AST* node)
{
    TraverseDecl(node->decl);
}

bool ContextBuilder::TraverseDecl(clang::Decl* decl)
{
    int ctxtType = declContextType(decl);
    if (ctxtType >= 0)
    {
        AST ast(decl);
        qDebug() << "traversing context";
        openContext(&ast, (DUContext::ContextType)ctxtType);
        clang::RecursiveASTVisitor<ContextBuilder>::TraverseDecl(decl);
        closeContext();
        qDebug() << "closed context";
        return true;
    }
    return clang::RecursiveASTVisitor<ContextBuilder>::TraverseDecl(decl);
}

bool ContextBuilder::VisitFunctionTypeLoc(clang::FunctionTypeLoc TL)
{
    qDebug() << "number of args for function is" << TL.getNumArgs();
    openContext(&ast, KDevelop::DUContext::Function);
    clang::RecursiveASTVisitor<ContextBuilder>::VisitFunctionTypeLoc(TL);
    closeContext();
    return true;
}

bool ContextBuilder::VisitCompoundStmt(clang::CompoundStmt* stmt)
{
    qDebug() << "visit compound expr" << stmt->getStmtClassName() << stmt->getLBracLoc().printToString(m_session->sourceManager()).c_str()
                                                                  << stmt->getRBracLoc().printToString(m_session->sourceManager()).c_str();
    return true;
}
