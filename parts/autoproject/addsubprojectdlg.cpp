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

#include "addsubprojectdlg.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtextstream.h>
#include <kbuttonbox.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include "autolistviewitems.h"

#include "kdevmakefrontend.h"
#include "misc.h"
#include "autoprojectpart.h"
#include "autosubprojectview.h"


AddSubprojectDialog::AddSubprojectDialog(AutoProjectPart *part, AutoSubprojectView *view,
                                         SubprojectItem *item, QWidget *parent, const char *name)
    : AddSubprojectDlgBase(parent, name, true)
{
    setIcon(SmallIcon("folder_new.png"));
    
    connect( createButton, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( cancelButton, SIGNAL(clicked()), this, SLOT(reject()) );

    m_subProject = item;
    m_subprojectView = view;
    m_part = part;
}


AddSubprojectDialog::~AddSubprojectDialog()
{}


void AddSubprojectDialog::accept()
{
    QString name = spEdit->text().stripWhiteSpace();

    if (name.isEmpty()) {
        KMessageBox::sorry(this, i18n("You have to give the subproject a name."));
        return;
    }

    QListViewItem *childItem = m_subProject->firstChild();
    while (childItem) {
        if (name == static_cast<SubprojectItem*>(childItem)->subdir) {
            KMessageBox::sorry(this, i18n("A subproject with this name already exists."));
            return;
        }
        childItem = childItem->nextSibling();
    }
 
    QDir      dir( m_subProject->path );
    QFileInfo file( dir, name );
    
    if( file.exists() && !file.isDir() ) {
        KMessageBox::sorry(this, i18n("A file named %1 already exists.").arg(name));
        return;
    } else if( file.isDir() ) {
        if( KMessageBox::warningContinueCancel(this,
               i18n("A subdirectory %1 already exists. "
                    "Do you wish to add it as a subproject?").arg(name)) 
            == KMessageBox::Cancel )
            return;
    } else if (!dir.mkdir(name)) {
        KMessageBox::sorry(this, i18n("Could not create subdirectory %1.").arg(name));
        return;
    }

    if(!dir.cd(name)) {
       KMessageBox::sorry(this, i18n("Could not access the subdirectory %1.").arg(name));
       return;
    }
    
    // Adjust SUBDIRS variable in containing Makefile.am
    m_subProject->variables["SUBDIRS"] += (" " + name);
    QMap<QString,QString> replaceMap;
    replaceMap.insert("SUBDIRS", m_subProject->variables["SUBDIRS"]);
    AutoProjectTool::modifyMakefileam(m_subProject->path + "/Makefile.am", replaceMap);

    // Create new item in tree view
    SubprojectItem *newitem = new SubprojectItem(m_subProject, name);
    newitem->subdir = name;
    newitem->path = m_subProject->path + "/" + name;
    newitem->variables["INCLUDES"] = m_subProject->variables["INCLUDES"];
    newitem->setOpen(true);
    
    // Move to the bottom of the list
    QListViewItem *lastItem = m_subProject->firstChild();
    while (lastItem->nextSibling())
        lastItem = lastItem->nextSibling();
    if (lastItem != newitem)
        newitem->moveItem(lastItem);
    
    // Create a Makefile in the new subdirectory
    
    QFile f( dir.filePath("Makefile.am") );
    if (f.exists()) {
        m_subprojectView->parse( newitem );
    } else {
        if (!f.open(IO_WriteOnly)) {
            KMessageBox::sorry(this, i18n("Could not create Makefile.am in subdirectory %1").arg(name));
            return;
        }
        QTextStream stream(&f);
        stream << "INCLUDES = " << newitem->variables["INCLUDES"] << endl;
        f.close();
    }
    
    QString relmakefile = (m_subProject->path + "/" + name + "/Makefile").mid(m_part->projectDirectory().length()+1);
    kdDebug(9020) << "Relative makefile path: " << relmakefile << endl;
    
    QString cmdline = "cd ";
    cmdline += m_part->projectDirectory();
    cmdline += " && automake ";
    cmdline += relmakefile;
    cmdline += " && CONFIG_HEADERS=config.h CONFIG_FILES=";
    cmdline += relmakefile;
    cmdline += " ./config.status";
    m_part->makeFrontend()->queueCommand(m_part->projectDirectory(), cmdline);
    
    QDialog::accept();
}

#include "addsubprojectdlg.moc"
