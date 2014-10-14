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
#include <QMimeDatabase>
#include <KPluginInfo>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/interfaces/ilanguagesupport.h>
#include <language/codegen/coderepresentation.h>
#include <interfaces/isourceformatter.h>
#include "core.h"
#include "debug.h"
#include <ktexteditor/view.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>

#include <kactioncollection.h>
#include <kaction.h>
#include <KLocalizedString>
#include <KIOCore/KIO/StoredTransferJob>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/command.h>
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
const QString SourceFormatterController::styleMimeTypesKey = "MimeTypes";
const QString SourceFormatterController::styleSampleKey = "StyleSample";

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
	QMimeType mime = QMimeDatabase().mimeTypeForUrl(doc->url());
	adaptEditorIndentationMode( doc->textDocument(), formatterForMimeType(mime) );
}

void SourceFormatterController::initialize()
{
}

SourceFormatterController::~SourceFormatterController()
{
}

ISourceFormatter* SourceFormatterController::formatterForUrl(const QUrl &url)
{
	QMimeType mime = QMimeDatabase().mimeTypeForUrl(url);
	return formatterForMimeType(mime);
}
KConfigGroup SourceFormatterController::configuration() const
{
	return Core::self()->activeSession()->config()->group( "SourceFormatter" );
}

ISourceFormatter* SourceFormatterController::findFirstFormatterForMimeType(const QMimeType& mime ) const
{
	static QHash<QString, ISourceFormatter*> knownFormatters;
	if (knownFormatters.contains(mime.name()))
		return knownFormatters[mime.name()];

	foreach( IPlugin* p, Core::self()->pluginController()->allPluginsForExtension( "org.kdevelop.ISourceFormatter" ) ) {
		KPluginInfo info = Core::self()->pluginController()->pluginInfo( p );
		ISourceFormatter *iformatter = p->extension<ISourceFormatter>();
		QSharedPointer<SourceFormatter> formatter(createFormatterForPlugin(iformatter));
		if( formatter->supportedMimeTypes().contains(mime.name()) ) {
			knownFormatters[mime.name()] = iformatter;
			return iformatter;
		}
	}
	knownFormatters[mime.name()] = 0;
	return 0;
}

static void populateStyleFromConfigGroup(SourceFormatterStyle* s, const KConfigGroup& stylegrp)
{
	s->setCaption( stylegrp.readEntry( SourceFormatterController::styleCaptionKey, QString() ) );
	s->setContent( stylegrp.readEntry( SourceFormatterController::styleContentKey, QString() ) );
	s->setMimeTypes( stylegrp.readEntry<QStringList>( SourceFormatterController::styleMimeTypesKey, QStringList() ) );
	s->setOverrideSample( stylegrp.readEntry( SourceFormatterController::styleSampleKey, QString() ) );
}

SourceFormatter* SourceFormatterController::createFormatterForPlugin(ISourceFormatter *ifmt) const
{
	SourceFormatter* formatter = new SourceFormatter();
	formatter->formatter = ifmt;

	// Inserted a new formatter. Now fill it with styles
	foreach( const KDevelop::SourceFormatterStyle& style, ifmt->predefinedStyles() ) {
		formatter->styles[ style.name() ] = new SourceFormatterStyle(style);
	}
	KConfigGroup grp = configuration();
	if( grp.hasGroup( ifmt->name() ) ) {
		KConfigGroup fmtgrp = grp.group( ifmt->name() );
		foreach( const QString& subgroup, fmtgrp.groupList() ) {
			SourceFormatterStyle* s = new SourceFormatterStyle( subgroup );
			KConfigGroup stylegrp = fmtgrp.group( subgroup );
			populateStyleFromConfigGroup(s, stylegrp);
			formatter->styles[ s->name() ] = s;
		}
	}
	return formatter;
}

ISourceFormatter* SourceFormatterController::formatterForMimeType(const QMimeType& mime)
{
	if( !m_enabled || !isMimeTypeSupported( mime ) ) {
		return 0;
	}
	QString formatter = configuration().readEntry( mime.name(), "" );

	if( formatter.isEmpty() )
	{
		return findFirstFormatterForMimeType( mime );
	}

	QStringList formatterinfo = formatter.split( "||", QString::SkipEmptyParts );

	if( formatterinfo.size() != 2 ) {
		qCDebug(SHELL) << "Broken formatting entry for mime:" << mime.name() << "current value:" << formatter;
		return 0;
	}

	return Core::self()->pluginControllerInternal()->extensionForPlugin<ISourceFormatter>( "org.kdevelop.ISourceFormatter", formatterinfo.at(0) );
}

bool SourceFormatterController::isMimeTypeSupported(const QMimeType& mime)
{
	if( findFirstFormatterForMimeType( mime ) ) {
		return true;
	}
	return false;
}

QString SourceFormatterController::indentationMode(const QMimeType& mime)
{
	if (mime.inherits("text/x-c++src") || mime.inherits("text/x-chdr") ||
		mime.inherits("text/x-c++hdr") || mime.inherits("text/x-csrc") ||
		mime.inherits("text/x-java") || mime.inherits("text/x-csharp")) {
		return "cstyle";
	}
	return "none";
}

