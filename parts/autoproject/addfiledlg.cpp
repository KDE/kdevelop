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

#include "addfiledlg.h"

#include <qcheckbox.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <kbuttonbox.h>
#include <kdialog.h>
#include <kmessagebox.h>

#include "filetemplate.h"
#include "misc.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"


AddFileDialog::AddFileDialog(AutoProjectPart *part, AutoProjectWidget *widget,
                             SubprojectItem *spitem, TargetItem *item,
                             QWidget *parent, const char *name)
    : QDialog(parent, name, true)
{
    setCaption(i18n("Add File to Target"));

    QLabel *filename_label = new QLabel(i18n("&File name:"), this);

    filename_edit = new QLineEdit(this);
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
    QPushButton *ok_button = buttonbox->addButton(i18n("&OK"));
    QPushButton *cancel_button = buttonbox->addButton(i18n("Cancel"));
    ok_button->setDefault(true);
    connect( ok_button, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancel_button, SIGNAL(clicked()), this, SLOT(reject()) );
    buttonbox->layout();

    QVBoxLayout *layout = new QVBoxLayout(this, 2*KDialog::marginHint(), KDialog::spacingHint());
    layout->addWidget(filename_label);
    layout->addWidget(filename_edit);
    layout->addWidget(usetemplate_box);
    layout->addWidget(frame, 0);
    layout->addWidget(buttonbox, 0);

    m_part = part;
    m_widget = widget;
    subProject = spitem;
    target = item;
}


AddFileDialog::~AddFileDialog()
{}


void AddFileDialog::accept()
{
    QString name = filename_edit->text();
    if (name.find('/') != -1) {
        KMessageBox::sorry(this, i18n("Please enter the file name without '/' and so on."));
        return;
    }

    QListViewItem *child = target->firstChild();
    while (child) {
        FileItem *item = static_cast<FileItem*>(child);
        if (name == item->name) {
            KMessageBox::sorry(this, i18n("This file is already in the target."));
            return;
        }
        child = child->nextSibling();
    }
        
    if (usetemplate_box->isChecked()) {
        QString srcdir = m_widget->projectDirectory();
        QString destdir = subProject->subdir;
        QString destpath = destdir + "/" + name;
        if (QFileInfo(destpath).exists()) {
            KMessageBox::sorry(this, i18n("A file with this name already exists."));
            return;
        }
        FileTemplate::copy(m_part, "cpp", destpath);
    }

    FileItem *fitem = m_widget->createFileItem(name);
    target->sources.append(fitem);
    target->insertItem(fitem);
    
    QCString canontargetname = AutoProjectTool::canonicalize(target->name);
    QCString varname = canontargetname + "_SOURCES";
    subProject->variables[varname] += (QCString(" ") + name.latin1());
    
    QMap<QCString,QCString> replaceMap;
    replaceMap.insert(varname, subProject->variables[varname]);
    
    AutoProjectTool::modifyMakefileam(subProject->path + "/Makefile.am", replaceMap);

    m_widget->emitAddedFile(subProject->path + "/" + name);
    
    QDialog::accept();
}

#include "addfiledlg.moc"
