/**************************************************************************
**                              parser.h
**                              --------
**      begin                   : Tue Jul 30 23:53:52 2002
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

#ifndef KDEVPLATFORM_PLUGIN_DIFF2_PARSER_H
#define KDEVPLATFORM_PLUGIN_DIFF2_PARSER_H

#include "diffmodellist.h"
#include "kompare.h"

namespace Diff2
{

class DiffModel;
class KompareModelList;

class Parser
{
public:
	Parser( const KompareModelList* list );
	~Parser();

public:
	DiffModelList* parse( QStringList& diffLines );

	enum Kompare::Generator generator() const { return m_generator; };
	enum Kompare::Format    format() const    { return m_format; };

private:
	/** Which program was used to generate the output */
	enum Kompare::Generator determineGenerator( const QStringList& diffLines );

	int cleanUpCrap( QStringList& diffLines );

private:
	enum Kompare::Generator m_generator;
	enum Kompare::Format    m_format;

	const KompareModelList* m_list;
};

} // End of namespace Diff2

#endif

