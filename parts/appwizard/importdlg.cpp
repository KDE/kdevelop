/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "importdlg.h"
#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kcursor.h>
#include <kfile.h>
#include <kurlrequester.h>

#include "kdevcore.h"
#include "kdevversioncontrol.h"

#include "appwizardfactory.h"
#include "appwizardpart.h"
#include "misc.h"


ImportDialog::ImportDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : ImportDialogBase(parent, name, true), m_part(part)
{
    QString author, email;
    AppWizardUtil::guessAuthorAndEmail(&author, &email);
    author_edit->setText(author);
    email_edit->setText(email);
    QToolTip::add( urlinput_edit->button(), i18n("Choose directory to import") );
    urlinput_edit->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);

    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    importNames = dirs->findAllResources("appimports", QString::null, false, true);
    importNames.sort();

    QStringList::ConstIterator it;
    for (it = importNames.begin(); it != importNames.end(); ++it) {
        KConfig config(KGlobal::dirs()->findResource("appimports", *it));
        config.setGroup("General");
        project_combo->insertItem(config.readEntry("Comment"));
    }

    setProjectType("c");
    connect( name_edit, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotProjectNameChanged( const QString & ) ) );
    scanAvailableVCS();
    connect( fetchModuleButton, SIGNAL(clicked()),
        this, SLOT(slotFetchModulesFromRepository()) );
    slotProjectNameChanged( name_edit->text() );
}


ImportDialog::~ImportDialog()
{}

void ImportDialog::slotProjectNameChanged( const QString &_text )
{
    ok_button->setEnabled( !_text.isEmpty() );
}

void ImportDialog::accept()
{
    QDir dir(urlinput_edit->url());
    if (urlinput_edit->url().isEmpty() || !dir.exists()) {
        KMessageBox::sorry(this, i18n("You have to choose a directory."));
        return;
    }

    QString projectName = name_edit->text();
    if (projectName.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to choose a project name."));
        return;
    }

    for (uint i=0; i < projectName.length(); ++i)
        if (!projectName[i].isLetterOrNumber() && projectName[i] != '_') {
            KMessageBox::sorry(this, i18n("Your application name should only contain letters and numbers."));
            return;
        }

    QString author = author_edit->text();
    QString email = email_edit->text();

    QFileInfo finfo(importNames[project_combo->currentItem()]);
    QDir importdir(finfo.dir());
    importdir.cdUp();
    QFile src(importdir.filePath("importfiles/" + finfo.fileName() + ".kdevelop"));
    kdDebug(9010) << "Import template " << src.name() << endl;
    if (!src.open(IO_ReadOnly)) {
        KMessageBox::sorry(this, i18n("Can not open project template."));
        return;
    }

    QFile dest(dir.filePath(projectName + ".kdevelop"));
    if (!dest.open(IO_WriteOnly)) {
        KMessageBox::sorry(this, i18n("Can not write the project file."));
        return;
    }

    QTextStream srcstream(&src);
    QTextStream deststream(&dest);

    while (!srcstream.atEnd()) {
        QString line = srcstream.readLine();
        line.replace(QRegExp("\\$APPNAMELC\\$"), projectName);
        line.replace(QRegExp("\\$AUTHOR\\$"), author);
        line.replace(QRegExp("\\$EMAIL\\$"), email);
        deststream << line << endl;
    }

    dest.close();
    src.close();

    m_part->core()->openProject(dir.filePath(projectName + ".kdevelop"));

    kdDebug(9010) << "OPENING PROJECT: " << dir.filePath(projectName + ".kdevelop") << endl;

    QDialog::accept();
}


// Checks if the directory dir and all of its subdirectories
// (one level recursion) have files that follow patterns
// patterns is comma-separated
static bool dirHasFiles(QDir &dir, const QString &patterns)
{
    QStringList::ConstIterator pit, sit;

    QStringList patternList = QStringList::split(",", patterns);
    for (pit = patternList.begin(); pit != patternList.end(); ++pit) {
        if (!dir.entryList(*pit, QDir::Files).isEmpty()) {
            kdDebug() << "Has files " << (*pit) << endl;
            return true;
        }
    }

    QStringList subdirList = dir.entryList("*", QDir::Dirs);
    for (sit = subdirList.begin(); sit != subdirList.end(); ++sit) {
        QDir subdir(dir);
        subdir.cd(*sit);
        for (pit = patternList.begin(); pit != patternList.end(); ++pit) {
            if (!subdir.entryList(*pit, QDir::Files).isEmpty()) {
                kdDebug() << "Has files " << (*pit) << " in " << (*sit) << endl;
                return true;
            }
        }
    }

    return false;
}


