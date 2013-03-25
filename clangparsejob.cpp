#include "clangparsejob.h"

#include <language/backgroundparser/urlparselock.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/duchain.h>
#include <language/duchain/parsingenvironment.h>
#include <interfaces/ilanguage.h>

#include "duchain/declarationbuilder.h"
#include "duchain/parsesession.h"
#include "duchain/duchainbuilder.h"

#include "debug.h"

#include <QReadLocker>

using namespace KDevelop;

ClangParseJob::ClangParseJob(const IndexedString& url, ILanguageSupport* languageSupport)
: ParseJob(url, languageSupport)
{
}

void ClangParseJob::run()
{
    UrlParseLock urlLock(document());
    if (abortRequested() || !isUpdateRequired(ParseSession::languageString())) {
        return;
    }

    ProblemPointer p = readContents();
    if (p) {
        //TODO: associate problem with topducontext
        return;
    }
    
    ParseSession session(document(), contents().contents);

    if (abortRequested()) {
        return;
    }

    ReferencedTopDUContext context;
    {
        DUChainReadLocker lock;
        context = DUChainUtils::standardContextForUrl(document().toUrl());
    }
    {
        QReadLocker parseLock(languageSupport()->language()->parseLock());

        if (abortRequested()) {
            return abortJob();
        }

        DeclarationBuilder builder(&session);
        AST ast = session.ast();
        context = builder.build(document(), &ast, context);
    }

    if (abortRequested()) {
        return abortJob();
    }
    
    //context = createFileContext(document(), contents().contents);
    
    Q_ASSERT(context);

    /*if (!context) {
        DUChainWriteLocker lock;
        ParsingEnvironmentFile *file = new ParsingEnvironmentFile(document());
        file->setLanguage(ParseSession::languageString());
        context = new TopDUContext(document(), RangeInRevision(0, 0, INT_MAX, INT_MAX), file);
        DUChain::self()->addDocumentChain(context);
    }*/

    setDuChain(context);
    {
        DUChainWriteLocker lock;
        //context->setProblems(session.problems());

        context->setFeatures(minimumFeatures());
        ParsingEnvironmentFilePointer file = context->parsingEnvironmentFile();
        Q_ASSERT(file);
        file->setModificationRevision(contents().modification);
        DUChain::self()->updateContextEnvironment( context->topContext(), file.data() );
    }
    highlightDUChain();
}

