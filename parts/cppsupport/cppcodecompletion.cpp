/***************************************************************************
                          cppcodecompletion.cpp  -  description
                             -------------------
    begin                : Sat Jul 21 2001
    copyright            : (C) 2001 by Victor Röder
    email                : victor_roeder@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define CodeCompletion 0

#include "cppcodecompletion.h"

#include <kdebug.h>
#include <kregexp.h>
#include <ktempfile.h>

#include <qstring.h>
#include <qfile.h>
#include <qstringlist.h>
#include <qdatastream.h>

CppCodeCompletion::CppCodeCompletion ( KDevCore* pCore, ClassStore* pStore )
{
	m_pEditor = pCore->editor();
	m_pStore = pStore;

	connect ( m_pEditor, SIGNAL ( documentActivated ( KEditor::Document* ) ),
		this, SLOT ( slotDocumentActivated ( KEditor::Document* ) ) );

	m_pParser = 0;
	m_pCursorIface = 0;
	m_pEditIface = 0;
	m_pCompletionIface = 0;

	m_pTmpFile = 0;

	m_bArgHintShow = false;
	m_bCompletionBoxShow = false;
}

CppCodeCompletion::~CppCodeCompletion()
{
	delete m_pParser;
}

void CppCodeCompletion::slotArgHintHided()
{
	m_bArgHintShow = false;
}

void CppCodeCompletion::slotCompletionBoxHided()
{
	kdDebug ( 9007 ) << "m_bCompletionBoxShow = false;" << endl;
	m_bCompletionBoxShow = false;
}


void CppCodeCompletion::slotDocumentActivated ( KEditor::Document* pDoc )
{

	kdDebug ( 9007 ) << "CppCodeCompletion::slotDocumentActivated" << endl;

	m_pCursorIface = KEditor::CursorDocumentIface::interface ( pDoc );
	if ( !m_pCursorIface )
	{
		kdDebug ( 9007 ) << "The editor doesn't support the CursorDocumentIface!" << endl;
		return;
	}

	m_pEditIface = KEditor::EditDocumentIface::interface ( pDoc );
	if ( !m_pEditIface )
	{
		kdDebug ( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
		return;
	}

	#if CodeCompletion

	disconnect ( m_pCursorIface, 0, this, 0 ); // to make sure that it isn't connected twice
	connect ( m_pCursorIface, SIGNAL ( cursorPositionChanged ( KEditor::Document*, int, int ) ),
		this, SLOT ( slotCursorPositionChanged ( KEditor::Document*, int, int ) ) );

	disconnect ( m_pEditIface, 0, this, 0 ); // to make sure that it isn't connected twice
	connect ( m_pEditIface, SIGNAL ( textChanged ( KEditor::Document*, int, int ) ),
		this, SLOT ( slotTextChanged ( KEditor::Document*, int, int ) ) );

	connect ( m_pCompletionIface, SIGNAL ( argHintHided() ), this, SLOT ( slotArgHintHided() ) );
	connect ( m_pCompletionIface, SIGNAL ( completionAborted() ), this, SLOT ( slotCompletionBoxHided() ) );
	connect ( m_pCompletionIface, SIGNAL ( completionDone() ), this, SLOT ( slotCompletionBoxHided() ) );

	#endif
}


void CppCodeCompletion::slotTextChanged( KEditor::Document *pDoc, int nLine, int nCol )
{
	/* the code from slotCursorPositionChanged has to be placed here */

	m_pEditIface = KEditor::EditDocumentIface::interface ( pDoc );
	if ( !m_pEditIface )
	{
		kdDebug ( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
		return;
	}

	m_pCompletionIface = KEditor::CodeCompletionDocumentIface::interface ( pDoc );
	if ( !m_pCompletionIface )
	{
		kdDebug ( 9007 ) << "Editor doesn't support the CodeCompletionDocumentIface";
		return;
	}

	QString strCurLine = m_pEditIface->line ( nLine );

	if ( !m_pParser ) m_pParser = new CppCCParser ();

	//kdDebug ( 9007 ) << "CppCodeCompletion::slotTextChanged()@" << nLine << ":" << nCol << endl;

	// CC for "Namespace::" for example
	if ( strCurLine.right ( 2 ) == "::" )
	{
		int nNodePos = getNodePos ( nLine, nCol );

		if ( nNodePos )
		{
			QString strNodeText = getNodeText ( nNodePos, nLine );

			QValueList<KEditor::CompletionEntry> completionList = getEntryListForNamespace ( strNodeText );
			if ( completionList.count() > 0 )
			{
				kdDebug ( 9007 ) << "Namespace: strNodeText(" << strNodeText << ") found in ClassStore" << endl;
				m_pCompletionIface->showCompletionBox ( completionList );
				return;
			}

			completionList = getEntryListForClass ( strNodeText );
			if ( completionList.count() > 0 )
			{
				kdDebug ( 9007 ) << "Class: strNodeText(" << strNodeText << ") found in ClassStore" << endl;
				m_pCompletionIface->showCompletionBox ( completionList );

//				QStringList functionList;
				return;
			}

			completionList = getEntryListForClassOfNamespace ( strNodeText, getNodeText ( ( getNodePos ( nLine, nCol ) - 1 ), nLine ) );
			if ( completionList.count() > 0 )
			{
				kdDebug ( 9007 ) << "Class: strNodeText(" << strNodeText << ") found in (KEditor) ClassStore" << endl;
				m_pCompletionIface->showCompletionBox ( completionList );

//				QStringList functionList;
				return;
			}
		}
	}

	if ( m_pEditIface->line ( nLine ) == "test(")
	{
		QValueList < KEditor::CompletionEntry > entryList;

		KEditor::CompletionEntry entry;
		entry.prefix = "int";
		entry.text = "setCurrentEditor";
		entry.postfix = "( KWrite* e )";

		entryList.append ( entry );

		//m_pCompletionIface->showCompletionBox ( entryList, 0 );

		QStringList functionList;
		QString strFunction = "int setCurrentEditor ( KWrite* e, WFlags fl )";
		functionList.append ( strFunction );
		strFunction = "int setCurrentEditor ( QMultiLineEdit* e, char* name )";
		functionList.append ( strFunction );
		strFunction = "int setCurrentEditor ( NEdit* e, const char* name )";
		functionList.append ( strFunction );

		m_pCompletionIface->showArgHint ( functionList, "()", "," );
	}


	if ( strCurLine.right ( 1 ) == "." )
	{
		int nNodePos = getNodePos ( nLine, nCol );

		if ( nNodePos )
		{
			QString strNodeText = getNodeText ( nNodePos, nLine );

			if ( m_pTmpFile ) delete m_pTmpFile;
			m_pTmpFile = NULL;
			if ( !m_pTmpFile ) m_pTmpFile = new KTempFile();
			if ( m_pTmpFile->status() != 0 ) return;

			QString strFileName = createTmpFileForParser ( nLine );

			if ( m_pParser ) m_pParser->parse ( strFileName );

			m_pTmpFile->unlink();
			if ( m_pTmpFile->status() != 0 ) return;

			for ( CParsedVariable* pVar = m_pParser->varList.first(); pVar != 0; pVar = m_pParser->varList.next() )
			{
				if ( pVar->sVariableName == strNodeText )
				{
					kdDebug ( 9007 ) << "Type of variable: " << pVar->sVariableType << endl;
					QValueList<KEditor::CompletionEntry> completionList = getEntryListForClass ( pVar->sVariableType );

					if ( completionList.count() > 0 )
					{
						m_pCompletionIface->showCompletionBox ( completionList );
						break;
					}

					break;
				}

			}
			// remove the parsed variables of the CppCCParser!
			m_pParser->varList.clear();
		}
	}
}