QString SourceFormatterController::addModelineForCurrentLang(QString input, const QUrl& url, const QMimeType& mime)
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

	qCDebug(SHELL) << "created modeline: " << modeline << endl;

	QRegExp kateModeline("^\\s*//\\s*kate:(.*)$");

	bool modelinefound = false;
	QRegExp knownOptions("\\s*(indent-width|space-indent|tab-width|indent-mode|replace-tabs)");
	while (!is.atEnd()) {
		QString line = is.readLine();
		// replace only the options we care about
		if (kateModeline.indexIn(line) >= 0) { // match
			qCDebug(SHELL) << "Found a kate modeline: " << line << endl;
			modelinefound = true;
			QString options = kateModeline.cap(1);
			QStringList optionList = options.split(';', QString::SkipEmptyParts);

			os <<  modeline;
			foreach(QString s, optionList) {
				if (knownOptions.indexIn(s) < 0) { // unknown option, add it
					if(s.startsWith(' '))
						s=s.mid(1);
					os << s << ";";
					qCDebug(SHELL) << "Found unknown option: " << s << endl;
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
		QMimeType mime = QMimeDatabase().mimeTypeForUrl(doc->url());
		if (isMimeTypeSupported(mime))
			enabled = true;
	}

	m_formatTextAction->setEnabled(enabled);
}

void SourceFormatterController::beautifySource()
{
	IDocument* idoc = KDevelop::ICore::self()->documentController()->activeDocument();
	KTextEditor::View* view = idoc->activeTextView();
	if (!view)
		return;
	KTextEditor::Document* doc = view->document();
	// load the appropriate formatter
	QMimeType mime = QMimeDatabase().mimeTypeForUrl(doc->url());
	ISourceFormatter *formatter = formatterForMimeType(mime);
        if( !formatter ) {
            qCDebug(SHELL) << "no formatter available for" << mime.name();
            return;
        }

	// Ignore the modeline, as the modeline will be changed anyway
	adaptEditorIndentationMode( doc, formatter, true );

	bool has_selection = view->selection();

	if (has_selection) {
		QString original = view->selectionText();

		QString output = formatter->formatSource(view->selectionText(), doc->url(), mime,
												  doc->text(KTextEditor::Range(KTextEditor::Cursor(0,0),view->selectionRange().start())),
												  doc->text(KTextEditor::Range(view->selectionRange().end(), doc->documentRange().end())));

		//remove the final newline character, unless it should be there
		if (!original.endsWith('\n')  && output.endsWith('\n'))
			output.resize(output.length() - 1);
		//there was a selection, so only change the part of the text related to it

		// We don't use KTextEditor::Document directly, because CodeRepresentation transparently works
		// around a possible tab-replacement incompatibility between kate and kdevelop
		DynamicCodeRepresentation::Ptr code( dynamic_cast<DynamicCodeRepresentation*>( KDevelop::createCodeRepresentation( IndexedString( doc->url() ) ).data() ) );
		Q_ASSERT( code );
		code->replace( view->selectionRange(), original, output );
	} else {
		formatDocument(idoc, formatter, mime);
	}
}

void SourceFormatterController::beautifyLine()
{
	KDevelop::IDocumentController *docController = KDevelop::ICore::self()->documentController();
	KDevelop::IDocument *doc = docController->activeDocument();
	if (!doc || !doc->isTextDocument())
		return;
	KTextEditor::Document *tDoc = doc->textDocument();
	KTextEditor::View* view = doc->activeTextView();
	if (!view)
		return;
	// load the appropriate formatter
	QMimeType mime = QMimeDatabase().mimeTypeForUrl(doc->url());
	ISourceFormatter *formatter = formatterForMimeType(mime);
	if( !formatter ) {
		qCDebug(SHELL) << "no formatter available for" << mime.name();
		return;
	}

	const KTextEditor::Cursor cursor = view->cursorPosition();
	const QString line = tDoc->line(cursor.line());
	const QString prev = tDoc->text(KTextEditor::Range(0, 0, cursor.line(), 0));
	const QString post = '\n' + tDoc->text(KTextEditor::Range(KTextEditor::Cursor(cursor.line() + 1, 0), tDoc->documentEnd()));

	const QString formatted = formatter->formatSource(line, doc->url(), mime, prev, post);

	// We don't use KTextEditor::Document directly, because CodeRepresentation transparently works
	// around a possible tab-replacement incompatibility between kate and kdevelop
	DynamicCodeRepresentation::Ptr code(dynamic_cast<DynamicCodeRepresentation*>( KDevelop::createCodeRepresentation( IndexedString( doc->url() ) ).data() ) );
	Q_ASSERT( code );
	code->replace( KTextEditor::Range(cursor.line(), 0, cursor.line(), line.length()), line, formatted );

	// advance cursor one line
	view->setCursorPosition(KTextEditor::Cursor(cursor.line() + 1, 0));
}

void SourceFormatterController::formatDocument(KDevelop::IDocument* doc, ISourceFormatter* formatter, const QMimeType& mime)
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
			adaptEditorIndentationMode( doc->textDocument(), formatterForUrl(doc->url()) );
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

void SourceFormatterController::adaptEditorIndentationMode(KTextEditor::Document *doc, ISourceFormatter *formatter, bool ignoreModeline )
{
	if( !formatter  || !configuration().readEntry( SourceFormatterController::kateOverrideIndentationConfigKey, false ) || !doc )
		return;

	qCDebug(SHELL) << "adapting mode for" << doc->url();

	QRegExp kateModelineWithNewline("\\s*\\n//\\s*kate:(.*)$");

	// modelines should always take precedence
	if( !ignoreModeline && kateModelineWithNewline.indexIn( doc->text() ) != -1 )
	{
		qCDebug(SHELL) << "ignoring because a kate modeline was found";
		return;
	}

	ISourceFormatter::Indentation indentation = formatter->indentation(doc->url());
	if(indentation.isValid())
	{
		struct CommandCaller {
			CommandCaller(KTextEditor::Document* _doc) : doc(_doc), editor(KTextEditor::Editor::instance()) {
				Q_ASSERT(editor);
			}
			void operator()(QString cmd) {
				KTextEditor::Command* command = editor->queryCommand( cmd );
				Q_ASSERT(command);
				QString msg;
				qCDebug(SHELL) << "calling" << cmd;
				foreach(KTextEditor::View* view, doc->views())
					if( !command->exec( view, cmd, msg ) )
						qWarning() << "setting indentation width failed: " << msg;
			}

			KTextEditor::Document* doc;
			KTextEditor::Editor* editor;
		} call(doc);

		if( indentation.indentWidth ) // We know something about indentation-width
			call( QString("set-indent-width %1").arg(indentation.indentWidth ) );

		if( indentation.indentationTabWidth != 0 ) // We know something about tab-usage
		{
			call( QString("set-replace-tabs %1").arg( (indentation.indentationTabWidth == -1) ? 1 : 0 ) );
			if( indentation.indentationTabWidth > 0 )
				call( QString("set-tab-width %1").arg(indentation.indentationTabWidth ) );
		}
	}else{
		qCDebug(SHELL) << "found no valid indentation";
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
			m_urls.append(item->file()->path().toUrl());
		else if (item->target()) {
			foreach(KDevelop::ProjectFileItem *f, item->fileList())
			m_urls.append(f->path().toUrl());
		}
	}

	while (!folders.isEmpty()) {
		KDevelop::ProjectFolderItem *item = folders.takeFirst();
		foreach(KDevelop::ProjectFolderItem *f, item->folderList())
		folders.append(f);
		foreach(KDevelop::ProjectTargetItem *f, item->targetList()) {
			foreach(KDevelop::ProjectFileItem *child, f->fileList())
			m_urls.append(child->path().toUrl());
		}
		foreach(KDevelop::ProjectFileItem *f, item->fileList())
		m_urls.append(f->path().toUrl());
	}

	formatFiles(m_urls);
}

void SourceFormatterController::formatFiles(QList<QUrl> &list)
{
	//! \todo IStatus
	for (int fileCount = 0; fileCount < list.size(); fileCount++) {
		// check mimetype
		QMimeType mime = QMimeDatabase().mimeTypeForUrl(list[fileCount]);
		qCDebug(SHELL) << "Checking file " << list[fileCount] << " of mime type " << mime.name() << endl;
		ISourceFormatter *formatter = formatterForMimeType(mime);
		if (!formatter) // unsupported mime type
			continue;

		// if the file is opened in the editor, format the text in the editor without saving it
		KDevelop::IDocumentController *docController = KDevelop::ICore::self()->documentController();
		KDevelop::IDocument *doc = docController->documentForUrl(list[fileCount]);
		if (doc) {
			qCDebug(SHELL) << "Processing file " << list[fileCount] << "opened in editor" << endl;
			formatDocument(doc, formatter, mime);
			continue;
		}

		qCDebug(SHELL) << "Processing file " << list[fileCount] << endl;
		KIO::StoredTransferJob *job = KIO::storedGet(list[fileCount]);
		if (job->exec()) {
			QByteArray data = job->data();
			QString output = formatter->formatSource(data, list[fileCount], mime);
			data += addModelineForCurrentLang(output, list[fileCount], mime).toUtf8();
			job = KIO::storedPut(data, list[fileCount], -1);
			if (!job->exec())
				KMessageBox::error(0, job->errorString());
		} else
			KMessageBox::error(0, job->errorString());
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

SourceFormatterStyle SourceFormatterController::styleForMimeType(const QMimeType& mime)
{
	QStringList formatter = configuration().readEntry( mime.name(), "" ).split( "||", QString::SkipEmptyParts );
	if( formatter.count() == 2 )
	{
		SourceFormatterStyle s( formatter.at( 1 ) );
		KConfigGroup fmtgrp = configuration().group( formatter.at(0) );
		if( fmtgrp.hasGroup( formatter.at(1) ) ) {
			KConfigGroup stylegrp = fmtgrp.group( formatter.at(1) );
			populateStyleFromConfigGroup(&s, stylegrp);
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


// kate: indent-mode cstyle; space-indent off; tab-width 4;

