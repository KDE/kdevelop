/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2012 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <KAboutData>

#include <language/util/debuglanguageparserhelper.h>

#include "../duchain/debugvisitor.h"
#include "../duchain/parsesession.h"

using namespace KDevelop;
using namespace KDevelopUtils;

class QmlParser {
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
    KAboutData aboutData( "cpp-parser", 0, ki18n( "cpp-parser" ),
                          "1", ki18n("KDevelop CPP parser debugging utility"), KAboutData::License_GPL,
                          ki18n( "2011 Milian Wolff" ), KLocalizedString(), "http://www.kdevelop.org" );

    return KDevelopUtils::initAndRunParser<QmlParser>(aboutData, argc, argv);
}
