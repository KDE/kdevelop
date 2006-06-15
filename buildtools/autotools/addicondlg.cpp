/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "addicondlg.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <klineedit.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kicontheme.h>
#include <kdeversion.h> // fix me!

#include "autolistviewitems.h"

#include "autoprojectpart.h"
#include "autoprojectwidget.h"


const char *type_map[] = {
    "app", "action", "device", "filesys", "mime"
};


const char *size_map[] = {
    "hi16", "hi22", "hi32", "hi48", "hi64", "hi128"
};


AddIconDialog::AddIconDialog(AutoProjectPart *part, AutoProjectWidget *widget,
                             SubprojectItem *spitem, TargetItem *titem,
                             QWidget *parent, const char *name)
    : AddIconDialogBase(parent, name, true)
{
    type_combo->insertItem(i18n("Application"));
    type_combo->insertItem(i18n("Action"));
    type_combo->insertItem(i18n("Device"));
    type_combo->insertItem(i18n("File System"));
    type_combo->insertItem(i18n("MIME Type"));

    size_combo->insertItem("16");
    size_combo->insertItem("22");
    size_combo->insertItem("32");
    size_combo->insertItem("48");
    size_combo->insertItem("64");
    size_combo->insertItem("128");

    somethingChanged();

    setIcon ( SmallIcon ( "iconadd_kdevelop" ) );

    m_part = part;
    m_widget = widget;
    m_subProject = spitem;
    m_target = titem;
}


AddIconDialog::~AddIconDialog()
{}


void AddIconDialog::somethingChanged()
{
    QString size = size_map[size_combo->currentItem()];
    QString type = type_map[type_combo->currentItem()];
    QString name = name_edit->text();

    filename_edit->setText(size + "-" + type + "-" + name + ".png");
}


void AddIconDialog::accept()
{
    QString name = filename_edit->text();
    
    QString destdir = m_subProject->subdir;
    QString destpath = destdir + "/" + name;

    QString size = size_combo->currentText();
#if KDE_VERSION < 310
    QString unknown = "unknown"; // fix me!
#else    
    QString unknown = KIconTheme::defaultThemeName()+ "/" + size + "x" + size + "/mimetypes/unknown.png";
#endif    
    QString templateFileName = locate("icon", unknown);
    kdDebug(9020) << "Unknown: " << unknown << ", template: " << templateFileName << endl;

    if (!templateFileName.isEmpty()) {
        KProcess proc;
        proc << "cp";
        proc << templateFileName;
        proc << destpath;
        proc.start(KProcess::DontCare);
    }
    
    FileItem *fitem = m_widget->createFileItem(name, m_subProject);
    m_target->sources.append(fitem);
    m_target->insertItem(fitem);

    m_part->startMakeCommand(destdir, QString::fromLatin1("force-reedit"));
	
	m_widget->emitAddedFile(destpath);

    QDialog::accept();
}

#include "addicondlg.moc"
