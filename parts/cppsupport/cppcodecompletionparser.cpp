// Here resides the C/C++ code completion parser


/***************************************************************************
                          cppcodecompletionparser.cpp  -  description
                             -------------------
	begin		: Fri Aug 3 21:10:00 CEST 2001
	copyright	: (C) 2001 by Daniel Haberkorn, Victor Röder
	email		: DHaberkorn@GMX.de, Victor_Roeder@GMX.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <kregexp.h>

#include "cppcodecompletionparser.h"

CppCodeCompletionParser::CppCodeCompletionParser ( KEditor::EditDocumentIface* pEditIface, ClassStore* pStore )
{
	m_pEditIface = pEditIface;
	m_pStore = pStore;
}

CppCodeCompletionParser::~CppCodeCompletionParser()
{

}

QString CppCodeCompletionParser::getCompletionText ( int nCol )
{
	int nOffset = nCol;
	QString strCompletionText;

	while ( nOffset > 0 )
	{
		if ( m_strCurLine[nOffset] == '.' ||
			m_strCurLine[nOffset] == '>' && m_strCurLine[nOffset - 1] == '-' ||
			m_strCurLine[nOffset] == ':' && m_strCurLine[nOffset - 1] == ':' ||
			m_strCurLine[nOffset] == ' ' || m_strCurLine[nOffset] == ';' )
		{
			nOffset++;
			break;
		}
		else
		{
			//kdDebug ( 9007 ) << "Offset--;" << endl;
			nOffset--;
		}

		if ( m_strCurLine[nOffset] == ':' && m_strCurLine[nOffset - 1] != ':' ||
			m_strCurLine[nOffset] == '-' && m_strCurLine[nOffset - 1] != '>' )
		{
			return "";
		}
	}

	if ( ( nCol - nOffset ) >= 0 )
		strCompletionText = m_strCurLine.mid ( nOffset, ( nCol - nOffset ) );

	//kdDebug ( 9007 ) << "nCol - nOffset: " << (nCol - nOffset) << endl;

	return strCompletionText;
}

int CppCodeCompletionParser::getNodePos ( int nCol )
{
	int nOffset = 0;
	int nNodePos = 0;

	while ( nOffset < nCol )
	{
		if ( m_strCurLine[nOffset] == '.' ||
			m_strCurLine[nOffset] == '-' && m_strCurLine[nOffset + 1] == '>' ||
			m_strCurLine[nOffset] == ':' && m_strCurLine[nOffset + 1] == ':' )
		{
			nNodePos++;
		}

		nOffset++;
	}

	return nNodePos;
}

QString CppCodeCompletionParser::getNodeText ( int nNode )
{
	if ( nNode <= 0 )
		return "";

	int nFrom = 0;
	int nTo = 0;
	int nNodePos = 0;

	while ( nTo < m_strCurLine.length() )
	{
		if ( m_strCurLine[nTo] == '.' )
		{
			nNodePos++;

			if ( nNodePos < nNode )
				nFrom = nTo + 1;
		}

		if ( m_strCurLine[nTo] == '-' && m_strCurLine[nTo + 1] == '>' ||
			m_strCurLine[nTo] == ':' && m_strCurLine[nTo + 1] == ':' )
		{
			nNodePos++;

			if ( nNodePos < nNode )
				nFrom = nTo + 2;
		}

		//kdDebug ( 9007 ) << "nNodePos: " << nNodePos << " nNode: " << nNode << endl;

		if ( nNodePos == nNode )
		{
			for ( nTo = nFrom; nTo < m_strCurLine.length(); nTo++ )
			{
				if ( m_strCurLine[nTo] == '.' )
				{
					return m_strCurLine.mid ( nFrom, ( nTo - nFrom ) );
				}

				if ( m_strCurLine[nTo] == '-' && m_strCurLine[nTo + 1] == '>' ||
					m_strCurLine[nTo] == ':' && m_strCurLine[nTo + 1] == ':' )
				{
					kdDebug ( 9007 ) << "strNodeText: " << m_strCurLine.mid ( nFrom, ( nTo - nFrom ) ) << endl;
					return m_strCurLine.mid ( nFrom, ( nTo - nFrom ) );
				}
			}
		}

		nTo++;

		//kdDebug ( 9007 ) << "getNodeText::while" << endl;
	}

	return "";
}

QString CppCodeCompletionParser::getNodeDelimiter ( int nNode )
{
	if ( nNode <= 0 )
		return "";

	int nFrom = 0;
	int nTo = 0;
	int nNodePos = 0;

	while ( nTo < m_strCurLine.length() )
	{
		if ( m_strCurLine[nTo] == '.' ||
			m_strCurLine[nTo] == '-' && m_strCurLine[nTo + 1] == '>' ||
			m_strCurLine[nTo] == ':' && m_strCurLine[nTo + 1] == ':' )
		{
			nNodePos++;

			if ( nNodePos < nNode )
				nFrom = nTo + 1;
		}

		//kdDebug ( 9007 ) << "nNodePos: " << nNodePos << " nNode: " << nNode << endl;

		if ( nNodePos == nNode )
		{
			for ( nTo = nFrom; nTo < m_strCurLine.length(); nTo++ )
			{
				if ( m_strCurLine[nTo] == '.' )
				{
					return m_strCurLine.mid ( nTo, 1 );
				}

				if ( m_strCurLine[nTo] == '-' && m_strCurLine[nTo + 1] == '>' ||
					m_strCurLine[nTo] == ':' && m_strCurLine[nTo + 1] == ':' )
				{
					return m_strCurLine.mid ( nTo, 2 );
				}
			}
		}

		nTo++;

		//kdDebug ( 9007 ) << "getNodeText::while" << endl;
	}

	return "";
}

QString CppCodeCompletionParser::getCurrentClassname ( int nLine )
{
	KRegExp reMethod ( "[ \t]*([A-Za-z_]+)::([A-Za-z_]+)[ \t]*\\(([0-9A-Za-z_,]*)\\) " ); // finds "blabla::blabla(blabla) "
	KRegExp reConstructor ( "[ \t]*([A-Za-z_]+)::([A-Za-z_]+)[ \t]*\\(([0-9A-Za-z_,]*)\\)[ \t\\{:]+" );
	KRegExp reClass ( "^[ \t]*class[ \t]+([A-Za-z_]+)" );

	for ( int i = nLine; i >= 0; i-- )
	{
		if ( reMethod.match ( ( m_pEditIface->line ( i ) + " " ) ) )
		{
			kdDebug ( 9007 ) << "getCurrentClassname => reMethod.match()" << endl;
			return reMethod.group ( 1 );
		}
		if ( reConstructor.match ( m_pEditIface->line ( i ) ) )
		{
			kdDebug ( 9007 ) << "getCurrentClassname => reConstuctor.match()" << endl;
			return reConstructor.group ( 1 );
		}
		if ( reClass.match ( m_pEditIface->line ( i ) ) )
		{
			kdDebug ( 9007 ) << "getCurrentClassname => reClass.match()" << endl;
			return reClass.group ( 1 );
		}
	}

	return "";
}

QString CppCodeCompletionParser::getTypeOfObject ( const QString& strObject, int nLine )
{
	return "";
}
















QString CppCodeCompletionParser::getReturnTypeOfMethod ( const QString& strMethod, int nNodePos )
{
/*	KRegExp reMethod ( "[ \t]*([A-Za-z_]+)[ \t]*\\(([0-9A-Za-z_,]*)\\)" );
	QValueList<KEditor::CompletionEntry> entryList;

	QString strMethodn;

	if ( reMethod.match ( strMethod ) )
		strMethodn = reMethod.group ( 1 );

	ParsedScopeContainer scope = m_pStore->globalContainer;
	QList<ParsedMethod>* pMethodList;

	pMethodList = scope.getSortedMethodList();

	for ( ParsedMethod* pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
	{
		KEditor::CompletionEntry entry;
		entry.text = pMethod->name();
		entry.postfix = "()";
		entryList << entry;
	}

	return entryList;*/
	return "";
}



bool CppCodeCompletionParser::isMethod ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isObject ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isClass ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isStruct ( const QString& strNodeText )
{
	return false;
}

bool CppCodeCompletionParser::isNamespace ( const QString& strNodeText )
{
	return false;
}