void ImportDialog::dirChanged()
{
    QString dirName = urlinput_edit->url();
    QDir dir(dirName);
    if (!dir.exists())
        return;

    // KDevelop legacy project?
    QStringList files = dir.entryList("*.kdevprj");
    if (!files.isEmpty()) {
        scanLegacyKDevelopProject(dir.absFilePath(files.first()));
        return;
    }

    // Studio legacy project?
    files = dir.entryList("*.studio");
    if (!files.isEmpty()) {
        scanLegacyStudioProject(dir.absFilePath(files.first()));
        return;
    }

    // Automake based?
    if (dir.exists("config.guess") || dir.exists("configure.in.in")) {
        scanAutomakeProject(dirName);
        return;
    }

    name_edit->setText(dir.dirName());

    // QMake based?
    files = dir.entryList("*.pro");
    if (!files.isEmpty()) {
        setProjectType("qtqmake");
        return;
    }

    // C++?
    if (dirHasFiles(dir, "*.cpp,*.c++,*.cxx,*.C,*.cc")) {
        setProjectType("cpp");
        return;
    }

    // Fortran?
    if (dirHasFiles(dir, "*.f77,*.f,*.for,*.ftn")) {
        setProjectType("fortran");
        return;
    }

    // Python?
    if (dirHasFiles(dir, "*.py")) {
        setProjectType("python");
        return;
    }

    // Perl?
    if (dirHasFiles(dir, "*.pl,*.pm")) {
        setProjectType("perl");
        return;
    }
}


void ImportDialog::scanLegacyKDevelopProject(const QString &fileName)
{
    kdDebug(9010) << "Scanning legacy KDevelop project file " << fileName << endl;

    KSimpleConfig config(fileName, true);
    config.setGroup("General");
    author_edit->setText(config.readEntry("author"));
    email_edit->setText(config.readEntry("email"));
    name_edit->setText(config.readEntry("project_name"));

    QString legacyType = config.readEntry("project_type");
    if (QStringList::split(",", "normal_kde,normal_kde2,kde2_normal,mdi_kde2").contains(legacyType))
        setProjectType("kde");
    else if (legacyType == "normal_gnome")
        setProjectType("gnome");
    else if (legacyType == "normal_empty")
        setProjectType("cpp-auto");
    else
        setProjectType("cpp");
}


void ImportDialog::scanLegacyStudioProject(const QString &fileName)
{
    kdDebug(9010) << "Scanning legacy studio project file " << fileName << endl;

    // Not much to do here...
    KSimpleConfig config(fileName, true);
    config.setGroup("kdestudio");
    name_edit->setText(config.readEntry("Name"));
}


void ImportDialog::scanAutomakeProject(const QString &dirName)
{
    kdDebug(9010) << "Scanning automake project directory " << dirName << endl;

    bool stop = false;
    if (QFile::exists(dirName + "/admin/am_edit")) {
        setProjectType("kde");
        stop = true;
    } else if (QFile::exists(dirName + "/macros/gnome.m4")) {
        setProjectType("gnome");
        stop = true;
    } else {
        setProjectType("c-auto");
    }

    QFile af(dirName + "/AUTHORS");
    if (!af.open(IO_ReadOnly))
        return;
    QTextStream astream(&af);

    QRegExp authorre("(.*)<(.*)>");
    while (!astream.atEnd()) {
        QString s = astream.readLine();
        if (authorre.search(s) != -1) {
            author_edit->setText(authorre.cap(1).stripWhiteSpace());
            email_edit->setText(authorre.cap(2).stripWhiteSpace());
            break;
        }
    }
    af.close();

    QFile cf(dirName + "/configure.in");
    if (!cf.open(IO_ReadOnly))
        return;
    QTextStream cstream(&cf);

    QRegExp namere("\\s*AM_INIT_AUTOMAKE\\((.*),.*\\).*");
    QRegExp cppre("\\s*AC_PROG_CXX");
    QRegExp f77re("\\s*AC_PROG_F77");
    while (!cstream.atEnd()) {
        QString s = cstream.readLine();
        if (namere.search(s) == 0)
            name_edit->setText(namere.cap(1).stripWhiteSpace());
        if (!stop)
            continue;
        else if (cppre.search(s) == 0)
            setProjectType("cpp-auto");
        else if (f77re.search(s) == 0)
            setProjectType("fortran-auto");
    }
    cf.close();
}


void ImportDialog::setProjectType(const QString &type)
{
    kdDebug(9010) << "Setting project type " << type << endl;
    QString suffix = "/" + type;
    int suffixLength = suffix.length();

    int i=0;
    QStringList::ConstIterator it;
    for (it = importNames.begin(); it != importNames.end(); ++it) {
        if ((*it).right(suffixLength) == suffix) {
            project_combo->setCurrentItem(i);
            break;
        }
        ++i;
    }
}

void ImportDialog::scanAvailableVCS()
{
    vcsCombo->insertStringList( m_part->registeredVersionControls() );
}

void ImportDialog::slotFinishedCheckout( QString destinationDir )
{
    urlinput_edit->setURL( destinationDir );

    setCursor( KCursor::arrowCursor() );
//    setEnabled( true );
}

void ImportDialog::slotFetchModulesFromRepository()
{
    KDevVersionControl *vcs = m_part->versionControlByName( vcsCombo->currentText() );
    if (!vcs)
        return;

    setCursor( KCursor::waitCursor() );
//    setEnabled( false );

    connect( vcs, SIGNAL(finishedFetching(QString)),
        this, SLOT(slotFinishedCheckout(QString)) );

    //restore cursor if we can't fetch repository
    if ( !vcs->fetchFromRepository() )
        setCursor( KCursor::arrowCursor() );
}


#include "importdlg.moc"
