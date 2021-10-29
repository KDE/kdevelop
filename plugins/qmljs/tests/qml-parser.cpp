/*
    SPDX-FileCopyrightText: 2012 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KAboutData>

#include <language/util/debuglanguageparserhelper.h>

#include "../duchain/debugvisitor.h"
#include "../duchain/parsesession.h"

using namespace KDevelop;
using namespace KDevelopUtils;

class QmlParser {
    using TextStreamFunction = QTextStream& (*)(QTextStream&);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    static constexpr TextStreamFunction endl = Qt::endl;
#else
    static constexpr TextStreamFunction endl = ::endl;
#endif

public:
    QmlParser(const bool printAst, const bool printTokens)
      : m_printAst(printAst), m_printTokens(printTokens)
    {
    }

    /// parse contents of a file
    void parseFile( const QString &fileName )
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        m_session.reset(new ParseSession(IndexedString(fileName), file.readAll(), 0));
        runSession();
    }

    /// parse code directly
    void parseCode( const QString &code )
    {
        m_session.reset(new ParseSession(IndexedString("-"), code, 0));
        runSession();
    }

private:
    /**
     * actually run the parse session
     */
    void runSession()
    {
        if (!m_session->isParsedCorrectly()) {
            qerr << "failed to parse code" << endl;
        }
        if (m_printTokens) {
            qerr << "token cannot be printed for qml/js source..." << endl;
        }

        if (!m_session->ast()) {
            qerr << "no AST tree could be generated" << endl;
        } else {
            qout << "AST tree successfully generated" << endl;
            if (m_printAst) {
                ///FIXME:
                DebugVisitor visitor(m_session.data());
                visitor.startVisiting(m_session->ast());
            }
        }
        if (!m_session->problems().isEmpty()) {
            qerr << endl << "problems encountered during parsing:" << endl;
            foreach(const ProblemPointer problem, m_session->problems()) {
                qerr << problem->toString();
            }
        } else {
            qout << "no problems encountered during parsing" << endl;
        }

        if (!m_session->ast()) {
            exit(255);
        }
    }

    QScopedPointer<ParseSession> m_session;
    const bool m_printAst;
    const bool m_printTokens;
};

int main(int argc, char* argv[])
{
    KAboutData aboutData(QStringLiteral("cpp-parser"), i18n( "cpp-parser" ),
                         QStringLiteral("1"), i18n("KDevelop CPP parser debugging utility"), KAboutLicense::GPL,
                         i18n( "2011 Milian Wolff" ), QString(), QStringLiteral("https://www.kdevelop.org/") );

    return KDevelopUtils::initAndRunParser<QmlParser>(aboutData, argc, argv);
}
