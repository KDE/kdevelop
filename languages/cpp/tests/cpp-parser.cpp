/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Niko Sams <niko.sams@gmail.com>                        *
 *   Copyright 2009 Milian Wolff <mail@milianw.de>                         *
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

#include "parsesession.h"
#include "lexer.h"
#include "parser.h"
#include "dumptree.h"
#include "ast.h"
#include "tokens.h"

#include <KAboutData>

#include <language/util/debuglanguageparserhelper.h>

#include "rpp/pp-location.h"
#include "rpp/preprocessor.h"
#include "rpp/pp-engine.h"

#include "environmentmanager.h"

#include "contextbuilder.h"
#include "cpppreprocessenvironment.h"
#include "cpputils.h"
#include "control.h"
#include <memorypool.h>

using namespace Cpp;
using namespace KDevelopUtils;

class CppParser {
public:
    CppParser(const bool printAst, const bool printTokens)
      : m_printAst(printAst), m_printTokens(printTokens)
    {
    }

    /// parse contents of a file
    void parseFile( const QString &fileName )
    {
      m_session.setUrl(IndexedString(fileName));
      rpp::Preprocessor preprocessor;
      rpp::pp pp(&preprocessor);
      m_session.setContentsAndGenerateLocationTable(pp.processFile(fileName));
      runSession();
    }

    /// parse code directly
    void parseCode( const QString &code )
    {
      qout << "Parsing input" << endl;
      m_session.setUrl(IndexedString("-"));

      rpp::Preprocessor preprocessor;
      rpp::pp pp(&preprocessor);
      m_session.setContentsAndGenerateLocationTable(pp.processFile("anonymous", code.toUtf8()));
      runSession();
    }

private:
    /**
     * actually run the parse session
     */
    void runSession()
    {
      Control control;
      Parser parser(&control);
      TranslationUnitAST* ast = parser.parse(&m_session);

      if (m_printTokens) {
        if ( !m_session.token_stream || m_session.token_stream->isEmpty() ) {
          qout << "empty token stream";
        } else {
          qout << "token stream:" << endl;
          for(int i = 0; i < m_session.token_stream->count(); ++i) {
            const Token& t = m_session.token_stream->at(i);
            const QString str = m_session.token_stream->symbolString(t);
            Q_ASSERT(t.size || str.isEmpty());
            qout << token_name(t.kind) << ": " << str << endl;
          }
        }
      }

      if (!ast) {
        qerr << "no AST tree could be generated" << endl;
      } else {
        ast->session = &m_session;
        qout << "AST tree successfully generated" << endl;
        if (m_printAst) {
          DumpTree dumper;
          dumper.dump(ast, m_session.token_stream, true);
        }
      }
      if (!control.problems().isEmpty()) {
        qout << endl << "problems encountered during parsing:" << endl;
        foreach(KDevelop::ProblemPointer p, control.problems()) {
          qout << p->toString() << endl;
        }
      } else {
        qout << "no problems encountered during parsing" << endl;
      }

      qout << "contents vector size: " << m_session.contentsVector().size() << endl;
      qout << "mempool size: " << m_session.mempool->size() << endl;

      if (!ast) {
        exit(255);
      }
    }

    ParseSession m_session;
    const bool m_printAst;
    const bool m_printTokens;
};

int main(int argc, char* argv[])
{
    KAboutData aboutData( "cpp-parser", 0, ki18n( "cpp-parser" ),
                          "1", ki18n("KDevelop CPP parser debugging utility"), KAboutData::License_GPL,
                          ki18n( "2011 Milian Wolff" ), KLocalizedString(), "http://www.kdevelop.org" );

    return KDevelopUtils::initAndRunParser<CppParser>(aboutData, argc, argv);
}
