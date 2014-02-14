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

#include <QDir>
#include <QLabel>
#include <QRegExp>
#include <QBoxLayout>
#include <QStringList>
#include <QMenu>

#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kurlcompletion.h>
#include <kurlcompletion.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <KCompletion/kcompletion.h>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/isession.h>

#include "grepviewplugin.h"
#include "grepjob.h"
#include "grepoutputview.h"
#include "grepfindthread.h"
#include "greputil.h"

using namespace KDevelop;

namespace {

QString allOpenFilesString = i18n("All Open Files");
QString allOpenProjectsString = i18n("All Open Projects");
    
const QStringList template_desc = QStringList()
    << "verbatim"
    << "word"
    << "assignment"
    << "->MEMBER("
    << "class::MEMBER("
    << "OBJECT->member(";

const QStringList template_str = QStringList()
    << "%s"
    << "\\b%s\\b"
    << "\\b%s\\b\\s*=[^=]"
    << "\\->\\s*\\b%s\\b\\s*\\("
    << "([a-z0-9_$]+)\\s*::\\s*\\b%s\\b\\s*\\("
    << "\\b%s\\b\\s*\\->\\s*([a-z0-9_$]+)\\s*\\(";

const QStringList repl_template = QStringList()
    << "%s"
    << "%s"
    << "%s = "
    << "->%s("
    << "\\1::%s("
    << "%s->\\1(";

const QStringList filepatterns = QStringList()
    << "*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.idl,*.c,*.m,*.mm,*.M,*.y,*.ypp,*.yxx,*.y++,*.l"
    << "*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.c,*.m,*.mm,*.M"
    << "*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.idl"
    << "*.adb"
    << "*.cs"
    << "*.f"
    << "*.html,*.htm"
    << "*.hs"
    << "*.java"
    << "*.js"
    << "*.php,*.php3,*.php4"
    << "*.pl"
    << "*.pp,*.pas"
    << "*.py"
    << "*.js,*.css,*.yml,*.rb,*.rhtml,*.html.erb,*.rjs,*.js.rjs,*.rxml,*.xml.builder"
    << "CMakeLists.txt,*.cmake"
    << "*";

const QStringList excludepatterns = QStringList()
    << "/CVS/,/SCCS/,/.svn/,/_darcs/,/build/,/.git/"
    << "";

///Separator used to separate search paths.
const QString pathsSeparator(";");

///Max number of items in paths combo box.
const int pathsMaxCount = 25;
}

const KDialog::ButtonCode GrepDialog::SearchButton  = KDialog::User1;

GrepDialog::GrepDialog( GrepViewPlugin * plugin, QWidget *parent )
    : KDialog(parent), Ui::GrepWidget(), m_plugin( plugin )
{
    setAttribute(Qt::WA_DeleteOnClose);

    setButtons( SearchButton | KDialog::Cancel );
    setButtonText( SearchButton, i18n("Search...") );
    setButtonIcon( SearchButton, QIcon::fromTheme("edit-find") );
    setCaption( i18n("Find/Replace in Files") );
    setDefaultButton( SearchButton );

    setupUi(mainWidget());

    KConfigGroup cg = ICore::self()->activeSession()->config()->group( "GrepDialog" );

    patternCombo->addItems( cg.readEntry("LastSearchItems", QStringList()) );
    patternCombo->setInsertPolicy(QComboBox::InsertAtTop);
    
    templateTypeCombo->addItems(template_desc);
    templateTypeCombo->setCurrentIndex( cg.readEntry("LastUsedTemplateIndex", 0) );
    templateEdit->addItems( cg.readEntry("LastUsedTemplateString", template_str) );
    templateEdit->setEditable(true);
    templateEdit->setCompletionMode(KCompletion::CompletionPopup);
    KCompletion* comp = templateEdit->completionObject();
    connect(templateEdit, SIGNAL(returnPressed(QString)), comp, SLOT(addItem(QString)));
    for(int i=0; i<templateEdit->count(); i++)
        comp->addItem(templateEdit->itemText(i));
    replacementTemplateEdit->addItems( cg.readEntry("LastUsedReplacementTemplateString", repl_template) );
    replacementTemplateEdit->setEditable(true);
    replacementTemplateEdit->setCompletionMode(KCompletion::CompletionPopup);
    comp = replacementTemplateEdit->completionObject();
    connect(replacementTemplateEdit, SIGNAL(returnPressed(QString)), comp, SLOT(addItem(QString)));
    for(int i=0; i<replacementTemplateEdit->count(); i++)
        comp->addItem(replacementTemplateEdit->itemText(i));
    
    regexCheck->setChecked(cg.readEntry("regexp", false ));

    caseSensitiveCheck->setChecked(cg.readEntry("case_sens", true));

    searchPaths->setCompletionObject(new KUrlCompletion());
    searchPaths->setAutoDeleteCompletionObject(true);

    QList<IProject*> projects = m_plugin->core()->projectController()->projects();

    searchPaths->addItems(cg.readEntry("SearchPaths", QStringList(!projects.isEmpty() ? allOpenProjectsString : QDir::homePath() ) ));
    searchPaths->setInsertPolicy(QComboBox::InsertAtTop);

    syncButton->setIcon(QIcon::fromTheme("dirsync"));
    syncButton->setMenu(createSyncButtonMenu());

    depthSpin->setValue(cg.readEntry("depth", -1));
    limitToProjectCheck->setChecked(cg.readEntry("search_project_files", true));

    filesCombo->addItems(cg.readEntry("file_patterns", filepatterns));
    excludeCombo->addItems(cg.readEntry("exclude_patterns", excludepatterns) );

    connect(this, SIGNAL(buttonClicked(KDialog::ButtonCode)), this, SLOT(performAction(KDialog::ButtonCode)));
    connect(templateTypeCombo, SIGNAL(activated(int)),
            this, SLOT(templateTypeComboActivated(int)));
    connect(patternCombo, SIGNAL(editTextChanged(QString)),
            this, SLOT(patternComboEditTextChanged(QString)));
    patternComboEditTextChanged( patternCombo->currentText() );
    patternCombo->setFocus();
    
    connect(searchPaths, SIGNAL(activated(QString)), this, SLOT(setSearchLocations(QString)));

    directorySelector->setIcon(QIcon::fromTheme("document-open"));
    connect(directorySelector, SIGNAL(clicked(bool)), this, SLOT(selectDirectoryDialog()) );
    directoryChanged(directorySelector->text());
}

