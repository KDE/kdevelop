/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sourceformattercontroller.h"

#include <QAction>
#include <QAbstractButton>
#include <QMimeDatabase>
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include <QPointer>
#include <QMessageBox>

#include <KActionCollection>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KTextEditor/Command>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>
#include <KParts/MainWindow>

#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/isession.h>
#include <interfaces/isourceformatter.h>
#include <interfaces/iuicontroller.h>
#include <language/codegen/coderepresentation.h>
#include <language/interfaces/ilanguagesupport.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include "core.h"
#include "debug.h"
#include "plugincontroller.h"
#include "sourceformatterjob.h"
#include "textdocument.h"

#include <algorithm>
#include <tuple>
#include <utility>

namespace {

namespace Strings {
QString SourceFormatter() { return QStringLiteral("SourceFormatter"); }
QString UseDefault() { return QStringLiteral("UseDefault"); }
}

}

namespace KDevelop {
class SourceFormatterControllerPrivate
{
public:
    // cache of formatter plugins, to avoid querying plugincontroller
    QVector<ISourceFormatter*> sourceFormatters;
    // GUI actions
    QAction* formatTextAction;
    QAction* formatFilesAction;
    QAction* formatLine;
    QList<KDevelop::ProjectBaseItem*> prjItems;
    QList<QUrl> urls;
    bool enabled = true;

