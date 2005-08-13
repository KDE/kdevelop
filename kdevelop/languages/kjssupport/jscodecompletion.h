//
// C++ Interface: jscodecompletion
//
// Description:
//
//
// Author: ian reinhart geiser <geiseri@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef JSCODECOMPLETION_H
#define JSCODECOMPLETION_H

#include <qobject.h>
#include <kdevelop/codemodel.h>
#include <kdevelop/kdevplugin.h>
#include <kdevelop/kdevlanguagesupport.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>

/**
The code completion engine for Javascript.
 
@author ian reinhart geiser
*/
class JSCodeCompletion : public QObject
{
		Q_OBJECT
	public:
		JSCodeCompletion(QObject *parent = 0, const char *name = 0);

		~JSCodeCompletion();
		void setActiveEditorPart(KParts::Part* editorPart);
		QValueList<KTextEditor::CompletionEntry> getVars(const QString& textHint);

	public slots:
		void cursorPositionChanged();
		void completionBoxHidden();
		void completionBoxAbort();

	private:
		bool m_argWidgetShow;
		bool m_completionBoxShow;
		KTextEditor::EditInterface *m_editInterface;
		KTextEditor::CodeCompletionInterface *m_codeInterface;
		KTextEditor::ViewCursorInterface *m_cursorInterface;

};

#endif