void GrepDialog::selectDirectoryDialog()
{
    QString dirName = KFileDialog::getExistingDirectory(searchPaths->lineEdit()->text(), this, tr("Select directory to search in"));

    if (!dirName.isEmpty()) {
        setSearchLocations(dirName);
    }
}

void GrepDialog::addUrlToMenu(QMenu* menu, const KUrl& url)
{
    QAction* action = menu->addAction(m_plugin->core()->projectController()->prettyFileName(url, KDevelop::IProjectController::FormatPlain));
    action->setData(QVariant(url.pathOrUrl()));
    connect(action, SIGNAL(triggered(bool)), SLOT(synchronizeDirActionTriggered(bool)));
}

void GrepDialog::addStringToMenu(QMenu* menu, QString string)
{
    QAction* action = menu->addAction(string);
    action->setData(QVariant(string));
    connect(action, SIGNAL(triggered(bool)), SLOT(synchronizeDirActionTriggered(bool)));
}

void GrepDialog::synchronizeDirActionTriggered(bool)
{
    QAction* action = qobject_cast<QAction*>(sender());
    Q_ASSERT(action);
    setSearchLocations(action->data().value<QString>());
}

QMenu* GrepDialog::createSyncButtonMenu()
{
    QMenu* ret = new QMenu;

    QSet<KUrl> hadUrls;
    
    IDocument *doc = m_plugin->core()->documentController()->activeDocument();
    if ( doc )
    {
        KUrl url = doc->url();
        url.cd("..");
        while(m_plugin->core()->projectController()->findProjectForUrl(url))
        {
            url.adjustPath(KUrl::RemoveTrailingSlash);
            if(hadUrls.contains(url))
                break;
            hadUrls.insert(url);
            addUrlToMenu(ret, url);
            if(!url.cd(".."))
                break;
        }

        // if the current file's parent directory is not in the project, add it
        url = doc->url().upUrl();
        url.adjustPath(KUrl::RemoveTrailingSlash);
        if(!hadUrls.contains(url))
        {
            hadUrls.insert(url);
            addUrlToMenu(ret, url);
        }
    }
    
    foreach(IProject* project, m_plugin->core()->projectController()->projects())
    {
        KUrl url = project->folder();
        url.adjustPath(KUrl::RemoveTrailingSlash);
        if(hadUrls.contains(url))
            continue;
        addUrlToMenu(ret, url);
    }
    
    addStringToMenu(ret, allOpenFilesString);
    addStringToMenu(ret, allOpenProjectsString);
    return ret;
}

void GrepDialog::directoryChanged(const QString& dir)
{
    KUrl currentUrl = dir;
    if( !currentUrl.isValid() ) {
        setEnableProjectBox(false);
        return;
    }
    
    bool projectAvailable = true;
    
    foreach(KUrl url, getDirectoryChoice())
    {
        IProject *proj = ICore::self()->projectController()->findProjectForUrl( currentUrl );
        if( !proj || !proj->folder().isLocalFile() )
            projectAvailable = false;
    }
    
    setEnableProjectBox(projectAvailable);
}

GrepDialog::~GrepDialog()
{
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
    templateEdit->setCurrentItem( template_str[index], true );
    replacementTemplateEdit->setCurrentItem( repl_template[index], true );
}

void GrepDialog::setEnableProjectBox(bool enable)
{
    limitToProjectCheck->setEnabled(enable);
    limitToProjectLabel->setEnabled(enable);
}

void GrepDialog::setPattern(const QString &pattern)
{
    patternCombo->setEditText(pattern);
}

