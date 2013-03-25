//This file is no longer used or needed


#include "duchainbuilder.h"
#include <language/duchain/declaration.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/functiondeclaration.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/integraltype.h>

static std::auto_ptr<CompilerInstance> compilerInstance()
{
    std::auto_ptr<CompilerInstance> ci(new CompilerInstance());
    ci->createDiagnostics(0,NULL);

    llvm::IntrusiveRefCntPtr<TargetOptions> pto( new TargetOptions());
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *pti = TargetInfo::CreateTargetInfo(ci->getDiagnostics(), *pto.getPtr());
    ci->setTarget(pti);
    
    LangOptions &langOpts = ci->getLangOpts();
    langOpts.GNUMode = 1;
    langOpts.CXXExceptions = 1;
    langOpts.RTTI = 1;
    langOpts.Bool = 1;
    langOpts.CPlusPlus = 1;

    HeaderSearchOptions &hso = ci->getHeaderSearchOpts();
    hso.AddPath("/usr/include", clang::frontend::Angled, false, false, false);
    
    ci->createFileManager();
    ci->createSourceManager(ci->getFileManager());
    ci->createPreprocessor();
    ci->createASTContext();

    return ci;
}

class ClangRecursiveASTVisitor
    : public RecursiveASTVisitor<ClangRecursiveASTVisitor>
{
public:
    ClangRecursiveASTVisitor(KDevelop::DUContext *context, SourceManager& manager)
    : m_context(context), m_sourceManager(manager) { }
    bool VisitDecl(Decl *D);
    bool TraverseDeclRefExpr(DeclRefExpr* refExpr);
private:
    KDevelop::DUContext *m_context;
    SourceManager& m_sourceManager;
    QHash<Decl*, KDevelop::Declaration*> m_decls;
};

template<class T>
KDevelop::RangeInRevision getRange(T *clangObj, const SourceManager& manager)
{
    int startLine = manager.getPresumedLineNumber(clangObj->getLocStart()) - 1;
    int startCol = manager.getPresumedColumnNumber(clangObj->getLocStart()) - 1;
    int endLine = manager.getPresumedLineNumber(clangObj->getLocEnd()) - 1;
    int endCol = manager.getPresumedColumnNumber(clangObj->getLocEnd()) - 1;
    return KDevelop::RangeInRevision(startLine, startCol, endLine, endCol);
}

template<class T>
KDevelop::RangeInRevision getDeclRange(T *decl, const SourceManager& manager)
{
    SourceLocation begin = decl->getLocation();
    SourceLocation end = decl->getLocation().getLocWithOffset(decl->getDeclName().getAsString().length());
    int startline = manager.getPresumedLineNumber(begin) - 1;
    int startcol = manager.getPresumedColumnNumber(begin) - 1;
    int endline = manager.getPresumedLineNumber(end) - 1;
    int endcol = manager.getPresumedColumnNumber(end) - 1;
    return KDevelop::RangeInRevision(startline, startcol, endline, endcol);
}

bool ClangRecursiveASTVisitor::TraverseDeclRefExpr(DeclRefExpr* refExpr)
{
    qWarning() << "Found ref" << refExpr->getLocation().printToString(m_sourceManager).c_str() << m_decls.size();
    Decl *d = refExpr->getDecl();
    if (m_decls.contains(d))
    {
        KDevelop::Declaration *kdevDecl = m_decls[d];
        KDevelop::RangeInRevision range = getRange(refExpr, m_sourceManager);
        range = KDevelop::RangeInRevision(range.start.line, range.start.column, range.end.line, range.end.column + kdevDecl->identifier().toString().size());
        qWarning() << "creating use for" << kdevDecl->toString() << "at" << range;
        m_context->createUse(m_context->topContext()->indexForUsedDeclaration(kdevDecl), range);
    }
    return true;
}

