/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   raggi@cli.di.unipi.it                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "backgroundparser.h"
#include "lexer.h"
#include "parser.h"
#include "driver.h"
#include "problemreporter.h"
#include <kdebug.h>

BackgroundParser::BackgroundParser( ProblemReporter* reporter,
                                    const QString& source,
                                    const QString& filename )
    : m_reporter( reporter ),
      m_source( source.unicode(), source.length() ),
      m_fileName( filename.unicode(), filename.length() )
{
}

BackgroundParser::~BackgroundParser()
{
}

void BackgroundParser::run()
{
    Driver driver;

    Lexer lexer;

    // TODO: remove hard coded special words

    // stl
    lexer.addSkipWord( "__STL_BEGIN_NAMESPACE" );
    lexer.addSkipWord( "__STL_END_NAMESPACE" );
    lexer.addSkipWord( "__STL_BEGIN_RELOPS_NAMESPACE" );
    lexer.addSkipWord( "__STL_END_RELOPS_NAMESPACE" );
    lexer.addSkipWord( "__STL_TEMPLATE_NULL" );
    lexer.addSkipWord( "__STL_TRY" );
    lexer.addSkipWord( "__STL_UNWIND" );
    lexer.addSkipWord( "__STL_NOTHROW" );
    lexer.addSkipWord( "__STL_NULL_TMPL_ARGS" );
    lexer.addSkipWord( "__GC_CONST" );
    lexer.addSkipWord( "__HASH_ALLOC_INIT", SkipWordAndArguments );
    lexer.addSkipWord( "_ROPE_SWAP_SPECIALIZATION", SkipWordAndArguments );
    lexer.addSkipWord( "__ROPE_DEFINE_ALLOCS", SkipWordAndArguments );

    // antlr
    lexer.addSkipWord( "ANTLR_USE_NAMESPACE", SkipWordAndArguments );
    lexer.addSkipWord( "ANTLR_USING_NAMESPACE", SkipWordAndArguments );
    // gnu
    lexer.addSkipWord( "__extension__" );
    lexer.addSkipWord( "__attribute__", SkipWordAndArguments );

    // kde
    lexer.addSkipWord( "K_SYCOCATYPE", SkipWordAndArguments );
    lexer.addSkipWord( "EXPORT_DOCKCLASS" );
    lexer.addSkipWord( "K_EXPORT_COMPONENT_FACTORY", SkipWordAndArguments );

    // qt
    lexer.addSkipWord( "Q_OVERRIDE", SkipWordAndArguments );
    lexer.addSkipWord( "Q_ENUMS", SkipWordAndArguments );
    lexer.addSkipWord( "Q_PROPERTY", SkipWordAndArguments );
    lexer.addSkipWord( "Q_SETS", SkipWordAndArguments );
    lexer.addSkipWord( "Q_UNUSED", SkipWordAndArguments );
    lexer.addSkipWord( "Q_CREATE_INSTANCE", SkipWordAndArguments );
    lexer.addSkipWord( "Q_DUMMY_COMPARISON_OPERATOR", SkipWordAndArguments );
    lexer.addSkipWord( "ACTIVATE_SIGNAL_WITH_PARAM", SkipWordAndArguments );
    lexer.addSkipWord( "Q_INLINE_TEMPLATES" );
    lexer.addSkipWord( "Q_TEMPLATE_EXTERN" );
    lexer.addSkipWord( "Q_TYPENAME" );
    lexer.addSkipWord( "Q_REFCOUNT" );
    lexer.addSkipWord( "Q_EXPLICIT" );
    lexer.addSkipWord( "QMAC_PASCAL" );
    lexer.addSkipWord( "__cdecl" );
    lexer.setSource( m_source );
    Parser parser( m_reporter, &driver,  &lexer );
    parser.setFileName( m_fileName );

    TranslationUnitAST::Ptr translationUnit;
    parser.parseTranslationUnit( translationUnit );
    
    // a simple test
    QPtrList<AST> decls = translationUnit->declarations();
    kdDebug(9007) << "--> found " << decls.count() << " toplevel declarations" << endl;

    kdDebug(9007) << "FINISHED!!" << endl;
}



