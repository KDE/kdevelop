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

	KRegExp reNode ( "[ \t]*([A-Za-z_\\(\\),]*)" );

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
					if ( reNode.match ( m_strCurLine.mid ( nFrom, ( nTo - nFrom ) ) ) )
						return reNode.group ( 1 );
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
	// Lets look if its a Member of actuall class

	QString strCurrentClassname;

	strCurrentClassname = getCurrentClassname (nLine);

	if (strCurrentClassname.isEmpty())
		return "";

	if (!m_pStore->hasClass (strCurrentClassname))
		return "";

	ParsedClass *pClass = m_pStore->getClassByName (strCurrentClassname);

	QList<ParsedAttribute>* pAttributeList = pClass->getSortedAttributeList();

	for ( ParsedAttribute* pAttribute = pAttributeList->first(); pAttribute != 0; pAttribute = pAttributeList->next() )
	{
		if (pAttribute->name() == strObject)
		{
			// check if it is an Object
			QString strType;

			strType = pAttribute->type();

			if (strType.find ('*') > -1)
			{
				// Type is a pointer, but the user want to access it as an object, who's false?
				return "";
			}
			else
			{
				return strType;
			}
		}
	}

	// If we are here now seems that the object isn't a member of class, so look if its declared local

	QString strRegDef;

	//strRegDef.sprintf ("([ \t]+)([A-Za-z_]+)([ \t]+)([A-Za-z_,]*)(%s)([A-Za-z_,]*;)", strObject);
	strRegDef += "([ \t]*)([A-Za-z_]+)([ \t]+)([A-Za-z_]+)([, \t]*)(";
	strRegDef += strObject;
	strRegDef += ")([A-Za-z_, \t=]*;)";

	KRegExp regObject (strRegDef);
	QString strLine;

	// Find out on wich line the function begins and whats the name of it.

	KRegExp reMethod ( "[ \t]*([A-Za-z_]+)::([A-Za-z_]+)[ \t]*\\(([0-9A-Za-z_,]*)\\) " ); // finds "blabla::blabla(blabla) "
	KRegExp reConstructor ( "[ \t]*([A-Za-z_]+)::([A-Za-z_]+)[ \t]*\\(([0-9A-Za-z_,]*)\\)[ \t\\{:]+" );

	int iBeginOfFunction = 0;
	QString strFunctionName;

	for (int i = nLine; i >= 0; i--)
	{
		strLine = m_pEditIface->line (i);

		if ( reMethod.match ( ( m_pEditIface->line ( i ) + " " ) ) )
		{
			strFunctionName = reMethod.group ( 2 );
			iBeginOfFunction = i;
			kdDebug (9007) << "Methoden deklaration gefunden. Zeile: " << i << "Methode: " << strFunctionName;
			break;
		}
		if ( reConstructor.match ( m_pEditIface->line ( i ) ) )
		{
			strFunctionName = reConstructor.group ( 2 );
			iBeginOfFunction = i;
			kdDebug (9007) << "Konstruktor deklaration gefunden. Zeile: " << i << "Methode: " << strFunctionName;
			break;
		}
	}

	// Start searching for declaration

	for ( int i = nLine; i >= 0; i-- )
	{
		strLine = m_pEditIface->line (i);

		kdDebug (9007) << "Current Line #:" << i << "line:" << strLine;

		/*if (strLine.contains ('{') > 0)
		{
			// local objects are living only between { and }
			kdDebug (9007) << "Parsed till found {";
			break;
		}*/

		if (iBeginOfFunction <= i)
		{
			kdDebug (9007) << "Parsed till function";
			break;
		}

		if ( regObject.match (strLine) )
		{
			// check if it is an Object
			QString strType;

			strType = regObject.group (2);

			kdDebug (9007) << "local object found type:" << strType;

			return strType;
		}
	}

	// Last chance: Let's look if its a param in actual function
	if (!strFunctionName.isEmpty() && pClass != NULL)
	{
		QList<ParsedMethod>* pMethodList;

		pMethodList = pClass->getSortedMethodList();
		ParsedMethod* pMethod;

		bool bFound = false;

		for ( pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
		{
			if (pMethod->name() == strFunctionName)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			pClass->getSortedSlotList();

			for ( pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
			{
				if (pMethod->name() == strFunctionName)
				{
					bFound = true;
					break;
				}
			}
		}

		if (!bFound)
		{
			pClass->getSortedSignalList();

			for ( pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
			{
				if (pMethod->name() == strFunctionName)
				{
					bFound = true;
					break;
				}
			}
		}

		if (bFound)
		{
			for (ParsedArgument *pArgument = pMethod->arguments.first(); pArgument != 0; pMethod->arguments.next())
			{
				if (pArgument->name() == strObject)
				{
					kdDebug (9007) << "Found object in argument of function";
					return pArgument->type();
				}
			}
		}
	}

	// We haven't found anything
	return "";
}


QString CppCodeCompletionParser::getReturnTypeOfMethod ( int nLine, int nCol )
{
	KRegExp reMethod ( "[ \t]*([A-Za-z_]+)[ \t]*\\(([0-9A-Za-z_,]*)\\)" );
	QValueList<KEditor::CompletionEntry> entryList;
	QString strMethod = getNodeText ( getNodePos ( nCol ) );
	int nNodePosOfMethod = getNodePos ( nCol ) - 1;

	QString strMethodName;
	QString strNodeTextOfMethod = getNodeText ( nNodePosOfMethod );

	if ( reMethod.match ( strMethod ) )
		strMethodName = reMethod.group ( 1 );
	else	// it's not a function call
		return "";

	kdDebug ( 9007 ) << "getReturnTypeOfMethod => nNodePosOfMethod: " << nNodePosOfMethod << endl;

	if ( nNodePosOfMethod == 0 ) // might be a member function
	{
		QString strCurClass = getCurrentClassname ( nLine );

		kdDebug ( 9007 ) << "getReturnTypeOfMethod => Current class: " << strCurClass << endl;

		ParsedClass* pClass = m_pStore->getClassByName ( strCurClass );

		if ( pClass )
		{
			QList<ParsedMethod>* pMethodList = pClass->getSortedMethodList();
			for ( ParsedMethod* pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
			{
				if ( strMethodName == pMethod->name() )
				{
					return pMethod->type();
				}
			}
		}
	}
	else if ( nNodePosOfMethod > 0 )
	{
		QString strType;
		/*if ( getNodeDelimiter ( nNodePosOfMethod ) == "->" )
			strType = getTypeOfPointerOnObject ( strNodeTextOfMethod, nLine );*/
		if ( getNodeDelimiter ( nNodePosOfMethod ) == "." )
			strType = getTypeOfObject ( strNodeTextOfMethod, nLine );

		if ( !strType.isEmpty() )
		{
			ParsedClass* pClass = m_pStore->getClassByName ( strType );

			if ( pClass )
			{
				QList<ParsedMethod>* pMethodList = pClass->getSortedMethodList();
				for ( ParsedMethod* pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
				{
					if ( strMethodName == pMethod->name() )
					{
						return pMethod->type();
					}
				}
			}
		}
	}

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
