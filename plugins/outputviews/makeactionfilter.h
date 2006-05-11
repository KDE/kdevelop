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

#include <QObject>
#include <QRegExp>
#include <q3valuelist.h>
#include "outputfilter.h"

class MakeItem;
class ActionItem;

class MakeActionFilter : public QObject, public OutputFilter
{
Q_OBJECT

public:
#ifdef DEBUG
	static void test();
#endif

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
	static ActionFormat* actionFormats();

	void processLine( const QString& line );

signals:
	void item( MakeItem* );

private:
	static ActionItem* matchLine( const QString& line );
};

#endif
