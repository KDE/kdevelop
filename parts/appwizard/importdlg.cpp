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

#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include "kdevcore.h"

#include "importdlg.h"
#include "misc.h"
#include "appwizardfactory.h"
#include "appwizardpart.h"


ImportDialog::ImportDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : ImportDialogBase(parent, name, true), m_part(part)
{
    QString author, email;
    AppWizardUtil::guessAuthorAndEmail(&author, &email);
    author_edit->setText(author);
    email_edit->setText(email);
    
    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    importNames = dirs->findAllResources("appimports", QString::null, false, true);
    importNames.sort();
    
    QStringList::ConstIterator it;
    for (it = importNames.begin(); it != importNames.end(); ++it) {
        KConfig config(KGlobal::dirs()->findResource("appimports", *it));
        config.setGroup("General");
        project_combo->insertItem(config.readEntry("Comment"));
    }
}


ImportDialog::~ImportDialog()
{}


void ImportDialog::accept()
{
    QDir dir(dir_edit->text());
    if (!dir.exists()) {
        KMessageBox::sorry(this, i18n("You have to choose a directory."));
        return;
    }

    QString projectName = name_edit->text();
    if (projectName.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to choose a project name."));
        return;
    }

    for (uint i=0; i < projectName.length(); ++i)
        if (!projectName[i].isLetterOrNumber()) {
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


void ImportDialog::dirButtonClicked()
{
    QString dir = KFileDialog::getExistingDirectory(QString::null, this,
                                                    i18n("Choose a directory to import"));
    dir_edit->setText(dir);
}


void ImportDialog::dirChanged()
{
    QString dirName = dir_edit->text();
    QDir dir(dir_edit->text());
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
    if (dir.exists("acinclude.m4")) {
        scanAutomakeProject(dirName);
        return;
    }

    // Python?
    if (!dir.entryList("*.py").isEmpty()) {
        setProjectType("python");
        return;
    }

    // Perl?
    if (!dir.entryList("*.pl").isEmpty()) {
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
    setProjectType("cpp-auto");

    QFile af(dirName + "/AUTHORS");
    if (!af.open(IO_ReadOnly))
        return;
    QTextStream astream(&af);
    
    QRegExp are("(.*)<(.*)>");
    while (!astream.atEnd()) {
        QString s = astream.readLine();
        if (are.search(s) != -1) {
            author_edit->setText(are.cap(1).stripWhiteSpace());
            email_edit->setText(are.cap(2).stripWhiteSpace());
            break;
        }
    }
    af.close();
      
    QFile cf(dirName + "/configure.in");
    if (!cf.open(IO_ReadOnly))
        return;
    QTextStream cstream(&cf);
    
    QRegExp cre("AM_INIT_AUTOMAKE\\((.*),.*\\).*");
    while (!cstream.atEnd()) {
        QString s = cstream.readLine();
        if (cre.search(s) != -1) {
            name_edit->setText(cre.cap(1).stripWhiteSpace());
            break;
        }
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

#include "importdlg.moc"
