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

#ifndef OutputFilter_h
#define OutputFilter_h

#include <qstring.h>

class OutputFilter
{
public:
	OutputFilter( OutputFilter& );
	virtual ~OutputFilter() {}

	virtual void processLine( const QString& line );

private:
	OutputFilter& m_next;
};

#endif
