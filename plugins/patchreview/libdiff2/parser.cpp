/**************************************************************************
**                             parser.cpp
**                             ----------
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

#include "parser.h"

#include <kdebug.h>

#include "cvsdiffparser.h"
#include "diffparser.h"
#include "perforceparser.h"
#include "diffmodel.h"

using namespace Diff2;

Parser::Parser( const KompareModelList* list ) :
	m_list( list )
{
}

Parser::~Parser()
{
}

int Parser::cleanUpCrap( QStringList& diffLines )
{
	QStringList::Iterator it = diffLines.begin();

	int nol = 0;

	QString noNewLine( "\\ No newline" );

	for ( ; it != diffLines.end(); ++it )
	{
		if ( (*it).startsWith( noNewLine ) )
		{
			it = diffLines.erase( it );
			// correcting the advance of the iterator because of the remove
			--it;
			QString temp( *it );
			temp.truncate( temp.indexOf( '\n' ) );
			*it = temp;
			++nol;
		}
	}

	return nol;
} 

DiffModelList* Parser::parse( QStringList& diffLines )
{
	/* Basically determine the generator then call the parse method */
	ParserBase* parser;

	m_generator = determineGenerator( diffLines );

	int nol = cleanUpCrap( diffLines );
	kDebug(8101) << "Cleaned up " << nol << " line(s) of crap from the diff..." << endl;

	switch( m_generator )
	{
	case Kompare::CVSDiff :
		kDebug(8101) << "It is a CVS generated diff..." << endl;
		parser = new CVSDiffParser( m_list, diffLines );
		break;
	case Kompare::Diff :
		kDebug(8101) << "It is a diff generated diff..." << endl;
		parser = new DiffParser( m_list, diffLines );
		break;
	case Kompare::Perforce :
		kDebug(8101) << "It is a Perforce generated diff..." << endl;
		parser = new PerforceParser( m_list, diffLines );
		break;
	default:
		// Nothing to delete, just leave...
		return 0L;
	}

	m_format = parser->format();
	DiffModelList* modelList = parser->parse();
	if ( modelList )
	{
		kDebug(8101) << "Modelcount: " << modelList->count() << endl;
		DiffModelListIterator modelIt = modelList->begin();
		DiffModelListIterator mEnd    = modelList->end();
		for ( ; modelIt != mEnd; ++modelIt )
		{
			kDebug(8101) << "Hunkcount:  " << (*modelIt)->hunkCount() << endl;
			kDebug(8101) << "Diffcount:  " << (*modelIt)->differenceCount() << endl;
		}
	}

	delete parser;

	return modelList;
}

enum Kompare::Generator Parser::determineGenerator( const QStringList& diffLines )
{
	// Shit have to duplicate some code with this method and the ParserBase derived classes
	QString cvsDiff     ( "Index: " );
	QString perforceDiff( "==== " );

	QStringList::ConstIterator it = diffLines.begin();
	QStringList::ConstIterator linesEnd = diffLines.end();

	while (  it != linesEnd )
	{
		if ( ( *it ).startsWith( cvsDiff ) )
		{
			kDebug(8101) << "Diff is a CVSDiff" << endl;
			return Kompare::CVSDiff;
		}
		else if ( ( *it ).startsWith( perforceDiff ) )
		{
			kDebug(8101) << "Diff is a Perforce Diff" << endl;
			return Kompare::Perforce;
		}
		++it;
	}

	kDebug(8101) << "We'll assume it is a diff Diff" << endl;

	// For now we'll assume it is a diff file diff, later we might
	// try to really determine if it is a diff file diff.
	return Kompare::Diff;
}
