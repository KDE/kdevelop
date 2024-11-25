/*
    SPDX-FileCopyrightText: 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>
    SPDX-FileCopyrightText: 1999-2001 the KDevelop Team
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>
    SPDX-FileCopyrightText: 2010 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grepdialog.h"

#include <algorithm>

#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QShowEvent>
#include <QStringList>
#include <QTimer>

#include <KComboBox>
#include <KCompletion>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KUrlCompletion>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include <util/algorithm.h>
#include <util/path.h>
#include <util/wildcardhelpers.h>

#include "debug.h"
#include "grepviewplugin.h"
#include "grepoutputview.h"
#include "grepfindthread.h"
#include "greputil.h"

#include <utility>

using namespace KDevelop;

namespace {

inline QString allOpenFilesString() { return i18nc("@item:inlistbox", "All Open Files"); }
inline QString allOpenProjectsString() { return i18nc("@item:inlistbox", "All Open Projects"); }

inline QStringList template_desc()
{
    return QStringList{
        QStringLiteral("verbatim"),
        QStringLiteral("word"),
        QStringLiteral("assignment"),
        QStringLiteral("->MEMBER("),
        QStringLiteral("class::MEMBER("),
        QStringLiteral("OBJECT->member("),
    };
}

inline QStringList template_str()
{
    return QStringList{
        QStringLiteral("%s"),
        QStringLiteral("\\b%s\\b"),
        QStringLiteral("\\b%s\\b\\s*=[^=]"),
        QStringLiteral("\\->\\s*\\b%s\\b\\s*\\("),
        QStringLiteral("([a-z0-9_$]+)\\s*::\\s*\\b%s\\b\\s*\\("),
        QStringLiteral("\\b%s\\b\\s*\\->\\s*([a-z0-9_$]+)\\s*\\("),
    };
}

inline QStringList repl_template()
{
    return QStringList{
        QStringLiteral("%s"),
        QStringLiteral("%s"),
        QStringLiteral("%s = "),
        QStringLiteral("->%s("),
        QStringLiteral("\\1::%s("),
        QStringLiteral("%s->\\1("),
    };
}

inline QStringList filepatterns()
{
    return QStringList{
        QStringLiteral("*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.cuh,*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.idl,*.c,*.cu,*.m,*.mm,*.M,*.y,*.ypp,*.yxx,*.y++,*.l,*.txt,*.xml,*.rc"),
        QStringLiteral("*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.c,*.cu,*.m,*.mm,*.M"),
        QStringLiteral("*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.cuh,*.idl"),
        QStringLiteral("*.adb"),
        QStringLiteral("*.cs"),
        QStringLiteral("*.f"),
        QStringLiteral("*.html,*.htm"),
        QStringLiteral("*.hs"),
        QStringLiteral("*.java"),
        QStringLiteral("*.js"),
        QStringLiteral("*.php,*.php3,*.php4"),
        QStringLiteral("*.pl"),
        QStringLiteral("*.pp,*.pas"),
        QStringLiteral("*.py"),
        QStringLiteral("*.js,*.css,*.yml,*.rb,*.rhtml,*.html.erb,*.rjs,*.js.rjs,*.rxml,*.xml.builder"),
        QStringLiteral("CMakeLists.txt,*.cmake"),
        QStringLiteral("*"),
    };
}

inline QStringList excludepatterns()
{
    return QStringList{
        QStringLiteral("/CVS/,/SCCS/,/.svn/,/_darcs/,/build/,/.git/"),
        QString(),
    };
}

KConfigGroup dialogConfigGroup()
{
    return ICore::self()->activeSession()->config()->group(QStringLiteral("GrepDialog"));
}

class DialogConfigReader
{
public:
    DialogConfigReader()
        : m_config{dialogConfigGroup()}
    {
    }

    QStringList patternList() const
    {
        return m_config.readEntry("LastSearchItems", QStringList{});
    }
    int templateIndex() const
    {
        return m_config.readEntry("LastUsedTemplateIndex", 0);
    }
    QStringList templateStringList() const
    {
        return m_config.readEntry("LastUsedTemplateString", template_str());
    }
    QStringList replacementTemplateStringList() const
    {
        return m_config.readEntry("LastUsedReplacementTemplateString", repl_template());
    }
    bool isRegex() const
    {
        return m_config.readEntry("regexp", false);
    }
    bool isCaseSensitive() const
    {
        return m_config.readEntry("case_sens", true);
    }
    QStringList searchPathsList(const GrepViewPlugin& plugin) const
    {
        const bool isAnyProjectOpen = plugin.core()->projectController()->projectCount() != 0;
        return m_config.readEntry("SearchPaths",
                                  QStringList{isAnyProjectOpen ? allOpenProjectsString() : QDir::homePath()});
    }
    int depth() const
    {
        return m_config.readEntry("depth", -1);
    }
    bool limitToProjectFiles() const
    {
        return m_config.readEntry("search_project_files", true);
    }
    QStringList filePatternsList() const
    {
        return m_config.readEntry("file_patterns", filepatterns());
    }
    QStringList excludePatternsList() const
    {
        return m_config.readEntry("exclude_patterns", excludepatterns());
    }

private:
    KConfigGroup m_config;
};

QString searchTemplateFromTemplateString(const QString& templateString)
{
    return templateString.isEmpty() ? QStringLiteral("%s") : templateString;
}

///Separator used to separate search paths.
inline QString pathsSeparator() { return (QStringLiteral(";")); }

///Returns the chosen directories or files (only the top directories, not subfiles)
QList<QUrl> getDirectoryChoice(const QString& text)
{
    QList<QUrl> ret;
    if (text == allOpenFilesString()) {
        const auto openDocuments = ICore::self()->documentController()->openDocuments();
        ret.reserve(openDocuments.size());
        for (auto* doc : openDocuments) {
            ret << doc->url();
        }
    } else if (text == allOpenProjectsString()) {
        const auto projects = ICore::self()->projectController()->projects();
        ret.reserve(projects.size());
        for (auto* project : projects) {
            ret << project->path().toUrl();
        }
    } else {
        const QStringList semicolonSeparatedFileList = text.split(pathsSeparator(), Qt::SkipEmptyParts);
        if (!semicolonSeparatedFileList.isEmpty() && QFileInfo::exists(semicolonSeparatedFileList[0])) {
            // We use QFileInfo to make sure this is really a semicolon-separated file list, not a file containing
            // a semicolon in the name.
            ret.reserve(semicolonSeparatedFileList.size());
            for (const QString& file : semicolonSeparatedFileList) {
                ret << QUrl::fromLocalFile(file).adjusted(QUrl::StripTrailingSlash | QUrl::NormalizePathSegments);
            }
        } else {
            auto url = QUrl::fromUserInput(text).adjusted(QUrl::StripTrailingSlash | QUrl::NormalizePathSegments);
            if (!url.isEmpty()) {
                ret.push_back(std::move(url));
            }
        }
    }
    return ret;
}

///Check if all directories are part of a project
bool directoriesInProject(const QString& dir)
{
    const auto urls = getDirectoryChoice(dir);
    return std::all_of(urls.begin(), urls.end(), [&](const QUrl& url) {
        IProject *proj = ICore::self()->projectController()->findProjectForUrl(url);
        return (proj && proj->path().toUrl().isLocalFile());
    });
}

///Max number of items in paths combo box.
const int pathsMaxCount = 25;
}

GrepDialog::GrepDialog(GrepViewPlugin* plugin, GrepOutputView* toolView, QWidget* parent, bool show)
    : QDialog(parent)
    , Ui::GrepWidget()
    , m_plugin(plugin)
    , m_toolView(toolView)
    , m_show(show)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // if we don't intend on showing the dialog, we can skip all UI setup
    if (!m_show) {
        return;
    }

    setWindowTitle(i18nc("@title:window", "Find/Replace in Files"));

    setupUi(this);
    patternCombo->lineEdit()->setClearButtonEnabled(true);
    adjustSize();

    auto searchButton = buttonBox->button(QDialogButtonBox::Ok);
    Q_ASSERT(searchButton);
    searchButton->setText(i18nc("@action:button", "Search..."));
    searchButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    connect(searchButton, &QPushButton::clicked, this, &GrepDialog::startSearch);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GrepDialog::reject);

    const DialogConfigReader configReader;

    patternCombo->addItems(configReader.patternList());
    patternCombo->setInsertPolicy(QComboBox::InsertAtTop);
    patternCombo->setCompleter(nullptr);

    templateTypeCombo->addItems(template_desc());
    templateTypeCombo->setCurrentIndex(configReader.templateIndex());
    templateEdit->addItems(configReader.templateStringList());
    templateEdit->setEditable(true);
    templateEdit->setCompletionMode(KCompletion::CompletionPopup);
    KCompletion* comp = templateEdit->completionObject();
    connect(templateEdit, QOverload<const QString&>::of(&KComboBox::returnPressed),
            comp, QOverload<const QString&>::of(&KCompletion::addItem));
    for(int i=0; i<templateEdit->count(); i++)
        comp->addItem(templateEdit->itemText(i));
    replacementTemplateEdit->addItems(configReader.replacementTemplateStringList());
    replacementTemplateEdit->setEditable(true);
    replacementTemplateEdit->setCompletionMode(KCompletion::CompletionPopup);
    comp = replacementTemplateEdit->completionObject();
    connect(replacementTemplateEdit, QOverload<const QString&>::of(&KComboBox::returnPressed),
            comp, QOverload<const QString&>::of(&KCompletion::addItem));
    for(int i=0; i<replacementTemplateEdit->count(); i++)
        comp->addItem(replacementTemplateEdit->itemText(i));

    regexCheck->setChecked(configReader.isRegex());

    caseSensitiveCheck->setChecked(configReader.isCaseSensitive());

    searchPaths->setCompletionObject(new KUrlCompletion());
    searchPaths->setAutoDeleteCompletionObject(true);
    searchPaths->addItems(configReader.searchPathsList(*m_plugin));
    searchPaths->setInsertPolicy(QComboBox::InsertAtTop);

    syncButton->setIcon(QIcon::fromTheme(QStringLiteral("dirsync")));
    syncButton->setMenu(createSyncButtonMenu());

    depthSpin->setValue(configReader.depth());
    limitToProjectCheck->setChecked(configReader.limitToProjectFiles());

    filesCombo->addItems(configReader.filePatternsList());
    excludeCombo->addItems(configReader.excludePatternsList());

    connect(templateTypeCombo, QOverload<int>::of(&KComboBox::activated),
            this, &GrepDialog::templateTypeComboActivated);
    connect(patternCombo, &QComboBox::editTextChanged,
            this, &GrepDialog::patternComboEditTextChanged);
    patternComboEditTextChanged( patternCombo->currentText() );
    patternCombo->setFocus();

    connect(searchPaths, &KComboBox::textActivated, this, &GrepDialog::setSearchLocations);

    directorySelector->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(directorySelector, &QPushButton::clicked, this, &GrepDialog::selectDirectoryDialog );

    Q_ASSERT(searchPaths->lineEdit());
    connect(searchPaths->lineEdit(), &QLineEdit::editingFinished, this, &GrepDialog::updateLimitToProjectEnabled);
    updateLimitToProjectEnabled();
}

void GrepDialog::selectDirectoryDialog()
{
    const QString dirName = QFileDialog::getExistingDirectory(
        this, i18nc("@title:window", "Select Directory to Search in"),
        searchPaths->lineEdit()->text());
    if (!dirName.isEmpty()) {
        setSearchLocations(dirName);
    }
}

void GrepDialog::addUrlToMenu(QMenu* menu, const QUrl& url)
{
    QAction* action = menu->addAction(m_plugin->core()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain));
    action->setData(QVariant(url.toString(QUrl::PreferLocalFile)));
    connect(action, &QAction::triggered, this, &GrepDialog::synchronizeDirActionTriggered);
}

void GrepDialog::addStringToMenu(QMenu* menu, const QString& string)
{
    QAction* action = menu->addAction(string);
    action->setData(QVariant(string));
    connect(action, &QAction::triggered, this, &GrepDialog::synchronizeDirActionTriggered);
}

void GrepDialog::synchronizeDirActionTriggered(bool)
{
    auto* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    setSearchLocations(action->data().toString());
}

QMenu* GrepDialog::createSyncButtonMenu()
{
    auto* ret = new QMenu(this);

    QSet<Path> hadUrls;

    IDocument *doc = m_plugin->core()->documentController()->activeDocument();
    if ( doc )
    {
        Path url = Path(doc->url()).parent();

        // always add the current file's parent directory
        hadUrls.insert(url);
        addUrlToMenu(ret, url.toUrl());

        url = url.parent();

        while(m_plugin->core()->projectController()->findProjectForUrl(url.toUrl()))
        {
            if (!Algorithm::insert(hadUrls, url).inserted) {
                break;
            }
            addUrlToMenu(ret, url.toUrl());
            url = url.parent();
        }
    }

    QVector<QUrl> otherProjectUrls;
    const auto projects = m_plugin->core()->projectController()->projects();
    for (IProject* project : projects) {
        if (!hadUrls.contains(project->path())) {
            otherProjectUrls.append(project->path().toUrl());
        }
    }

    // sort the remaining project URLs alphabetically
    std::sort(otherProjectUrls.begin(), otherProjectUrls.end());
    for (const QUrl& url : std::as_const(otherProjectUrls)) {
        addUrlToMenu(ret, url);
    }

    ret->addSeparator();
    addStringToMenu(ret, allOpenFilesString());
    addStringToMenu(ret, allOpenProjectsString());
    return ret;
}

GrepDialog::~GrepDialog()
{
}

void GrepDialog::setLastUsedSettings()
{
    Q_ASSERT_X(!m_show, Q_FUNC_INFO, "Precondition");

    const auto currentItem = [](const QStringList& itemList) {
        return itemList.value(0); // the first item (if any) in the list stored in config is current
    };

    const DialogConfigReader configReader;

    m_settings.pattern = currentItem(configReader.patternList());
    m_settings.searchTemplate = searchTemplateFromTemplateString(currentItem(configReader.templateStringList()));
    m_settings.replacementTemplate = currentItem(configReader.replacementTemplateStringList());
    m_settings.regexp = configReader.isRegex();
    m_settings.caseSensitive = configReader.isCaseSensitive();
    m_settings.searchPaths = currentItem(configReader.searchPathsList(*m_plugin));
    m_settings.depth = configReader.depth();
    m_settings.projectFilesOnly = configReader.limitToProjectFiles() && directoriesInProject(m_settings.searchPaths);
    m_settings.files = currentItem(configReader.filePatternsList());
    m_settings.exclude = currentItem(configReader.excludePatternsList());
}

void GrepDialog::setVisible(bool visible)
{
    QDialog::setVisible(visible && m_show);
}

void GrepDialog::closeEvent(QCloseEvent* closeEvent)
{
    Q_UNUSED(closeEvent);

    if (!m_show) {
        return;
    }

    auto cg = dialogConfigGroup();
    // memorize the last patterns and paths
    cg.writeEntry("LastSearchItems", qCombo2StringList(patternCombo));
    cg.writeEntry("regexp", regexCheck->isChecked());
    cg.writeEntry("depth", depthSpin->value());
    cg.writeEntry("search_project_files", limitToProjectCheck->isChecked());
    cg.writeEntry("case_sens", caseSensitiveCheck->isChecked());
    cg.writeEntry("exclude_patterns", qCombo2StringList(excludeCombo));
    cg.writeEntry("file_patterns", qCombo2StringList(filesCombo));
    cg.writeEntry("LastUsedTemplateIndex", templateTypeCombo->currentIndex());
    cg.writeEntry("LastUsedTemplateString", qCombo2StringList(templateEdit));
    cg.writeEntry("LastUsedReplacementTemplateString", qCombo2StringList(replacementTemplateEdit));
    cg.writeEntry("SearchPaths", qCombo2StringList(searchPaths));
    cg.sync();
}

void GrepDialog::templateTypeComboActivated(int index)
{
    templateEdit->setCurrentItem( template_str().at(index), true );
    replacementTemplateEdit->setCurrentItem( repl_template().at(index), true );
}

void GrepDialog::setPattern(const QString& pattern)
{
    if (m_show) {
        patternCombo->setEditText(pattern);
        patternComboEditTextChanged(pattern);
    }
    m_settings.pattern = pattern;
}

void GrepDialog::historySearch(QList<GrepJobSettings>&& settingsHistory)
{
    Q_ASSERT(!settingsHistory.empty());
    m_historyJobSettings = std::move(settingsHistory);

    if (!checkProjectsOpened()) {
        connect(KDevelop::ICore::self()->projectController(),
                &KDevelop::IProjectController::projectOpened,
                this, &GrepDialog::checkProjectsOpened);
    }
}

void GrepDialog::setSearchLocations(const QString& dir)
{
    if (dir.isEmpty()) {
        return; // ignore an attempt to set invalid empty search location
    }

    if (!m_show) {
        m_settings.searchPaths = dir;
        return;
    }

    if (QDir::isAbsolutePath(dir)) {
        static_cast<KUrlCompletion*>(searchPaths->completionObject())->setDir(QUrl::fromLocalFile(dir));
    }

    if (searchPaths->contains(dir)) {
        searchPaths->removeItem(searchPaths->findText(dir));
    }

    searchPaths->insertItem(0, dir);
    searchPaths->setCurrentItem(dir);

    if (searchPaths->count() > pathsMaxCount) {
        searchPaths->removeItem(searchPaths->count() - 1);
    }

    updateLimitToProjectEnabled();
}

void GrepDialog::patternComboEditTextChanged( const QString& text)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

bool GrepDialog::checkProjectsOpened()
{
    // only proceed if all projects have been opened
    if (KDevelop::ICore::self()->activeSession()->config()->group(QStringLiteral("General Options")).readEntry("Open Projects", QList<QUrl>()).count() !=
        KDevelop::ICore::self()->projectController()->projects().count())
        return false;

    const auto projects = KDevelop::ICore::self()->projectController()->projects();
    for (IProject* p : projects) {
        if (!p->isReady())
            return false;
    }

    // do the grep jobs one by one
    connect(m_plugin, &GrepViewPlugin::grepJobFinished, this, &GrepDialog::nextHistory);
    QTimer::singleShot(0, this, [=]() {nextHistory(true);});

    return true;
}

void GrepDialog::nextHistory(bool next)
{
    if (next && !m_historyJobSettings.empty()) {
        m_settings = m_historyJobSettings.takeFirst();
        startSearch();
    } else {
        close();
    }
}

bool GrepDialog::isPartOfChoice(const QUrl& url) const
{
    const auto choices = getDirectoryChoice(m_settings.searchPaths);
    for (const QUrl& choice : choices) {
        if(choice.isParentOf(url) || choice == url)
            return true;
    }
    return false;
}

bool GrepDialog::saveSearchedDocuments() const
{
    const QStringList include = GrepFindFilesThread::parseInclude(m_settings.files);
    const QStringList exclude = GrepFindFilesThread::parseExclude(m_settings.exclude);

    // search for unsaved documents
    QList<IDocument*> unsavedFiles;
    const auto documents = ICore::self()->documentController()->openDocuments();
    for (IDocument* doc : documents) {
        const auto state = doc->state();
        if (state != IDocument::Modified && state != IDocument::DirtyAndModified) {
            continue; // no modifications to save
        }
        QUrl docUrl = doc->url();
        if (isPartOfChoice(docUrl) && QDir::match(include, docUrl.fileName())
            && !WildcardHelpers::match(exclude, docUrl.toLocalFile())) {
            unsavedFiles << doc;
        }
    }

    return unsavedFiles.empty() || ICore::self()->documentController()->saveSomeDocuments(unsavedFiles);
}

void GrepDialog::startSearch()
{
    // if m_show is false, all settings are fixed in m_settings
    if (m_show)
        updateSettings();

    if (!m_settings.fromHistory && !saveSearchedDocuments()) {
        close();
        return;
    }

    const QString descriptionOrUrl(m_settings.searchPaths);
    QList<QUrl> choice = getDirectoryChoice(descriptionOrUrl);
    QString description = descriptionOrUrl;

    // Shorten the description
    if(descriptionOrUrl != allOpenFilesString() && descriptionOrUrl != allOpenProjectsString()) {
        auto prettyFileName = [](const QUrl& url) {
            return ICore::self()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain);
        };

        if (choice.size() > 1) {
            description = i18np("%2, and %1 more item", "%2, and %1 more items", choice.size() - 1, prettyFileName(choice[0]));
        } else if (!choice.isEmpty()) {
            description = prettyFileName(choice[0]);
        }
    }

    GrepOutputView* toolView = m_toolView;
    if (!toolView) {
        toolView = static_cast<GrepOutputView*>(ICore::self()->uiController()->findToolView(
            i18nc("@title:window", "Find/Replace in Files"), m_plugin->toolViewFactory(),
            m_settings.fromHistory ? IUiController::Create : IUiController::CreateAndRaise));
        Q_ASSERT_X(toolView, Q_FUNC_INFO, "This branch may be taken only after UiController::loadAllAreas() returns.");
    }

    if (m_settings.fromHistory) {
        // when restored from history, only display the parameters
        toolView->renewModel(m_settings, i18nc("@item search result", "Search \"%1\" in %2", m_settings.pattern, description));
        emit m_plugin->grepJobFinished(true);
    } else {
        GrepOutputModel* outputModel =
            toolView->renewModel(m_settings,
                                i18nc("@item search result", "Search \"%1\" in %2 (at time %3)", m_settings.pattern, description,
                                    QTime::currentTime().toString(QStringLiteral("hh:mm"))));

        GrepJob* job = m_plugin->newGrepJob();
        // outputModel stores the messages and forwards them to toolView
        connect(job, &GrepJob::showMessage,
                outputModel, &GrepOutputModel::showMessageSlot);
        connect(job, &GrepJob::showErrorMessage, outputModel, &GrepOutputModel::showErrorMessageSlot);

        connect(toolView, &GrepOutputView::outputViewIsClosed, job, [=]() {job->kill();});

        qCDebug(PLUGIN_GREPVIEW) << "starting search with settings" << m_settings;
        job->setOutputModel(outputModel);
        job->setDirectoryChoice(choice);
        job->setSettings(m_settings);

        ICore::self()->runController()->registerJob(job);
    }

    m_plugin->rememberSearchDirectory(descriptionOrUrl);

    // If m_show is false, the dialog is closed somewhere else,
    // or not closed but still destroyed via deleteLater() in GrepViewPlugin::showDialog().
    if (m_show)
        close();
}

void GrepDialog::updateSettings()
{
    m_settings.projectFilesOnly = limitToProjectCheck->isEnabled() && limitToProjectCheck->isChecked();

    m_settings.caseSensitive = caseSensitiveCheck->isChecked();
    m_settings.regexp = regexCheck->isChecked();

    m_settings.depth = depthSpin->value();

    m_settings.pattern = patternCombo->currentText();
    m_settings.searchTemplate = searchTemplateFromTemplateString(templateEdit->currentText());
    m_settings.replacementTemplate = replacementTemplateEdit->currentText();
    m_settings.files = filesCombo->currentText();
    m_settings.exclude = excludeCombo->currentText();

    m_settings.searchPaths = searchPaths->currentText();
}

void GrepDialog::updateLimitToProjectEnabled()
{
    Q_ASSERT_X(m_show, Q_FUNC_INFO, "The UI must be initialized.");
    const bool enabled = directoriesInProject(searchPaths->currentText());
    limitToProjectLabel->setEnabled(enabled);
    limitToProjectCheck->setEnabled(enabled);
}

#include "moc_grepdialog.cpp"
