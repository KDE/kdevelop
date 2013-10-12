#include "parsesession.h"
#include "debug.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Parse/ParseAST.h>
#include <llvm/Support/Host.h>

#include <language/editor/simplerange.h>
#include <language/interfaces/iproblem.h>
#include <language/duchain/stringhelpers.h>

static clang::CompilerInstance* compilerInstance()
{
    clang::CompilerInstance *ci = new clang::CompilerInstance();
    ci->createDiagnostics(0,NULL);

    llvm::IntrusiveRefCntPtr<clang::TargetOptions> pto( new clang::TargetOptions() );
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    clang::TargetInfo *pti = clang::TargetInfo::CreateTargetInfo(ci->getDiagnostics(), *pto.getPtr());
    ci->setTarget(pti);
    
    clang::LangOptions &langOpts = ci->getLangOpts();

    clang::HeaderSearchOptions &hso = ci->getHeaderSearchOpts();
    hso.AddPath("/usr/include", clang::frontend::Angled, false, false, false);
    hso.AddPath("/usr/include/c++/4.7.2", clang::frontend::Angled, false, false, false);
    
    ci->createFileManager();
    ci->createSourceManager(ci->getFileManager());
    ci->createPreprocessor();
    ci->createASTContext();

    return ci;
}

KDevelop::IndexedString ParseSession::languageString()
{
    return KDevelop::IndexedString("Clang");
}

ParseSession::ParseSession(const KDevelop::IndexedString& url, const QByteArray& contents)
: m_url(url), m_ci(compilerInstance())
{
    llvm::MemoryBuffer *b = llvm::MemoryBuffer::getMemBuffer(contents.constData());
    m_fileID = m_ci->getSourceManager().createMainFileIDForMemBuffer(b);
    m_ci->getDiagnosticClient().BeginSourceFile(m_ci->getLangOpts(), &m_ci->getPreprocessor());
    clang::ParseAST(m_ci->getPreprocessor(), this, m_ci->getASTContext());
    m_ci->getDiagnosticClient().EndSourceFile();
}

ParseSession::~ParseSession()
{
    delete m_ci;
}

KDevelop::IndexedString ParseSession::url() const
{
    return m_url;
}

AST ParseSession::ast() const
{
    return m_ast;
}

const clang::SourceManager& ParseSession::sourceManager() const
{
    return m_ci->getSourceManager();
}

clang::Preprocessor& ParseSession::preprocessor() const
{
    return m_ci->getPreprocessor();
}

void ParseSession::HandleTranslationUnit(clang::ASTContext& Ctx)
{
    m_ast.decl = Ctx.getTranslationUnitDecl();
    clang::ASTConsumer::HandleTranslationUnit(Ctx);
}

const clang::FileID& ParseSession::fileID() const
{
    return m_fileID;
}
