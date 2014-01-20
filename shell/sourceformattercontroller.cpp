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

#include "sourceformattercontroller.h"

#include <QVariant>
#include <QStringList>
#include <QRegExp>
#include <KPluginInfo>
#include <KDebug>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/codegen/coderepresentation.h>
#include <interfaces/isourceformatter.h>
#include "core.h"
#include <ktexteditor/view.h>
#include <project/projectmodel.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <ktexteditor/commandinterface.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/configinterface.h>
#include "plugincontroller.h"
#include <interfaces/isession.h>

namespace KDevelop
{

const QString SourceFormatterController::kateModeLineConfigKey = "ModelinesEnabled";
const QString SourceFormatterController::kateOverrideIndentationConfigKey = "OverrideKateIndentation";
const QString SourceFormatterController::styleCaptionKey = "Caption";
const QString SourceFormatterController::styleContentKey = "Content";
const QString SourceFormatterController::supportedMimeTypesKey = "X-KDevelop-SupportedMimeTypes";

SourceFormatterController::SourceFormatterController(QObject *parent)
		: ISourceFormatterController(parent), m_enabled(true)
{
	setObjectName("SourceFormatterController");
	setComponentName("kdevsourceformatter", "kdevsourceformatter");
	setXMLFile("kdevsourceformatter.rc");

	if (Core::self()->setupFlags() & Core::NoUi) return;

	m_formatTextAction = actionCollection()->addAction("edit_reformat_source");
	m_formatTextAction->setText(i18n("&Reformat Source"));
	m_formatTextAction->setToolTip(i18n("Reformat source using AStyle"));
	m_formatTextAction->setWhatsThis(i18n("Source reformatting functionality using <b>astyle</b> library."));
	connect(m_formatTextAction, SIGNAL(triggered()), this, SLOT(beautifySource()));

	m_formatLine = actionCollection()->addAction("edit_reformat_line");
	m_formatLine->setText(i18n("Reformat Line"));
	m_formatLine->setToolTip(i18n("Reformat current line using AStyle"));
	m_formatLine->setWhatsThis(i18n("Source reformatting of line under cursor using <b>astyle</b> library."));
	connect(m_formatLine, SIGNAL(triggered()), this, SLOT(beautifyLine()));

	m_formatFilesAction = actionCollection()->addAction("tools_astyle");
	m_formatFilesAction->setText(i18n("Format Files"));
	m_formatFilesAction->setToolTip(i18n("Format file(s) using the current theme"));
	m_formatFilesAction->setWhatsThis(i18n("Formatting functionality using <b>astyle</b> library."));
	connect(m_formatFilesAction, SIGNAL(triggered()), this, SLOT(formatFiles()));

	m_formatTextAction->setEnabled(false);
	m_formatFilesAction->setEnabled(true);

	connect(Core::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)),
	        this, SLOT(activeDocumentChanged(KDevelop::IDocument*)));
	// Use a queued connection, because otherwise the view is not yet fully set up
	connect(Core::self()->documentController(), SIGNAL(documentLoaded(KDevelop::IDocument*)),
	        this, SLOT(documentLoaded(KDevelop::IDocument*)), Qt::QueuedConnection);

	activeDocumentChanged(Core::self()->documentController()->activeDocument());
}

void SourceFormatterController::documentLoaded( IDocument* doc )
{
	// NOTE: explicitly check this here to prevent crashes on shutdown
	//       when this slot gets called (note: delayed connection)
	//       but the text document was already destroyed
	//       there have been unit tests that failed due to that...
	if (!doc->textDocument()) {
		return;
	}
	KMimeType::Ptr mime = KMimeType::findByUrl(doc->url());
	adaptEditorIndentationMode( doc, formatterForMimeType(mime) );
}

void SourceFormatterController::initialize()
{
}

SourceFormatterController::~SourceFormatterController()
{
}

ISourceFormatter* SourceFormatterController::formatterForUrl(const KUrl &url)
{
	KMimeType::Ptr mime = KMimeType::findByUrl(url);
	return formatterForMimeType(mime);
}
KConfigGroup SourceFormatterController::configuration()
{
	return Core::self()->activeSession()->config()->group( "SourceFormatter" );
}

