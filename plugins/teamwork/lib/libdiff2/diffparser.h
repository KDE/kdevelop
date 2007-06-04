/**************************************************************************
**                              diffparser.h
**                              -----------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      copyright               : (C) 2002-2004 Otto Bruggeman
**      email                   : otto.bruggeman@home.nl
**
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/

#ifndef _DIFF_PARSER_H
#define _DIFF_PARSER_H

#include "parserbase.h"

namespace Diff2
{

class DiffParser : public ParserBase
{
public:
	DiffParser( const KompareModelList* list, const QStringList& diff );
	virtual ~DiffParser();

protected:
	virtual enum Kompare::Format determineFormat();
};

} // End of namespace Diff2

#endif
