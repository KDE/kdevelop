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

#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "appwizardfactory.h"
#include "appwizardpart.h"
#include "importdlg.h"
#include "kdevcore.h"
#include <qpushbutton.h>
#include <kinstance.h>


ImportDialog::ImportDialog(AppWizardPart *part, QWidget *parent, const char *name)
    : QDialog(parent, name, true), m_part(part)
{
    setCaption(i18n("Import Directory"));

    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

    QLabel *name_label = new QLabel(i18n("Project &name:"), this);
    
    name_edit = new QLineEdit(this);
    name_edit->setFocus();
    name_label->setBuddy(name_edit);
    
    QLabel *dir_label = new QLabel(i18n("&Directory:"), this);

    dir_edit = new QLineEdit(this);
    QFontMetrics fm(dir_edit->fontMetrics());
    dir_edit->setMinimumWidth(fm.width("X")*35);
    dir_label->setBuddy(dir_edit);
    
    QPushButton *dir_button = new QPushButton("...", this);
    dir_button->setFixedSize(30, 25);
    connect( dir_button, SIGNAL(clicked()), this, SLOT(dirButtonClicked()) );
    
    QLabel *project_label = new QLabel(i18n("&Project type:"), this);

    project_combo = new QComboBox(this);
    project_label->setBuddy(project_combo);

    QGridLayout *grid = new QGridLayout(3, 2);
    layout->addLayout(grid);

    grid->addWidget(name_label, 0, 0);
    grid->addWidget(name_edit, 0, 1);
    grid->addWidget(dir_label, 1, 0);
    grid->addWidget(dir_edit, 1, 1);
    grid->addWidget(dir_button, 1, 2);
    grid->addWidget(project_label, 2, 0);
    grid->addWidget(project_combo, 2, 1);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *box = new KButtonBox(this);
    box->addStretch();
    QPushButton *ok_button = box->addButton(i18n("&OK"));
    QPushButton *cancel_button = box->addButton(i18n("&Cancel"));
    ok_button->setDefault(true);
    box->layout();
    layout->addWidget(box);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );

    KStandardDirs *dirs = AppWizardFactory::instance()->dirs();
    importNames = dirs->findAllResources("appimports", QString::null, false, true);
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

#include "importdlg.moc"