static ISourceFormatter* findFirstFormatterForMimeType( const KMimeType::Ptr& mime )
{
	static QHash<QString, ISourceFormatter*> knownFormatters;
	if (knownFormatters.contains(mime->name()))
		return knownFormatters[mime->name()];
	
	foreach( IPlugin* p, Core::self()->pluginController()->allPluginsForExtension( "org.kdevelop.ISourceFormatter" ) ) {
		KPluginInfo info = Core::self()->pluginController()->pluginInfo( p );
		if( info.property( SourceFormatterController::supportedMimeTypesKey ).toStringList().contains( mime->name() ) ) {
			ISourceFormatter *formatter = p->extension<ISourceFormatter>();
			knownFormatters[mime->name()] = formatter;
			return formatter;
		}
	}
	knownFormatters[mime->name()] = 0;
	return 0;
}

ISourceFormatter* SourceFormatterController::formatterForMimeType(const KMimeType::Ptr &mime)
{
	if( !m_enabled || !isMimeTypeSupported( mime ) ) {
		return 0;
	}
	QString formatter = configuration().readEntry( mime->name(), "" );

	if( formatter.isEmpty() )
	{
		return findFirstFormatterForMimeType( mime );
	}

	QStringList formatterinfo = formatter.split( "||", QString::SkipEmptyParts );

	if( formatterinfo.size() != 2 ) {
		kDebug() << "Broken formatting entry for mime:" << mime << "current value:" << formatter;
		return 0;
	}

	return Core::self()->pluginControllerInternal()->extensionForPlugin<ISourceFormatter>( "org.kdevelop.ISourceFormatter", formatterinfo.at(0) );
}

bool SourceFormatterController::isMimeTypeSupported(const KMimeType::Ptr &mime)
{
	if( findFirstFormatterForMimeType( mime ) ) {
		return true;
	}
	return false;
}

QString SourceFormatterController::indentationMode(const KMimeType::Ptr &mime)
{
	if (mime->is("text/x-c++src") || mime->is("text/x-chdr") ||
	        mime->is("text/x-c++hdr") || mime->is("text/x-csrc") ||
	        mime->is("text/x-java") || mime->is("text/x-csharp"))
		return "cstyle";
	return "none";
}

QString SourceFormatterController::addModelineForCurrentLang(QString input, const KUrl& url, const KMimeType::Ptr& mime)
{
	if( !isMimeTypeSupported(mime) )
		return input;
	
	QRegExp kateModelineWithNewline("\\s*\\n//\\s*kate:(.*)$");
	
	// If there already is a modeline in the document, adapt it while formatting, even
	// if "add modeline" is disabled.
	if( !configuration().readEntry( SourceFormatterController::kateModeLineConfigKey, false ) && 
		      kateModelineWithNewline.indexIn( input ) == -1 )
		return input;

	ISourceFormatter* fmt = formatterForMimeType( mime );
	ISourceFormatter::Indentation indentation = fmt->indentation(url);
	
	if( !indentation.isValid() )
		return input;
	
	QString output;
	QTextStream os(&output, QIODevice::WriteOnly);
	QTextStream is(&input, QIODevice::ReadOnly);

	Q_ASSERT(fmt);

	
    QString modeline("// kate: ");
	QString indentLength = QString::number(indentation.indentWidth);
	QString tabLength = QString::number(indentation.indentationTabWidth);
	// add indentation style
	modeline.append("indent-mode ").append(indentationMode(mime).append("; "));

	if(indentation.indentWidth) // We know something about indentation-width
		modeline.append(QString("indent-width %1; ").arg(indentation.indentWidth));

	if(indentation.indentationTabWidth != 0) // We know something about tab-usage
	{
		modeline.append(QString("replace-tabs %1; ").arg((indentation.indentationTabWidth == -1) ? "on" : "off"));
		if(indentation.indentationTabWidth > 0)
			modeline.append(QString("tab-width %1; ").arg(indentation.indentationTabWidth));
	}

	kDebug() << "created modeline: " << modeline << endl;

	QRegExp kateModeline("^\\s*//\\s*kate:(.*)$");
	
	bool modelinefound = false;
	QRegExp knownOptions("\\s*(indent-width|space-indent|tab-width|indent-mode|replace-tabs)");
	while (!is.atEnd()) {
		QString line = is.readLine();
		// replace only the options we care about
		if (kateModeline.indexIn(line) >= 0) { // match
			kDebug() << "Found a kate modeline: " << line << endl;
			modelinefound = true;
			QString options = kateModeline.cap(1);
			QStringList optionList = options.split(';', QString::SkipEmptyParts);

			os <<  modeline;
			foreach(QString s, optionList) {
				if (knownOptions.indexIn(s) < 0) { // unknown option, add it
					if(s.startsWith(' '))
						s=s.mid(1);
					os << s << ";";
					kDebug() << "Found unknown option: " << s << endl;
				}
			}
			os << endl;
		} else
			os << line << endl;
	}

	if (!modelinefound)
		os << modeline << endl;
	return output;
}