/**** TODO: replace this method with a parsing mechanism - very buggy! ****/

QString CppCodeCompletion::createTmpFileForParser (int iLine)
{
	KRegExp regMethod ("[ \t]*[A-Za-z_]+::[~A-Za-z_]+[\t]*\\([0-9A-Za-z_,\\)\\*]*");

	int iMethodBegin = 0;

	QString strLine;

	for (int i = iLine; i > 0; i--)
	{
		strLine = m_pEditIface->line (i);

		if (regMethod.match (strLine.latin1()) )
		{
			iMethodBegin = i;

			kdDebug (9007) << "iMethodBegin: " << iMethodBegin << endl;
			break;
		}
	}

	if (iMethodBegin == 0)
	{
		kdDebug (9007) << "no method declaration found" << endl;

		return QString::null;
	}

	QString strCopy;

	for (int i = iMethodBegin; i < iLine; i++)
	{
		strCopy += m_pEditIface->line (i) + "\n";
	}

	kdDebug (9007) << "strCopy: " << strCopy << endl;


	QFile *pFile = m_pTmpFile->file();

	//QDataStream DataStream( pFile );
//	QDataStream *pDataStream = tmpFile.dataStream();
	//DataStream << strCopy;

	pFile->writeBlock (strCopy.latin1(), strCopy.length());

	pFile->flush();

	kdDebug (9007) << "Name der Temp-Datei: " << m_pTmpFile->name() << endl;

	return m_pTmpFile->name();
}