    ISourceFormatter* formatterForConfigEntry(const QString& entry, const QString& mimename) const;
};

ISourceFormatter* SourceFormatterControllerPrivate::formatterForConfigEntry(const QString& entry, const QString& mimename) const
{
    const int pos = entry.indexOf(QLatin1String("||"));

    if ((pos <= 0) || (pos + 2 >= entry.size())) {
        qCDebug(SHELL) << "Broken formatting entry for mime:" << mimename << "current value:" << entry;
    }

    const auto formatterName = entry.leftRef(pos);
    auto it = std::find_if(sourceFormatters.begin(), sourceFormatters.end(), [&](ISourceFormatter* iformatter) {
        return (iformatter->name() == formatterName);
    });

    return (it != sourceFormatters.end()) ? *it : nullptr;
}

QString SourceFormatterController::kateModeLineConfigKey()
{
    return QStringLiteral("ModelinesEnabled");
}

QString SourceFormatterController::kateOverrideIndentationConfigKey()
{
    return QStringLiteral("OverrideKateIndentation");
}

QString SourceFormatterController::styleCaptionKey()
{
    return QStringLiteral("Caption");
}

QString SourceFormatterController::styleShowPreviewKey()
{
    return QStringLiteral("ShowPreview");
}

QString SourceFormatterController::styleContentKey()
{
    return QStringLiteral("Content");
}

QString SourceFormatterController::styleMimeTypesKey()
{
    return QStringLiteral("MimeTypes");
}

QString SourceFormatterController::styleSampleKey()
{
    return QStringLiteral("StyleSample");
}

SourceFormatterController::SourceFormatterController(QObject *parent)
    : ISourceFormatterController(parent)
    , d_ptr(new SourceFormatterControllerPrivate)
{
    Q_D(SourceFormatterController);

    setObjectName(QStringLiteral("SourceFormatterController"));
    setComponentName(QStringLiteral("kdevsourceformatter"), i18n("Source Formatter"));
    setXMLFile(QStringLiteral("kdevsourceformatter.rc"));

    if (Core::self()->setupFlags() & Core::NoUi) return;

    d->formatTextAction = actionCollection()->addAction(QStringLiteral("edit_reformat_source"));
    d->formatTextAction->setText(i18nc("@action", "&Reformat Source"));
    connect(d->formatTextAction, &QAction::triggered, this, &SourceFormatterController::beautifySource);

    d->formatLine = actionCollection()->addAction(QStringLiteral("edit_reformat_line"));
    d->formatLine->setText(i18nc("@action", "Reformat Line"));
    connect(d->formatLine, &QAction::triggered, this, &SourceFormatterController::beautifyLine);

    d->formatFilesAction = actionCollection()->addAction(QStringLiteral("tools_astyle"));
    d->formatFilesAction->setText(i18nc("@action", "Reformat Files..."));
    d->formatFilesAction->setToolTip(i18nc("@info:tooltip", "Format file(s) using the configured source formatter(s)"));
    d->formatFilesAction->setWhatsThis(i18nc("@info:whatsthis",
                                             "Formatting functionality using the configured source formatter(s)."));
    d->formatFilesAction->setEnabled(false);
    connect(d->formatFilesAction, &QAction::triggered,
            this, QOverload<>::of(&SourceFormatterController::formatFiles));


    connect(Core::self()->pluginController(), &IPluginController::pluginLoaded,
            this, &SourceFormatterController::pluginLoaded);
    connect(Core::self()->pluginController(), &IPluginController::unloadingPlugin,
            this, &SourceFormatterController::unloadingPlugin);

    // connect to both documentActivated & documentClosed,
    // otherwise we miss when the last document was closed
    connect(Core::self()->documentController(), &IDocumentController::documentActivated,
            this, &SourceFormatterController::updateFormatTextAction);
    connect(Core::self()->documentController(), &IDocumentController::documentClosed,
            this, &SourceFormatterController::updateFormatTextAction);

    qRegisterMetaType<QPointer<KDevelop::TextDocument>>();
    connect(Core::self()->documentController(), &IDocumentController::documentLoaded,
            // Use a queued connection, because otherwise the view is not yet fully set up
            // but wrap the document in a smart pointer to guard against crashes when it
            // gets deleted in the meantime
            this, [this](IDocument *doc) {
                const auto textDoc = QPointer<TextDocument>(dynamic_cast<TextDocument*>(doc));
                QMetaObject::invokeMethod(this, "documentLoaded", Qt::QueuedConnection, Q_ARG(QPointer<KDevelop::TextDocument>, textDoc));
            });
    connect(Core::self()->projectController(), &IProjectController::projectOpened, this, &SourceFormatterController::projectOpened);

    updateFormatTextAction();
}

void SourceFormatterController::documentLoaded(const QPointer<TextDocument>& doc)
{
    // NOTE: explicitly check this here to prevent crashes on shutdown
    //       when this slot gets called (note: delayed connection)
    //       but the text document was already destroyed
    //       there have been unit tests that failed due to that...
    if (!doc || !doc->textDocument()) {
        return;
    }
    const auto url = doc->url();
    const auto mime = QMimeDatabase().mimeTypeForUrl(url);
    adaptEditorIndentationMode(doc->textDocument(), formatterForUrl(url, mime), url);
}

void SourceFormatterController::projectOpened(const IProject* project)
{
    Q_D(SourceFormatterController);

    // Adapt the indentation mode if a project was just opened. Otherwise if a document
    // is loaded before its project, it might not have the correct indentation mode set.

    auto config = project->projectConfiguration()->group(Strings::SourceFormatter());
    if (!config.isValid() || config.readEntry(Strings::UseDefault(), true)) {
        return;
    }

    QHash<QString, ISourceFormatter*> formatters;
    const auto documents = ICore::self()->documentController()->openDocuments();
    for (const KDevelop::IDocument* doc : documents) {
        if (project->inProject(IndexedString(doc->url()))) {
            const QString mimename = QMimeDatabase().mimeTypeForUrl(doc->url()).name();
            auto it = formatters.find(mimename);
            if (it == formatters.end()) {
                const auto entry = config.readEntry(mimename, QString());
                it = formatters.insert(mimename, entry.isEmpty() ? nullptr : d->formatterForConfigEntry(entry, mimename));
            }
            if (it.value()) {
                adaptEditorIndentationMode(doc->textDocument(), it.value(), doc->url());
            }
        }
    }
}

void SourceFormatterController::pluginLoaded(IPlugin* plugin)
{
    Q_D(SourceFormatterController);

    auto* sourceFormatter = plugin->extension<ISourceFormatter>();

    if (!sourceFormatter) {
        return;
    }

    d->sourceFormatters << sourceFormatter;

    resetUi();

    emit formatterLoaded(sourceFormatter);
    // with one plugin now added, hasFormatters turned to true, so report to listeners
    if (d->sourceFormatters.size() == 1) {
        emit hasFormattersChanged(true);
    }
}

void SourceFormatterController::unloadingPlugin(IPlugin* plugin)
{
    Q_D(SourceFormatterController);

    auto* sourceFormatter = plugin->extension<ISourceFormatter>();

    if (!sourceFormatter) {
        return;
    }

    const int idx = d->sourceFormatters.indexOf(sourceFormatter);
    Q_ASSERT(idx != -1);
    d->sourceFormatters.remove(idx);

    resetUi();

    emit formatterUnloading(sourceFormatter);
    if (d->sourceFormatters.isEmpty()) {
        emit hasFormattersChanged(false);
    }
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
    return formatterForUrl(url, mime);
}

KConfigGroup SourceFormatterController::configForUrl(const QUrl& url) const
{
    auto core = KDevelop::Core::self();
    auto project = core->projectController()->findProjectForUrl(url);
    if (project) {
        auto config = project->projectConfiguration()->group(Strings::SourceFormatter());
        if (config.isValid() && !config.readEntry(Strings::UseDefault(), true)) {
            return config;
        }
    }

    return core->activeSession()->config()->group( Strings::SourceFormatter() );
}

KConfigGroup SourceFormatterController::sessionConfig() const
{
    return KDevelop::Core::self()->activeSession()->config()->group( Strings::SourceFormatter() );
}

KConfigGroup SourceFormatterController::globalConfig() const
{
    return KSharedConfig::openConfig()->group( Strings::SourceFormatter() );
}

ISourceFormatter* SourceFormatterController::findFirstFormatterForMimeType(const QMimeType& mime ) const
{
    Q_D(const SourceFormatterController);

    static QHash<QString, ISourceFormatter*> knownFormatters;
    const auto formatterIt = knownFormatters.constFind(mime.name());
    if (formatterIt != knownFormatters.constEnd())
        return *formatterIt;

    auto it = std::find_if(d->sourceFormatters.constBegin(), d->sourceFormatters.constEnd(),
                           [&](ISourceFormatter* iformatter) {
        QSharedPointer<SourceFormatter> formatter(createFormatterForPlugin(iformatter));
        return (formatter->supportedMimeTypes().contains(mime.name()));
    });

    ISourceFormatter* iformatter = (it != d->sourceFormatters.constEnd()) ? *it : nullptr;

    // cache result in any case
    knownFormatters.insert(mime.name(), iformatter);
    return iformatter;
}

static void populateStyleFromConfigGroup(SourceFormatterStyle* s, const KConfigGroup& stylegrp)
{
    s->setCaption( stylegrp.readEntry( SourceFormatterController::styleCaptionKey(), QString() ) );
    s->setUsePreview( stylegrp.readEntry( SourceFormatterController::styleShowPreviewKey(), false ) );
    s->setContent( stylegrp.readEntry( SourceFormatterController::styleContentKey(), QString() ) );
    s->setMimeTypes( stylegrp.readEntry<QStringList>( SourceFormatterController::styleMimeTypesKey(), QStringList() ) );
    s->setOverrideSample( stylegrp.readEntry( SourceFormatterController::styleSampleKey(), QString() ) );
}

SourceFormatter* SourceFormatterController::createFormatterForPlugin(ISourceFormatter *ifmt) const
{
    auto* formatter = new SourceFormatter();
    formatter->formatter = ifmt;

    // Inserted a new formatter. Now fill it with styles
    const auto predefinedStyles = ifmt->predefinedStyles();
    for (const KDevelop::SourceFormatterStyle& style : predefinedStyles) {
        formatter->styles[ style.name() ] = new SourceFormatterStyle(style);
    }
    KConfigGroup grp = globalConfig();
    if( grp.hasGroup( ifmt->name() ) ) {
        KConfigGroup fmtgrp = grp.group( ifmt->name() );
        const auto subgroups = fmtgrp.groupList();
        for (const QString& subgroup : subgroups) {
            auto* s = new SourceFormatterStyle( subgroup );
            KConfigGroup stylegrp = fmtgrp.group( subgroup );
            populateStyleFromConfigGroup(s, stylegrp);
            formatter->styles[ s->name() ] = s;
        }
    }
    return formatter;
}

ISourceFormatter* SourceFormatterController::formatterForUrl(const QUrl& url, const QMimeType& mime)
{
    Q_D(SourceFormatterController);

    if (!d->enabled || !isMimeTypeSupported(mime)) {
        return nullptr;
    }

    const auto formatter = configForUrl(url).readEntry(mime.name(), QString());

    if( formatter.isEmpty() )
    {
        return findFirstFormatterForMimeType( mime );
    }

    return d->formatterForConfigEntry(formatter, mime.name());
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
    if (mime.inherits(QStringLiteral("text/x-c++src")) || mime.inherits(QStringLiteral("text/x-chdr")) ||
        mime.inherits(QStringLiteral("text/x-c++hdr")) || mime.inherits(QStringLiteral("text/x-csrc")) ||
        mime.inherits(QStringLiteral("text/x-java")) || mime.inherits(QStringLiteral("text/x-csharp"))) {
        return QStringLiteral("cstyle");
    }
    return QStringLiteral("none");
}

QString SourceFormatterController::addModelineForCurrentLang(QString input, const QUrl& url, const QMimeType& mime)
{
    if( !isMimeTypeSupported(mime) )
        return input;

    QRegExp kateModelineWithNewline(QStringLiteral("\\s*\\n//\\s*kate:(.*)$"));

    // If there already is a modeline in the document, adapt it while formatting, even
    // if "add modeline" is disabled.
    if (!configForUrl(url).readEntry(SourceFormatterController::kateModeLineConfigKey(), false) &&
            kateModelineWithNewline.indexIn( input ) == -1 )
        return input;

    ISourceFormatter* fmt = formatterForUrl(url, mime);
    ISourceFormatter::Indentation indentation = fmt->indentation(url);

    if( !indentation.isValid() )
        return input;

    QString output;
    QTextStream os(&output, QIODevice::WriteOnly);
    QTextStream is(&input, QIODevice::ReadOnly);

    Q_ASSERT(fmt);


    QString modeline(QStringLiteral("// kate: ")
                   + QLatin1String("indent-mode ") + indentationMode(mime) + QLatin1String("; "));

    if(indentation.indentWidth) // We know something about indentation-width
        modeline.append(QStringLiteral("indent-width %1; ").arg(indentation.indentWidth));

    if(indentation.indentationTabWidth != 0) // We know something about tab-usage
    {
        const auto state = (indentation.indentationTabWidth == -1) ? QLatin1String("on") : QLatin1String("off");
        modeline += QLatin1String("replace-tabs ") + state + QLatin1String("; ");
        if(indentation.indentationTabWidth > 0)
            modeline.append(QStringLiteral("tab-width %1; ").arg(indentation.indentationTabWidth));
    }

    qCDebug(SHELL) << "created modeline: " << modeline;

    QRegExp kateModeline(QStringLiteral("^\\s*//\\s*kate:(.*)$"));

    bool modelinefound = false;
    QRegExp knownOptions(QStringLiteral("\\s*(indent-width|space-indent|tab-width|indent-mode|replace-tabs)"));
    while (!is.atEnd()) {
        QString line = is.readLine();
        // replace only the options we care about
        if (kateModeline.indexIn(line) >= 0) { // match
            qCDebug(SHELL) << "Found a kate modeline: " << line;
            modelinefound = true;
            QString options = kateModeline.cap(1);
            const QStringList optionList = options.split(QLatin1Char(';'), Qt::SkipEmptyParts);

            os <<  modeline;
            for (QString s : optionList) {
                if (knownOptions.indexIn(s) < 0) { // unknown option, add it
                    if(s.startsWith(QLatin1Char(' ')))
                        s.remove(0, 1);
                    os << s << ";";
                    qCDebug(SHELL) << "Found unknown option: " << s;
                }
            }
            os << Qt::endl;
        } else
            os << line << Qt::endl;
    }

    if (!modelinefound)
        os << modeline << Qt::endl;
    return output;
}

void SourceFormatterController::cleanup()
{
}

void SourceFormatterController::updateFormatTextAction()
{
    Q_D(SourceFormatterController);

    const auto [enabled, tool, style] = [this, d] {
        auto disabled = std::tuple { false, QString(), QString() };
        if (d->sourceFormatters.empty()) {
            return disabled;
        }

        const auto* doc = KDevelop::ICore::self()->documentController()->activeDocument();
        if (!doc) {
            return disabled;
        }

        const auto url = doc->url();
        const auto mime = QMimeDatabase().mimeTypeForUrl(url);
        const auto* const formatter = formatterForUrl(url, mime);
        if (!formatter) {
            return disabled;
        }

        const auto style = styleForUrl(url, mime);
        const auto styleName = style.name();
        if (styleName.isEmpty()) {
            return disabled; // A valid style's name is never empty => no style is configured for url.
        }

        auto styleCaption = style.caption();
        if (styleCaption.isEmpty()) {
            // This could be an incomplete predefined style, for which only the name is stored in config.
            styleCaption = formatter->predefinedStyle(styleName).caption();
        }

        return std::tuple { true, formatter->caption(), std::move(styleCaption) };
    }();

    d->formatTextAction->setEnabled(enabled);
    d->formatLine->setEnabled(enabled);

    if (enabled) {
        d->formatTextAction->setToolTip(i18nc("@info:tooltip", "Reformat selection or file using <i>%1</i> (<b>%2</b>)",
                                              tool, style));
        d->formatTextAction->setWhatsThis(i18nc("@info:whatsthis",
                                                "Reformats selected text or the entire file if nothing is selected, using <i>%1</i> tool with <b>%2</b> style.",
                                                tool, style));
        d->formatLine->setToolTip(i18nc("@info:tooltip", "Reformat current line using <i>%1</i> (<b>%2</b>)",
                                        tool, style));
        d->formatLine->setWhatsThis(i18nc("@info:whatsthis",
                                          "Source reformatting of line under cursor using <i>%1</i> tool with <b>%2</b> style.",
                                          tool, style));
    } else {
        d->formatTextAction->setToolTip(i18nc("@info:tooltip",
                                              "Reformat selection or file using the configured source formatter"));
        d->formatTextAction->setWhatsThis(i18nc("@info:whatsthis",
                                                "Reformats selected text or the entire file if nothing is selected, using the configured source formatter."));
        d->formatLine->setToolTip(i18nc("@info:tooltip",
                                        "Reformat current line using the configured source formatter"));
        d->formatLine->setWhatsThis(i18nc("@info:whatsthis",
                                          "Source reformatting of line under cursor using the configured source formatter."));
    }
}

void SourceFormatterController::beautifySource()
{
    IDocument* idoc = KDevelop::ICore::self()->documentController()->activeDocument();
    if (!idoc)
        return;
    KTextEditor::View* view = idoc->activeTextView();
    if (!view)
        return;
    KTextEditor::Document* doc = view->document();
    // load the appropriate formatter
    const auto url = idoc->url();
    const auto mime = QMimeDatabase().mimeTypeForUrl(url);
    ISourceFormatter* formatter = formatterForUrl(url, mime);
        if( !formatter ) {
            qCDebug(SHELL) << "no formatter available for" << mime.name();
            return;
        }

    // Ignore the modeline, as the modeline will be changed anyway
    adaptEditorIndentationMode(doc, formatter, url, true);

    bool has_selection = view->selection();

    if (has_selection) {
        QString original = view->selectionText();

        QString output = formatter->formatSource(view->selectionText(), url, mime,
                                                doc->text(KTextEditor::Range(KTextEditor::Cursor(0,0),view->selectionRange().start())),
                                                doc->text(KTextEditor::Range(view->selectionRange().end(), doc->documentRange().end())));

        //remove the final newline character, unless it should be there
        if (!original.endsWith(QLatin1Char('\n'))  && output.endsWith(QLatin1Char('\n')))
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
    const auto url = doc->url();
    const auto mime = QMimeDatabase().mimeTypeForUrl(url);
    ISourceFormatter* formatter = formatterForUrl(url, mime);
    if( !formatter ) {
        qCDebug(SHELL) << "no formatter available for" << mime.name();
        return;
    }

    const KTextEditor::Cursor cursor = view->cursorPosition();
    const QString line = tDoc->line(cursor.line());
    const QString prev = tDoc->text(KTextEditor::Range(0, 0, cursor.line(), 0));
    const QString post = QLatin1Char('\n') + tDoc->text(KTextEditor::Range(KTextEditor::Cursor(cursor.line() + 1, 0), tDoc->documentEnd()));

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
    Q_ASSERT(doc);
    Q_ASSERT(formatter);

    qCDebug(SHELL) << "Running" << formatter->name() << "on" << doc->url();

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
    const auto documents = ICore::self()->documentController()->openDocuments();
    for (KDevelop::IDocument* doc : documents) {
        adaptEditorIndentationMode(doc->textDocument(), formatterForUrl(doc->url()), doc->url());
    }
    updateFormatTextAction();
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

void SourceFormatterController::adaptEditorIndentationMode(KTextEditor::Document *doc, ISourceFormatter *formatter,
                                                           const QUrl& url, bool ignoreModeline)
{
    if (!formatter || !configForUrl(url).readEntry(SourceFormatterController::kateOverrideIndentationConfigKey(), false) || !doc)
        return;

    qCDebug(SHELL) << "adapting mode for" << url;

    QRegExp kateModelineWithNewline(QStringLiteral("\\s*\\n//\\s*kate:(.*)$"));

    // modelines should always take precedence
    if( !ignoreModeline && kateModelineWithNewline.indexIn( doc->text() ) != -1 )
    {
        qCDebug(SHELL) << "ignoring because a kate modeline was found";
        return;
    }

    ISourceFormatter::Indentation indentation = formatter->indentation(url);
    if(indentation.isValid())
    {
        struct CommandCaller {
            explicit CommandCaller(KTextEditor::Document* _doc) : doc(_doc), editor(KTextEditor::Editor::instance()) {
                Q_ASSERT(editor);
            }
            void operator()(const QString& cmd) {
                KTextEditor::Command* command = editor->queryCommand( cmd );
                Q_ASSERT(command);
                QString msg;
                qCDebug(SHELL) << "calling" << cmd;
                const auto views = doc->views();
                for (KTextEditor::View* view : views) {
                    if (!command->exec(view, cmd, msg))
                        qCWarning(SHELL) << "setting indentation width failed: " << msg;
                }
            }

            KTextEditor::Document* doc;
            KTextEditor::Editor* editor;
        } call(doc);

        if( indentation.indentWidth ) // We know something about indentation-width
            call( QStringLiteral("set-indent-width %1").arg(indentation.indentWidth ) );

        if( indentation.indentationTabWidth != 0 ) // We know something about tab-usage
        {
            call( QStringLiteral("set-replace-tabs %1").arg( (indentation.indentationTabWidth == -1) ? 1 : 0 ) );
            if( indentation.indentationTabWidth > 0 )
                call( QStringLiteral("set-tab-width %1").arg(indentation.indentationTabWidth ) );
        }
    }else{
        qCDebug(SHELL) << "found no valid indentation";
    }
}

void SourceFormatterController::formatFiles()
{
    Q_D(SourceFormatterController);

    if (d->prjItems.isEmpty() && d->urls.isEmpty())
        return;

    //get a list of all files in this folder recursively
    QList<KDevelop::ProjectFolderItem*> folders;
    for (KDevelop::ProjectBaseItem* item : qAsConst(d->prjItems)) {
        if (!item)
            continue;
        if (item->folder())
            folders.append(item->folder());
        else if (item->file())
            d->urls.append(item->file()->path().toUrl());
        else if (item->target()) {
            const auto files = item->fileList();
            for (KDevelop::ProjectFileItem* f : files) {
                d->urls.append(f->path().toUrl());
            }
        }
    }

    while (!folders.isEmpty()) {
        KDevelop::ProjectFolderItem *item = folders.takeFirst();
        const auto folderList = item->folderList();
        for (KDevelop::ProjectFolderItem* f : folderList) {
            folders.append(f);
        }
        const auto targets = item->targetList();
        for (KDevelop::ProjectTargetItem* f : targets) {
            const auto childs = f->fileList();
            for (KDevelop::ProjectFileItem* child : childs) {
                d->urls.append(child->path().toUrl());
            }
        }
        const auto files = item->fileList();
        for (KDevelop::ProjectFileItem* f : files) {
            d->urls.append(f->path().toUrl());
        }
    }

    auto win = ICore::self()->uiController()->activeMainWindow()->window();

    QMessageBox msgBox(QMessageBox::Question, i18nc("@title:window", "Reformat Files?"),
                       i18n("Reformat all files in the selected folder?"),
                       QMessageBox::Ok|QMessageBox::Cancel, win);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    auto okButton = msgBox.button(QMessageBox::Ok);
    okButton->setText(i18nc("@action:button", "Reformat"));
    msgBox.exec();

    if (msgBox.clickedButton() == okButton) {
        auto formatterJob = new SourceFormatterJob(this);
        formatterJob->setFiles(d->urls);
        ICore::self()->runController()->registerJob(formatterJob);
    }
}

KDevelop::ContextMenuExtension SourceFormatterController::contextMenuExtension(KDevelop::Context* context, QWidget* parent)
{
    Q_D(SourceFormatterController);

    Q_UNUSED(parent);

    KDevelop::ContextMenuExtension ext;
    d->urls.clear();
    d->prjItems.clear();

    if (d->sourceFormatters.isEmpty()) {
        return ext;
    }

    if (context->hasType(KDevelop::Context::EditorContext))
    {
        if (d->formatTextAction->isEnabled())
            ext.addAction(KDevelop::ContextMenuExtension::EditGroup, d->formatTextAction);
    } else if (context->hasType(KDevelop::Context::FileContext)) {
        auto* filectx = static_cast<KDevelop::FileContext*>(context);
        d->urls = filectx->urls();
        ext.addAction(KDevelop::ContextMenuExtension::EditGroup, d->formatFilesAction);
    } else if (context->hasType(KDevelop::Context::CodeContext)) {
    } else if (context->hasType(KDevelop::Context::ProjectItemContext)) {
        auto* prjctx = static_cast<KDevelop::ProjectItemContext*>(context);
        d->prjItems = prjctx->items();
        if (!d->prjItems.isEmpty()) {
            ext.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, d->formatFilesAction);
        }
    }
    return ext;
}

SourceFormatterStyle SourceFormatterController::styleForUrl(const QUrl& url, const QMimeType& mime)
{
    const QString formatter = configForUrl(url).readEntry(mime.name(), QString());
    const int pos = formatter.indexOf(QLatin1String("||"));
    if ((pos <= 0) || (pos + 2 >= formatter.size())) {
        return SourceFormatterStyle();
    }

    const QString formatterName = formatter.left(pos);
    const QString styleName = formatter.mid(pos + 2);

    SourceFormatterStyle s(styleName);
    KConfigGroup fmtgrp = globalConfig().group(formatterName);
    if (fmtgrp.hasGroup(styleName)) {
        KConfigGroup stylegrp = fmtgrp.group(styleName);
        populateStyleFromConfigGroup(&s, stylegrp);
    }
    return s;
}

void SourceFormatterController::disableSourceFormatting(bool disable)
{
    Q_D(SourceFormatterController);

    d->enabled = !disable;
}

bool SourceFormatterController::sourceFormattingEnabled()
{
    Q_D(SourceFormatterController);

    return d->enabled;
}

bool SourceFormatterController::hasFormatters() const
{
    Q_D(const SourceFormatterController);

    return !d->sourceFormatters.isEmpty();
}

QVector<ISourceFormatter*> SourceFormatterController::formatters() const
{
    Q_D(const SourceFormatterController);

    return d->sourceFormatters;
}

void SourceFormatterController::resetUi()
{
    Q_D(SourceFormatterController);

    d->formatFilesAction->setEnabled(!d->sourceFormatters.isEmpty());

    updateFormatTextAction();
}

}