bool ClangRecursiveASTVisitor::VisitDecl(Decl* d)
{
    qWarning() << "Decl at" << d->getLocation().printToString(m_sourceManager).c_str() << "of type" << d->getDeclKindName();
    KDevelop::Declaration *kdevDecl = 0;
    switch(d->getKind())
    {
    case Decl::Var:
    {
        VarDecl *varD = dynamic_cast<VarDecl*>(d);
        KDevelop::Declaration *varDecl = new KDevelop::Declaration(getDeclRange(varD, m_sourceManager), m_context);
        KDevelop::Identifier id(QString(varD->getDeclName().getAsString().c_str()));
        varDecl->setIdentifier(id);
        kdevDecl = varDecl;
        KDevelop::AbstractType::Ptr type(new KDevelop::IntegralType());
        varDecl->setAbstractType(type);
        SplitQualType sqt;
        qWarning() << varD->getType().getAsString().c_str();
    }
    break;
    case Decl::Function:
    {
        FunctionDecl* funcD = dynamic_cast<FunctionDecl*>(d);
        Q_ASSERT(funcD);
        KDevelop::QualifiedIdentifier qid(QString(funcD->getDeclName().getAsString().c_str()));
        qWarning() << "   Which is a function named:" << qid.toString();
        KDevelop::RangeInRevision exprRange = getRange(funcD->getBody(), m_sourceManager);
        KDevelop::DUContext *exprContext = new KDevelop::DUContext(exprRange, m_context);
        exprContext->setType(KDevelop::DUContext::Other);
        exprContext->setLocalScopeIdentifier(qid);
        exprContext->setInSymbolTable(false);
        KDevelop::FunctionDeclaration *functionDeclaration =
                new KDevelop::FunctionDeclaration(getDeclRange(funcD, m_sourceManager), m_context);
        functionDeclaration->setIdentifier(qid.last());
        functionDeclaration->setInternalContext(exprContext);
        qWarning() << "   Decl is:" << functionDeclaration->toString() << functionDeclaration->range();
        kdevDecl = functionDeclaration;
    }
    break;
    default:
    break;
    }
    if (kdevDecl)
        m_decls[d] = kdevDecl;
    return true;
}

class ClangASTConsumer: public clang::ASTConsumer{
public:
    ClangASTConsumer(ClangRecursiveASTVisitor *visitor) : m_visitor(visitor) {};

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef declGroupRef)
    {
        qWarning() << "HANDLING TLD";
        typedef clang::DeclGroupRef::iterator iter;
        for (iter b = declGroupRef.begin(), e = declGroupRef.end(); b != e; ++b)
            m_visitor->TraverseDecl(*b);
        return true;
    }
private:
    ClangRecursiveASTVisitor *m_visitor;
};

KDevelop::TopDUContext *createFileContext(KDevelop::IndexedString url, const QByteArray& content)
{
    using namespace KDevelop;
    DUChainWriteLocker lock;
    
    std::auto_ptr<CompilerInstance> ci = compilerInstance();

    ParsingEnvironmentFile *envFile = new ParsingEnvironmentFile(url);
    TopDUContext *context = new TopDUContext(url, RangeInRevision( CursorInRevision( 0, 0 ), CursorInRevision( INT_MAX, INT_MAX ) ), envFile);
    context->setType(DUContext::Global);
    DUChain::self()->addDocumentChain( context );
    ClangRecursiveASTVisitor *visitor = new ClangRecursiveASTVisitor(context, ci->getSourceManager());
    ClangASTConsumer *consumer = new ClangASTConsumer(visitor);

    llvm::MemoryBuffer *b = llvm::MemoryBuffer::getMemBuffer(content.constData());
    ci->getSourceManager().createMainFileIDForMemBuffer(b);

    ci->getDiagnosticClient().BeginSourceFile(ci->getLangOpts(), &ci->getPreprocessor());
    clang::ParseAST(ci->getPreprocessor(), consumer, ci->getASTContext());
    ci->getDiagnosticClient().EndSourceFile();
    delete consumer;
    delete visitor;
    return context;
}