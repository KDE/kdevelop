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

CppCodeCompletion::CppCodeCompletion ( KDevCore* pCore, ClassStore* pStore )
{
	m_pEditor = pCore->editor();
	m_pStore = pStore;

	connect ( m_pEditor, SIGNAL ( documentActivated ( KEditor::Document* ) ),
		this, SLOT ( slotDocumentActivated ( KEditor::Document* ) ) );

	kdDebug ( 9007 ) << "constructor of CppCodeCompletion" << endl;
}

CppCodeCompletion::~CppCodeCompletion()
{
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

	//kdDebug ( 9007 ) << "!!!!!!!!!!!Before!!!!!!!!!!!!!!!!" << endl;

	CppCodeCompletionParser parser ( pEditIface, m_pStore );
	parser.setLine ( pEditIface->line ( nLine ) );
	kdDebug ( 9007 ) << parser.getNodePos ( nCol ) << endl << parser.getNodeText ( parser.getNodePos ( nCol ) ) << endl << parser.getNodeDelimiter ( parser.getNodePos ( nCol ) ) << endl << parser.getCompletionText ( nCol ) << endl;


	//kdDebug ( 9007 ) << "!!!!!!!!!!!Afterwards!!!!!!!!!!!!!!!!" << endl;

	if ( pEditIface->line ( nLine ) == "test(")
	{
		/*QValueList < KEditor::CompletionEntry > entryList;

		KEditor::CompletionEntry entry;
		entry.prefix = "int";
		entry.text = "setCurrentEditor";
		entry.postfix = "( KWrite* e )";

		entryList.append ( entry );

		pCompletionIface->showCompletionBox ( entryList, 0 );*/

		QStringList functionList;
		QString strFunction = "int setCurrentEditor ( KWrite* e, WFlags fl )";
		functionList.append ( strFunction );
/*		strFunction = "int setCurrentEditor ( QMultiLineEdit* e, char* name )";
		functionList.append ( strFunction );
		strFunction = "int setCurrentEditor ( NEdit* e, const char* name )";
		functionList.append ( strFunction );*/

		pCompletionIface->showArgHint ( functionList, "()", "," );
	}
}

#include "cppcodecompletion.moc"
