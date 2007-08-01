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
#include "grepviewpart.h"
#include <icore.h>
#include <idocument.h>
#include <idocumentcontroller.h>
#include <kglobal.h>

#include <QDir>
#include <QLabel>
#include <QRegExp>
#include <QBoxLayout>
#include <QStringList>

#include <kfiledialog.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>

QStringList __template_desc = QStringList()
    << "verbatim"
    << "assignment"
    << "->MEMBER("
    << "class::MEMBER("
    << "OBJECT->member(";

QStringList __template_str = QStringList()
    << "%s"
    << "\\<%s\\>[\\t ]*=[^=]"
    << "\\->[\\t ]*\\<%s\\>[\\t ]*\\("
    << "[a-z0-9_$]+[\\t ]*::[\\t ]*\\<%s\\>[\\t ]*\\("
    << "\\<%s\\>[\\t ]*\\->[\\t ]*[a-z0-9_$]+[\\t ]*\\(";

QStringList __filepatterns = QStringList()
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
    << "*.js,*.css,*.yml,*.rb,*.rhtml,*.rjs,*.rxml"
    << "CMakeLists.txt,*.cmake"
    << "*";

GrepDialog::GrepDialog( GrepViewPart * part, QWidget *parent )
    : KDialog(parent), Ui::GrepWidget(), m_part( part )
{

    setButtons( KDialog::Ok | KDialog::Cancel );
    setButtonText( KDialog::Ok, i18n("Search") );
    setCaption( i18n("Find In Files") );
    setDefaultButton( KDialog::Ok );

    setupUi(mainWidget());

    KConfigGroup cg = KGlobal::config()->group( "GrepDialog" );

    patternCombo->addItems( cg.readEntry("LastSearchItems", QStringList()) );
    patternCombo->setInsertPolicy(QComboBox::InsertAtTop);

    templateEdit->setText(__template_str[0]);

    templateTypeCombo->addItems(__template_desc);

    regexCheck->setChecked(cg.readEntry("regexp", false ));

    caseSensitiveCheck->setChecked(cg.readEntry("case_sens", true));

    directoryRequester->comboBox()->setEditable( true );
    directoryRequester->comboBox()->addItems(cg.readPathListEntry("LastSearchPaths"));
    directoryRequester->setPath( QDir::homePath() );
    directoryRequester->fileDialog()->setUrl( KUrl( QDir::homePath() ) );
    directoryRequester->completionObject()->setDir( QDir::homePath() );
    directoryRequester->completionObject()->setMode(KUrlCompletion::DirCompletion);
    directoryRequester->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );

    syncButton->setIcon(KIcon("dirsync"));

    recursiveCheck->setChecked(cg.readEntry("recursive", true));
    limitToProjectCheck->setChecked(cg.readEntry("search_project_files", true));

    filesCombo->addItems(__filepatterns);
    excludeCombo->addItems(cg.readEntry("exclude_patterns", QStringList() << "/CVS/,/SCCS/,/\\.svn/,/_darcs/" << "") );

    suppressErrorsCheck->setChecked(cg.readEntry("no_find_errs", true));

    connect(this, SIGNAL(hidden()), this, SLOT(slotHidden()));
    connect(this, SIGNAL(okClicked()), this, SIGNAL(search()));
    connect(syncButton, SIGNAL(clicked()), this, SLOT(syncButtonClicked()));
    connect(templateTypeCombo, SIGNAL(activated(int)),
            this, SLOT(templateTypeComboActivated(int)));
    connect(patternCombo, SIGNAL(editTextChanged(const QString&)),
            this, SLOT(patternComboEditTextChanged( const QString& )));
    patternComboEditTextChanged( patternCombo->currentText() );
}

// Returns the contents of a QComboBox as a QStringList
static QStringList qCombo2StringList( QComboBox* combo )
{
    QStringList list;
    if (!combo)
    return list;
    for (int i = 0; i < 15; ++i ) {
        list << combo->itemText(i);
    }
    return list;
}

GrepDialog::~GrepDialog()
{
    KConfigGroup cg = KGlobal::config()->group( "GrepDialog" );
    // memorize the last patterns and paths
    cg.writeEntry("LastSearchItems", qCombo2StringList(patternCombo));
    cg.writePathEntry("LastSearchPaths", qCombo2StringList(directoryRequester->comboBox()));
    cg.writeEntry("regexp", regexCheck->isChecked());
    cg.writeEntry("recursive", recursiveCheck->isChecked());
    cg.writeEntry("search_project_files", limitToProjectCheck->isChecked());
    cg.writeEntry("case_sens", caseSensitiveCheck->isChecked());
    cg.writeEntry("no_find_errs", suppressErrorsCheck->isChecked());
    cg.writeEntry("exclude_patterns", qCombo2StringList(excludeCombo));
    cg.sync();
}

void GrepDialog::templateTypeComboActivated(int index)
{
    templateEdit->setText(__template_str[index]);
}

void GrepDialog::showEvent(QShowEvent* ev)
{
    KDialog::showEvent(ev);
    patternCombo->setFocus();
}

void GrepDialog::slotHidden()
{
    patternCombo->setFocus();
}

void GrepDialog::syncButtonClicked( )
{
    KDevelop::IDocument *doc = m_part->core()->documentController()->activeDocument();
    kDebug() << doc;
    if ( doc )
    {
        KUrl url = doc->url();
        if ( url.isLocalFile() )
        {
            directoryRequester->comboBox()->setEditText( url.upUrl().path( KUrl::LeaveTrailingSlash ) );
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
    directoryRequester->comboBox()->setEditText(dir);
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

bool GrepDialog::noFindErrorsFlag() const
{
    return suppressErrorsCheck->isChecked();
}

bool GrepDialog::caseSensitiveFlag() const
{
    return caseSensitiveCheck->isChecked();
}

void GrepDialog::patternComboEditTextChanged( const QString& text)
{
    enableButtonOk( !text.isEmpty() );
}

#include "grepdialog.moc"

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
