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

#include "cppcodecompletion.h"
#include "keditor/cursor_iface.h"
#include "keditor/codecompletion_iface.h"

#include <kdebug.h>
#include <kregexp.h>

CppCodeCompletion::CppCodeCompletion ( KDevCore* pCore, ClassStore* pStore )
{
	m_pEditor = pCore->editor();
	m_pStore = pStore;

	connect ( m_pEditor, SIGNAL ( documentActivated ( KEditor::Document* ) ),
		this, SLOT ( slotDocumentActivated ( KEditor::Document* ) ) );

//	m_pParser = new CppCodeCompletionParser ( m_pEditor, pStore );

	kdDebug ( 9007 ) << "constructor of CppCodeCompletion" << endl;
}

CppCodeCompletion::~CppCodeCompletion()
{
	//delete m_pParser;
}

void CppCodeCompletion::slotDocumentActivated ( KEditor::Document* pDoc )
{
	kdDebug ( 9007 ) << "CppCodeCompletion::slotDocumentActivated" << endl;

	KEditor::CursorDocumentIface* pCursorIface = KEditor::CursorDocumentIface::interface ( pDoc );
	if ( !pCursorIface )
	{
		kdDebug ( 9007 ) << "The editor doesn't support the CursorDocumentIface!" << endl;
		return;
	}

	disconnect ( pCursorIface, 0, this, 0 ); // to make sure that it isn't connected twice or more
	connect ( pCursorIface, SIGNAL ( cursorPositionChanged ( KEditor::Document*, int, int ) ),
		this, SLOT ( slotCursorPositionChanged ( KEditor::Document*, int, int ) ) );
}

