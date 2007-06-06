/**************************************************************************
**                              cvsdiffparser.cpp
**                              -----------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      copyright               : (C) 2002-2004 Otto Bruggeman
**      email                   : otto.bruggeman@home.nl
**
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/
#include "cvsdiffparser.h"
#include <qregexp.h>

#include <kdebug.h>


#include "komparemodellist.h"


using namespace Diff2;

CVSDiffParser::CVSDiffParser( const KompareModelList* list, const QStringList& diff ) : ParserBase( list, diff )
{
	// The regexps needed for context cvs diff parsing, the rest is the same as in parserbase.cpp
	// third capture in header1 is non optional for cvs diff, it is the revision
	m_contextDiffHeader1.setPattern( "\\*\\*\\* ([^\\t]+)\\t([^\\t]+)\\t(.*)\\n" );
	m_contextDiffHeader2.setPattern( "--- ([^\\t]+)\\t([^\\t]+)(|\\t(.*))\\n" );

	m_normalDiffHeader.setPattern( "Index: (.*)\\n" );
}

CVSDiffParser::~CVSDiffParser()
{
}

enum Kompare::Format CVSDiffParser::determineFormat()
{
//	kDebug(8101) << "Determining the format of the CVSDiff" << endl;

	QRegExp normalRE ( "[0-9]+[0-9,]*[acd][0-9]+[0-9,]*" );
	QRegExp unifiedRE( "^--- [^\\t]+\\t" );
	QRegExp contextRE( "^\\*\\*\\* [^\\t]+\\t" );
	QRegExp rcsRE    ( "^[acd][0-9]+ [0-9]+" );
	QRegExp edRE     ( "^[0-9]+[0-9,]*[acd]" );

	QStringList::ConstIterator it = m_diffLines.begin();

	while( it != m_diffLines.end() )
	{
		if( (*it).indexOf( normalRE, 0 ) == 0 )
		{
//			kDebug(8101) << "Difflines are from a Normal diff..." << endl;
			return Kompare::Normal;
		}
		else if( (*it).indexOf( unifiedRE, 0 ) == 0 )
		{
//			kDebug(8101) << "Difflines are from a Unified diff..." << endl;
			return Kompare::Unified;
		}
		else if( (*it).indexOf( contextRE, 0 ) == 0 )
		{
//			kDebug(8101) << "Difflines are from a Context diff..." << endl;
			return Kompare::Context;
		}
		else if( (*it).indexOf( rcsRE, 0 ) == 0 )
		{
//			kDebug(8101) << "Difflines are from a RCS diff..." << endl;
			return Kompare::RCS;
		}
		else if( (*it).indexOf( edRE, 0 ) == 0 )
		{
//			kDebug(8101) << "Difflines are from an ED diff..." << endl;
			return Kompare::Ed;
		}
		++it;
	}
//	kDebug(8101) << "Difflines are from an unknown diff..." << endl;
	return Kompare::UnknownFormat;
}

bool CVSDiffParser::parseNormalDiffHeader()
{
	kDebug(8101) << "CVSDiffParser::parseNormalDiffHeader()" << endl;
	bool result = false;

	QStringList::ConstIterator diffEnd = m_diffLines.end();

	while ( m_diffIterator != diffEnd )
	{
		if ( m_normalDiffHeader.exactMatch( *m_diffIterator ) )
		{
			kDebug(8101) << "Matched length Header = " << m_normalDiffHeader.matchedLength() << endl;
			kDebug(8101) << "Matched string Header = " << m_normalDiffHeader.cap( 0 ) << endl;

			m_currentModel = new DiffModel();
			QObject::connect( m_currentModel, SIGNAL( setModified( bool ) ), m_list, SLOT( slotSetModified( bool ) ) );
			m_currentModel->setSourceFile          ( m_normalDiffHeader.cap( 1 ) );
			m_currentModel->setDestinationFile     ( m_normalDiffHeader.cap( 1 ) );

			result = true;

			++m_diffIterator;
			break;
		}
		else
		{
			kDebug(8101) << "No match for: " << ( *m_diffIterator ) << endl;
		}
		++m_diffIterator;
	}

	if ( result == false )
	{
		// Set this to the first line again and hope it is a single file diff
		m_diffIterator = m_diffLines.begin();
		m_currentModel = new DiffModel();
		QObject::connect( m_currentModel, SIGNAL( setModified( bool ) ), m_list, SLOT( slotSetModified( bool ) ) );
		m_singleFileDiff = true;
	}

	return result;
}


bool CVSDiffParser::parseEdDiffHeader()
{
	return false;
}

bool CVSDiffParser::parseRCSDiffHeader()
{
	return false;
}

bool CVSDiffParser::parseEdHunkHeader()
{
	return false;
}

bool CVSDiffParser::parseRCSHunkHeader()
{
	return false;
}

bool CVSDiffParser::parseEdHunkBody()
{
	return false;
}

bool CVSDiffParser::parseRCSHunkBody()
{
	return false;
}

