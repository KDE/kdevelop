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

class ISourceFormatter;
namespace KDevelop
{
	class IPlugin;
	class IDocument;
	class ContextMenuExtension;
	class ProjectBaseItem;
}
/**
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
		void beautifySource();
		void formatItem();

	protected:
		void formatDocument(KDevelop::IDocument *doc, ISourceFormatter *formatter,
							 const KMimeType::Ptr &mime);
		QString replaceSpacesWithTab(const QString &input, ISourceFormatter *formatter);
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