void GrepDialog::setSearchLocations(const QString &dir)
{
    if(!dir.isEmpty()) {
        if(QDir::isAbsolutePath(dir))
        {
            static_cast<KUrlCompletion*>(searchPaths->completionObject())->setDir( dir );
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

QString GrepDialog::patternString() const
{
    return patternCombo->currentText();
}

QString GrepDialog::templateString() const
{
    return templateEdit->currentText().isEmpty() ? "%s" : templateEdit->currentText();
}

QString GrepDialog::replacementTemplateString() const
{
    return replacementTemplateEdit->currentText();
}

QString GrepDialog::filesString() const
{
    return filesCombo->currentText();
}

QString GrepDialog::excludeString() const
{
    return excludeCombo->currentText();
}

bool GrepDialog::useProjectFilesFlag() const
{
    if (!limitToProjectCheck->isEnabled()) return false;
    return limitToProjectCheck->isChecked();
}

bool GrepDialog::regexpFlag() const
{
    return regexCheck->isChecked();
}

int GrepDialog::depthValue() const
{
    return depthSpin->value();
}

bool GrepDialog::caseSensitiveFlag() const
{
    return caseSensitiveCheck->isChecked();
}

void GrepDialog::patternComboEditTextChanged( const QString& text)
{
    enableButton( SearchButton,  !text.isEmpty() );
}

QList< KUrl > GrepDialog::getDirectoryChoice() const
{
    QList< KUrl > ret;
    QString text = searchPaths->currentText();
    if(text == allOpenFilesString)
    {
        foreach(IDocument* doc, ICore::self()->documentController()->openDocuments())
            ret << doc->url();
    }else if(text == allOpenProjectsString)
    {
        foreach(IProject* project, ICore::self()->projectController()->projects())
            ret << project->folder();
    }else{
        QStringList semicolonSeparatedFileList = text.split(pathsSeparator);
        if(!semicolonSeparatedFileList.isEmpty() && QFileInfo(semicolonSeparatedFileList[0]).exists())
        {
            // We use QFileInfo to make sure this is really a semicolon-separated file list, not a file containing
            // a semicolon in the name.
            foreach(QString file, semicolonSeparatedFileList)
                ret << KUrl::fromPath(file);
        }else{
            ret << KUrl(searchPaths->currentText());
        }
    }
    return ret;
}

bool GrepDialog::isPartOfChoice(KUrl url) const
{
    foreach(KUrl choice, getDirectoryChoice())
        if(choice.isParentOf(url) || choice.equals(url))
            return true;
    return false;
}

void GrepDialog::start()
{
    performAction(SearchButton);
}

void GrepDialog::performAction(KDialog::ButtonCode button)
{
    // a click on cancel trigger this signal too
    if( button != SearchButton ) return;
    
    // search for unsaved documents
    QList<IDocument*> unsavedFiles;
    QStringList include = GrepFindFilesThread::parseInclude(filesString());
    QStringList exclude = GrepFindFilesThread::parseExclude(excludeString());
    
    foreach(IDocument* doc, ICore::self()->documentController()->openDocuments())
    {
        KUrl docUrl = doc->url();
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
    
    QList<KUrl> choice = getDirectoryChoice();
    
    GrepJob* job = m_plugin->newGrepJob();
    
    QString descriptionOrUrl(searchPaths->currentText());
    QString description = descriptionOrUrl;
    // Shorten the description
    if(descriptionOrUrl != allOpenFilesString && descriptionOrUrl != allOpenProjectsString && choice.size() > 1)
        description = i18np("%2, and %1 more item", "%2, and %1 more items", choice.size() - 1, choice[0].pathOrUrl());
    
    GrepOutputView *toolView = (GrepOutputView*)ICore::self()->uiController()->
                               findToolView(i18n("Find/Replace in Files"), m_plugin->toolViewFactory(), IUiController::CreateAndRaise);
    GrepOutputModel* outputModel = toolView->renewModel(patternString(), description);
    
    connect(job, SIGNAL(showErrorMessage(QString,int)),
            toolView, SLOT(showErrorMessage(QString)));
    //the GrepOutputModel gets the 'showMessage' signal to store it and forward
    //it to toolView
    connect(job, SIGNAL(showMessage(KDevelop::IStatus*,QString,int)),
            outputModel, SLOT(showMessageSlot(KDevelop::IStatus*,QString)));    
    connect(outputModel, SIGNAL(showMessage(KDevelop::IStatus*,QString)),
            toolView, SLOT(showMessage(KDevelop::IStatus*,QString)));
    
    
    connect(toolView, SIGNAL(outputViewIsClosed()),
            job, SLOT(kill()));
    
    job->setOutputModel(outputModel);
    job->setPatternString(patternString());
    job->setReplacementTemplateString(replacementTemplateString());
    job->setTemplateString(templateString());
    job->setFilesString(filesString());
    job->setExcludeString(excludeString());
    job->setDirectoryChoice(choice);

    job->setProjectFilesFlag( useProjectFilesFlag() );
    job->setRegexpFlag( regexpFlag() );
    job->setDepth( depthValue() );
    job->setCaseSensitive( caseSensitiveFlag() );

    ICore::self()->runController()->registerJob(job);
    
    m_plugin->rememberSearchDirectory(descriptionOrUrl);
    
    close();
}

#include "grepdialog.moc"

