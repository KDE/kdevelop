/***************************************************************************
*   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*   Copyright 2010 Julien Desgats <julien.desgats@gmail.com>              *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepdialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QStringList>

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

#include <util/path.h>

#include "grepviewplugin.h"
#include "grepoutputview.h"
#include "grepfindthread.h"
#include "greputil.h"


using namespace KDevelop;

namespace {

inline QString allOpenFilesString() { return i18n("All Open Files"); }
inline QString allOpenProjectsString() { return i18n("All Open Projects"); }

inline QStringList template_desc() { return QStringList()
    << QStringLiteral("verbatim")
    << QStringLiteral("word")
    << QStringLiteral("assignment")
    << QStringLiteral("->MEMBER(")
    << QStringLiteral("class::MEMBER(")
    << QStringLiteral("OBJECT->member(");
}

inline QStringList template_str() { return QStringList()
    << QStringLiteral("%s")
    << QStringLiteral("\\b%s\\b")
    << QStringLiteral("\\b%s\\b\\s*=[^=]")
    << QStringLiteral("\\->\\s*\\b%s\\b\\s*\\(")
    << QStringLiteral("([a-z0-9_$]+)\\s*::\\s*\\b%s\\b\\s*\\(")
    << QStringLiteral("\\b%s\\b\\s*\\->\\s*([a-z0-9_$]+)\\s*\\(");
}

inline QStringList repl_template() { return QStringList()
    << QStringLiteral("%s")
    << QStringLiteral("%s")
    << QStringLiteral("%s = ")
    << QStringLiteral("->%s(")
    << QStringLiteral("\\1::%s(")
    << QStringLiteral("%s->\\1(");
}

inline QStringList filepatterns() { return QStringList()
    << QStringLiteral("*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.idl,*.c,*.m,*.mm,*.M,*.y,*.ypp,*.yxx,*.y++,*.l")
    << QStringLiteral("*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.c,*.m,*.mm,*.M")
    << QStringLiteral("*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.idl")
    << QStringLiteral("*.adb")
    << QStringLiteral("*.cs")
    << QStringLiteral("*.f")
    << QStringLiteral("*.html,*.htm")
    << QStringLiteral("*.hs")
    << QStringLiteral("*.java")
    << QStringLiteral("*.js")
    << QStringLiteral("*.php,*.php3,*.php4")
    << QStringLiteral("*.pl")
    << QStringLiteral("*.pp,*.pas")
    << QStringLiteral("*.py")
    << QStringLiteral("*.js,*.css,*.yml,*.rb,*.rhtml,*.html.erb,*.rjs,*.js.rjs,*.rxml,*.xml.builder")
    << QStringLiteral("CMakeLists.txt,*.cmake")
    << QStringLiteral("*");
}

inline QStringList excludepatterns() { return QStringList()
    << QStringLiteral("/CVS/,/SCCS/,/.svn/,/_darcs/,/build/,/.git/")
    << QLatin1String("");
}

///Separator used to separate search paths.
inline QString pathsSeparator() { return (QStringLiteral(";")); }

///Max number of items in paths combo box.
const int pathsMaxCount = 25;
}

GrepDialog::GrepDialog( GrepViewPlugin * plugin, QWidget *parent )
    : QDialog(parent), Ui::GrepWidget(), m_plugin( plugin )
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle( i18n("Find/Replace in Files") );

    setupUi(this);
    adjustSize();

    auto searchButton = buttonBox->button(QDialogButtonBox::Ok);
    Q_ASSERT(searchButton);
    searchButton->setText(i18nc("@action:button", "Search..."));
    searchButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
    connect(searchButton, &QPushButton::clicked, this, &GrepDialog::startSearch);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GrepDialog::reject);

    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );

    patternCombo->addItems( cg.readEntry("LastSearchItems", QStringList()) );
    patternCombo->setInsertPolicy(QComboBox::InsertAtTop);

    templateTypeCombo->addItems(template_desc());
    templateTypeCombo->setCurrentIndex( cg.readEntry("LastUsedTemplateIndex", 0) );
    templateEdit->addItems( cg.readEntry("LastUsedTemplateString", template_str()) );
    templateEdit->setEditable(true);
    templateEdit->setCompletionMode(KCompletion::CompletionPopup);
    KCompletion* comp = templateEdit->completionObject();
    connect(templateEdit, static_cast<void(KComboBox::*)(const QString&)>(&KComboBox::returnPressed), comp, static_cast<void(KCompletion::*)(const QString&)>(&KCompletion::addItem));
    for(int i=0; i<templateEdit->count(); i++)
        comp->addItem(templateEdit->itemText(i));
    replacementTemplateEdit->addItems( cg.readEntry("LastUsedReplacementTemplateString", repl_template()) );
    replacementTemplateEdit->setEditable(true);
    replacementTemplateEdit->setCompletionMode(KCompletion::CompletionPopup);
    comp = replacementTemplateEdit->completionObject();
    connect(replacementTemplateEdit, static_cast<void(KComboBox::*)(const QString&)>(&KComboBox::returnPressed), comp, static_cast<void(KCompletion::*)(const QString&)>(&KCompletion::addItem));
    for(int i=0; i<replacementTemplateEdit->count(); i++)
        comp->addItem(replacementTemplateEdit->itemText(i));

    regexCheck->setChecked(cg.readEntry("regexp", false ));

    caseSensitiveCheck->setChecked(cg.readEntry("case_sens", true));

    searchPaths->setCompletionObject(new KUrlCompletion());
    searchPaths->setAutoDeleteCompletionObject(true);

    QList<IProject*> projects = m_plugin->core()->projectController()->projects();

    searchPaths->addItems(cg.readEntry("SearchPaths", QStringList(!projects.isEmpty() ? allOpenProjectsString() : QDir::homePath() ) ));
    searchPaths->setInsertPolicy(QComboBox::InsertAtTop);

    syncButton->setIcon(QIcon::fromTheme(QStringLiteral("dirsync")));
    syncButton->setMenu(createSyncButtonMenu());

    depthSpin->setValue(cg.readEntry("depth", -1));
    limitToProjectCheck->setChecked(cg.readEntry("search_project_files", true));

    filesCombo->addItems(cg.readEntry("file_patterns", filepatterns()));
    excludeCombo->addItems(cg.readEntry("exclude_patterns", excludepatterns()) );

    connect(templateTypeCombo, static_cast<void(KComboBox::*)(int)>(&KComboBox::activated),
            this, &GrepDialog::templateTypeComboActivated);
    connect(patternCombo, &QComboBox::editTextChanged,
            this, &GrepDialog::patternComboEditTextChanged);
    patternComboEditTextChanged( patternCombo->currentText() );
    patternCombo->setFocus();

    connect(searchPaths, static_cast<void(KComboBox::*)(const QString&)>(&KComboBox::activated), this, &GrepDialog::setSearchLocations);

    directorySelector->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    connect(directorySelector, &QPushButton::clicked, this, &GrepDialog::selectDirectoryDialog );
    directoryChanged(directorySelector->text());
}

void GrepDialog::selectDirectoryDialog()
{
    const QString dirName = QFileDialog::getExistingDirectory(
        this, i18nc("@title:window", "Select directory to search in"),
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

void GrepDialog::addStringToMenu(QMenu* menu, QString string)
{
    QAction* action = menu->addAction(string);
    action->setData(QVariant(string));
    connect(action, &QAction::triggered, this, &GrepDialog::synchronizeDirActionTriggered);
}

void GrepDialog::synchronizeDirActionTriggered(bool)
{
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    setSearchLocations(action->data().toString());
}

QMenu* GrepDialog::createSyncButtonMenu()
{
    QMenu* ret = new QMenu;

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
            if(hadUrls.contains(url))
                break;
            hadUrls.insert(url);
            addUrlToMenu(ret, url.toUrl());
            url = url.parent();
        }
    }

    foreach(IProject* project, m_plugin->core()->projectController()->projects())
    {
        if (!hadUrls.contains(project->path())) {
            addUrlToMenu(ret, project->path().toUrl());
        }
    }

    addStringToMenu(ret, allOpenFilesString());
    addStringToMenu(ret, allOpenProjectsString());
    return ret;
}

void GrepDialog::directoryChanged(const QString& dir)
{
    QUrl currentUrl = QUrl::fromLocalFile(dir);
    if( !currentUrl.isValid() ) {
        m_settings.projectFilesOnly = false;
        return;
    }

    bool projectAvailable = true;

    foreach(const QUrl& url, getDirectoryChoice())
    {
        IProject *proj = ICore::self()->projectController()->findProjectForUrl( url );
        if( !proj || !proj->path().toUrl().isLocalFile() )
            projectAvailable = false;
    }

    m_settings.projectFilesOnly =  projectAvailable;
}

GrepDialog::~GrepDialog()
{
}

void GrepDialog::closeEvent(QCloseEvent* closeEvent)
{
    Q_UNUSED(closeEvent);

    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );
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

void GrepDialog::setSettings(const GrepJobSettings& settings)
{
    patternCombo->setEditText(settings.pattern);
    patternComboEditTextChanged(settings.pattern);
    m_settings.pattern = settings.pattern;

    limitToProjectCheck->setEnabled(settings.projectFilesOnly);
    limitToProjectLabel->setEnabled(settings.projectFilesOnly);
    m_settings.projectFilesOnly = settings.projectFilesOnly;

    // Note: everything else is set by a user
}

GrepJobSettings GrepDialog::settings() const
{
    return m_settings;
}

void GrepDialog::setSearchLocations(const QString &dir)
{
    if(!dir.isEmpty()) {
        if(QDir::isAbsolutePath(dir))
        {
            static_cast<KUrlCompletion*>(searchPaths->completionObject())->setDir( QUrl::fromLocalFile(dir) );
        }

            if (searchPaths->contains(dir)) {
                searchPaths->removeItem(searchPaths->findText(dir));
            }

            searchPaths->insertItem(0, dir);
            searchPaths->setCurrentItem(dir);

            if (searchPaths->count() > pathsMaxCount) {
                searchPaths->removeItem(searchPaths->count() - 1);
            }
    }
    directoryChanged(dir);
}

void GrepDialog::patternComboEditTextChanged( const QString& text)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

QList< QUrl > GrepDialog::getDirectoryChoice() const
{
    QList< QUrl > ret;
    QString text = searchPaths->currentText();
    if(text == allOpenFilesString())
    {
        foreach(IDocument* doc, ICore::self()->documentController()->openDocuments())
            ret << doc->url();
    }else if(text == allOpenProjectsString())
    {
        foreach(IProject* project, ICore::self()->projectController()->projects())
            ret << project->path().toUrl();
    }else{
        QStringList semicolonSeparatedFileList = text.split(pathsSeparator());
        if(!semicolonSeparatedFileList.isEmpty() && QFileInfo::exists(semicolonSeparatedFileList[0]))
        {
            // We use QFileInfo to make sure this is really a semicolon-separated file list, not a file containing
            // a semicolon in the name.
            foreach(const QString& file, semicolonSeparatedFileList)
                ret << QUrl::fromLocalFile(file).adjusted(QUrl::StripTrailingSlash);
        }else{
            ret << QUrl::fromUserInput(searchPaths->currentText()).adjusted(QUrl::StripTrailingSlash);
        }
    }
    return ret;
}

bool GrepDialog::isPartOfChoice(QUrl url) const
{
    foreach(const QUrl& choice, getDirectoryChoice())
        if(choice.isParentOf(url) || choice == url)
            return true;
    return false;
}

void GrepDialog::startSearch()
{
    updateSettings();

    // search for unsaved documents
    QList<IDocument*> unsavedFiles;
    QStringList include = GrepFindFilesThread::parseInclude(m_settings.files);
    QStringList exclude = GrepFindFilesThread::parseExclude(m_settings.exclude);

    foreach(IDocument* doc, ICore::self()->documentController()->openDocuments())
    {
        QUrl docUrl = doc->url();
        if(doc->state() != IDocument::Clean && isPartOfChoice(docUrl) &&
           QDir::match(include, docUrl.fileName()) && !QDir::match(exclude, docUrl.toLocalFile()))
        {
            unsavedFiles << doc;
        }
    }

    if(!ICore::self()->documentController()->saveSomeDocuments(unsavedFiles))
    {
        close();
        return;
    }

    QList<QUrl> choice = getDirectoryChoice();

    GrepJob* job = m_plugin->newGrepJob();

    const QString descriptionOrUrl(searchPaths->currentText());
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

    GrepOutputView *toolView = (GrepOutputView*)ICore::self()->uiController()->
                               findToolView(i18n("Find/Replace in Files"), m_plugin->toolViewFactory(), IUiController::CreateAndRaise);
    GrepOutputModel* outputModel = toolView->renewModel(m_settings.pattern, description);

    connect(job, &GrepJob::showErrorMessage,
            toolView, &GrepOutputView::showErrorMessage);
    //the GrepOutputModel gets the 'showMessage' signal to store it and forward
    //it to toolView
    connect(job, &GrepJob::showMessage,
            outputModel, &GrepOutputModel::showMessageSlot);
    connect(outputModel, &GrepOutputModel::showMessage,
            toolView, &GrepOutputView::showMessage);

    connect(toolView, &GrepOutputView::outputViewIsClosed, job, [=]() {job->kill();});

    job->setOutputModel(outputModel);
    job->setDirectoryChoice(choice);

    job->setSettings(m_settings);

    ICore::self()->runController()->registerJob(job);

    m_plugin->rememberSearchDirectory(descriptionOrUrl);

    close();
}

void GrepDialog::updateSettings()
{
    if (limitToProjectCheck->isEnabled())
        m_settings.projectFilesOnly = limitToProjectCheck->isChecked();

    m_settings.caseSensitive = caseSensitiveCheck->isChecked();
    m_settings.regexp = regexCheck->isChecked();

    m_settings.depth = depthSpin->value();

    m_settings.pattern = patternCombo->currentText();
    m_settings.searchTemplate = templateEdit->currentText().isEmpty() ? QStringLiteral("%s") : templateEdit->currentText();
    m_settings.replacementTemplate = replacementTemplateEdit->currentText();
    m_settings.files = filesCombo->currentText();
    m_settings.exclude = excludeCombo->currentText();
}

