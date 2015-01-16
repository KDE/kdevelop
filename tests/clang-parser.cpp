/*
    This file is part of KDevelop

    Copyright 2013 Milian Wolff <mail@milianw.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <KAboutData>

#include <language/util/debuglanguageparserhelper.h>

#include "../duchain/parsesession.h"
#include "../duchain/debugvisitor.h"
#include "../duchain/clangindex.h"
#include "../util/clangtypes.h"

using namespace KDevelop;
using namespace KDevelopUtils;

class ClangParser {
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
            qerr << "File to parse does not exist: " << fileName << endl;
            return;
        }
        ClangParsingEnvironment environment;
        environment.setTranslationUnitUrl(IndexedString(fileName));
        m_session.setData(ParseSessionData::Ptr(new ParseSessionData({}, &m_index, environment)));
        runSession();
    }

    /// parse code directly
    void parseCode( const QString &code )
    {
        ClangParsingEnvironment environment;
        const QString fileName = QStringLiteral("stdin.cpp");
        environment.setTranslationUnitUrl(IndexedString(fileName));
        m_session.setData(ParseSessionData::Ptr(new ParseSessionData({UnsavedFile(fileName, {code})},
                                                                     &m_index, environment)));
        runSession();
    }

private:
    /**
     * actually run the parse session
     */
    void runSession()
    {
        if (!m_session.unit()) {
            qerr << "failed to parse code" << endl;
        }
        if (m_printTokens) {
            CXTranslationUnit TU = m_session.unit();
            auto cursor = clang_getTranslationUnitCursor(TU);
            CXSourceRange range = clang_getCursorExtent(cursor);
            CXToken *tokens = 0;
            unsigned int nTokens = 0;
            clang_tokenize(TU, range, &tokens, &nTokens);
            for (unsigned int i = 0; i < nTokens; i++)
            {
                CXString spelling = clang_getTokenSpelling(TU, tokens[i]);
                qout << "token= " << clang_getCString(spelling) << endl;
                clang_disposeString(spelling);
            }
            clang_disposeTokens(TU, tokens, nTokens);
        }

        if (!m_session.unit()) {
            qerr << "no AST tree could be generated" << endl;
            exit(255);
            return;
        }

        qout << "AST tree successfully generated" << endl;
        auto file = m_session.mainFile();

        if (m_printAst) {
            DebugVisitor visitor(&m_session);
            visitor.visit(m_session.unit(), file);
        }

        const auto problems = m_session.problemsForFile(file);
        if (!problems.isEmpty()) {
            qerr << endl << "problems encountered during parsing:" << endl;
            foreach(const ProblemPointer problem, problems) {
                qerr << problem->toString() << endl;
            }
        } else {
            qout << "no problems encountered during parsing" << endl;
        }
    }

    ParseSession m_session;
    const bool m_printAst;
    const bool m_printTokens;
    ClangIndex m_index;
};

int main(int argc, char* argv[])
{
    KAboutData aboutData( "clang-parser", i18n( "clang-parser" ),
                          "1", i18n("KDevelop Clang parser debugging utility"), KAboutLicense::GPL,
                          i18n( "2013 Milian Wolff" ), QString(), "http://www.kdevelop.org" );

    return KDevelopUtils::initAndRunParser<ClangParser>(aboutData, argc, argv);
}
