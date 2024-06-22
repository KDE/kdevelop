/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QTextStream>

namespace KDevelopUtils {
QTextStream qout(stdout);
QTextStream qerr(stderr);
QTextStream qin(stdin);

using TokenTextFunc = QString (*)(int);
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
template <class SessionT, class TokenStreamT, class TokenT, class LexerT,
    class StartAstT, class DebugVisitorT, TokenTextFunc TokenTextT>
class DebugLanguageParserHelper
{
public:
    DebugLanguageParserHelper(const bool printAst, const bool printTokens)
        : m_printAst(printAst)
        , m_printTokens(printTokens)
    {
        m_session.setDebug(printAst);
    }

    /// parse contents of a file
    void parseFile(const QString& fileName)
    {
        if (!m_session.readFile(fileName)) {
            qerr << "Can't open file " << fileName << Qt::endl;
            std::exit(255);
        } else {
            qout << "Parsing file " << fileName << Qt::endl;
        }
        runSession();
    }

    /// parse code directly
    void parseCode(const QString& code)
    {
        m_session.setContents(code);

        qout << "Parsing input" << Qt::endl;
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
                TokenT& t = tokenStream.push();
                t.begin = lexer.tokenBegin();
                t.end = lexer.tokenEnd();
                t.kind = token;
                printToken(token, lexer);
            }
            printToken(token, lexer);
            if (tokenStream.size() > 0) {
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
            qerr << "no AST tree could be generated" << Qt::endl;
        } else {
            qout << "AST tree successfully generated" << Qt::endl;
            if (m_printAst) {
                DebugVisitorT debugVisitor(m_session.tokenStream(), m_session.contents());
                debugVisitor.visitStart(ast);
            }
        }
        const auto problems = m_session.problems();
        if (!problems.isEmpty()) {
            qout << "\nproblems encountered during parsing:" << Qt::endl;
            for (auto& p : problems) {
                qout << p->description() << Qt::endl;
            }
        } else {
            qout << "no problems encountered during parsing" << Qt::endl;
        }

        if (!ast) {
            exit(255);
        }
    }

    void printToken(int token, const LexerT& lexer) const
    {
        int begin = lexer.tokenBegin();
        int end = lexer.tokenEnd();
        qout << m_session.contents().mid(begin, end - begin + 1).replace('\n', "\\n") << ' ' << TokenTextT(token)
             << Qt::endl;
    }

    SessionT m_session;
    const bool m_printAst;
    const bool m_printTokens;
};

template <class ParserT>
void setupCustomArgs(QCommandLineParser* parser)
{
    Q_UNUSED(parser);
}

template <class ParserT>
void setCustomArgs(ParserT* parser, QCommandLineParser* commandLineParser)
{
    Q_UNUSED(parser);
    Q_UNUSED(commandLineParser);
}

/// call this after setting up @p aboutData in your @c main() function.
template <class ParserT>
int initAndRunParser(KAboutData& aboutData, int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);

    parser.addPositionalArgument("files",
                                 i18n(
                                     "files or - to read from STDIN, the latter is the default if nothing is provided"),
                                 "[FILE...]");

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

    for (const QString& fileName : std::as_const(files)) {
        if (fileName == "-") {
#ifndef Q_OS_WIN
            if (isatty(STDIN_FILENO)) {
                qerr << "no STDIN given" << Qt::endl;
                return 255;
            }
#endif
            parserT.parseCode(qin.readAll().toUtf8());
        } else {
            parserT.parseFile(QFileInfo(fileName).absoluteFilePath());
        }
    }

    KDevelop::TestCore::shutdown();

    return 0;
}
}

#endif // KDEVPLATFORM_DEBUGLANGUAGEPARSERHELPER_H
