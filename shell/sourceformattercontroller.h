/* This file is part of KDevelop
Copyright 2009 Andreas Pakulat <apaku@gmx.de>
Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_SOURCEFORMATTERCONTROLLER_H
#define KDEVPLATFORM_SOURCEFORMATTERCONTROLLER_H

#include <interfaces/isourceformattercontroller.h>
#include <interfaces/isourceformatter.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <kxmlguiclient.h>
#include <KDE/KMimeType>
#include <KDE/KConfigGroup>
#include <KDE/KPluginInfo>

#include "shellexport.h"

class QAction;
namespace KDevelop
{

class Context;

class ContextMenuExtension;

class ProjectBaseItem;

class IDocument;
class ISourceFormatter;
class IPlugin;

/** \short A singleton class managing all source formatter plugins
 */
class KDEVPLATFORMSHELL_EXPORT SourceFormatterController : public ISourceFormatterController, public KXMLGUIClient
{
	Q_OBJECT
	public:
		static const QString kateModeLineConfigKey;
		static const QString kateOverrideIndentationConfigKey;
		static const QString styleCaptionKey;
		static const QString styleContentKey;
		static const QString supportedMimeTypesKey;
		
		SourceFormatterController(QObject *parent = 0);
		virtual ~SourceFormatterController();
		void initialize();
		void cleanup();
		//----------------- Public API defined in interfaces -------------------
		/** \return The formatter corresponding to the language
		* of the document corresponding to the \arg url.
		*/
		ISourceFormatter* formatterForUrl(const KUrl &url);
		/** Loads and returns a source formatter for this mime type.
		* The language is then activated and the style is loaded.
		* The source formatter is then ready to use on a file.
		*/
		ISourceFormatter* formatterForMimeType(const KMimeType::Ptr &mime);
		/** \return Whether this mime type is supported by any plugin.
		*/
		bool isMimeTypeSupported(const KMimeType::Ptr &mime);

		KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);

		virtual KDevelop::SourceFormatterStyle styleForMimeType( const KMimeType::Ptr& mime );
		
		KConfigGroup configuration();

		void settingsChanged();
		
		virtual void disableSourceFormatting(bool disable);
		virtual bool sourceFormattingEnabled();

	private Q_SLOTS:
		void activeDocumentChanged(KDevelop::IDocument *doc);
		void beautifySource();
		void beautifyLine();
		void formatFiles();
		void documentLoaded( KDevelop::IDocument* );
	private:
		/** \return A modeline string (to add at the end or the beginning of a file)
		* corresponding to the settings of the active language.
		*/
		QString addModelineForCurrentLang(QString input, const KUrl& url, const KMimeType::Ptr&);
		/** \return The name of kate indentation mode for the mime type.
		* examples are cstyle, python, etc.
		*/
		QString indentationMode(const KMimeType::Ptr &mime);
		void formatDocument(KDevelop::IDocument *doc, ISourceFormatter *formatter, const KMimeType::Ptr &mime);
		// Adapts the mode of the editor regarding indentation-style
		void adaptEditorIndentationMode(KDevelop::IDocument* doc, KDevelop::ISourceFormatter* formatter, bool ignoreModeline = false);
		void formatFiles(KUrl::List &list);
		// GUI actions
		QAction* m_formatTextAction;
		QAction* m_formatFilesAction;
		QAction* m_formatLine;
		QList<KDevelop::ProjectBaseItem*> m_prjItems;
		KUrl::List m_urls;
		bool m_enabled;
};

}

#endif // KDEVPLATFORM_SOURCEFORMATTERMANAGER_H

// kate: indent-mode cstyle; space-indent off; tab-width 4;