/**** Here begins some "parsing" stuff - to be replaced by a real parser ****/
QString CppCodeCompletion::getCompletionText ( int nLine, int nCol ) // seems to be okay
{
	int nOffset = nCol;

	QString strCurLine = m_pEditIface->line ( nLine );

	while ( nOffset > 0 )
	{
		if ( strCurLine[nOffset] == '.' ||
			strCurLine[nOffset] == '>' && strCurLine[nOffset - 1] == '-' ||
			strCurLine[nOffset] == ':' && strCurLine[nOffset - 1] == ':' ||
			strCurLine[nOffset] == ' ' || strCurLine[nOffset] == ';' ||
			strCurLine[nOffset] == '\t' || strCurLine[nOffset] == '}')
		{
			nOffset++;
			break;
		}
		else
		{
			//kdDebug ( 9007 ) << "Offset--;" << endl;
			nOffset--;
		}

		if ( strCurLine[nOffset] == ':' && strCurLine[nOffset - 1] != ':' ||
			strCurLine[nOffset] == '-' && strCurLine[nOffset - 1] != '>' )
		{
			return QString::null;
		}
	}

	if ( ( nCol - nOffset ) >= 0 )
		return ( strCurLine.mid ( nOffset, ( nCol - nOffset ) ) );

	//kdDebug ( 9007 ) << "nCol - nOffset: " << (nCol - nOffset) << endl;

	return QString::null;
}

int CppCodeCompletion::getNodePos ( int nLine, int nCol ) // seems to be okay
{
	int nOffset = 0;
	int nNodePos = 0;

	QString strCurLine = m_pEditIface->line ( nLine );

	while ( nOffset < nCol )
	{
		if ( strCurLine[nOffset] == '.' ||
			strCurLine[nOffset] == '-' && strCurLine[nOffset + 1] == '>' ||
			strCurLine[nOffset] == ':' && strCurLine[nOffset + 1] == ':' )
		{
			nNodePos++;
		}

		nOffset++;
	}

	return nNodePos;
}

QString CppCodeCompletion::getNodeText ( int nNode, int nLine ) // seems to be okay
{
	if ( nNode <= 0 )
		return QString::null;

	int nFrom = 0;
	int nTo = 0;
	int nNodePos = 0;

	QString strCurLine = m_pEditIface->line ( nLine );


	while ( nTo < strCurLine.length() )
	{
		if ( strCurLine[nTo] == '.' )
		{
			nNodePos++;

			if ( nNodePos < nNode )
				nFrom = nTo + 1;
		}

		if ( strCurLine[nTo] == '-' && strCurLine[nTo + 1] == '>' ||
			strCurLine[nTo] == ':' && strCurLine[nTo + 1] == ':' )
		{
			nNodePos++;

			if ( nNodePos < nNode )
				nFrom = nTo + 2;
		}

		//kdDebug ( 9007 ) << "nNodePos: " << nNodePos << " nNode: " << nNode << endl;

		if ( nNodePos == nNode )
		{
			for ( nTo = nFrom; nTo < strCurLine.length(); nTo++ )
			{
				if ( strCurLine[nTo] == '.' )
				{
					//kdDebug ( 9007 ) << "CppCodeCompletionParser::getNodeText1 => nFrom: " << nFrom << " nTo: " << nTo << endl;

					if ( nFrom == 0 )
					{
						for ( nFrom = nTo -1; nFrom > 0; --nFrom )
						{
							if ( strCurLine[nFrom] == ' ' || strCurLine[nFrom] == '\t' ||
								strCurLine[nFrom] == '}' ||  strCurLine[nFrom] == ';' ||
								strCurLine[nFrom] == ':' && strCurLine[nFrom - 1] == ':' ||
								strCurLine[nFrom] == '>' && strCurLine[nFrom - 1] == '-' ||
								strCurLine[nFrom] == '.')
							{
								nFrom++;
								break;
							}
						}
					}

					//kdDebug ( 9007 ) << "CppCodeCompletionParser::getNodeText2 => nFrom: " << nFrom << " nTo: " << nTo << endl;
					return strCurLine.mid ( nFrom, ( nTo - nFrom ) );
				}

				if ( strCurLine[nTo] == '-' && strCurLine[nTo + 1] == '>' ||
					strCurLine[nTo] == ':' && strCurLine[nTo + 1] == ':' )
				{
					if ( nFrom == 0 )
					{
						for ( nFrom = nTo; nFrom > 0; --nFrom )
						{
							if ( strCurLine[nFrom] == ' ' || strCurLine[nFrom] == '\t' ||
								strCurLine[nFrom] == '}' ||  strCurLine[nFrom] == ';' ||
								strCurLine[nFrom] == ':' && strCurLine[nFrom - 1] == ':' ||
								strCurLine[nFrom] == '>' && strCurLine[nFrom - 1] == '-' ||
								strCurLine[nFrom] == '.' )
							{
								nFrom++;
								break;
							}
						}
					}

					//kdDebug ( 9007 ) << "CppCodeCompletionParser::getNodeText => strNodeText: " << strCurLine.mid ( nFrom, ( nTo - nFrom ) ) << endl;
					return strCurLine.mid ( nFrom, ( nTo - nFrom ) );

				}
			}
		}

		nTo++;

		//kdDebug ( 9007 ) << "getNodeText::while" << endl;
	}

	return QString::null;
}

