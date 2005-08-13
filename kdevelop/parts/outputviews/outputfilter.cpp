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

#include "outputfilter.h"

#include <kdebug.h>

OutputFilter::OutputFilter( OutputFilter& next )
	: m_next( next )
{
}

void OutputFilter::processLine( const QString& line )
{
 m_next.processLine( line );
}
