/***************************************************************************
                          cppcodecompletion.h  -  description
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

#ifndef __CPPCODECOMPLETION_H__
#define __CPPCODECOMPLETION_H__

#include <qobject.h>

#include "keditor/editor.h"
#include "keditor/edit_iface.h"
#include "keditor/cursor_iface.h"
#include "keditor/codecompletion_iface.h"

#include "classstore.h"
/*#include "parsedclass.h"
#include "parsedmethod.h"*/

#include "kdevcore.h"

#include "cppcodecompletionparser.h"

class KTempFile;

class CppCodeCompletion : public QObject
{
	Q_OBJECT

	public:
		CppCodeCompletion ( KDevCore* pCore, ClassStore* pStore );
		virtual ~CppCodeCompletion();

	protected slots:
		void slotDocumentActivated ( KEditor::Document* pDoc );
		void slotCursorPositionChanged ( KEditor::Document* pDoc, int nLine, int nCol );
		void slotArgHintHided();
		void slotCompletionBoxHided();
		void slotTextChanged( KEditor::Document *pDoc, int nLine, int nCol );

	protected:
		QString getCompletionText ( int nLine, int nCol );
		QString getNodeText ( int nNode, int nLine );
		QString getNodeDelimiter ( int nNode, int nLine );
		int getNodePos ( int nLine, int nCol );

		QString createTmpFileForParser (int iLine);

		//bool checkIfArgHintIsNeeded ( int nLine, int nCol );
		//QString getClassName ( const QString& strName );

		//bool doCodeCompletion ( int nLine, int nCol );

		QValueList<KEditor::CompletionEntry> getEntryListForClass ( QString strClass );
		QValueList<KEditor::CompletionEntry> getEntryListForNamespace ( const QString& strNamespace );
		QValueList<KEditor::CompletionEntry> getEntryListForStruct ( const QString& strStruct );
		QValueList<KEditor::CompletionEntry> getEntryListForClassOfNamespace ( QString strClass, const QString& strNamespace );

		/* methods which are called recursively by getEntryListForClass(...) */
		QList<ParsedMethod>* getParentMethodListForClass ( ParsedClass* pClass, QList<ParsedMethod>* pList );
		QList<ParsedAttribute>* getParentAttributeListForClass ( ParsedClass* pClass, QList<ParsedAttribute>* pList );
	private:
		KEditor::Editor* m_pEditor;
		ClassStore* m_pStore;
		CppCCParser* m_pParser;
		KEditor::CursorDocumentIface* m_pCursorIface;
		KEditor::EditDocumentIface* m_pEditIface;
		KEditor::CodeCompletionDocumentIface* m_pCompletionIface;

		KTempFile* m_pTmpFile;

		bool m_bArgHintShow;
		bool m_bCompletionBoxShow;
};

#endif
