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

#include "filerepository.h"
#include "cppsupportpart.h"

#include "driver.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"

#include <qasciidict.h>
#include <qpair.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qguardedptr.h>

#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevpartcontroller.h>

#include <kdebug.h>

class Unit
{
public:
    Unit( const QString& fn )
	: fileName( fn ), translationUnit( 0 )
	{}
    
    virtual ~Unit()
    {
	delete( translationUnit );
	translationUnit = 0;
    }
    
    virtual bool isModified() = 0;
    virtual QString contents() = 0;
        
    QString fileName;
    TranslationUnitAST* translationUnit;
    QValueList<Problem> problems;    
    
private:
    Unit( const Unit& );
    void operator = ( const Unit& );
};

class SourceUnit: public Unit
{
public:
    SourceUnit( const QString& fileName )
	: Unit( fileName ), m_fileInfo( fileName )
    {
	m_lastModified = m_fileInfo.lastModified();
    }
    
    virtual bool isModified() 
    {
	m_fileInfo.refresh();
	return translationUnit == 0 || m_lastModified != m_fileInfo.lastModified();
    }
    
    virtual QString contents()
    {
	kdDebug(9032) << "SourceUnit::contents()" << endl;
	
	m_fileInfo.refresh();
	m_lastModified = m_fileInfo.lastModified();
	
	QString source;
	QFile f( fileName );
	if( f.open(IO_ReadOnly) ){
	    QTextStream stream( &f );
	    source = stream.read();
	    f.close();
	}
	return source;
    }
            
private:
    QFileInfo m_fileInfo;
    QDateTime m_lastModified;
};

class TextEditorUnit: public Unit
{
public:
    TextEditorUnit( KTextEditor::Document* d, const QString& fileName )
	: Unit( fileName ), doc(d) 
    {
	edit = dynamic_cast<KTextEditor::EditInterface*>( doc );
    }
    
    virtual bool isModified()
    {
	return doc->isModified();
    }
    
    virtual QString contents()
    {
	kdDebug(9032) << "TextEditorUnit::contents()" << endl;
	return edit->text();
    }
    
private:
    KTextEditor::Document* doc;
    KTextEditor::EditInterface* edit;
};

struct FileRepositoryData
{
    CppSupportPart* cppSupport;
    QAsciiDict<Unit> units;
    
    FileRepositoryData()
	: cppSupport( 0 )
    {
	units.setAutoDelete( true );
    }
    
    KTextEditor::Document* findDocument( const QString& fileName )
    {
        if( !cppSupport->partController()->parts() )
	   return 0;

	QPtrListIterator<KParts::Part> it( *cppSupport->partController()->parts() );
	while( it.current() ){
	    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( it.current() );
	    ++it;
	    
	    if( doc && doc->url().path() == fileName )
		return doc;
	}
	return 0;
    }
    
    void parse( Unit* unit, bool force )
    {
	if( unit->isModified() || force ){
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
	    
	    lexer.setSource( unit->contents() );
	    Parser parser( &driver,  &lexer );
	    parser.setFileName( unit->fileName );
	    
	    TranslationUnitAST::Node translationUnit;
	    parser.parseTranslationUnit( translationUnit );
	    
	    delete( unit->translationUnit );
	    unit->translationUnit = translationUnit.release();
	    
	    unit->problems = parser.problems();    
	    
	    kdDebug(9007) << "found " << unit->problems.size() << " problems!!" << endl;
	}
    }        
    
};

FileRepository::FileRepository( CppSupportPart* part )
    : QObject( part ), d( new FileRepositoryData() )
{
    d->cppSupport = part;
    connect( d->cppSupport->partController(), SIGNAL(partAdded(KParts::Part*)),
	     this, SLOT(slotPartAdded(KParts::Part*)) );
    connect( d->cppSupport->partController(), SIGNAL(partRemoved(KParts::Part*)),
	     this, SLOT(slotPartRemoved(KParts::Part*)) );
}

FileRepository::~FileRepository()
{
    delete( d );
    d = 0;
}

TranslationUnitAST* FileRepository::translationUnit( const QString& fileName, bool force )
{
    if( fileName.isEmpty() )
        return 0;
	
    Unit* unit = d->units.find( fileName );
    if( !unit ){
	KTextEditor::Document* doc = d->findDocument( fileName );
	if( doc )
	    unit = new TextEditorUnit( doc, fileName );
	else
	    unit = new SourceUnit( fileName );
	d->units.insert( fileName, unit );
    }
    
    d->parse( unit, force );
    
    return unit->translationUnit;
}

QValueList<Problem> FileRepository::problems( const QString& fileName, bool force )
{
    if( fileName.isEmpty() )
        return QValueList<Problem>();
	
    Unit* unit = d->units.find( fileName );
    if( !unit ){
	KTextEditor::Document* doc = d->findDocument( fileName );
	if( doc )
	    unit = new TextEditorUnit( doc, fileName );
	else
	    unit = new SourceUnit( fileName );
	d->units.insert( fileName, unit );
    }
    
    d->parse( unit, force );
    
    return unit->problems;
}

void FileRepository::slotPartAdded( KParts::Part* part )
{
    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
    if( doc ){
	QString fileName = doc->url().path();
	if( !fileName.isEmpty() && d->units.find(fileName) ){
	    d->units.remove( fileName );
	}
    }
}

void FileRepository::slotPartRemoved( KParts::Part* part )
{
    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
    if( doc ){
	QString fileName = doc->url().path();
	if( !fileName.isEmpty() && d->units.find(fileName) ){
	    d->units.remove( fileName );
	}
    }
}

