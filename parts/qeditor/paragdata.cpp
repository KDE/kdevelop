// (C) 2001-2002 Trolltech AS

/* $Id$
 *
 *  This file is part of Klint
 *  Copyright (C) 2001 Roberto Raggi (raggi@cli.di.unipi.it)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */


#include "paragdata.h"

ParagData::ParagData()
{
    lastLengthForCompletion = 0;
    m_level = 0;
}

ParagData::~ParagData()
{

}

void ParagData::join( QTextParagData* )
{
    // not implemented yet!!
}

void ParagData::clear()
{
    m_symbolList.clear();
}

void ParagData::add( int tp, const QChar& ch, int pos )
{
    m_symbolList << Symbol( tp, ch, pos );
}

int ParagData::level() const
{
    return m_level;
}

void ParagData::setLevel( int level )
{
    m_level = level;
}
