// Here resides the C/C++ code completion parser

/***************************************************************************
                          cppcodecompletionparser.h  -  description
                             -------------------
	begin		: Fri Aug 3 21:10:00 CEST 2001
	copyright	: (C) 2001 by Victor Röder, Daniel Haberkorn
	email		: Victor_Roeder@GMX.de, DHaberkorn@GMX.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef __CPPCODECOMPLETIONPARSER_H__
#define __CPPCODECOMPLETIONPARSER_H__

#include <qstring.h>
#include <qvaluelist.h>

#include "keditor/edit_iface.h"
#include "keditor/codecompletion_iface.h"
#include "classstore.h"
#include "parsedclass.h"
#include "parsedmethod.h"

class CppCodeCompletionParser
{
	public:
		CppCodeCompletionParser ( KEditor::EditDocumentIface* pEditIface, ClassStore* pStore );
		virtual ~CppCodeCompletionParser();

	public:
		QString getCompletionText ( int nCol );
		int getNodePos ( int nCol );
		QString getNodeText ( int nNode );
		QString getNodeDelimiter ( int nNode );
		QString getTypeOfObject ( const QString& strObject, int nLine );
		QString getCurrentClassname ( int nLine );
		QString getReturnTypeOfMethod ( const QString& strMethod );
		bool isMethod ( const QString& strNodeText );
		bool isObject ( const QString& strNodeText );
		bool isClass ( const QString& strNodeText );
		bool isStruct ( const QString& strNodeText );
		bool isNamespace ( const QString& strNodeText );

		void setLineToBeParsed ( const QString& strLine ) { m_strCurLine = strLine; };

	private:
		KEditor::EditDocumentIface* m_pEditIface;
		ClassStore* m_pStore;
		QString m_strCurLine;

};

#endif
