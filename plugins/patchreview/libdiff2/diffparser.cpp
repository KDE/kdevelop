/**************************************************************************
**                              diffparser.cpp
**                              --------------
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

#include "diffparser.h"

#include <QtCore/QRegExp>

#include <kdebug.h>

using namespace Diff2;

DiffParser::DiffParser( const KompareModelList* list, const QStringList& diff ) : ParserBase( list, diff )
{
	// The regexps needed for context diff parsing, the rest is the same as in parserbase.cpp
	m_contextDiffHeader1.setPattern( "\\*\\*\\* ([^\\t]+)\\t([^\\t]+)\\n" );
	m_contextDiffHeader2.setPattern( "--- ([^\\t]+)\\t([^\\t]+)\\n" );
}

DiffParser::~DiffParser()
{
}

enum Kompare::Format DiffParser::determineFormat()
{
	kDebug(8101) << "Determining the format of the diff Diff" << m_diffLines << endl;

	QRegExp normalRE ( "[0-9]+[0-9,]*[acd][0-9]+[0-9,]*" );
	QRegExp unifiedRE( "^--- " );
	QRegExp contextRE( "^\\*\\*\\* " );
	QRegExp rcsRE    ( "^[acd][0-9]+ [0-9]+" );
	QRegExp edRE     ( "^[0-9]+[0-9,]*[acd]" );

	QStringList::ConstIterator it = m_diffLines.begin();

	while( it != m_diffLines.end() )
	{
		kDebug(8101) << (*it) << endl;
		if( it->indexOf( normalRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from a Normal diff..." << endl;
			return Kompare::Normal;
		}
		else if( it->indexOf( unifiedRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from a Unified diff..." << endl;
			return Kompare::Unified;
		}
		else if( it->indexOf( contextRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from a Context diff..." << endl;
			return Kompare::Context;
		}
		else if( it->indexOf( rcsRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from an RCS diff..." << endl;
			return Kompare::RCS;
		}
		else if( it->indexOf( edRE, 0 ) == 0 )
		{
			kDebug(8101) << "Difflines are from an ED diff..." << endl;
			return Kompare::Ed;
		}
		++it;
	}
	kDebug(8101) << "Difflines are from an unknown diff..." << endl;
	return Kompare::UnknownFormat;
}
