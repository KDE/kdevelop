/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann and the KDevelop Team       *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MakeActionFilter_h
#define MakeActionFilter_h

#include <qobject.h>
#include <qregexp.h>
#include <qvaluelist.h>
#include "outputfilter.h"

class MakeItem;

class MakeActionFilter : public QObject, public OutputFilter
{
Q_OBJECT

public:
	MakeActionFilter( OutputFilter& );

	struct ActionFormat
	{
		ActionFormat() {}
		ActionFormat( const QString&, const QString&, const char *, int );
		QString action;
		QString tool;
		QRegExp expression;
		int fileGroup;
	};
	static const QValueList<ActionFormat>& actionFormats();

	void processLine( const QString& line );

signals:
	void item( MakeItem* );
};

#endif
