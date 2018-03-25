/* This file is part of KDevelop

   Copyright 2010 Milian Wolff <mail@milianw.de>
   Copyright 2014 Kevin Funk <kfunk@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_DEBUGLANGUAGEPARSERHELPER_H
#define KDEVPLATFORM_DEBUGLANGUAGEPARSERHELPER_H

#include <cstdlib>
#include <cstdio>

#ifndef Q_OS_WIN
#include <unistd.h> // for isatty
#endif

#include <tests/autotestshell.h>
#include <language/duchain/duchain.h>
#include <language/duchain/problem.h>
#include <language/codegen/coderepresentation.h>
#include <tests/testcore.h>

#include <KAboutData>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>

namespace KDevelopUtils {


QTextStream qout(stdout);
QTextStream qerr(stderr);
QTextStream qin(stdin);

typedef QString (*TokenTextFunc)(int);
/**
 * This class is a pure helper to use for binaries that you can
 * run on short snippets of test code or whole files and let
 * it print the generated tokens or AST.
 *
 * It should work fine for any KDevelop-PG-Qt based parser.
 *
 *
 * @tparam SessionT the parse session for your language.
 * @tparam TokenStreamT the token stream for your language, based on KDevPG::TokenStreamBase.
 * @tparam TokenT the token class for your language, based on KDevPG::Token.
 * @tparam LexerT the Lexer for your language.
 * @tparam StartAstT the AST node that is returned from @c SessionT::parse().
 * @tparam DebugVisitorT the debug visitor for your language.
 * @tparam TokenTextT function pointer to the function that returns a string representation for an integral token.
 */
template<class SessionT, class TokenStreamT, class TokenT, class LexerT,
         class StartAstT, class DebugVisitorT, TokenTextFunc TokenTextT>
class DebugLanguageParserHelper {
public:
    DebugLanguageParserHelper(const bool printAst, const bool printTokens)
        : m_printAst(printAst), m_printTokens(printTokens)
    {
        m_session.setDebug(printAst);
    }

    /// parse contents of a file
    void parseFile( const QString &fileName )
    {
        if (!m_session.readFile(fileName, "utf-8")) {
            qerr << "Can't open file " << fileName << endl;
            std::exit(255);
        } else {
            qout << "Parsing file " << fileName << endl;
        }
        runSession();
    }

    /// parse code directly
    void parseCode( const QString &code )
    {
        m_session.setContents(code);

        qout << "Parsing input" << endl;
        runSession();
    }

private:
    /**
     * actually run the parse session
     */
    void runSession()
    {
        if (m_printTokens) {
            TokenStreamT tokenStream;
            LexerT lexer(&tokenStream, m_session.contents());
            int token;
            while ((token = lexer.nextTokenKind())) {
                TokenT &t = tokenStream.push();
                t.begin = lexer.tokenBegin();
                t.end = lexer.tokenEnd();
                t.kind = token;
                printToken(token, lexer);
            }
            printToken(token, lexer);
            if ( tokenStream.size() > 0 ) {
                qint64 line;
                qint64 column;
                tokenStream.endPosition(tokenStream.size() - 1, &line, &column);
                qDebug() << "last token endPosition: line" << line << "column" << column;
            } else {
                qDebug() << "empty token stream";
            }
        }

        StartAstT* ast = 0;
        if (!m_session.parse(&ast)) {
            qerr << "no AST tree could be generated" << endl;
        } else {
            qout << "AST tree successfully generated" << endl;
            if (m_printAst) {
                DebugVisitorT debugVisitor(m_session.tokenStream(), m_session.contents());
                debugVisitor.visitStart(ast);
            }
        }
        if (!m_session.problems().isEmpty()) {
            qout << endl << "problems encountered during parsing:" << endl;
            foreach(KDevelop::ProblemPointer p, m_session.problems()) {
                qout << p->description() << endl;
            }
        } else {
            qout << "no problems encountered during parsing" << endl;
        }

        if (!ast) {
            exit(255);
        }
    }

    void printToken(int token, const LexerT& lexer) const
    {
        int begin = lexer.tokenBegin();
        int end = lexer.tokenEnd();
        qout << m_session.contents().mid(begin, end - begin + 1).replace('\n', "\\n")
             << ' ' << TokenTextT(token) << endl;
    }

    SessionT m_session;
    const bool m_printAst;
    const bool m_printTokens;
};

template<class ParserT>
void setupCustomArgs(QCommandLineParser* parser)
{
    Q_UNUSED(parser);
}

template<class ParserT>
void setCustomArgs(ParserT* parser, QCommandLineParser* commandLineParser)
{
    Q_UNUSED(parser);
    Q_UNUSED(commandLineParser);
}

/// call this after setting up @p aboutData in your @c main() function.
template<class ParserT>
int initAndRunParser(KAboutData& aboutData, int argc, char* argv[])
{
    qout.setCodec("UTF-8");
    qerr.setCodec("UTF-8");
    qin.setCodec("UTF-8");

    QApplication app(argc, argv);

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();

    aboutData.setupCommandLine(&parser);

    parser.addPositionalArgument("files", i18n("files or - to read from STDIN, the latter is the default if nothing is provided"), "[FILE...]");

    parser.addOption(QCommandLineOption{QStringList{"a", "print-ast"}, i18n("print generated AST tree")});
    parser.addOption(QCommandLineOption{QStringList{"t", "print-tokens"}, i18n("print generated token stream")});
    parser.addOption(QCommandLineOption{QStringList{"c", "code"}, i18n("code to parse"), "code"});
    setupCustomArgs<ParserT>(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    QStringList files = parser.positionalArguments();
    bool printAst = parser.isSet("print-ast");
    bool printTokens = parser.isSet("print-tokens");

    KDevelop::AutoTestShell::init();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    KDevelop::DUChain::self()->disablePersistentStorage();
    KDevelop::CodeRepresentation::setDiskChangesForbidden(true);

    ParserT parserT(printAst, printTokens);
    setCustomArgs(&parserT, &parser);

    if (parser.isSet("code")) {
        parserT.parseCode(parser.value("code"));
    } else if (files.isEmpty()) {
        files << "-";
    }

    foreach(const QString &fileName, files) {
        if ( fileName == "-" ) {
#ifndef Q_OS_WIN
            if ( isatty(STDIN_FILENO) ) {
                qerr << "no STDIN given" << endl;
                return 255;
            }
#endif
            parserT.parseCode( qin.readAll().toUtf8() );
        } else {
            parserT.parseFile(fileName);
        }

    }

    KDevelop::TestCore::shutdown();

    return 0;
}
}

#endif // KDEVPLATFORM_DEBUGLANGUAGEPARSERHELPER_H