void SourceFormatterController::cleanup()
{
}


void SourceFormatterController::activeDocumentChanged(IDocument* doc)
{
	bool enabled = false;

	if (doc) {
		KMimeType::Ptr mime = KMimeType::findByUrl(doc->url());
		if (isMimeTypeSupported(mime))
			enabled = true;
	}

	m_formatTextAction->setEnabled(enabled);
}

void SourceFormatterController::beautifySource()
{
	KDevelop::IDocumentController *docController = KDevelop::ICore::self()->documentController();
	KDevelop::IDocument *doc = docController->activeDocument();
	if (!doc)
		return;
	// load the appropriate formatter
	KMimeType::Ptr mime = KMimeType::findByUrl(doc->url());
	ISourceFormatter *formatter = formatterForMimeType(mime);
        if( !formatter ) {
            kDebug() << "no formatter available for" << mime;
            return;
        }

	// Ignore the modeline, as the modeline will be changed anyway
	adaptEditorIndentationMode( doc, formatter, true );

	bool has_selection = false;
	KTextEditor::View *view = doc->textDocument()->activeView();
	if (view && view->selection())
		has_selection = true;

	if (has_selection) {
		QString original = view->selectionText();

		QString output = formatter->formatSource(view->selectionText(), doc->url(), mime,
												  view->document()->text(KTextEditor::Range(KTextEditor::Cursor(0,0),view->selectionRange().start())),
												  view->document()->text(KTextEditor::Range(view->selectionRange().end(), view->document()->documentRange().end())));

		//remove the final newline character, unless it should be there
		if (!original.endsWith('\n')  && output.endsWith('\n'))
			output.resize(output.length() - 1);
		//there was a selection, so only change the part of the text related to it
			
		// We don't use KTextEditor::Document directly, because CodeRepresentation transparently works
		// around a possible tab-replacement incompatibility between kate and kdevelop
		DynamicCodeRepresentation::Ptr code = DynamicCodeRepresentation::Ptr::dynamicCast( KDevelop::createCodeRepresentation( IndexedString( doc->url() ) ) );
		Q_ASSERT( code );
		code->replace( view->selectionRange(), original, output );
	} else {
		formatDocument(doc, formatter, mime);
	}
}

void SourceFormatterController::beautifyLine()
{
	KDevelop::IDocumentController *docController = KDevelop::ICore::self()->documentController();
	KDevelop::IDocument *doc = docController->activeDocument();
	if (!doc || !doc->isTextDocument())
		return;
	KTextEditor::Document *tDoc = doc->textDocument();
	if (!tDoc->activeView())
		return;
	// load the appropriate formatter
	KMimeType::Ptr mime = KMimeType::findByUrl(doc->url());
	ISourceFormatter *formatter = formatterForMimeType(mime);
	if( !formatter ) {
		kDebug() << "no formatter available for" << mime;
		return;
	}

	const KTextEditor::Cursor cursor = tDoc->activeView()->cursorPosition();
	const QString line = tDoc->line(cursor.line());
	const QString prev = tDoc->text(KTextEditor::Range(0, 0, cursor.line(), 0));
	const QString post = '\n' + tDoc->text(KTextEditor::Range(KTextEditor::Cursor(cursor.line() + 1, 0), tDoc->documentEnd()));
	
	const QString formatted = formatter->formatSource(line, doc->url(), mime, prev, post);
	
	// We don't use KTextEditor::Document directly, because CodeRepresentation transparently works
	// around a possible tab-replacement incompatibility between kate and kdevelop
	DynamicCodeRepresentation::Ptr code = DynamicCodeRepresentation::Ptr::dynamicCast( KDevelop::createCodeRepresentation( IndexedString( doc->url() ) ) );
	Q_ASSERT( code );
	code->replace( KTextEditor::Range(cursor.line(), 0, cursor.line(), line.length()), line, formatted );
	
	// advance cursor one line
	tDoc->activeView()->setCursorPosition(KTextEditor::Cursor(cursor.line() + 1, 0));
}

