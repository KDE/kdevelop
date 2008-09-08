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

#include "sourceformatterplugin.h"

#include <QTextStream>
#include <QMenu>
#include <QLineEdit>
#include <QRegExp>
#include <QStringList>
#include <QFile>
#include <QVariant>

#include <KDebug>
#include <KDialog>
#include <KIconLoader>
#include <KLocale>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <KApplication>
#include <KConfig>
#include <KFileDialog>
#include <KMessageBox>
#include <KGlobal>
#include <KAction>
#include <KActionCollection>
#include <kio/netaccess.h>
#include <KMimeType>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <project/projectmodel.h>
#include <util/interfaces/isourceformatter.h>
#include <util/sourceformattermanager.h>

K_PLUGIN_FACTORY(SourceFormatterFactory, registerPlugin<SourceFormatterPlugin>();)
K_EXPORT_PLUGIN(SourceFormatterFactory("kdevsourceformatter"))

SourceFormatterPlugin::SourceFormatterPlugin(QObject *parent, const QVariantList &)
		: KDevelop::IPlugin(SourceFormatterFactory::componentData(), parent)
{
	setXMLFile("kdevsourceformatter.rc");

	m_formatTextAction = actionCollection()->addAction("edit_astyle_reformat");
	m_formatTextAction->setText(i18n("&Reformat Source"));
	m_formatTextAction->setToolTip(i18n("Reformat source using AStyle"));
	m_formatTextAction->setWhatsThis(i18n("<b>Reformat source</b><p>Source reformatting "
	                                      "functionality using <b>astyle</b> library. "));
	connect(m_formatTextAction, SIGNAL(triggered()), this, SLOT(beautifySource()));

	m_formatFilesAction = actionCollection()->addAction("tools_astyle");
	m_formatFilesAction->setText(i18n("Format files"));
	m_formatFilesAction->setToolTip(i18n("Format file(s) using the current theme"));
	m_formatFilesAction->setWhatsThis(i18n("<b>Format files</b><p>Formatting functionality using <b>astyle</b> library. "));
	connect(m_formatFilesAction, SIGNAL(triggered()), this, SLOT(formatItem()));

	m_formatTextAction->setEnabled(false);
	m_formatFilesAction->setEnabled(true);

	connect(core()->partManager(), SIGNAL(activePartChanged(KParts::Part*)),
	        this, SLOT(activePartChanged(KParts::Part*)));
}

SourceFormatterPlugin::~SourceFormatterPlugin()
{
	SourceFormatterManager::self()->deleteLater();
}

void SourceFormatterPlugin::beautifySource()
{
	KTextEditor::Document *doc =
	  core()->documentController()->activeDocument()->textDocument();
	if (!doc)
		return;
	// load the appropriate formatter
	KMimeType::Ptr mime = KMimeType::findByUrl(doc->url());
    SourceFormatterManager *manager = SourceFormatterManager::self();
    manager->loadConfig();
	ISourceFormatter *formatter = manager->formatterForMimeType(mime);

	bool has_selection = false;
	KTextEditor::View *view = doc->views().first();
	if (view && view->selection())
		has_selection = true;
//
// 	// put the selection back to the same indent level.
// 	// taking note of the config options.
// 	if (has_selection) {
// 		unsigned int indentCount = 0;
// 		QString indentWith("");
// 		QString original = view->selectionText();
//
// 		for (; indentCount < original.length(); indentCount++) {
// 			QChar ch = original[indentCount];
// 			if (!ch.isSpace())
// 				break;
//
// 			if (ch == QChar('\n') || ch == QChar('\r'))
// 				indentWith = "";
// 			else
// 				indentWith += original[indentCount];
// 		}
// 		replaceSpacesWithTab(indentWith);
//
// 		QString output = formatter->formatSource(view->selectionText(), mime);
//
// 		//remove the final newline character, unless it should be there
// 		if (!original.endsWith('\n'))
// 			output.resize(output.length() - 1);
// 		//there was a selection, so only change the part of the text related to it
// 		view->removeSelectionText();
// 		doc->insertText(view->cursorPosition(), output);
// 	} else {
	KTextEditor::Cursor cursor = view->cursorPosition();
	doc->setText(formatter->formatSource(doc->text(), mime));
	view->setCursorPosition(cursor);
// 	}
}

void SourceFormatterPlugin::replaceSpacesWithTab(QString &input)
{
// 	int wsCount = m_formatter->option("FillCount").toInt();
// 	if (m_formatter->option("Fill").toString() == "Tabs") {
// 		// tabs and wsCount spaces to be a tab
// 		QString replace;
// 		for (int i = 0; i < wsCount;i++)
// 			replace += ' ';
//
// 		input = input.replace(replace, QChar('\t'));
// 		input = input.remove(' ');
// 	} else {
// 		if (m_formatter->option("FillForce").toBool()) {
// 			//convert tabs to spaces
// 			QString replace;
// 			for (int i = 0;i < wsCount;i++)
// 				replace += ' ';
//
// 			input = input.replace(QChar('\t'), replace);
// 		}
// 	}
}

