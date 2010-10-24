/***************************************************************************
*   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
*   bernd@kdevelop.org                                                    *
*   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "grepdialog.h"

#include <algorithm>

#include <QDir>
#include <QLabel>
#include <QRegExp>
#include <QBoxLayout>
#include <QStringList>

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
#include <kurlrequester.h>

#include <interfaces/icore.h>
#include <interfaces/idocument.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#include <kstandarddirs.h>

#include "grepviewplugin.h"
#include "grepjob.h"

namespace {

static int const MAX_LAST_SEARCH_ITEMS_COUNT = 15;

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
    << "[a-z0-9_$]+\\s*::\\s*\\b%s\\b\\s*\\("
    << "\\b%s\\b\\s*\\->\\s*[a-z0-9_$]+\\s*\\(";

const QStringList filepatterns = QStringList()
    << "*.h,*.hxx,*.hpp,*.hh,*.h++,*.H,*.tlh,*.cpp,*.cc,*.C,*.c++,*.cxx,*.ocl,*.inl,*.idl,*.c,*.m,*.mm,*.M"
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

}

const KDialog::ButtonCode GrepDialog::SearchButton  = KDialog::User1;
const KDialog::ButtonCode GrepDialog::ReplaceButton = KDialog::User2;

GrepDialog::GrepDialog( GrepViewPlugin * plugin, QWidget *parent )
    : KDialog(parent), Ui::GrepWidget(), m_plugin( plugin )
{
    setAttribute(Qt::WA_DeleteOnClose);

    setButtons( SearchButton | ReplaceButton | KDialog::Cancel );
    setButtonText( SearchButton, i18n("Search") );
    setButtonText( ReplaceButton, i18n("Replace") );
    setCaption( i18n("Find In Files") );
    setDefaultButton( SearchButton );

    setupUi(mainWidget());

    KConfigGroup cg = KGlobal::config()->group( "GrepDialog" );

    patternCombo->addItem( "" );
    patternCombo->addItems( cg.readEntry("LastSearchItems", QStringList()) );
    patternCombo->setInsertPolicy(QComboBox::InsertAtTop);
    
    templateEdit->setText(template_str[0]);

    templateTypeCombo->addItems(template_desc);
    templateTypeCombo->setCurrentIndex( cg.readEntry("LastUsedTemplateIndex", 0) );
    templateEdit->setText( cg.readEntry("LastUsedTemplateString", template_str[0]) );
    
    replacementCombo->addItem( "" );
    replacementCombo->addItems( cg.readEntry("LastReplacementItems", QStringList()) );
    replacementCombo->setInsertPolicy(QComboBox::InsertAtTop);

    regexCheck->setChecked(cg.readEntry("regexp", false ));

    caseSensitiveCheck->setChecked(cg.readEntry("case_sens", true));

    setDirectory( QDir::homePath() );
    directoryRequester->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

    syncButton->setIcon(KIcon("dirsync"));

    recursiveCheck->setChecked(cg.readEntry("recursive", true));
    limitToProjectCheck->setChecked(cg.readEntry("search_project_files", true));

    filesCombo->addItems(cg.readEntry("file_patterns", filepatterns));
    excludeCombo->addItems(cg.readEntry("exclude_patterns", excludepatterns) );

    connect(this, SIGNAL(buttonClicked(KDialog::ButtonCode)), this, SLOT(performAction(KDialog::ButtonCode)));
    connect(syncButton, SIGNAL(clicked()), this, SLOT(syncButtonClicked()));
    connect(templateTypeCombo, SIGNAL(activated(int)),
            this, SLOT(templateTypeComboActivated(int)));
    connect(patternCombo, SIGNAL(editTextChanged(const QString&)),
            this, SLOT(patternComboEditTextChanged( const QString& )));
    connect(replacementCombo, SLOT(editTextChanged(const QString& )),
            this, SLOT(replacementComboEditTextChanged( const QString& )));
    patternComboEditTextChanged( patternCombo->currentText() );
    patternCombo->setFocus();
    
    connect(directoryRequester, SIGNAL(textChanged(const QString&)), this, SLOT(directoryChanged(const QString&)));
}

void GrepDialog::directoryChanged(const QString& dir)
{
    setEnableProjectBox(false);
    KUrl currentUrl = dir;
    if( !currentUrl.isValid() )
        return;
    KDevelop::IProject *proj = KDevelop::ICore::self()->projectController()->findProjectForUrl( currentUrl );
    if( proj && proj->folder().isLocalFile() )
    {
        setEnableProjectBox(! proj->files().isEmpty() );
    }
}


// Returns the contents of a QComboBox as a QStringList
static QStringList qCombo2StringList( QComboBox* combo )
{
    QStringList list;
    if (!combo) {
        return list;
    }
    int skippedItem = -1;
    if (!combo->currentText().isEmpty()) {
        list << combo->currentText();
    }
    if (combo->currentIndex() != -1 && !combo->itemText(combo->currentIndex()).isEmpty()) {
        skippedItem = combo->currentIndex();
    }
    for (int i = 0; i < std::min(MAX_LAST_SEARCH_ITEMS_COUNT, combo->count()); ++i) {
        if (i != skippedItem && !combo->itemText(i).isEmpty()) {
            list << combo->itemText(i);
        }
    }
    return list;
}

GrepDialog::~GrepDialog()
{
    KConfigGroup cg = KGlobal::config()->group( "GrepDialog" );
    // memorize the last patterns and paths
    cg.writeEntry("LastSearchItems", qCombo2StringList(patternCombo));
    cg.writeEntry("LastReplacementItems", qCombo2StringList(replacementCombo));
    cg.writeEntry("regexp", regexCheck->isChecked());
    cg.writeEntry("recursive", recursiveCheck->isChecked());
    cg.writeEntry("search_project_files", limitToProjectCheck->isChecked());
    cg.writeEntry("case_sens", caseSensitiveCheck->isChecked());
    cg.writeEntry("exclude_patterns", qCombo2StringList(excludeCombo));
    cg.writeEntry("file_patterns", qCombo2StringList(filesCombo));
    cg.writeEntry("LastUsedTemplateIndex", templateTypeCombo->currentIndex());
    cg.writeEntry("LastUsedTemplateString", templateEdit->text());
    cg.sync();
}

void GrepDialog::templateTypeComboActivated(int index)
{
    templateEdit->setText(template_str[index]);
}

void GrepDialog::syncButtonClicked( )
{
    KDevelop::IDocument *doc = m_plugin->core()->documentController()->activeDocument();
    kDebug(9001) << doc;
    if ( doc )
    {
        KUrl url = doc->url();
        if ( url.isLocalFile() )
        {
            setDirectory( url.upUrl().toLocalFile() );
        }
    }
}

void GrepDialog::setEnableProjectBox(bool enable)
{
    limitToProjectCheck->setEnabled(enable);
    if (!enable) limitToProjectCheck->setChecked(false);
}

void GrepDialog::setPattern(const QString &pattern)
{
    patternCombo->setEditText(pattern);
}

void GrepDialog::setDirectory(const QString &dir)
{
    directoryRequester->lineEdit()->setText(dir);
    directoryRequester->fileDialog()->setUrl( KUrl( dir ) );
    directoryRequester->completionObject()->setDir( dir );
}

QString GrepDialog::patternString() const
{
    return patternCombo->currentText();
}

QString GrepDialog::templateString() const
{
    return templateEdit->text();
}

QString GrepDialog::replacementString() const
{
    return replacementCombo->currentText();
}

QString GrepDialog::filesString() const
{
    return filesCombo->currentText();
}

QString GrepDialog::excludeString() const
{
    return excludeCombo->currentText();
}

KUrl GrepDialog::directory() const
{
    return directoryRequester->url();
}

bool GrepDialog::useProjectFilesFlag() const
{
    return limitToProjectCheck->isChecked();
}

bool GrepDialog::regexpFlag() const
{
    return regexCheck->isChecked();
}

bool GrepDialog::recursiveFlag() const
{
    return recursiveCheck->isChecked();
}

bool GrepDialog::caseSensitiveFlag() const
{
    return caseSensitiveCheck->isChecked();
}

void GrepDialog::patternComboEditTextChanged( const QString& text)
{
    enableButton( SearchButton, !text.isEmpty() );
}

void GrepDialog::replacementComboEditTextChanged( const QString& text )
{
    enableButton( ReplaceButton, !text.isEmpty() );
}

void GrepDialog::performAction(KDialog::ButtonCode button)
{
    // a click on cancel trigger this signal too
    if( button != SearchButton && button != ReplaceButton ) return;
    
    GrepJob* job = new GrepJob();

    job->setPatternString(patternString());
    job->setTemplateString(templateString());
    job->setReplaceString(replacementString());
    job->setFilesString(filesString());
    job->setExcludeString(excludeString());
    job->setDirectory(directory());

    job->setProjectFilesFlag( useProjectFilesFlag() );
    job->setRegexpFlag( regexpFlag() );
    job->setRecursive( recursiveFlag() );
    job->setCaseSensitive( caseSensitiveFlag() );
    job->setReplaceFlag( button == ReplaceButton );

    kDebug() << "registering job";
    KDevelop::ICore::self()->runController()->registerJob(job);
    
    m_plugin->rememberSearchDirectory(directory().toLocalFile(KUrl::AddTrailingSlash));
    
    close();
}

#include "grepdialog.moc"

