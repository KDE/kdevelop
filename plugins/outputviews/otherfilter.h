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

#ifndef OtherFilter_h
#define OtherFilter_h

#include "outputfilter.h"

#include <qobject.h>

class MakeItem;

class OtherFilter : public QObject, public OutputFilter
{
Q_OBJECT

public:
	OtherFilter();

	virtual void processLine( const QString& line );

signals:
	void item( MakeItem* );
};

#endif
