/**************************************************************************
**                              perforceparser.cpp
**                              ------------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      Copyright 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/

#include "perforceparser.h"

#include <QtCore/QRegExp>

#include <kdebug.h>

using namespace Diff2;

PerforceParser::PerforceParser( const KompareModelList* list, const QStringList& diff ) : ParserBase( list, diff )
{
	m_contextDiffHeader1.setPattern( "==== (.*) - (.*) ====\\n" );
	m_contextDiffHeader1.setMinimal( true );
	m_normalDiffHeader.setPattern  ( "==== (.*) - (.*) ====\\n" );
	m_normalDiffHeader.setMinimal  ( true );
	m_rcsDiffHeader.setPattern     ( "==== (.*) - (.*) ====\\n" );
	m_rcsDiffHeader.setMinimal     ( true );
	m_unifiedDiffHeader1.setPattern( "==== (.*) - (.*) ====\\n" );
	m_unifiedDiffHeader1.setMinimal( true );
}

PerforceParser::~PerforceParser()
{
}

enum Kompare::Format PerforceParser::determineFormat()
{
	kDebug(8101) << "Determining the format of the Perforce Diff" << endl;

	QRegExp unifiedRE( "^@@" );
	QRegExp contextRE( "^\\*{15}" );
	QRegExp normalRE ( "^\\d+(|,\\d+)[acd]\\d+(|,\\d+)" );
	QRegExp rcsRE    ( "^[acd]\\d+ \\d+" );
	// Summary is not supported since it gives no useful parsable info

	QStringList::ConstIterator it = m_diffLines.begin();

	while( it != m_diffLines.end() )
	{
		if( it->indexOf( unifiedRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from a Unified diff..." << endl;
			return Kompare::Unified;
		}
		else if( it->indexOf( contextRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from a Context diff..." << endl;
			return Kompare::Context;
		}
		else if( it->indexOf( normalRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from a Normal diff..." << endl;
			return Kompare::Normal;
		}
		else if( it->indexOf( rcsRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from a RCS diff..." << endl;
			return Kompare::RCS;
		}
		++it;
	}
	kDebug(8101) << "Difflines are from an unknown diff..." << endl;
	return Kompare::UnknownFormat;
}

bool PerforceParser::parseContextDiffHeader()
{
//	kDebug(8101) << "ParserBase::parseContextDiffHeader()" << endl;
	bool result = false;

	QStringList::ConstIterator itEnd = m_diffLines.end();

	QRegExp sourceFileRE     ( "([^\\#]+)#(\\d+)" );
	QRegExp destinationFileRE( "([^\\#]+)#(|\\d+)" );

	while ( m_diffIterator != itEnd )
	{
		if ( m_contextDiffHeader1.exactMatch( *(m_diffIterator)++ ) )
		{
//			kDebug(8101) << "Matched length Header1 = " << m_contextDiffHeader1.matchedLength() << endl;
//			kDebug(8101) << "Matched string Header1 = " << m_contextDiffHeader1.cap( 0 ) << endl;
//			kDebug(8101) << "First capture  Header1 = " << m_contextDiffHeader1.cap( 1 ) << endl;
//			kDebug(8101) << "Second capture Header1 = " << m_contextDiffHeader1.cap( 2 ) << endl;

			m_currentModel = new DiffModel();
			sourceFileRE.exactMatch( m_contextDiffHeader1.cap( 1 ) );
			destinationFileRE.exactMatch( m_contextDiffHeader1.cap( 2 ) );
			kDebug(8101) << "Matched length   = " << sourceFileRE.matchedLength() << endl;
			kDebug(8101) << "Matched length   = " << destinationFileRE.matchedLength() << endl;
			kDebug(8101) << "Captured texts   = " << sourceFileRE.capturedTexts() << endl;
			kDebug(8101) << "Captured texts   = " << destinationFileRE.capturedTexts() << endl;
			kDebug(8101) << "Source File      : " << sourceFileRE.cap( 1 ) << endl;
			kDebug(8101) << "Destination File : " << destinationFileRE.cap( 1 ) << endl;
			m_currentModel->setSourceFile     ( sourceFileRE.cap( 1 ) );
			m_currentModel->setDestinationFile( destinationFileRE.cap( 1 ) );

			result = true;

			break;
		}
		else
		{
			kDebug(8101) << "Matched length = " << m_contextDiffHeader1.matchedLength() << endl;
			kDebug(8101) << "Captured texts = " << m_contextDiffHeader1.capturedTexts() << endl;
		}
	}

	return result;
}

bool PerforceParser::parseNormalDiffHeader()
{
	bool result = false;

	QStringList::ConstIterator itEnd = m_diffLines.end();

	QRegExp sourceFileRE     ( "([^\\#]+)#(\\d+)" );
	QRegExp destinationFileRE( "([^\\#]+)#(|\\d+)" );

	while ( m_diffIterator != itEnd )
	{
		kDebug(8101) << "Line = " << *m_diffIterator << endl;
		kDebug(8101) << "String length  = " << (*m_diffIterator).length() << endl;
		if ( m_normalDiffHeader.exactMatch( *(m_diffIterator)++ ) )
		{
			kDebug(8101) << "Matched length Header1 = " << m_normalDiffHeader.matchedLength() << endl;
			kDebug(8101) << "Matched string Header1 = " << m_normalDiffHeader.cap( 0 ) << endl;
			kDebug(8101) << "First  capture Header1 = \"" << m_normalDiffHeader.cap( 1 ) << "\"" << endl;
			kDebug(8101) << "Second capture Header1 = \"" << m_normalDiffHeader.cap( 2 ) << "\"" << endl;
			
			m_currentModel = new DiffModel();
			sourceFileRE.exactMatch( m_normalDiffHeader.cap( 1 ) );
			destinationFileRE.exactMatch( m_normalDiffHeader.cap( 2 ) );
			kDebug(8101) << "Matched length   = " << sourceFileRE.matchedLength() << endl;
			kDebug(8101) << "Matched length   = " << destinationFileRE.matchedLength() << endl;
			kDebug(8101) << "Captured texts   = " << sourceFileRE.capturedTexts() << endl;
			kDebug(8101) << "Captured texts   = " << destinationFileRE.capturedTexts() << endl;
			kDebug(8101) << "Source File      : " << sourceFileRE.cap( 1 ) << endl;
			kDebug(8101) << "Destination File : " << destinationFileRE.cap( 1 ) << endl;
			m_currentModel->setSourceFile     ( sourceFileRE.cap( 1 ) );
			m_currentModel->setDestinationFile( destinationFileRE.cap( 1 ) );

			result = true;

			break;
		}
		else
		{
			kDebug(8101) << "Matched length = " << m_normalDiffHeader.matchedLength() << endl;
			kDebug(8101) << "Captured texts = " << m_normalDiffHeader.capturedTexts() << endl;
		}
	}

	return result;
}

bool PerforceParser::parseRCSDiffHeader()
{
	return false;
}

bool PerforceParser::parseUnifiedDiffHeader()
{
	bool result = false;

	QStringList::ConstIterator itEnd = m_diffLines.end();

	QRegExp sourceFileRE     ( "([^\\#]+)#(\\d+)" );
	QRegExp destinationFileRE( "([^\\#]+)#(|\\d+)" );

	while ( m_diffIterator != itEnd )
	{
//		kDebug(8101) << "Line = " << *m_diffIterator << endl;
//		kDebug(8101) << "String length  = " << (*m_diffIterator).length() << endl;
		if ( m_unifiedDiffHeader1.exactMatch( *(m_diffIterator)++ ) )
		{
//			kDebug(8101) << "Matched length Header1 = " << m_unifiedDiffHeader1.matchedLength() << endl;
//			kDebug(8101) << "Matched string Header1 = " << m_unifiedDiffHeader1.cap( 0 ) << endl;
//			kDebug(8101) << "First  capture Header1 = \"" << m_unifiedDiffHeader1.cap( 1 ) << "\"" << endl;
//			kDebug(8101) << "Second capture Header1 = \"" << m_unifiedDiffHeader1.cap( 2 ) << "\"" << endl;
			
			m_currentModel = new DiffModel();
			sourceFileRE.exactMatch( m_unifiedDiffHeader1.cap( 1 ) );
			destinationFileRE.exactMatch( m_unifiedDiffHeader1.cap( 2 ) );
//			kDebug(8101) << "Matched length   = " << sourceFileRE.matchedLength() << endl;
//			kDebug(8101) << "Matched length   = " << destinationFileRE.matchedLength() << endl;
//			kDebug(8101) << "Captured texts   = " << sourceFileRE.capturedTexts() << endl;
//			kDebug(8101) << "Captured texts   = " << destinationFileRE.capturedTexts() << endl;
//			kDebug(8101) << "Source File      : " << sourceFileRE.cap( 1 ) << endl;
//			kDebug(8101) << "Destination File : " << destinationFileRE.cap( 1 ) << endl;
			m_currentModel->setSourceFile     ( sourceFileRE.cap( 1 ) );
			m_currentModel->setDestinationFile( destinationFileRE.cap( 1 ) );

			result = true;

			break;
		}
		else
		{
//			kDebug(8101) << "Matched length = " << m_unifiedDiffHeader1.matchedLength() << endl;
//			kDebug(8101) << "Captured texts = " << m_unifiedDiffHeader1.capturedTexts() << endl;
		}
	}

	return result;
}

