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

#include "driver.h"
#include "lexer.h"
#include "parser.h"
#include <kdebug.h>

Driver::Driver()
{
    m_dependences.setAutoDelete( true );
    m_macros.setAutoDelete( true );
    m_problems.setAutoDelete( true );
}

Driver::~Driver()
{
}

void Driver::reset( )
{
    m_dependences.clear();
    m_macros.clear();
    m_problems.clear();
}

void Driver::clear( const QString & fileName )
{
    m_dependences.remove( fileName );
    m_macros.remove( fileName );
    m_problems.remove( fileName );
}

void Driver::addDependence( const QString & fileName, const Dependence & dep )
{
    findOrInsertDependenceList( fileName )->append( dep );
}

void Driver::addMacro( const QString & fileName, const Macro & macro )
{
    findOrInsertMacroList( fileName )->append( macro );
}

void Driver::addProblem( const QString & fileName, const Problem & problem )
{
    findOrInsertProblemList( fileName )->append( problem );
}

QValueList < Dependence > * Driver::findOrInsertDependenceList( const QString & fileName )
{
    QValueList < Dependence > * l = m_dependences.find( fileName );
    if( l == 0 ){
	l = new QValueList < Dependence >();
	m_dependences.insert( fileName, l );
    }
    return l;
}

QValueList < Macro > * Driver::findOrInsertMacroList( const QString & fileName )
{
    QValueList < Macro > * l = m_macros.find( fileName );
    if( l == 0 ){
	l = new QValueList < Macro >();
	m_macros.insert( fileName, l );
    }
    return l;
}

QValueList < Problem > * Driver::findOrInsertProblemList( const QString & fileName )
{
    QValueList < Problem > * l = m_problems.find( fileName );
    if( l == 0 ){
	l = new QValueList < Problem >();
	m_problems.insert( fileName, l );
    }
    return l;
}

TranslationUnitAST :: Node Driver::parseFile( const QString & fileName, const QString& source )
{
    clear( fileName );
    
    m_currentFileName = fileName;
    
    Lexer lexer( this );
    setupLexer( &lexer );
    
    lexer.setSource( source );
    Parser parser( this, &lexer );
    setupParser( &parser );
        
    TranslationUnitAST :: Node translationUnit;
    parser.parseTranslationUnit( translationUnit );
    
    m_currentFileName = QString::null;
    
    return translationUnit;
}

void Driver::setupLexer( Lexer * lexer )
{
    // stl
    lexer->addSkipWord( "__STL_BEGIN_NAMESPACE" );
    lexer->addSkipWord( "__STL_END_NAMESPACE" );
    lexer->addSkipWord( "__STL_BEGIN_RELOPS_NAMESPACE" );
    lexer->addSkipWord( "__STL_END_RELOPS_NAMESPACE" );
    lexer->addSkipWord( "__STL_TEMPLATE_NULL" );
    lexer->addSkipWord( "__STL_TRY" );
    lexer->addSkipWord( "__STL_UNWIND" );
    lexer->addSkipWord( "__STL_NOTHROW" );
    lexer->addSkipWord( "__STL_NULL_TMPL_ARGS" );
    lexer->addSkipWord( "__STL_UNWIND", SkipWordAndArguments );
    lexer->addSkipWord( "__GC_CONST" );
    lexer->addSkipWord( "__HASH_ALLOC_INIT", SkipWordAndArguments );
    lexer->addSkipWord( "_ROPE_SWAP_SPECIALIZATION", SkipWordAndArguments );
    lexer->addSkipWord( "__ROPE_DEFINE_ALLOCS", SkipWordAndArguments );
    
    // antlr
    lexer->addSkipWord( "ANTLR_USE_NAMESPACE", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_USING_NAMESPACE", SkipWordAndArguments );
    
    // gnu
    lexer->addSkipWord( "__extension__" );
    lexer->addSkipWord( "__attribute__", SkipWordAndArguments );
    
    // kde
    lexer->addSkipWord( "K_SYCOCATYPE", SkipWordAndArguments );
    lexer->addSkipWord( "EXPORT_DOCKCLASS" );
    lexer->addSkipWord( "K_EXPORT_COMPONENT_FACTORY", SkipWordAndArguments );
    
    // qt
    lexer->addSkipWord( "Q_OVERRIDE", SkipWordAndArguments );
    lexer->addSkipWord( "Q_ENUMS", SkipWordAndArguments );
    lexer->addSkipWord( "Q_PROPERTY", SkipWordAndArguments );
    lexer->addSkipWord( "Q_CLASSINFO", SkipWordAndArguments );
    lexer->addSkipWord( "Q_SETS", SkipWordAndArguments );
    lexer->addSkipWord( "Q_UNUSED", SkipWordAndArguments );
    lexer->addSkipWord( "Q_CREATE_INSTANCE", SkipWordAndArguments );
    lexer->addSkipWord( "Q_DUMMY_COMPARISON_OPERATOR", SkipWordAndArguments );
    lexer->addSkipWord( "ACTIVATE_SIGNAL_WITH_PARAM", SkipWordAndArguments );
    lexer->addSkipWord( "Q_INLINE_TEMPLATES" );
    lexer->addSkipWord( "Q_TEMPLATE_EXTERN" );
    lexer->addSkipWord( "Q_TYPENAME" );
    lexer->addSkipWord( "Q_REFCOUNT" );
    lexer->addSkipWord( "Q_EXPLICIT" );
    lexer->addSkipWord( "QMAC_PASCAL" );
    lexer->addSkipWord( "QT_STATIC_CONST" );
    lexer->addSkipWord( "QT_WIN_PAINTER_MEMBERS" );
    lexer->addSkipWord( "QT_NC_MSGBOX" );

    // flex
    lexer->addSkipWord( "yyconst" );
    lexer->addSkipWord( "YY_RULE_SETUP" );
    lexer->addSkipWord( "YY_BREAK" );
    lexer->addSkipWord( "YY_RESTORE_YY_MORE_OFFSET" );

    // windows
    lexer->addSkipWord( "WINAPI" );

    lexer->addSkipWord( "__cdecl" );
}

void Driver::setupParser( Parser * parser )
{
    Q_UNUSED( parser );
}

QValueList < Dependence > Driver::dependences( const QString & fileName ) const
{
    if( m_dependences.find(fileName) )
	return *m_dependences.find(fileName);
    return QValueList<Dependence>();
}

QValueList < Macro > Driver::macros( const QString & fileName ) const
{
    if( m_macros.find(fileName) )
	return *m_macros.find(fileName);
    return QValueList<Macro>();
}

QValueList < Problem > Driver::problems( const QString & fileName ) const
{
    if( m_problems.find(fileName) )
	return *m_problems.find(fileName);
    return QValueList<Problem>();
}