void SourceFormatterPlugin::activePartChanged(KParts::Part *part)
{
	bool enabled = false;
	KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(part);
	if (rw_part) {
		KTextEditor::Document *doc = dynamic_cast<KTextEditor::Document*>(rw_part);
		if (doc) {
            KMimeType::Ptr mime = KMimeType::findByUrl(doc->url());
			if (SourceFormatterManager::self()->isMimeTypeSupported(mime))
				enabled = true;
		}
	}

	m_formatTextAction->setEnabled(enabled);
}

KDevelop::ContextMenuExtension
SourceFormatterPlugin::contextMenuExtension(KDevelop::Context* context)
{
	KDevelop::ContextMenuExtension ext;
	m_urls.clear();
	m_prjItems.clear();

	if (context->hasType(KDevelop::Context::EditorContext))
		ext.addAction(KDevelop::ContextMenuExtension::EditGroup, m_formatTextAction);
	else if (context->hasType(KDevelop::Context::FileContext)) {
		KDevelop::FileContext* filectx = dynamic_cast<KDevelop::FileContext*>(context);
		m_urls = filectx->urls();
		ext.addAction(KDevelop::ContextMenuExtension::EditGroup, m_formatFilesAction);
	} else if (context->hasType(KDevelop::Context::CodeContext)) {
	} else if (context->hasType(KDevelop::Context::ProjectItemContext)) {
		KDevelop::ProjectItemContext* prjctx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
		m_prjItems = prjctx->items();
		ext.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_formatFilesAction);
	}
	return ext;
}

void SourceFormatterPlugin::formatItem()
{
	if (m_prjItems.isEmpty())
		return;

	//get a list of all files in this folder recursively
	QList<KDevelop::ProjectFolderItem*> folders;
	foreach(KDevelop::ProjectBaseItem *item, m_prjItems) {
		if (!item)
			continue;
		if (item->folder())
			folders.append(item->folder());
		else if (item->file())
			m_urls.append(item->file()->url());
		else if (item->target()) {
			foreach(KDevelop::ProjectFileItem *f, item->fileList())
			m_urls.append(f->url());
		}
	}

	while (!folders.isEmpty()) {
		KDevelop::ProjectFolderItem *item = folders.takeFirst();
		foreach(KDevelop::ProjectFolderItem *f, item->folderList())
		folders.append(f);
		foreach(KDevelop::ProjectTargetItem *f, item->targetList()) {
			foreach(KDevelop::ProjectFileItem *child, f->fileList())
			m_urls.append(child->url());
		}
		foreach(KDevelop::ProjectFileItem *f, item->fileList())
		m_urls.append(f->url());
	}

	formatFiles(m_urls);
}

void SourceFormatterPlugin::formatFiles(KUrl::List &list)
{
// 	m_formatter->loadConfig(KGlobal::config());
    SourceFormatterManager *manager = SourceFormatterManager::self();
    manager->loadConfig();
	//! \todo IStatus
	for (int fileCount = 0; fileCount < list.size(); fileCount++) {
		// check mimetype
		KMimeType::Ptr mime = KMimeType::findByUrl(list[fileCount]);
		kDebug() << "Checking file " << list[fileCount].pathOrUrl() << " of mime type " << mime->name() << endl;
		ISourceFormatter *formatter = manager->formatterForMimeType(mime);
		if (!formatter) // unsupported mime type
			continue;

		// if the file is opened in the editor, format the text in the editor without saving it
		KDevelop::IDocumentController *docController = KDevelop::ICore::self()->documentController();
		KDevelop::IDocument *doc = docController->documentForUrl(list[fileCount]);
		if (doc) {
			kDebug() << "Processing file " << list[fileCount].pathOrUrl() << "opened in editor" << endl;
			KTextEditor::Cursor cursor = doc->cursorPosition();
			doc->textDocument()->setText(formatter->formatSource(doc->textDocument()->text(), mime));
			doc->setCursorPosition(cursor);
			return;
		}

		kDebug() << "Processing file " << list[fileCount].pathOrUrl() << endl;
		QString tmpFile, output;
		if (KIO::NetAccess::download(list[fileCount], tmpFile, 0)) {
			QFile file(tmpFile);
			// read file
			if (file.open(QFile::ReadOnly)) {
				QTextStream is(&file);
				output = formatter->formatSource(is.readAll(), mime);
				file.close();
			} else
				KMessageBox::error(0, i18n("Unable to read %1").arg(list[fileCount].prettyUrl()));

			//write new content
			if (file.open(QFile::WriteOnly | QIODevice::Truncate)) {
				QTextStream os(&file);
				os << output;
				file.close();
			} else
				KMessageBox::error(0, i18n("Unable to write to %1").arg(list[fileCount].prettyUrl()));

			if (!KIO::NetAccess::upload(tmpFile, list[fileCount], 0))
				KMessageBox::error(0, KIO::NetAccess::lastErrorString());

			KIO::NetAccess::removeTempFile(tmpFile);
		} else
			KMessageBox::error(0, KIO::NetAccess::lastErrorString());
	}
}

#include "sourceformatterplugin.moc"
