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

    // stl
    lexer.addSkipWord( "__STL_BEGIN_NAMESPACE" );
    lexer.addSkipWord( "__STL_END_NAMESPACE" );
    lexer.addSkipWord( "__STL_BEGIN_RELOPS_NAMESPACE" );
    lexer.addSkipWord( "__STL_END_RELOPS_NAMESPACE" );

    lexer.addSkipWord( "__STL_NULL_TMPL_ARGS" );
    lexer.addSkipWord( "__GC_CONST" );
    lexer.addSkipWord( "__STL_TEMPLATE_NULL" );
    lexer.addSkipWord( "__STL_TRY" );
    lexer.addSkipWord( "__STL_UNWIND" );
    lexer.addSkipWord( "__STL_NOTHROW" );
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
    lexer.addSkipWord( "EXPORT_DOCKCLASS" );

    // qt
    lexer.addSkipWord( "QT_STATIC_CONST" );
    lexer.addSkipWord( "QMAC_PASCAL" );
    lexer.addSkipWord( "Q_TEMPLATE_EXTERN" );
    lexer.addSkipWord( "Q_EXPORT" );
    lexer.addSkipWord( "Q_OVERRIDE", SkipWordAndArguments );
    lexer.addSkipWord( "Q_ENUMS", SkipWordAndArguments );
    lexer.addSkipWord( "Q_PROPERTY", SkipWordAndArguments );
    lexer.addSkipWord( "Q_SETS", SkipWordAndArguments );
    lexer.addSkipWord( "Q_INLINE_TEMPLATES" );
    lexer.addSkipWord( "Q_TYPENAME" );
    lexer.addSkipWord( "Q_REFCOUNT" );
    lexer.addSkipWord( "Q_EXPORT_STYLE_WINDOWS" );
    lexer.addSkipWord( "Q_EXPLICIT" );
    lexer.addSkipWord( "Q_UNUSED", SkipWordAndArguments );
    lexer.addSkipWord( "QT_WIN_PAINTER_MEMBERS" );
    lexer.addSkipWord( "Q_PNGEXPORT" );
    lexer.addSkipWord( "Q_CREATE_INSTANCE", SkipWordAndArguments );

    lexer.addSkipWord( "Q_EXPORT_CODECS_JP" );
    lexer.addSkipWord( "Q_EXPORT_CODECS_KR" );
    lexer.addSkipWord( "Q_EXPORT_CODECS_CN" );

    lexer.addSkipWord( "Q_EXPORT_STYLE_SGI" );
    lexer.addSkipWord( "Q_EXPORT_STYLE_CDE" );
    lexer.addSkipWord( "Q_EXPORT_STYLE_COMPACT" );
    lexer.addSkipWord( "Q_EXPORT_STYLE_MOTIFPLUS" );
    lexer.addSkipWord( "Q_EXPORT_STYLE_MOTIF" );
    lexer.addSkipWord( "Q_EXPORT_STYLE_PLATINUM" );

    lexer.addSkipWord( "QM_EXPORT_NETWORK" );
    lexer.addSkipWord( "QM_EXPORT_XML" );
    lexer.addSkipWord( "QM_EXPORT_WORKSPACE" );
    lexer.addSkipWord( "QM_EXPORT_TABLE" );
    lexer.addSkipWord( "QM_EXPORT_SQL" );
    lexer.addSkipWord( "QM_EXPORT_CANVAS" );
    lexer.addSkipWord( "QM_EXPORT_DNS" );
    lexer.addSkipWord( "QM_EXPORT_DOM" );
    lexer.addSkipWord( "QM_EXPORT_FTP" );
    lexer.addSkipWord( "QM_EXPORT_OPENGL" );
    lexer.addSkipWord( "QM_EXPORT_HTTP" );
    lexer.addSkipWord( "QM_EXPORT_ICONVIEW" );

    lexer.setSource( m_source );
    Parser parser( m_reporter, &driver,  &lexer );
    parser.setFileName( m_fileName );

    parser.parseTranslationUnit();

    kdDebug(9007) << "FINISHED!!" << endl;
}



