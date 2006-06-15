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

#include "scriptnewfiledlg.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klineedit.h>
#include <qpushbutton.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdguiitem.h>
#include <kdeversion.h>

#include "scriptprojectpart.h"
#include "filetemplate.h"


ScriptNewFileDialog::ScriptNewFileDialog(ScriptProjectPart *part,
                                         QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("New File"));

    QLabel *filename_label = new QLabel(i18n("&File name:"), this);

    filename_edit = new KLineEdit(this);
    filename_edit->setFocus();
    filename_label->setBuddy(this);
    QFontMetrics fm(filename_edit->fontMetrics());
    filename_edit->setMinimumWidth(fm.width('X')*35);

    usetemplate_box = new QCheckBox(i18n("&Use file template"), this);
    usetemplate_box->setChecked(true);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    KButtonBox *buttonbox = new KButtonBox(this);
    buttonbox->addStretch();
#if KDE_IS_VERSION( 3, 2, 90 )
    QPushButton *ok_button = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel_button = buttonbox->addButton(KStdGuiItem::cancel());
#else
    QPushButton *ok_button = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel_button = buttonbox->addButton(i18n("Cancel"));
#endif
    ok_button->setDefault(true);
    connect( ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();

    QVBoxLayout *layout = new QVBoxLayout(this, 10, 4);
    layout->addWidget(filename_label);
    layout->addWidget(filename_edit);
    layout->addWidget(usetemplate_box);
    layout->addWidget(frame, 0);
    layout->addWidget(buttonbox, 0);

    m_part = part;
}


ScriptNewFileDialog::~ScriptNewFileDialog()
{}


void ScriptNewFileDialog::accept()
{
    QString fileName = filename_edit->text();
    if (fileName.find('/') != -1) {
        KMessageBox::sorry(this, i18n("Please enter the file name without '/' and so on."));
        return;
    }

    KDevProject *project = m_part->project();
    if (!project->activeDirectory().isEmpty())
        fileName.prepend(project->activeDirectory() + "/");
    QString destpath = project->projectDirectory() + "/" + fileName;

    if (QFileInfo(destpath).exists()) {
        KMessageBox::sorry(this, i18n("A file with this name already exists."));
        return;
    }

    bool success = false;

    if (usetemplate_box->isChecked()) {
        QString extension = QFileInfo(destpath).extension();
        if (!FileTemplate::exists(m_part, extension)) {
            KMessageBox::sorry(this, i18n("A file template for this extension does not exist."));
            return;
        }
        success = FileTemplate::copy(m_part, extension, destpath);
    } else {
        QFile f(destpath);
        success = f.open(IO_WriteOnly);
        if (success)
            f.close();
    }

    if (!success)
        KMessageBox::sorry(this, i18n("Could not create the new file."));

    kdDebug(9015) << "AddFile1: " << fileName << endl;
    m_part->addFile(fileName);

    QDialog::accept();
}

#include "scriptnewfiledlg.moc"
