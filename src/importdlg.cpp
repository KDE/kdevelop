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

#include <qdir.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtextstream.h>
#include <kbuttonbox.h>
#include <kdialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <ktrader.h>

#include "servicecombobox.h"
#include "importdlg.h"


ImportDialog::ImportDialog(QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Import directory"));

    QBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

    QLabel *info_label = new QLabel(i18n("<qt>This dialog allows you to create a project file in an "
                                         "existing directory. Using a project file has the advantage "
                                         "that you can use project management functionality and you "
                                         "can configure which plugins you want to use for that project</qt>"), this);
    layout->addWidget(info_label);

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
    
    QLabel *project_label = new QLabel(i18n("&Project management:"), this);

    KTrader::OfferList projectOffers =
        KTrader::self()->query("KDevelop/Project", QString::null);
    project_combo = new ServiceComboBox(projectOffers, this);
    project_label->setBuddy(project_combo);

    QLabel *language_label = new QLabel(i18n("Primary programming &language:"), this);

    language_combo = new QComboBox(this);
    language_label->setBuddy(language_combo);
    KTrader::OfferList languageOffers =
        KTrader::self()->query("KDevelop/LanguageSupport", QString::null);
    QValueList<KService::Ptr>::ConstIterator it;
    for (it = languageOffers.begin(); it != languageOffers.end(); ++it)
        language_combo->insertItem((*it)->property("X-KDevelop-Language").toString());

    QGridLayout *grid = new QGridLayout(2, 2);
    layout->addLayout(grid);

    grid->addWidget(name_label, 0, 0);
    grid->addWidget(name_edit, 0, 1);
    grid->addWidget(dir_label, 1, 0);
    grid->addWidget(dir_edit, 1, 1);
    grid->addWidget(dir_button, 1, 2);
    grid->addWidget(project_label, 2, 0);
    grid->addMultiCellWidget(project_combo, 2, 2, 1, 2);
    grid->addWidget(language_label, 3, 0);
    grid->addMultiCellWidget(language_combo, 3, 3, 1, 2);

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
}


ImportDialog::~ImportDialog()
{}


void ImportDialog::accept()
{
    QString project = project_combo->currentText();
    if (project.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to choose a projectmanagement component."));
        return;
    }

    QString language = language_combo->currentText();
    
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
    
    QFile f(dir.filePath(projectName + ".kdevelop"));
    if (!f.open(IO_WriteOnly)) {
        KMessageBox::sorry(this, i18n("Can not write the project file."));
        return;
    }

    QTextStream stream(&f);
    stream << "<!DOCTYPE kdevelop>\n<kdevelop>\n  <general>\n"
           << "    <projectmanagement>" << project << "</projectmanagement>\n";
    if (!language.isEmpty())
        stream << "    <primarylanguage>" << language << "</primarylanguage>\n";
    stream << "  </general>\n</kdevelop>\n";
    f.close();
    
    QDialog::accept();
}


void ImportDialog::dirButtonClicked()
{
    QString dir = KFileDialog::getExistingDirectory(QString::null, this,
                                                    i18n("Choose a directory to import"));
    dir_edit->setText(dir);
}

#include "importdlg.moc"