void SourceFormatterController::formatDocument(KDevelop::IDocument *doc, ISourceFormatter *formatter, const KMimeType::Ptr &mime)
{
	// We don't use KTextEditor::Document directly, because CodeRepresentation transparently works
	// around a possible tab-replacement incompatibility between kate and kdevelop
	CodeRepresentation::Ptr code = KDevelop::createCodeRepresentation( IndexedString( doc->url() ) );

	KTextEditor::Cursor cursor = doc->cursorPosition();
	QString text = formatter->formatSource(code->text(), doc->url(), mime);
	text = addModelineForCurrentLang(text, doc->url(), mime);
	code->setText(text);
	
	doc->setCursorPosition(cursor);
}

void SourceFormatterController::settingsChanged()
{
	if( configuration().readEntry( SourceFormatterController::kateOverrideIndentationConfigKey, false ) )
		foreach( KDevelop::IDocument* doc, ICore::self()->documentController()->openDocuments() )
			adaptEditorIndentationMode( doc, formatterForUrl(doc->url()) );
}

/**
 * Kate commands:
 * Use spaces for indentation:
 *   "set-replace-tabs 1"
 * Use tabs for indentation (eventually mixed):
 *   "set-replace-tabs 0"
 * Indent width:
 * 	 "set-indent-width X"
 * Tab width:
 *   "set-tab-width X"
 * */

void SourceFormatterController::adaptEditorIndentationMode(KDevelop::IDocument *doc, ISourceFormatter *formatter, bool ignoreModeline )
{
	if( !formatter  || !configuration().readEntry( SourceFormatterController::kateOverrideIndentationConfigKey, false ) || !doc->isTextDocument() )
		return;

	KTextEditor::Document *textDoc = doc->textDocument();
	kDebug() << "adapting mode for" << doc->url();
	Q_ASSERT(textDoc);
	
	QRegExp kateModelineWithNewline("\\s*\\n//\\s*kate:(.*)$");
	
	// modelines should always take precedence
	if( !ignoreModeline && kateModelineWithNewline.indexIn( textDoc->text() ) != -1 )
	{
		kDebug() << "ignoring because a kate modeline was found";
		return;
	}
	
	ISourceFormatter::Indentation indentation = formatter->indentation(doc->url());
	if(indentation.isValid())
	{
		struct CommandCaller {
			CommandCaller(KTextEditor::Document* _doc) : doc(_doc), ci(qobject_cast<KTextEditor::CommandInterface*>(doc->editor())) {
				Q_ASSERT(ci);
			}
			void operator()(QString cmd) {
				KTextEditor::Command* command = ci->queryCommand( cmd );
				Q_ASSERT(command);
				QString msg;
				kDebug() << "calling" << cmd;
				if( !command->exec( doc->activeView(), cmd, msg ) )
					kWarning() << "setting indentation width failed: " << msg;
			}
			
			KTextEditor::Document* doc;
			KTextEditor::CommandInterface* ci;
		} call(textDoc);
		
		if( indentation.indentWidth ) // We know something about indentation-width
			call( QString("set-indent-width %1").arg(indentation.indentWidth ) );

		if( indentation.indentationTabWidth != 0 ) // We know something about tab-usage
		{
			call( QString("set-replace-tabs %1").arg( (indentation.indentationTabWidth == -1) ? 1 : 0 ) );
			if( indentation.indentationTabWidth > 0 )
				call( QString("set-tab-width %1").arg(indentation.indentationTabWidth ) );
		}
	}else{
		kDebug() << "found no valid indentation";
	}
}

void SourceFormatterController::formatFiles()
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

void SourceFormatterController::formatFiles(KUrl::List &list)
{
	//! \todo IStatus
	for (int fileCount = 0; fileCount < list.size(); fileCount++) {
		// check mimetype
		KMimeType::Ptr mime = KMimeType::findByUrl(list[fileCount]);
		kDebug() << "Checking file " << list[fileCount].pathOrUrl() << " of mime type " << mime->name() << endl;
		ISourceFormatter *formatter = formatterForMimeType(mime);
		if (!formatter) // unsupported mime type
			continue;

		// if the file is opened in the editor, format the text in the editor without saving it
		KDevelop::IDocumentController *docController = KDevelop::ICore::self()->documentController();
		KDevelop::IDocument *doc = docController->documentForUrl(list[fileCount]);
		if (doc) {
			kDebug() << "Processing file " << list[fileCount].pathOrUrl() << "opened in editor" << endl;
			formatDocument(doc, formatter, mime);
			continue;
		}

		kDebug() << "Processing file " << list[fileCount].pathOrUrl() << endl;
		QString tmpFile, output;
		if (KIO::NetAccess::download(list[fileCount], tmpFile, 0)) {
			QFile file(tmpFile);
			// read file
			if (file.open(QFile::ReadOnly)) {
				QTextStream is(&file);
				output = formatter->formatSource(is.readAll(), list[fileCount], mime);
				file.close();
			} else
				KMessageBox::error(0, i18n("Unable to read %1", list[fileCount].prettyUrl()));

			//write new content
			if (file.open(QFile::WriteOnly | QIODevice::Truncate)) {
				QTextStream os(&file);
				os << addModelineForCurrentLang(output, list[fileCount], mime);
				file.close();
			} else
				KMessageBox::error(0, i18n("Unable to write to %1", list[fileCount].prettyUrl()));

			if (!KIO::NetAccess::upload(tmpFile, list[fileCount], 0))
				KMessageBox::error(0, KIO::NetAccess::lastErrorString());

			KIO::NetAccess::removeTempFile(tmpFile);
		} else
			KMessageBox::error(0, KIO::NetAccess::lastErrorString());
	}
}