void CppCodeCompletion::slotCursorPositionChanged ( KEditor::Document* pDoc, int nLine, int nCol )
{
	kdDebug ( 9007 ) << "CppCodeCompletion::cursorPositionChanged (nLine=" << nLine << ", nCol=" << nCol << ")" << endl;

	KEditor::EditDocumentIface* pEditIface = KEditor::EditDocumentIface::interface ( pDoc );
	if ( !pEditIface )
	{
		kdDebug ( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
		return;
	}

	KEditor::CodeCompletionDocumentIface* pCompletionIface = KEditor::CodeCompletionDocumentIface::interface ( pDoc );
	if ( !pCompletionIface )
	{
		kdDebug ( 9007 ) << "Editor doesn't support the CodeCompletionDocumentIface";
		return;
	}

	QString strCurLine = pEditIface->line ( nLine );

	CppCodeCompletionParser* m_pParser = new CppCodeCompletionParser ( pEditIface, m_pStore );
	m_pParser->setLineToBeParsed ( strCurLine );

	//kdDebug ( 9007 ) << "Current Classname: " << m_pParser->getCurrentClassname ( nLine ) << endl;

	if ( strCurLine.right ( 2 ) == "::" )
	{
		int nNodePos = m_pParser->getNodePos ( nCol );

		if ( nNodePos )
		{
			QString strNodeText = m_pParser->getNodeText ( nNodePos );

			QValueList<KEditor::CompletionEntry> completionList = getEntryListForNamespace ( strNodeText );
			if ( completionList.count() > 0 )
			{
				kdDebug ( 9007 ) << "Namespace: strNodeText(" << strNodeText << ") found in ClassStore" << endl;
				pCompletionIface->showCompletionBox ( completionList );
				return;
			}

			completionList = getEntryListForClass ( strNodeText );
			if ( completionList.count() > 0 )
			{
				kdDebug ( 9007 ) << "Class: strNodeText(" << strNodeText << ") found in ClassStore" << endl;
				pCompletionIface->showCompletionBox ( completionList );

//				QStringList functionList;
				return;
			}

			completionList = getEntryListForClassOfNamespace ( strNodeText, m_pParser->getNodeText ( ( m_pParser->getNodePos ( nCol ) - 1 ) ) );
			if ( completionList.count() > 0 )
			{
				kdDebug ( 9007 ) << "Class: strNodeText(" << strNodeText << ") found in (KEditor) ClassStore" << endl;
				pCompletionIface->showCompletionBox ( completionList );

//				QStringList functionList;
				return;
			}
		}
	}

//	QValueList<KEditor::CompletionEntry> completionList = m_pParser->getReturnTypeOfMethod ( "hallo()" );
//	pCompletionIface->showCompletionBox ( completionList );

	if ( strCurLine.right ( 2 ) == "->" )
	{
		int nNodePos = m_pParser->getNodePos ( nCol );

		if ( nNodePos )
		{
			QString strNodeText = m_pParser->getNodeText ( nNodePos );

			QValueList<KEditor::CompletionEntry> completionList = getEntryListForStruct ( strNodeText );
			if ( completionList.count() > 0 )
			{
				kdDebug ( 9007 ) << "Struct: strNodeText(" << strNodeText << ") found in ClassStore" << endl;
				pCompletionIface->showCompletionBox ( completionList );
				return;
			}
		}
	}


	if ( strCurLine.right ( 1 ) == "." )
	{
		KRegExp reMethod ( "[ \t]*([A-Za-z_]+)[ \t]*\\(([0-9A-Za-z_,]*)\\)" );
		int nNodePos = m_pParser->getNodePos ( nCol );
		QString strNodeText = m_pParser->getNodeText ( nNodePos );

		if ( reMethod.match ( strNodeText ) )
		{
			QString strType = m_pParser->getReturnTypeOfMethod ( nLine, nCol );

			kdDebug ( 9007 ) << "Return type of " << strNodeText << ": " << strType << endl;

			QValueList<KEditor::CompletionEntry> completionList = getEntryListForClass ( strType );
			if ( completionList.count() > 0 )
			{
				pCompletionIface->showCompletionBox ( completionList );
				return;
			}
		}
	}


/*	if ( pEditIface->line ( nLine ) == "test(")
	{
		QValueList < KEditor::CompletionEntry > entryList;

		KEditor::CompletionEntry entry;
		entry.prefix = "int";
		entry.text = "setCurrentEditor";
		entry.postfix = "( KWrite* e )";

		entryList.append ( entry );

		pCompletionIface->showCompletionBox ( entryList, 0 );

		QStringList functionList;
		QString strFunction = "int setCurrentEditor ( KWrite* e, WFlags fl )";
		functionList.append ( strFunction );
		strFunction = "int setCurrentEditor ( QMultiLineEdit* e, char* name )";
		functionList.append ( strFunction );
		strFunction = "int setCurrentEditor ( NEdit* e, const char* name )";
		functionList.append ( strFunction );

		pCompletionIface->showArgHint ( functionList, "()", "," );
	}*/

	//doCodeCompletion ( pDoc, nLine, nCol );
}

bool CppCodeCompletion::doCodeCompletion ( KEditor::Document* pDoc, int nLine, int nCol )
{
	KEditor::EditDocumentIface* pEditIface = KEditor::EditDocumentIface::interface ( pDoc );
	if ( !pEditIface )
	{
		kdDebug ( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
		return false;
	}

	KEditor::CodeCompletionDocumentIface* pCompletionIface = KEditor::CodeCompletionDocumentIface::interface ( pDoc );
	if ( !pCompletionIface )
	{
		kdDebug ( 9007 ) << "Editor doesn't support the CodeCompletionDocumentIface";
		return false;
	}


	CppCodeCompletionParser parser ( pEditIface, m_pStore );
	parser.setLineToBeParsed ( pEditIface->line ( nLine ) );

	QValueList<KEditor::CompletionEntry> completionList = getEntryListForStruct ( "Block" );

	if ( completionList.count() > 0 )
	{
		pCompletionIface->showCompletionBox ( completionList );
		return true;
	}

	//kdDebug ( 9007 ) << parser.getNodePos ( nCol ) << endl << parser.getNodeText ( parser.getNodePos ( nCol ) ) << endl << parser.getNodeDelimiter ( parser.getNodePos ( nCol ) ) << endl << parser.getCompletionText ( nCol ) << endl;



	return false;
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
	}

	return pList;
}

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

#include "cppcodecompletion.moc"
