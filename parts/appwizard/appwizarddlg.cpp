/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qgrid.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstddirs.h>
#include <ktempfile.h>

#include "kdevmakefrontend.h"
#include "appwizardpart.h"
#include "appwizarddlg.h"


static void guessAuthorAndEmail(QString *author, QString *email)
{
    char hostname[512];
    
    struct passwd *pw = ::getpwuid(getuid());
    // pw==0 => the system must be really fucked up
    if (!pw)
        return;

    // I guess we don't have to support users with longer host names ;-)
    (void) ::gethostname(hostname, sizeof hostname);

    *author = pw->pw_gecos;
    *email = QString(pw->pw_name) + "@" + hostname;
}


AppWizardDialog::AppWizardDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : QWizard(parent, name)
{
    firstpage = new QVBox(this);
    firstpage->setMargin(10);
    firstpage->setSpacing(4);
    
    QButtonGroup *templatebox = new QButtonGroup(1, Horizontal, i18n("Templates"), firstpage);
    firstpage->setStretchFactor(templatebox, 1);

    KGlobal::dirs()->addResourceType("apptemplates", KStandardDirs::kde_default("data") + "gideon/templates/");
    templateNames = KGlobal::dirs()->findAllResources("apptemplates", QString::null, false, true);
    kdDebug(9010) << "Templates: " << endl;
    
    QStringList::Iterator it;
    for (it = templateNames.begin(); it != templateNames.end(); ++it) {
        kdDebug(9010) << (*it) << endl;
        KConfig config(KGlobal::dirs()->findResource("apptemplates", *it));
        config.setGroup("General");
        QString name = config.readEntry("Name");
        QString comment = config.readEntry("Comment");
        QRadioButton *button = new QRadioButton(name, templatebox);
        templateButtons.append(button);
        QToolTip::add(button, QString("<qt>") + comment + "</qt>");
        if (it == templateNames.begin()) {
            button->setChecked(true);
            button->setFocus();
        }
    }

    QGrid *firstpagegrid = new QGrid(2, firstpage);
    firstpagegrid->setSpacing(4);
    firstpage->setStretchFactor(firstpagegrid, 2);

    QLabel *appname_label = new QLabel(i18n("Application &name:"), firstpagegrid);
    appname_edit = new QLineEdit(firstpagegrid);
    appname_label->setBuddy(appname_edit);
    

    QLabel *dest_label = new QLabel(i18n("&Location:"), firstpagegrid);
    dest_edit = new QLineEdit(firstpagegrid);
    dest_label->setBuddy(dest_edit);

    QString author, email;
    guessAuthorAndEmail(&author, &email);
    
    QLabel *author_label = new QLabel(i18n("&Author:"), firstpagegrid);
    author_edit = new QLineEdit(firstpagegrid);
    author_edit->setText(author);
    author_label->setBuddy(author_edit);
    
    QLabel *email_label = new QLabel(i18n("&Email:"), firstpagegrid);
    email_edit = new QLineEdit(firstpagegrid);
    email_edit->setText(email);
    email_label->setBuddy(email_edit);
    
    QLabel *version_label = new QLabel(i18n("&Version:"), firstpagegrid);
    version_edit = new QLineEdit(firstpagegrid);
    version_edit->setText("0.1");
    version_label->setBuddy(version_edit);
    
    QLabel *license_label = new QLabel(i18n("L&icense:"), firstpagegrid);
    license_combo = new QComboBox(false, firstpagegrid);
    license_combo->insertItem("BSD");
    license_combo->insertItem("QPL");
    license_combo->insertItem("GPL");
    license_combo->insertItem("LGPL");
    license_combo->insertItem(i18n("Custom"));
    license_label->setBuddy(license_combo);
    
    secondpage = new QVBox(this);
    secondpage->setMargin(10);
    secondpage->setSpacing(4);
    
    (void) new QLabel(i18n("File template"), secondpage);
    filetemplate_edit = new QMultiLineEdit(secondpage);
    filetemplate_edit->setFont(KGlobalSettings::fixedFont());
    QFontMetrics fm(filetemplate_edit->fontMetrics());
    filetemplate_edit->setMinimumSize(fm.width("X")*81, fm.lineSpacing()*22);

    addPage(firstpage, i18n("General"));
    addPage(secondpage, i18n("File headers"));
    helpButton()->hide();

    connect( appname_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( dest_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( author_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( version_edit, SIGNAL(textChanged(const QString&)),
             this, SLOT(textChanged()) );
    connect( license_combo, SIGNAL(activated(int)),
             this, SLOT(licenseChanged()) );
    licenseChanged();

    tempFile = 0;
    m_part = part;
}


AppWizardDialog::~AppWizardDialog()
{
    delete tempFile;
}


void AppWizardDialog::textChanged()
{
    bool invalid = appname_edit->text().isEmpty()
        || dest_edit->text().isEmpty()
        || author_edit->text().isEmpty()
        || version_edit->text().isEmpty();
    setFinishEnabled(secondpage, !invalid);
}


void AppWizardDialog::licenseChanged()
{
    QString str =
        "/***************************************************************************\n"
        " *   Copyright (C) 2001 by $AUTHOR$                                        *\n"
        " *   $EMAIL$                                                               *\n"
        " *                                                                         *\n";

    QString author = author_edit->text();
    QString email = email_edit->text();
    str.replace(str.find("2001"), 4, QString::number(QDate::currentDate().year()));
    str.replace(str.find("$AUTHOR$                      "), QMIN(30, author.length()), author);
    str.replace(str.find("$EMAIL$                       "), QMIN(30, email.length()), email);
    
    switch (license_combo->currentItem())
        {
        case 0:
            str +=
                " *   Permission is hereby granted, free of charge, to any person obtaining *\n"
                " *   a copy of this software and associated documentation files (the       *\n"
                " *   \"Software\"), to deal in the Software without restriction, including   *\n"
                " *   without limitation the rights to use, copy, modify, merge, publish,   *\n"
                " *   distribute, sublicense, and/or sell copies of the Software, and to    *\n"
                " *   permit persons to whom the Software is furnished to do so, subject to *\n"
                " *   the following conditions:                                             *\n"
                " *                                                                         *\n"
                " *   The above copyright notice and this permission notice shall be        *\n"
                " *   included in all copies or substantial portions of the Software.       *\n"
                " *                                                                         *\n"
                " *   THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,       *\n"
                " *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *\n"
                " *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*\n"
                " *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR     *\n"
                " *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, *\n"
                " *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR *\n"
                " *   OTHER DEALINGS IN THE SOFTWARE.                                       *\n";
            break;
        case 1:
            str +=
                " *   This program may be distributed under the terms of the Q Public       *\n"
                " *   License as defined by Trolltech AS of Norway and appearing in the     *\n"
                " *   file LICENSE.QPL included in the packaging of this file.              *\n"
                " *                                                                         *\n"
                " *   This program is distributed in the hope that it will be useful,       *\n"
                " *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
                " *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *\n";
            break;
        case 2:
            str +=
                " *   This program is free software; you can redistribute it and/or modify  *\n"
                " *   it under the terms of the GNU General Public License as published by  *\n"
                " *   the Free Software Foundation; either version 2 of the License, or     *\n"
                " *   (at your option) any later version.                                   *\n";
            break;
        case 3:
            str +=
                " *   This program is free software; you can redistribute it and/or modify  *\n"
                " *   it under the terms of the GNU Library General Public License as       *\n"
                " *   published by the Free Software Foundation; either version 2 of the    *\n"
                " *   License, or (at your option) any later version.                       *\n";
        }

    str += " ***************************************************************************/\n";
    
    filetemplate_edit->setText(str);
}


void AppWizardDialog::accept()
{
    QFileInfo fi(dest_edit->text());
    if (fi.exists()) {
        KMessageBox::sorry(this, i18n("The directory you have chosen as location for\n"
                                      "the project already exists."));
        showPage(firstpage);
        dest_edit->setFocus();
        return;
    }

    if (!fi.dir().exists()) {
        KMessageBox::sorry(this, i18n("The directory above the chosen location does not exist."));
        showPage(firstpage);
        dest_edit->setFocus();
        return;
    }

    QString appname = appname_edit->text();
    for (uint i=0; i < appname.length(); ++i)
        if (!appname[i].isLetterOrNumber()) {
            KMessageBox::sorry(this, i18n("Your application name should only contain letters and numbers."));
            showPage(firstpage);
            appname_edit->setFocus();
            return;
        }

    // Do something smarter here...
    if (m_part->makeFrontend()->isRunning()) {
        KMessageBox::sorry(0, i18n("There is currently a job running."));
        return;
    }

    QString source, script;
    for (uint i=0; i < templateButtons.count(); ++i) {
        // Argl
        QList<QRadioButton> *list = const_cast< QList<QRadioButton>* >(&templateButtons);
        if (list->at(i)->isChecked()) {
            QFileInfo fi(templateNames[i]);
            QDir dir(fi.dir());
            dir.cdUp();
            source = dir.absPath();
            script = dir.filePath("template-" + fi.fileName() + "/script");
        }
    }
    
    QString license =
        (license_combo->currentItem()<4)? license_combo->currentText() : QString("Custom");
    
    QString licensefile;
    switch (license_combo->currentItem())
        {
        case 0: licensefile = "LICENSE.BSD"; break;
        case 1: licensefile = "LICENSE.QPL"; break;
        case 2: licensefile = "COPYING";     break;
        case 3: licensefile = "COPYING.LIB"; break;
        default: ;
        }

    if (!tempFile) {
        tempFile = new KTempFile();
        tempFile->setAutoDelete(true);
    }
    // KTempFile sucks
    QFile f;
    f.open(IO_WriteOnly, tempFile->handle());
    QTextStream temps(&f);
    temps << filetemplate_edit->text();
    f.flush();
    
    
    QString cmdline = "perl ";
    cmdline += script;
    cmdline += " --author=";
    cmdline += KShellProcess::quote(author_edit->text());
    cmdline += " --email=";
    cmdline +=  KShellProcess::quote(email_edit->text());
    cmdline += " --version=";
    cmdline +=  KShellProcess::quote(version_edit->text());
    cmdline += " --appname=";
    cmdline +=  KShellProcess::quote(appname_edit->text());
    cmdline += " --dest=";
    cmdline +=  KShellProcess::quote(dest_edit->text());
    cmdline += " --source=";
    cmdline +=  KShellProcess::quote(source);
    cmdline += " --license=";
    cmdline +=  KShellProcess::quote(license);
    cmdline += " --licensefile=";
    cmdline += KShellProcess::quote(licensefile);
    cmdline += " --filetemplate=";
    cmdline += KShellProcess::quote(tempFile->name());

    m_part->makeFrontend()->startMakeCommand(QDir::homeDirPath(), cmdline);

    QWizard::accept();
}
#include "appwizarddlg.moc"
