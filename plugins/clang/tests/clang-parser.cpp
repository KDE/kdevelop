/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KAboutData>

#include <language/util/debuglanguageparserhelper.h>

#include "../duchain/parsesession.h"
#include "../duchain/debugvisitor.h"
#include "../duchain/clangindex.h"
#include "../util/clangtypes.h"

using namespace KDevelop;
using namespace KDevelopUtils;

class ClangParser
{
public:
    ClangParser(const bool printAst, const bool printTokens)
      : m_session({})
      , m_printAst(printAst)
      , m_printTokens(printTokens)
    {
    }

    /// parse contents of a file
    void parseFile( const QString &fileName )
    {
        if (!QFile::exists(fileName)) {
            qerr << "File to parse does not exist: " << fileName << Qt::endl;
            return;
        }
        m_session.setData(ParseSessionData::Ptr(new ParseSessionData({}, &m_index, environment(fileName))));
        runSession();
    }

    /// parse code directly
    void parseCode( const QString &code )
    {
        const QString fileName = QStringLiteral("stdin.cpp");
        m_session.setData(ParseSessionData::Ptr(new ParseSessionData({UnsavedFile(fileName, {code})},
                                                                     &m_index, environment(fileName))));
        runSession();
    }

    void setIncludePaths(const QStringList& paths)
    {
        m_includePaths = paths;
    }

    void setCustomArgs(const QString& args)
    {
        m_customArgs = args;
    }
private:
    /**
     * actually run the parse session
     */
    void runSession()
    {
        if (!m_session.unit()) {
            qerr << "failed to parse code" << Qt::endl;
        }
        if (m_printTokens) {
            CXTranslationUnit TU = m_session.unit();
            auto cursor = clang_getTranslationUnitCursor(TU);
            CXSourceRange range = clang_getCursorExtent(cursor);
            const ClangTokens tokens(TU, range);
            for (CXToken token : tokens) {
                CXString spelling = clang_getTokenSpelling(TU, token);
                qout << "token= " << clang_getCString(spelling) << Qt::endl;
                clang_disposeString(spelling);
            }
        }

        if (!m_session.unit()) {
            qerr << "no AST tree could be generated" << Qt::endl;
            exit(255);
            return;
        }

        qout << "AST tree successfully generated" << Qt::endl;
        auto file = m_session.mainFile();

        if (m_printAst) {
            DebugVisitor visitor(&m_session);
            visitor.visit(m_session.unit(), file);
        }

        const auto problems = m_session.problemsForFile(file);
        if (!problems.isEmpty()) {
            qerr << Qt::endl << "problems encountered during parsing:" << Qt::endl;
            for (const ProblemPointer& problem : problems) {
                qerr << problem->toString() << Qt::endl;
            }
        } else {
            qout << "no problems encountered during parsing" << Qt::endl;
        }
    }

    ClangParsingEnvironment environment(const QString& fileName) const
    {
        ClangParsingEnvironment environment;
        environment.setTranslationUnitUrl(IndexedString(fileName));
        environment.addIncludes(toPathList(m_includePaths));
        environment.addParserArguments(m_customArgs);
        return environment;
    }
    ParseSession m_session;
    const bool m_printAst;
    const bool m_printTokens;
    ClangIndex m_index;
    QStringList m_includePaths;
    QString m_customArgs;
};

namespace KDevelopUtils {
template<>
void setupCustomArgs<ClangParser>(QCommandLineParser* args)
{
    args->addOption(QCommandLineOption{QStringList{"I", "include"}, i18n("add include path"), QStringLiteral("include")});
    args->addOption(QCommandLineOption{QStringList{"custom-arg"}, i18n("custom clang args"), QStringLiteral("arg")});
}

template<>
void setCustomArgs<ClangParser>(ClangParser* parser, QCommandLineParser* args)
{
    parser->setIncludePaths(args->values(QStringLiteral("include")));
    parser->setCustomArgs(args->values(QStringLiteral("custom-arg")).join(QLatin1String(" ")));
}
}

int main(int argc, char* argv[])
{
    KAboutData aboutData( QStringLiteral("clang-parser"), i18n( "clang-parser" ),
                          QStringLiteral("1"), i18n("KDevelop Clang parser debugging utility"), KAboutLicense::GPL,
                          i18n( "2013 Milian Wolff" ), QString(), QStringLiteral("https://www.kdevelop.org/") );

    return KDevelopUtils::initAndRunParser<ClangParser>(aboutData, argc, argv);
}
