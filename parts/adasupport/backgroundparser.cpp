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

#include <strstream>

#include "backgroundparser.h"
#include "problemreporter.h"
#include "AdaLexer.hpp"
#include "AdaParser.hpp"
#include "AdaAST.hpp"
#include <kdebug.h>
#include <qfile.h>


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
    QCString _fn = QFile::encodeName(m_fileName);
    std::string fn( _fn.data() );

    QCString text = m_source.utf8();
    std::istrstream stream( text );

    AdaLexer lexer( stream );
    lexer.setFilename( fn );
    lexer.setProblemReporter( m_reporter );

    AdaParser parser( lexer );
    parser.setFilename( fn );
    parser.setProblemReporter( m_reporter );

    // make an ast factory
    antlr::ASTFactory ast_factory;
    // initialize and put it in the parser...
    parser.initializeASTFactory (ast_factory);
    parser.setASTFactory (&ast_factory);
    // parser.setASTNodeType ("RefAdaAST");

    try{
        lexer.resetErrors();
        parser.resetErrors();

        parser.compilation_unit();
        int errors = lexer.numberOfErrors() + parser.numberOfErrors();

    } catch( antlr::ANTLRException& ex ){
        kdDebug() << "*exception*: " << ex.toString().c_str() << endl;
        m_reporter->reportError( ex.getMessage(),
				 m_fileName,
				 lexer.getLine(),
				 lexer.getColumn() );
    }

    kdDebug() << "BackgroundParser::run() FINISHED." << endl;
}



