//
//
// C++ Implementation: svnentry
//
// Description:
//
//
// Author: Mario Scalas <mario.scalas@libero.it>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qfile.h>
#include <qtextstream.h>

#include "svnentry.h"

const QString SvnEntry::invalidMarker = "<Invalid entry>";
const QString SvnEntry::directoryMarker = "D";
const QString SvnEntry::fileMarker = "";
const QString SvnEntry::entrySeparator = "/";

SvnEntry::SvnEntry()
{
	clean();
}

SvnEntry::SvnEntry( const QString &aLine )
{
	parse( aLine );
}

void SvnEntry::clean()
{
	type = invalidMarker;
	fileName = revision = timeStamp = options = tagDate = QString::null;
}

SvnEntry::EntryState SvnEntry::state() const
{
	if (type == invalidMarker)
		return invalidEntry;
	else if (type == directoryMarker)
		return directoryEntry;
	else
		return fileEntry;
}

bool SvnEntry::read( QTextStream &t )
{
	if (t.eof())
		return false;

	parse( t.readLine() );
	return true;
}

void SvnEntry::write( QTextStream &t )
{
	if (state() != invalidEntry)
	{
		t << pack();
	}
}

void SvnEntry::parse( const QString &aLine )
{
	clean();

	if (aLine.startsWith( entrySeparator )) // Is a file?
	{
		type = fileMarker; // Is a file

		int start = 1;
		int length = aLine.find( entrySeparator, start ) - start;
		fileName = aLine.mid( start, length );

		start = start + length + 1;
		length = aLine.find( entrySeparator, start ) - start;
		revision = aLine.mid( start, length );

		start = start + length + 1;
		length = aLine.find( entrySeparator, start ) - start;
		timeStamp = aLine.mid( start, length );

		start = start + length + 1;
		length = aLine.find( entrySeparator, start ) - start;
		options = aLine.mid( start, length );

		start = start + length + 1;
		length = aLine.find( entrySeparator, start ) - start;
		tagDate = aLine.mid( start, length );
	}
	else // Must be a directory then
	{
		type = directoryMarker; // Is a file

		int start = 2;
		int length = aLine.find( entrySeparator, start ) - start;
		fileName = aLine.mid( start, length ); // Ok, it is a directory name really ;)
	}
}

QString SvnEntry::pack() const
{
	return QString::null;
}
