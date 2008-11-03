/* This file is part of KDevelop
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

#ifndef SOURCEFORMATTERPLUGIN_H
#define SOURCEFORMATTERPLUGIN_H

#include <QHash>
#include <QList>
#include <KMimeType>
#include <KUrl>

class KAction;
namespace KParts
{
	class Part;
}
namespace KTextEditor
{
	class Document;
}

#include <interfaces/iplugin.h>

namespace KDevelop
{
	class IPlugin;
	class IDocument;
	class ContextMenuExtension;
	class ProjectBaseItem;
        class ISourceFormatter;
}

/** \short A plugin to format and beautify source files
 * It can support any backend using plugins implementing
 * the \ref ISourceFormatter interface.
 */
class SourceFormatterPlugin : public KDevelop::IPlugin
{
		Q_OBJECT

	public:
		SourceFormatterPlugin(QObject *parent, const QVariantList &args = QVariantList());
		virtual ~SourceFormatterPlugin();

		KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);

		void formatFiles(KUrl::List &list);

	public slots:
		/** Formats the file in the currently active document, or only
		* the selected part if a selection exists. In this case, the indentation of
		* the selection is kept.
		*/
		void beautifySource();
		/** Formats the items in m_prjItems or m_urls.
		*/
		void formatItem();

	protected:
		/** Formats the document corresponding to \arg doc , using \arg formatter.
		* \arg mime is the mime type of the file. The content of the editor is
		* modified but the file is not saved.
		*/
		void formatDocument(KDevelop::IDocument *doc, KDevelop::ISourceFormatter *formatter,
							 const KMimeType::Ptr &mime);
		/** Replaces spaces with tabs using the number of spaces per tabs defined
		* in the \arg formatter or convert tabs to spaces, depending on the config.
		* \return The modified string
		*/
		QString replaceSpacesWithTab(const QString &input, KDevelop::ISourceFormatter *formatter);
		/** Adds the string \arg indentWith at the beginning of each line in \arg input.
		* \return The modified string
		*/
		QString addIndentation(QString input, const QString indentWith);

	protected slots:
		void activePartChanged(KParts::Part *part);

	private:
		KAction *m_formatTextAction;
		KAction *m_formatFilesAction;

		KUrl::List m_urls;
		QList<KDevelop::ProjectBaseItem*> m_prjItems;
};

#endif // SOURCEFORMATTERPLUGIN_H

// kate: indent-mode cstyle; space-indent off; tab-width 4; 
