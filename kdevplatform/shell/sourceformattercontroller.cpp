/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Cédric Pasteur <cedric.pasteur@free.fr>
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sourceformattercontroller.h"

#include <QAction>
#include <QAbstractButton>
#include <QByteArray>
#include <QMimeDatabase>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QStringView>
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
#include <util/owningrawpointercontainer.h>

#include "core.h"
#include "debug.h"
#include "plugincontroller.h"
#include "sourceformatterconfig.h"
#include "sourceformatterjob.h"
#include "textdocument.h"

#include <algorithm>
#include <memory>
#include <tuple>
#include <utility>

using namespace KDevelop;

namespace {
namespace Config {
namespace Strings {
QString sourceFormatter()
{
    return QStringLiteral("SourceFormatter");
}
constexpr const char* useDefault = "UseDefault";
}

KConfigGroup projectConfig(const IProject& project)
{
    return project.projectConfiguration()->group(Strings::sourceFormatter());
}

KConfigGroup sessionConfig()
{
    return Core::self()->activeSession()->config()->group(Strings::sourceFormatter());
}

KConfigGroup globalConfig()
{
    return KSharedConfig::openConfig()->group(Strings::sourceFormatter());
}

bool projectOverridesSession(const KConfigGroup& projectConfig)
{
    return projectConfig.isValid() && !projectConfig.readEntry(Strings::useDefault, true);
}

KConfigGroup configForUrl(const QUrl& url)
{
    const auto* const project = Core::self()->projectController()->findProjectForUrl(url);
    if (project) {
        auto config = projectConfig(*project);
        if (projectOverridesSession(config)) {
            return config;
        }
    }
    return sessionConfig();
}

void populateStyleFromConfig(SourceFormatterStyle& style, const KConfigGroup& styleConfig)
{
    style.setCaption(styleConfig.readEntry(SourceFormatterController::styleCaptionKey(), QString{}));
    style.setUsePreview(styleConfig.readEntry(SourceFormatterController::styleShowPreviewKey(), false));
    style.setContent(styleConfig.readEntry(SourceFormatterController::styleContentKey(), QString{}));
    style.setMimeTypes(
        styleConfig.readEntry<QStringList>(SourceFormatterController::styleMimeTypesKey(), QStringList{}));
    style.setOverrideSample(styleConfig.readEntry(SourceFormatterController::styleSampleKey(), QString{}));
}

struct FormatterData
{
    const ISourceFormatter* formatter = nullptr;
    QString styleName;

    bool isValid() const
    {
        return formatter;
    }

    SourceFormatterStyle style() const
    {
        Q_ASSERT(formatter);
        SourceFormatterStyle style(styleName);
        const KConfigGroup config = globalConfig().group(formatter->name());
        if (config.hasGroup(styleName)) {
            populateStyleFromConfig(style, config.group(styleName));
        }
        return style;
    }
};

FormatterData readFormatterData(const KConfigGroup& sourceFormatterConfig, const QString& mimeTypeName,
                                const QVector<ISourceFormatter*>& formatters)
{
    FormatterData result{};

    SourceFormatter::ConfigForMimeType parser(sourceFormatterConfig, mimeTypeName);
    if (!parser.isValid()) {
        return result;
    }

    const auto it = std::find_if(formatters.cbegin(), formatters.cend(),
                                 [formatterName = parser.formatterName()](const ISourceFormatter* f) {
                                     return f->name() == formatterName;
                                 });
    if (it != formatters.cend()) {
        result.formatter = *it;
        result.styleName = std::move(parser).takeStyleName();
    }

    return result;
}

} // namespace Config
} // unnamed namespace

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
};

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
    connect(Core::self()->documentController(), &IDocumentController::documentUrlChanged, this,
            &SourceFormatterController::updateFormatTextAction);

    qRegisterMetaType<QPointer<KDevelop::TextDocument>>();
    connect(Core::self()->documentController(), &IDocumentController::documentLoaded,
            // Use a queued connection, because otherwise the view is not yet fully set up
            // but wrap the document in a smart pointer to guard against crashes when it
            // gets deleted in the meantime
            this, [this](IDocument *doc) {
                const auto textDoc = QPointer<TextDocument>(dynamic_cast<TextDocument*>(doc));
                QMetaObject::invokeMethod(this, "documentLoaded", Qt::QueuedConnection, Q_ARG(QPointer<KDevelop::TextDocument>, textDoc));
            });
    connect(Core::self()->projectController(), &IProjectController::projectOpened, this, [d](const IProject* project) {
        FileFormatter::projectOpened(*project, d->sourceFormatters);
    });

    updateFormatTextAction();
}