QString CppCodeCompletion::getNodeDelimiter ( int nNode, int nLine )
{
	if ( nNode <= 0 )
		return QString::null;

	QString strCurLine = m_pEditIface->line ( nLine );

	int nFrom = 0;
	int nTo = 0;
	int nNodePos = 0;

	while ( nTo < strCurLine.length() )
	{
		if ( strCurLine[nTo] == '.' ||
			strCurLine[nTo] == '-' && strCurLine[nTo + 1] == '>' ||
			strCurLine[nTo] == ':' && strCurLine[nTo + 1] == ':' )
		{
			nNodePos++;

			if ( nNodePos < nNode )
				nFrom = nTo + 1;
		}

		//kdDebug ( 9007 ) << "nNodePos: " << nNodePos << " nNode: " << nNode << endl;

		if ( nNodePos == nNode )
		{
			for ( nTo = nFrom; nTo < strCurLine.length(); nTo++ )
			{
				if ( strCurLine[nTo] == '.' )
				{
					return strCurLine.mid ( nTo, 1 );
				}

				if ( strCurLine[nTo] == '-' && strCurLine[nTo + 1] == '>' ||
					strCurLine[nTo] == ':' && strCurLine[nTo + 1] == ':' )
				{
					return strCurLine.mid ( nTo, 2 );
				}
			}
		}

		nTo++;

		//kdDebug ( 9007 ) << "getNodeText::while" << endl;
	}

	return QString::null;
}

/**** Here begin some ClassStore queries - I think they are nearly stable (expected that nothing else is mentioned) ****/

