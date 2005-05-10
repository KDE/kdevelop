/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sstream>

#include "backgroundparser.h"
#include "problemreporter.h"
#include "PascalLexer.hpp"
#include "PascalParser.hpp"
#include "PascalAST.hpp"
#include <kdebug.h>
#include <qfile.h>
#include <antlr/ASTFactory.hpp>

BackgroundParser::BackgroundParser( ProblemReporter* reporter,
                                    const QString& source,
                                    const QString& filename )
    : m_reporter( reporter ),
      m_source( source.unicode(), source.length() ),
      m_fileName( filename )
{
}

BackgroundParser::~BackgroundParser()
{
}

void BackgroundParser::run()
{
    kdDebug() << "11" << endl;

    QCString _fn = QFile::encodeName(m_fileName);
    std::string fn( _fn.data() );

    QCString text = m_source.utf8();
    std::istringstream stream( text.data() );

    kdDebug() << "12" << endl;

    PascalLexer lexer( stream );
    lexer.setFilename( fn );
    lexer.setProblemReporter( m_reporter );

    kdDebug() << "13" << endl;

    PascalParser parser( lexer );
    parser.setFilename( fn );
    parser.setProblemReporter( m_reporter );

        antlr::ASTFactory my_factory( "PascalAST", PascalAST::factory );
        parser.initializeASTFactory(my_factory);
        parser.setASTFactory( &my_factory );

    kdDebug() << "14" << endl;

    try{

        kdDebug() << "15" << endl;

        lexer.resetErrors();
        parser.resetErrors();

        kdDebug() << "16" << endl;

        parser.compilationUnit();

        kdDebug() << "17" << endl;

        int errors = lexer.numberOfErrors() + parser.numberOfErrors();

        kdDebug() << "18" << endl;
    } catch( antlr::ANTLRException& ex ){

        kdDebug() << "19" << endl;

        kdDebug() << "*exception*: " << ex.toString().c_str() << endl;
        m_reporter->reportError( ex.getMessage().c_str(),
            m_fileName,
            lexer.getLine(),
            lexer.getColumn() );
    }

    kdDebug(9013) << "FINISHED!!" << endl;
}