void SourceFormatterController::documentLoaded(const QPointer<TextDocument>& doc)
{
    Q_D(const SourceFormatterController);

    // NOTE: explicitly check this here to prevent crashes on shutdown
    //       when this slot gets called (note: delayed connection)
    //       but the text document was already destroyed
    //       there have been unit tests that failed due to that...
    if (!doc || !doc->textDocument()) {
        return;
    }
    FileFormatter ff(doc->url());
    if (ff.readFormatterAndStyle(d->sourceFormatters)) {
        ff.adaptEditorIndentationMode(doc->textDocument());
    }
}

void SourceFormatterController::FileFormatter::projectOpened(const IProject& project,
                                                             const QVector<ISourceFormatter*>& formatters)
{
    // Adapt the indentation mode if a project was just opened. Otherwise if a document
    // is loaded before its project, it might not have the correct indentation mode set.

    if (formatters.empty()) {
        return;
    }

    const auto config = Config::projectConfig(project);
    if (!Config::projectOverridesSession(config)) {
        return; // The opened project does not specify indentation => nothing to do.
    }

    OwningRawPointerContainer<QHash<QString, FileFormatter*>> fileFormatterCache;
    const auto fileFormatterForUrl = [&fileFormatterCache, &config, &formatters](QUrl&& url) {
        auto mimeType = QMimeDatabase().mimeTypeForUrl(url);
        const auto mimeTypeName = mimeType.name();

        auto ff = fileFormatterCache->value(mimeTypeName);
        if (ff) {
            ff->m_url = std::move(url);
            Q_ASSERT_X(ff->m_mimeType == mimeType, Q_FUNC_INFO,
                       "When MIME type names are equal, the MIME types must also compare equal.");
            // All other ff's data members already have correct values:
            // * m_sourceFormatterConfig equals config for all elements of fileFormatterCache;
            // * m_formatter and m_style are determined by config and m_mimeType.
        } else {
            const auto data = Config::readFormatterData(config, mimeTypeName, formatters);
            if (data.isValid()) {
                ff = new FileFormatter(std::move(url), std::move(mimeType), config, data.formatter, data.style());
            }
            fileFormatterCache->insert(mimeTypeName, ff);
        }
        return std::as_const(ff);
    };

    const auto documents = ICore::self()->documentController()->openDocuments();
    for (const IDocument* doc : documents) {
        auto url = doc->url();
        if (!project.inProject(IndexedString{url})) {
            continue;
        }
        if (const auto* const ff = fileFormatterForUrl(std::move(url))) {
            ff->adaptEditorIndentationMode(doc->textDocument());
        }
    }
}

