/*
 *  Copyright (C) 2003 Ian Reinhart Geiser <geiseri@kde.org>
 */


#ifndef __KDEVPART_BASHSUPPORT_H__
#define __KDEVPART_BASHSUPPORT_H__


#include <qguardedptr.h>

#include <kprocess.h>
#include <kdialogbase.h>

#include <codemodel.h>
#include <kdevplugin.h>
#include <kdevlanguagesupport.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>

//class BashSupportWidget;

class BashCodeCompletion : QObject
{
	Q_OBJECT
	public:
		BashCodeCompletion();
		~BashCodeCompletion();
		void setActiveEditorPart(KParts::Part*);
		void setVars(QStringList);
		QValueList<KTextEditor::CompletionEntry> getVars(const QString& textHint);

	public slots:
		void cursorPositionChanged();
		void completionBoxHidden();
		void completionBoxAbort();

	private:
		QStringList m_vars;
		bool m_argWidgetShow;
		bool m_completionBoxShow;
		KTextEditor::EditInterface *m_editInterface;
		KTextEditor::CodeCompletionInterface *m_codeInterface;
		KTextEditor::ViewCursorInterface *m_cursorInterface;
};

class BashSupportPart : public KDevLanguageSupport
{
	Q_OBJECT
	public:
		BashSupportPart(QObject *parent, const char *name, const QStringList &);
		virtual ~BashSupportPart();

	protected:
		virtual Features features();
		virtual KMimeType::List mimeTypes();

	private slots:
		void slotRun();
		void projectConfigWidget(KDialogBase *dlg);
		void projectOpened();
		void projectClosed();
		void savedFile(const KURL &fileName);
		void addedFilesToProject(const QStringList &fileList);
		void removedFilesFromProject(const QStringList &fileList);
		void parse();
		void slotActivePartChanged(KParts::Part *part);

	private:
		void startApplication(const QString &program);
		QString interpreter();
		void parse(const QString &fileName);
		void addAttribute(const QString &name, FileDom file, uint line);
		BashCodeCompletion *m_cc;
		QStringList m_vars;

};

#endif
