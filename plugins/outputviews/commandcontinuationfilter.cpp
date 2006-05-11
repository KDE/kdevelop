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
#include <QRegExp>
#include <kdebug.h>

CommandContinuationFilter::CommandContinuationFilter( OutputFilter& next )
	: OutputFilter( next )
{
}

void CommandContinuationFilter::processLine( const QString& line )
{
   int index=line.length()-1;
   while (index>=0)
   {
      if (line[index]=='\\')
      {
         m_text += line.left(index);
         return;
      }

      if (!line[index].isSpace())
         break;
      index--;
   }

   m_text+=line;
   OutputFilter::processLine( m_text );
   m_text = "";
}
