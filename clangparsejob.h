#ifndef CLANGLANG_CLANGPARSEJOB_H
#define CLANGLANG_CLANGPARSEJOB_H

#include <language/backgroundparser/parsejob.h>

class ClangParseJob : public KDevelop::ParseJob
{
public:
    ClangParseJob(const KDevelop::IndexedString& url,
                  KDevelop::ILanguageSupport* languageSupport);

protected:
    virtual void run();
};

#endif // CLANGLANG_CLANGPARSEJOB_H
