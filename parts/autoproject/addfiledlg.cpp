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
#include <ksqueezedtextlabel.h>
#include <klineedit.h>

#include "filetemplate.h"
#include "misc.h"
#include "autoprojectpart.h"
#include "autoprojectwidget.h"


AddFileDialog::AddFileDialog(AutoProjectPart *part, AutoProjectWidget *widget,
                             SubprojectItem *spitem, TargetItem *item,
                             QWidget *parent, const char *name)
    : AddFileDlgBase(parent, name, true)
{
    connect ( createButton, SIGNAL ( clicked() ), this, SLOT ( accept() ) );
    connect ( cancelButton, SIGNAL ( clicked() ), this, SLOT ( reject() ) );

    directoryLabel->setText ( spitem->path );
    targetLabel->setText ( item->name );

    setIcon ( SmallIcon ( "filenew.png" ) );

    m_part = part;
    m_widget = widget;
    subProject = spitem;
    target = item;
}


AddFileDialog::~AddFileDialog()
{}


void AddFileDialog::accept()
{
    QString name = fileEdit->text();
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
        
    if (templateCheckBox->isChecked()) {
        QString srcdir = m_widget->projectDirectory();
        QString destdir = subProject->subdir;
        QString destpath = destdir + "/" + name;
        if (QFileInfo(destpath).exists()) {
            KMessageBox::sorry(this, i18n("<b>A file with this name already exists!</b><br><br>Please use the \"Add existing file\" dialog!"));
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
