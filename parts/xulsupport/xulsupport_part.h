// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
/**
 * xulsupport_part.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 */

#ifndef KDEVPART_XULSUPPORT_H
#define KDEVPART_XULSUPPORT_H


#include <qguardedptr.h>

#include <kprocess.h>
#include <kdialogbase.h>

#include <codemodel.h>
#include <kdevplugin.h>
#include <kdevlanguagesupport.h>

#include <ktexteditor/editinterface.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/codecompletioninterface.h>

class XulCodeCompletion : public QObject
{
	Q_OBJECT
public:
	XulCodeCompletion();
	~XulCodeCompletion();
	void setActiveEditorPart( KParts::Part* );
	void setVars( QStringList );
	QValueList<KTextEditor::CompletionEntry> getVars( const QString& textHint );

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

class XulSupportPart : public KDevLanguageSupport
{
	Q_OBJECT
public:
	XulSupportPart( QObject *parent, const char *name, const QStringList & );
	virtual ~XulSupportPart();

protected:
	virtual Features features();
	virtual KMimeType::List mimeTypes();

private slots:
	void slotView();
	void projectConfigWidget( KDialogBase *dlg );
	void projectOpened();
	void projectClosed();
	void savedFile( const QString& fileName );
	void addedFilesToProject( const QStringList& fileList );
	void removedFilesFromProject( const QStringList& fileList );
	void parse();
	void slotActivePartChanged( KParts::Part* part );

private:
	void initActions();
	void initConnections();
	void startApplication( const QString &program );
	void parse( const QString &fileName );
	void addAttribute( const QString &name, FileDom file, uint line );
private:
	XulCodeCompletion *m_cc;
	QStringList m_vars;
};

#endif
