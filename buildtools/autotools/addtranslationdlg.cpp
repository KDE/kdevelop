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

#include "addtranslationdlg.h"

#include <qcombobox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstrlist.h>
#include <kbuttonbox.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>
#include <kdeversion.h>

#include "misc.h"
#include "autoprojectpart.h"


AddTranslationDialog::AddTranslationDialog(AutoProjectPart *part, QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Add Translation"));

    m_part = part;

    QHBox *hbox = new QHBox(this);
    (void) new QLabel(i18n("Language:"), hbox);
    lang_combo = new QComboBox(hbox);

    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
    layout->addWidget(hbox);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addWidget(frame, 0);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
    QPushButton *ok_button = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel_button = buttonbox->addButton(KStdGuiItem::cancel());
    ok_button->setDefault(true);
    connect( ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();
    layout->addWidget(buttonbox, 0);

    QStringList rawlist, list;
    rawlist << "af" << "ar" << "bg" << "bo" << "br" << "bs" << "ca" << "cs" << "cy" << "da"
            << "de" << "el" << "en_GB"   << "eo" << "es" << "et" << "eu" << "fi" << "fr";
    rawlist << "ga" << "gl" << "gu" << "he" << "hi" << "hu" << "id" << "is" << "it" << "ja"
            << "km" << "ko" << "lt" << "lv" << "mi" << "mk" << "mr" << "nl" << "no" << "no_NY";
    rawlist << "oc" << "pl" << "pt" << "pt_BR" << "ro" << "ru" << "sk" << "sl" << "sr" << "sv"
            << "ta" << "th" << "tr" << "uk" << "wa" << "zh_CN.GB2312" << "zh_TW.Big5";

    // Remove already added languages
    QStringList::ConstIterator it;
    for (it = rawlist.begin(); it != rawlist.end(); ++it) {
        QFileInfo fi(m_part->projectDirectory() + "/po/" + (*it) + ".po");
        if (!fi.exists())
            list.append(*it);
    }

    if (list.isEmpty()) {
        KMessageBox::information(this, i18n("Your sourcecode is already translated to all supported languages."));
        ok_button->setEnabled(false);
    }
    lang_combo->insertStringList(list);
}


AddTranslationDialog::~AddTranslationDialog()
{}


void AddTranslationDialog::accept()
{
    QString dir = m_part->projectDirectory() + "/po";
    QString fileName = dir + "/" + lang_combo->currentText() + ".po";

    QFile f(fileName);
    if (f.exists()) {
        KMessageBox::information(this, i18n("A translation file for the language %1 exists already."));
        return;
    }
    f.open(IO_WriteOnly);
    f.close();

    dir = m_part->buildDirectory() + "/po";
    m_part->startMakeCommand(dir, QString::fromLatin1("force-reedit"));

    QDialog::accept();
}

#include "addtranslationdlg.moc"
