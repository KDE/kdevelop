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
    m_driver.reset();
    unlock();
}

void BackgroundParser::removeFile( const QString& fileName )
{
    lock();
    m_unitDict.remove( fileName );
    m_driver.clear( fileName );
    unlock();
}

Unit* BackgroundParser::parseFile( const QString& fileName, const QString& contents )
{
    TranslationUnitAST::Node translationUnit = m_driver.parseFile( fileName, contents );
 
    Unit* unit = new Unit;
    unit->fileName = fileName;
    unit->translationUnit = translationUnit.release();
    unit->problems = m_driver.problems( fileName );

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

    unit = m_unitDict.find( fileName );
    if( !unit && 0 != (unit = parseFile(fileName)) )
	m_unitDict.insert( fileName, unit );

    return unit;
}

TranslationUnitAST* BackgroundParser::translationUnit( const QString& fileName )
{
    Unit* u = findOrCreateUnit( fileName );
    return u ? u->translationUnit : 0;
}

QValueList<Problem> BackgroundParser::problems( const QString& fileName )
{
    Unit* u = findOrCreateUnit( fileName );
    return u ? u->problems : QValueList<Problem>();
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

	    m_unitDict.remove( fileName );
	    m_unitDict.insert( fileName, unit );

	    KApplication::postEvent( m_cppSupport, new FileParsedEvent(fileName) );
	    KApplication::postEvent( m_cppSupport, new FoundProblemsEvent(fileName, unit->problems) );
	    unlock();

	    kdDebug(9007) << "!!!!!!!!!!!!!!! PARSED !!!!!!!!!!!!!!!!!!" << endl;
	}
    }
}