void SourceFormatterController::pluginLoaded(IPlugin* plugin)
{
    Q_D(SourceFormatterController);

    auto* sourceFormatter = plugin->extension<ISourceFormatter>();

    if (!sourceFormatter || !d->enabled) {
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

    if (!sourceFormatter || !d->enabled) {
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

KConfigGroup SourceFormatterController::sessionConfig() const
{
    return Config::sessionConfig();
}

KConfigGroup SourceFormatterController::globalConfig() const
{
    return Config::globalConfig();
}

auto SourceFormatterController::stylesForFormatter(const ISourceFormatter& formatter) const -> StyleMap
{
    StyleMap styles;

    const auto predefinedStyles = formatter.predefinedStyles();
    for (const auto& style : predefinedStyles) {
        const auto [it, inserted] = styles.try_emplace(style.name(), style);
        Q_ASSERT(it->second.name() == it->first);
        Q_ASSERT_X(inserted, Q_FUNC_INFO, "Duplicate predefined style!");
    }

    const auto commonConfig = globalConfig();
    const QString formatterKey = formatter.name();
    if (commonConfig.hasGroup(formatterKey)) {
        const auto formatterConfig = commonConfig.group(formatterKey);
        const auto subgroups = formatterConfig.groupList();
        for (const QString& subgroup : subgroups) {
            const auto [it, inserted] = styles.insert_or_assign(subgroup, SourceFormatterStyle{subgroup});
            Q_ASSERT(it->second.name() == it->first);
            if (!inserted) {
                // This overriding is an undocumented and possibly unintentional feature, which has existed
                // for more than 10 years. Source Formatter configuration UI creates styles named "UserN",
                // which cannot override predefined styles. But if the user edits kdeveloprc manually and
                // renames a style, it correctly overrides the predefined style for all intents and purposes.
                qCDebug(SHELL).noquote() << QStringLiteral(
                                                "A user-defined style config group [%1][%2][%3] "
                                                "overrides a predefined style with the same name.")
                                                .arg(Config::Strings::sourceFormatter(), formatterKey, subgroup);
            }
            Config::populateStyleFromConfig(it->second, formatterConfig.group(subgroup));
        }
    }

    return styles;
}

SourceFormatterController::FileFormatter::FileFormatter(QUrl url)
    : m_url{std::move(url)}
    , m_mimeType{QMimeDatabase().mimeTypeForUrl(m_url)}
{
}

SourceFormatterController::FileFormatter::FileFormatter(QUrl&& url, QMimeType&& mimeType,
                                                        const KConfigGroup& sourceFormatterConfig,
                                                        const ISourceFormatter* formatter, SourceFormatterStyle&& style)
    : m_url{std::move(url)}
    , m_mimeType{std::move(mimeType)}
    , m_sourceFormatterConfig{sourceFormatterConfig}
    , m_formatter{formatter}
    , m_style{std::move(style)}
{
}

bool SourceFormatterController::FileFormatter::readFormatterAndStyle(const QVector<ISourceFormatter*>& formatters)
{
    Q_ASSERT_X(!m_sourceFormatterConfig.isValid() && !m_formatter && m_style.name().isEmpty(), Q_FUNC_INFO,
               "This reinitialization must be a mistake.");

    if (formatters.empty()) {
        return false;
    }

    m_sourceFormatterConfig = Config::configForUrl(m_url);

    const auto data = Config::readFormatterData(m_sourceFormatterConfig, m_mimeType.name(), formatters);
    if (!data.isValid()) {
        return false;
    }

    m_formatter = data.formatter;
    m_style = data.style();
    return true;
}

QString SourceFormatterController::FileFormatter::formatterCaption() const
{
    Q_ASSERT(m_formatter);
    return m_formatter->caption();
}

QString SourceFormatterController::FileFormatter::styleCaption() const
{
    Q_ASSERT(m_formatter);
    auto styleCaption = m_style.caption();
    if (styleCaption.isEmpty()) {
        // This could be an incomplete predefined style, for which only the name is stored in config.
        styleCaption = m_formatter->predefinedStyle(m_style.name()).caption();
    }
    return styleCaption;
}

QString SourceFormatterController::FileFormatter::format(const QString& text, const QString& leftContext,
                                                         const QString& rightContext) const
{
    Q_ASSERT(m_formatter);
    return m_formatter->formatSourceWithStyle(m_style, text, m_url, m_mimeType, leftContext, rightContext);
}

/**
 * @return the name of kate indentation mode for @p mime, e.g. "cstyle", "python"
 */
static QString indentationMode(const QMimeType& mime)
{
    if (mime.inherits(QStringLiteral("text/x-c++src")) || mime.inherits(QStringLiteral("text/x-chdr")) ||
        mime.inherits(QStringLiteral("text/x-c++hdr")) || mime.inherits(QStringLiteral("text/x-csrc")) ||
        mime.inherits(QStringLiteral("text/x-java")) || mime.inherits(QStringLiteral("text/x-csharp"))) {
        return QStringLiteral("cstyle");
    }
    return QStringLiteral("none");
}

QString SourceFormatterController::FileFormatter::addModeline(QString input) const
{
    Q_ASSERT(m_formatter);

    QRegExp kateModelineWithNewline(QStringLiteral("\\s*\\n//\\s*kate:(.*)$"));

    // If there already is a modeline in the document, adapt it while formatting, even
    // if "add modeline" is disabled.
    if (!m_sourceFormatterConfig.readEntry(SourceFormatterController::kateModeLineConfigKey(), false)
        && kateModelineWithNewline.indexIn(input) == -1)
        return input;

    const auto indentation = m_formatter->indentation(m_style, m_url, m_mimeType);
    if( !indentation.isValid() )
        return input;

    QString output;
    QTextStream os(&output, QIODevice::WriteOnly);
    QTextStream is(&input, QIODevice::ReadOnly);

    QString modeline(QStringLiteral("// kate: ") + QLatin1String("indent-mode ") + indentationMode(m_mimeType)
                     + QLatin1String("; "));

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
    Q_D(const SourceFormatterController);

    const auto [enabled, tool, style] = [d] {
        auto disabled = std::tuple{false, QString(), QString()};

        const auto* doc = KDevelop::ICore::self()->documentController()->activeDocument();
        if (!doc) {
            return disabled;
        }

        FileFormatter ff(doc->url());
        if (!ff.readFormatterAndStyle(d->sourceFormatters)) {
            return disabled;
        }

        return std::tuple{true, ff.formatterCaption(), ff.styleCaption()};
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
    Q_D(const SourceFormatterController);

    IDocument* idoc = KDevelop::ICore::self()->documentController()->activeDocument();
    if (!idoc)
        return;
    KTextEditor::View* view = idoc->activeTextView();
    if (!view)
        return;
    KTextEditor::Document* doc = view->document();
    // load the appropriate formatter
    const auto url = idoc->url();
    FileFormatter ff(url);
    if (!ff.readFormatterAndStyle(d->sourceFormatters)) {
        qCDebug(SHELL) << "no formatter available for" << url;
        return;
    }

    // Ignore the modeline, as the modeline will be changed anyway
    ff.adaptEditorIndentationMode(doc, true);

    bool has_selection = view->selection();

    if (has_selection) {
        QString original = view->selectionText();

        QString output =
            ff.format(view->selectionText(),
                      doc->text(KTextEditor::Range(KTextEditor::Cursor(0, 0), view->selectionRange().start())),
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
        ff.formatDocument(*idoc);
    }
}

void SourceFormatterController::beautifyLine()
{
    Q_D(const SourceFormatterController);

    KDevelop::IDocumentController *docController = KDevelop::ICore::self()->documentController();
    KDevelop::IDocument *doc = docController->activeDocument();
    if (!doc)
        return;
    KTextEditor::Document *tDoc = doc->textDocument();
    if (!tDoc)
        return;
    KTextEditor::View* view = doc->activeTextView();
    if (!view)
        return;
    // load the appropriate formatter
    const auto url = doc->url();
    FileFormatter ff(url);
    if (!ff.readFormatterAndStyle(d->sourceFormatters)) {
        qCDebug(SHELL) << "no formatter available for" << url;
        return;
    }

    const KTextEditor::Cursor cursor = view->cursorPosition();
    const QString line = tDoc->line(cursor.line());
    const QString prev = tDoc->text(KTextEditor::Range(0, 0, cursor.line(), 0));
    const QString post = QLatin1Char('\n') + tDoc->text(KTextEditor::Range(KTextEditor::Cursor(cursor.line() + 1, 0), tDoc->documentEnd()));

    const QString formatted = ff.format(line, prev, post);

    // We don't use KTextEditor::Document directly, because CodeRepresentation transparently works
    // around a possible tab-replacement incompatibility between kate and kdevelop
    DynamicCodeRepresentation::Ptr code(dynamic_cast<DynamicCodeRepresentation*>( KDevelop::createCodeRepresentation( IndexedString( doc->url() ) ).data() ) );
    Q_ASSERT( code );
    code->replace( KTextEditor::Range(cursor.line(), 0, cursor.line(), line.length()), line, formatted );

    // advance cursor one line
    view->setCursorPosition(KTextEditor::Cursor(cursor.line() + 1, 0));
}

void SourceFormatterController::FileFormatter::formatDocument(IDocument& doc) const
{
    Q_ASSERT(m_formatter);
    Q_ASSERT(doc.url() == m_url);

    qCDebug(SHELL) << "Running" << m_formatter->name() << "on" << m_url;

    // We don't use KTextEditor::Document directly, because CodeRepresentation transparently works
    // around a possible tab-replacement incompatibility between kate and kdevelop
    auto code = KDevelop::createCodeRepresentation(IndexedString{m_url});

    const auto cursor = doc.cursorPosition();

    QString text = format(code->text());
    text = addModeline(text);
    code->setText(text);

    doc.setCursorPosition(cursor);
}

void SourceFormatterController::settingsChanged()
{
    Q_D(const SourceFormatterController);

    const auto documents = ICore::self()->documentController()->openDocuments();
    for (KDevelop::IDocument* doc : documents) {
        FileFormatter ff(doc->url());
        if (ff.readFormatterAndStyle(d->sourceFormatters)) {
            ff.adaptEditorIndentationMode(doc->textDocument());
        }
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

void SourceFormatterController::FileFormatter::adaptEditorIndentationMode(KTextEditor::Document* doc,
                                                                          bool ignoreModeline) const
{
    Q_ASSERT(m_formatter);
    if (!doc
        || !m_sourceFormatterConfig.readEntry(SourceFormatterController::kateOverrideIndentationConfigKey(), false))
        return;

    qCDebug(SHELL) << "adapting mode for" << m_url;

    QRegExp kateModelineWithNewline(QStringLiteral("\\s*\\n//\\s*kate:(.*)$"));

    // modelines should always take precedence
    if( !ignoreModeline && kateModelineWithNewline.indexIn( doc->text() ) != -1 )
    {
        qCDebug(SHELL) << "ignoring because a kate modeline was found";
        return;
    }

    const auto indentation = m_formatter->indentation(m_style, m_url, m_mimeType);
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
    for (KDevelop::ProjectBaseItem* item : std::as_const(d->prjItems)) {
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

auto SourceFormatterController::fileFormatter(const QUrl& url) const -> FileFormatterPtr
{
    Q_D(const SourceFormatterController);

    auto ff = std::make_unique<FileFormatter>(url);
    if (ff->readFormatterAndStyle(d->sourceFormatters)) {
        return ff;
    }
    return nullptr;
}

void SourceFormatterController::disableSourceFormatting()
{
    Q_D(SourceFormatterController);

    d->enabled = false;

    if (d->sourceFormatters.empty()) {
        return;
    }

    decltype(d->sourceFormatters) loadedFormatters{};
    d->sourceFormatters.swap(loadedFormatters);

    resetUi();

    for (auto* formatter : std::as_const(loadedFormatters)) {
        emit formatterUnloading(formatter);
    }

    Q_ASSERT(!loadedFormatters.empty());
    Q_ASSERT(d->sourceFormatters.empty());
    emit hasFormattersChanged(false);
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

#include "moc_sourceformattercontroller.cpp"