KDevelop::ContextMenuExtension SourceFormatterController::contextMenuExtension(KDevelop::Context* context)
{
	KDevelop::ContextMenuExtension ext;
	m_urls.clear();
	m_prjItems.clear();

	if (context->hasType(KDevelop::Context::EditorContext))
	{
		if(m_formatTextAction->isEnabled())
			ext.addAction(KDevelop::ContextMenuExtension::EditGroup, m_formatTextAction);
	} else if (context->hasType(KDevelop::Context::FileContext)) {
		KDevelop::FileContext* filectx = dynamic_cast<KDevelop::FileContext*>(context);
		m_urls = filectx->urls();
		ext.addAction(KDevelop::ContextMenuExtension::EditGroup, m_formatFilesAction);
	} else if (context->hasType(KDevelop::Context::CodeContext)) {
	} else if (context->hasType(KDevelop::Context::ProjectItemContext)) {
		KDevelop::ProjectItemContext* prjctx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
		m_prjItems = prjctx->items();
		if ( !m_prjItems.isEmpty() ) {
			ext.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_formatFilesAction);
		}
	}
	return ext;
}

SourceFormatterStyle SourceFormatterController::styleForMimeType( const KMimeType::Ptr& mime )
{
	QStringList formatter = configuration().readEntry( mime->name(), "" ).split( "||", QString::SkipEmptyParts );
	if( formatter.count() == 2 )
	{
		SourceFormatterStyle s( formatter.at( 1 ) );
		KConfigGroup fmtgrp = configuration().group( formatter.at(0) );
		if( fmtgrp.hasGroup( formatter.at(1) ) ) {
			KConfigGroup stylegrp = fmtgrp.group( formatter.at(1) );
			s.setCaption( stylegrp.readEntry( styleCaptionKey, "" ) );
			s.setContent( stylegrp.readEntry( styleContentKey, "" ) );
		}
		return s;
	}
	return SourceFormatterStyle();
}

void SourceFormatterController::disableSourceFormatting(bool disable)
{
	m_enabled = !disable;
}

bool SourceFormatterController::sourceFormattingEnabled()
{
	return m_enabled;
}

/*
 Code copied from source formatter plugin, unused currently but shouldn't be just thrown away
QString SourceFormatterPlugin::replaceSpacesWithTab(const QString &input, ISourceFormatter *formatter)
{
	QString output(input);
	int wsCount = formatter->indentationLength();
	ISourceFormatter::IndentationType type = formatter->indentationType();

	if (type == ISourceFormatter::IndentWithTabs) {
		// tabs and wsCount spaces to be a tab
		QString replace;
		for (int i = 0; i < wsCount;i++)
			replace += ' ';

		output = output.replace(replace, QChar('\t'));
// 		input = input.remove(' ');
	} else if (type == ISourceFormatter::IndentWithSpacesAndConvertTabs) {
		//convert tabs to spaces
		QString replace;
		for (int i = 0;i < wsCount;i++)
			replace += ' ';

		output = output.replace(QChar('\t'), replace);
	}
	return output;
}

QString SourceFormatterPlugin::addIndentation(QString input, const QString indentWith)
{
	QString output;
	QTextStream os(&output, QIODevice::WriteOnly);
	QTextStream is(&input, QIODevice::ReadOnly);

	while (!is.atEnd())
		os << indentWith << is.readLine() << endl;
	return output;
}
*/

}

#include "sourceformattercontroller.moc"

// kate: indent-mode cstyle; space-indent off; tab-width 4;

