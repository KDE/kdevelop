/**************************************************************************
**                              cvsdiffparser.h
**                              ----------------
**      begin                   : Sun Aug  4 15:05:35 2002
**      Copyright 2002-2004 Otto Bruggeman <otto.bruggeman@home.nl>
***************************************************************************/
/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   ( at your option ) any later version.
**
***************************************************************************/

#ifndef CVSDIFF_PARSER_H
#define CVSDIFF_PARSER_H

#include <QtCore/QRegExp>

#include "parserbase.h"

namespace Diff2
{

class KompareModelList;

class CVSDiffParser : public ParserBase
{
public:
	CVSDiffParser( const KompareModelList* list, const QStringList& diff );
	virtual ~CVSDiffParser();

protected:
	virtual enum Kompare::Format determineFormat();

protected:
//	virtual bool parseContextDiffHeader();
	virtual bool parseEdDiffHeader();
	virtual bool parseNormalDiffHeader();
	virtual bool parseRCSDiffHeader();
//	virtual bool parseUnifiedDiffHeader();

//	virtual bool parseContextHunkHeader();
	virtual bool parseEdHunkHeader();
//	virtual bool parseNormalHunkHeader();
	virtual bool parseRCSHunkHeader();
//	virtual bool parseUnifiedHunkHeader();

//	virtual bool parseContextHunkBody();
	virtual bool parseEdHunkBody();
//	virtual bool parseNormalHunkBody();
	virtual bool parseRCSHunkBody();
//	virtual bool parseUnifiedHunkBody();
};

} // End of namespace Diff2

#endif