QValueList<KEditor::CompletionEntry> CppCodeCompletion::getEntryListForClass ( QString strClass )
{
	QValueList<KEditor::CompletionEntry> entryList;

	ParsedClass* pClass = m_pStore->getClassByName ( strClass );
	if ( pClass )
	{
		QList<ParsedMethod>* pMethodList;
		QList<ParsedAttribute>* pAttributeList;

		// Load the methods, slots, signals of the current class and its parents into the list
		pMethodList = pClass->getSortedMethodList();

		QList<ParsedMethod>* pTmpList = pClass->getSortedSlotList();
		for ( ParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
		{
			pMethodList->append ( pMethod );
		}

		pTmpList = pClass->getSortedSignalList();
		for ( ParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
		{
			pMethodList->append ( pMethod );
		}

		pMethodList = getParentMethodListForClass ( pClass, pMethodList );

		for ( ParsedMethod* pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
		{
			KEditor::CompletionEntry entry;
			entry.text = pMethod->name();
			entry.postfix = "()";
			entryList << entry;
		}

		// Load the attributes of the current class and its parents into the list
		pAttributeList = pClass->getSortedAttributeList();

		pAttributeList = getParentAttributeListForClass ( pClass, pAttributeList );

		for ( ParsedAttribute* pAttribute = pAttributeList->first(); pAttribute != 0; pAttribute = pAttributeList->next() )
		{
			KEditor::CompletionEntry entry;
			entry.text = pAttribute->name();
			entry.postfix = "";
			entryList << entry;
		}
	}

	return entryList;
}

QValueList<KEditor::CompletionEntry> CppCodeCompletion::getEntryListForClassOfNamespace ( QString strClass, const QString& strNamespace )
{
	QValueList<KEditor::CompletionEntry> entryList;
	ParsedScopeContainer *pScope = m_pStore->getScopeByName ( strNamespace );

	if ( pScope )
	{
		QList<ParsedMethod>* pMethodList;
		QList<ParsedAttribute>* pAttributeList;

		ParsedClass* pClass = pScope->getClassByName ( strClass );
		if ( pClass )
		{
			// Load the methods, slots, signals of the current class and its parents into the list
			pMethodList = pClass->getSortedMethodList();

			QList<ParsedMethod>* pTmpList = pClass->getSortedSlotList();
			for ( ParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
			{
				pMethodList->append ( pMethod );
			}

			pTmpList = pClass->getSortedSignalList();
			for ( ParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
			{
				pMethodList->append ( pMethod );
			}

			pMethodList = getParentMethodListForClass ( pClass, pMethodList );

			for ( ParsedMethod* pMethod = pMethodList->first(); pMethod != 0; pMethod = pMethodList->next() )
			{
				KEditor::CompletionEntry entry;
				entry.text = pMethod->name();
				entry.postfix = "()";
				entryList << entry;
			}

			// Load the attributes of the current class and its parents into the list
			pAttributeList = pClass->getSortedAttributeList();

			pAttributeList = getParentAttributeListForClass ( pClass, pAttributeList );

			for ( ParsedAttribute* pAttribute = pAttributeList->first(); pAttribute != 0; pAttribute = pAttributeList->next() )
			{
				KEditor::CompletionEntry entry;
				entry.text = pAttribute->name();
				entry.postfix = "";
				entryList << entry;
			}
		}
	}

	return entryList;
}

QValueList<KEditor::CompletionEntry> CppCodeCompletion::getEntryListForNamespace ( const QString& strNamespace )
{
	QValueList<KEditor::CompletionEntry> entryList;
	ParsedScopeContainer *pScope = m_pStore->getScopeByName ( strNamespace );

	if ( pScope )
	{
		QList<ParsedClass>* pClassList = pScope->getSortedClassList();

		for ( ParsedClass *pClass = pClassList->first(); pClass != 0; pClass = pClassList->next() )
		{
			KEditor::CompletionEntry entry;
			entry.text = pClass->name();
			entry.postfix = "";
			entryList << entry;
		}
	}

	return entryList;
}

QValueList<KEditor::CompletionEntry> CppCodeCompletion::getEntryListForStruct ( const QString& strStruct )
{
	QValueList<KEditor::CompletionEntry> entryList;
	ParsedScopeContainer *pScope = &m_pStore->globalContainer;

	if ( pScope )
	{
		ParsedStruct* pStruct = pScope->getStructByName ( strStruct );

		if ( pStruct )
		{
			QList<ParsedAttribute>* pAttributeList = pStruct->getSortedAttributeList();

			for ( ParsedAttribute *pAttribute = pAttributeList->first(); pAttribute != 0; pAttribute = pAttributeList->next() )
			{
				KEditor::CompletionEntry entry;
				entry.text = pAttribute->name();
				entry.postfix = "";
				entryList << entry;
			}
		}
	}

	return entryList;
}

QList<ParsedMethod>* CppCodeCompletion::getParentMethodListForClass ( ParsedClass* pClass, QList<ParsedMethod>* pList )
{
	QList<ParsedParent> parentList = pClass->parents;

	for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
	{
		pClass = m_pStore->getClassByName ( pParentClass->name() );

		if ( pClass )
		{
			QList<ParsedMethod>* pTmpList = pClass->getSortedMethodList();
			for ( ParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
			{
				pList->append ( pMethod );
			}

			pTmpList = pClass->getSortedSlotList();
			for ( ParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
			{
				pList->append ( pMethod );
			}

			pTmpList = pClass->getSortedSignalList();
			for ( ParsedMethod* pMethod = pTmpList->first(); pMethod != 0; pMethod = pTmpList->next() )
			{
				pList->append ( pMethod );
			}

			pList = getParentMethodListForClass ( pClass, pList );
		}
		/*else
		{
			// TODO: look in ClassStore for Namespace classes
		} */
	}

	return pList;
}

QList<ParsedAttribute>* CppCodeCompletion::getParentAttributeListForClass ( ParsedClass* pClass, QList<ParsedAttribute>* pList )
{
	QList<ParsedParent> parentList = pClass->parents;

	for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
	{
		pClass = m_pStore->getClassByName ( pParentClass->name() );

		if ( pClass )
		{
			QList<ParsedAttribute>* pTmpList = pClass->getSortedAttributeList();
			for ( ParsedAttribute* pAttribute = pTmpList->first(); pAttribute != 0; pAttribute = pTmpList->next() )
			{
				pList->append ( pAttribute );
			}

			pList = getParentAttributeListForClass ( pClass, pList );
		}
		/*else
		{
			// TODO: look in ClassStore for Namespace classes
		} */
	}

	return pList;
}

#include "cppcodecompletion.moc"
