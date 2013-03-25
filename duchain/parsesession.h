#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <language/duchain/indexedstring.h>
#include <language/interfaces/iproblem.h>

#include "duchainexport.h"
#include "clangduchainhelpers.h"
#include <clang/AST/ASTConsumer.h>

namespace clang {
class CompilerInstance;
class TranslationUnitDecl;
}

class KDEVCLANGDUCHAIN_EXPORT ParseSession : clang::ASTConsumer
{
public:
    /**
     * @return a unique identifier for Clang documents.
     */
    static KDevelop::IndexedString languageString();

    /**
     * Parse the given @p contents.
     *
     * @param url The url for the document you want to parse.
     * @param contents The contents of the document you want to parse.
     */
    ParseSession(const KDevelop::IndexedString& url, const QByteArray& contents);
    ~ParseSession();

    /**
     * @return the URL of this session
     */
    KDevelop::IndexedString url() const;

    /**
     * @return the root TU decl
     */
    AST ast() const;

    /**
     * @return the clang source manager;
     */
    const clang::SourceManager& sourceManager() const;

    /**
     * @return the clang preprocessor;
     */
    clang::Preprocessor& preprocessor() const;

    /**
     * @return the session's fileID;
     */
    const clang::FileID& fileID() const;

private:
    virtual void HandleTranslationUnit(clang::ASTContext &Ctx);
    KDevelop::IndexedString m_url;
    clang::CompilerInstance *m_ci;
    AST m_ast;
    clang::FileID m_fileID;
};

#endif // PARSESESSION_H
