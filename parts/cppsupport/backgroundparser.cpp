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

#include "backgroundparser.h"
#include "cppsupportpart.h"
#include "cppsupport_events.h"

#include "driver.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "ast_utils.h"

#include <kparts/part.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kdevpartcontroller.h>

#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>

BackgroundParser::BackgroundParser( CppSupportPart* part )
    : m_cppSupport( part ), m_close( false )
{
    m_unitDict.setAutoDelete( true );
}

BackgroundParser::~BackgroundParser()
{
}

void BackgroundParser::removeAllFiles()
{
    lock();
    m_unitDict.clear();
    unlock();
}

void BackgroundParser::removeFile( const QString& fileName )
{
    lock();
    m_unitDict.remove( fileName );
    unlock();
}

void BackgroundParser::addFile( const QString& fileName)
{
    Unit* unit = parseFile( fileName );
    
    lock();
    m_unitDict.remove( fileName );
    m_unitDict.insert( fileName, unit );
    KApplication::postEvent( m_cppSupport, new FileParsedEvent(fileName) );    
    KApplication::postEvent( m_cppSupport, new FoundProblemsEvent(fileName, unit->problems) );
    unlock();
}

Unit* BackgroundParser::parseFile( const QString& fileName, const QString& contents )
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
    
    lexer.setSource( contents );
    Parser parser( &driver,  &lexer );
    parser.setFileName( fileName );
    
    TranslationUnitAST::Node translationUnit;
    parser.parseTranslationUnit( translationUnit );
 
    Unit* unit = new Unit;
    unit->fileName = fileName;
    unit->translationUnit = translationUnit.release();
    unit->problems = parser.problems();
 
    return unit;
}

Unit* BackgroundParser::parseFile( const QString& fileName )
{
    Unit* unit = 0;
    
    kapp->lock();
    QPtrList<KParts::Part> parts( *m_cppSupport->partController()->parts() );
    QPtrListIterator<KParts::Part> it( parts );
    while( it.current() ){
	KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( it.current() );
	++it;

	KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
	if( !doc || !editIface || doc->url().path() != fileName )
	    continue;
	
	QString contents = editIface->text();
	unit = parseFile( fileName, contents );
    }
    kapp->unlock();

    if( !unit ){
	QFile f( fileName );
	QTextStream stream( &f );
	if( f.open(IO_ReadOnly) ){
	    QString contents = stream.read();
	    f.close();
	    unit = parseFile( fileName, contents );
	}
    }
    
    return unit;
}

Unit* BackgroundParser::findOrCreateUnit( const QString& fileName )
{
    Unit* unit = 0;
    
    m_unitDict.find( fileName );
    if( !unit ){
	unit = parseFile( fileName );
	m_unitDict.insert( fileName, unit );
    }
    
    return unit;
}

TranslationUnitAST* BackgroundParser::translationUnit( const QString& fileName )
{
    TranslationUnitAST* ast = 0;
    ast = findOrCreateUnit( fileName )->translationUnit;    
    return ast;
}

QValueList<Problem> BackgroundParser::problems( const QString& fileName )
{
    QValueList<Problem> problems;
    problems = findOrCreateUnit( fileName )->problems;
    return problems;
}

void BackgroundParser::reparse()
{
    m_changed.wakeOne();
}

void BackgroundParser::close()
{
    m_close = true;
}

void BackgroundParser::run()   
{
    while( true ){
	m_changed.wait();
	
	if( m_close )
	    QThread::exit();
	
	kapp->lock();
	
	KTextEditor::Document* docIface=dynamic_cast<KTextEditor::Document*>(m_cppSupport->partController()->activePart());
	KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( docIface );
	
	QString fileName;
	QString contents;
	
	if( docIface ){
	    fileName = docIface->url().path();
	    contents = editIface->text();
	}
	
	kapp->unlock();
	
	if( editIface && m_cppSupport->fileExtensions().contains(QFileInfo(fileName).extension()) ){
	    lock();
	    Unit* unit = parseFile( fileName, contents );
	    QValueList<Problem> problems = unit->problems;
#ifdef AST_DEBUG
	    TranslationUnitAST* ast = unit->translationUnit;
	    if( ast )
		printDeclarations( ast );
#endif
	    
	    m_unitDict.remove( fileName );
	    m_unitDict.insert( fileName, unit );

	    KApplication::postEvent( m_cppSupport, new FileParsedEvent(fileName) );
	    KApplication::postEvent( m_cppSupport, new FoundProblemsEvent(fileName, unit->problems) );
	    unlock();

	    kdDebug(9007) << "!!!!!!!!!!!!!!! PARSED !!!!!!!!!!!!!!!!!!" << endl;
	}
    }
}


