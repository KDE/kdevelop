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

#include "commandcontinuationfilter.h"
#include <qregexp.h>

CommandContinuationFilter::CommandContinuationFilter( OutputFilter& next )
	: OutputFilter( next )
{
}

void CommandContinuationFilter::processLine( const QString& line )
{
  bool foundLineCont = false;
  QString s = line.stripWhiteSpace();
  if (s.endsWith("\\"))
  {
    m_text += s.left(s.length() - 1);
    foundLineCont = true;
  } else
  {
    m_text += line;
  }

	if ( !foundLineCont )
	{
		OutputFilter::processLine( m_text );
		m_text = "";
	}
}
