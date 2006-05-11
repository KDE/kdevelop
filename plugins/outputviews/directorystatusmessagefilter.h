/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DirectoryStatusMessageFilter_h
#define DirectoryStatusMessageFilter_h

#include <QObject>
#include "outputfilter.h"

class EnteringDirectoryItem;
class ExitingDirectoryItem;

class DirectoryStatusMessageFilter :  public QObject, public OutputFilter
{
Q_OBJECT

public:
	DirectoryStatusMessageFilter( OutputFilter& );

	void processLine( const QString& line );

signals:
	void item( EnteringDirectoryItem* );
	void item( ExitingDirectoryItem*  );

private:
	bool matchEnterDir( const QString& line, QString& dir );
	bool matchLeaveDir( const QString& line, QString& dir );
};

#endif
