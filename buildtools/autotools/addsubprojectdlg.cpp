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
#include <qstringlist.h>
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


#if 0
    // check for config.status
    if( !QFileInfo(m_part->projectDirectory(), "config.status").exists() ){
        KMessageBox::sorry(this, i18n("There is no config.status in the project root directory. Run 'Configure' first"));
        QDialog::accept();
	return;
    }
#endif

    QDir      dir( m_subProject->path );
    QFileInfo file( dir, name );

    if( file.exists() && !file.isDir() ) {
        KMessageBox::sorry(this, i18n("A file named %1 already exists.").arg(name));
        QDialog::accept();
        return;
    } else if( file.isDir() ) {
        if( KMessageBox::warningContinueCancel(this,
               i18n("A subdirectory %1 already exists. "
                    "Do you wish to add it as a subproject?").arg(name))
            == KMessageBox::Cancel ){
            QDialog::accept();
            return;
	}
    } else if (!dir.mkdir(name)) {
        KMessageBox::sorry(this, i18n("Could not create subdirectory %1.").arg(name));
        QDialog::accept();
        return;
    }

    if(!dir.cd(name)) {
       KMessageBox::sorry(this, i18n("Could not access the subdirectory %1.").arg(name));
       QDialog::accept();
       return;
    }

    // Adjust SUBDIRS variable in containing Makefile.am
    if (m_subProject->variables["SUBDIRS"].find("$(TOPSUBDIRS)") != -1)
    {
        QFile subdirsfile( m_subProject->path + "/subdirs" );
        if ( subdirsfile.open( IO_WriteOnly | IO_Append ) )
        {
            QTextStream subdirsstream( &subdirsfile );
            subdirsstream << name << endl;
            subdirsfile.close();
        }
    }
    else if (m_subProject->variables["SUBDIRS"].find("$(AUTODIRS)") != -1)
    {
    }
    else
    {
        m_subProject->variables["SUBDIRS"] += (" " + name);
        QMap<QString,QString> replaceMap;
        replaceMap.insert("SUBDIRS", m_subProject->variables["SUBDIRS"]);
        AutoProjectTool::modifyMakefileam(m_subProject->path + "/Makefile.am", replaceMap);
    }

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
            KMessageBox::sorry(this, i18n("Could not create Makefile.am in subdirectory %1.").arg(name));
            return;
        }
        QTextStream stream(&f);
        stream << "INCLUDES = " << newitem->variables["INCLUDES"] << endl << "METASOURCES = AUTO" << endl;
        f.close();
    }



    // if !isKDE: add the new sub-proj to configure.in
    if ( !m_part->isKDE() ) {
        QString projroot = m_part->projectDirectory() + "/";
        QString subdirectory = dir.path();
        QString relpath = subdirectory.replace(0, projroot.length(),"");

        QString configurein = projroot + "configure.in";

        QStringList list = AutoProjectTool::configureinLoadMakefiles(configurein);
        list.push_back( relpath + "/Makefile" );
        AutoProjectTool::configureinSaveMakefiles(configurein, list);
    }

#if 0
    QString relmakefile = (m_subProject->path + "/" + name + "/Makefile").mid(m_part->projectDirectory().length()+1);
    kdDebug(9020) << "Relative makefile path: " << relmakefile << endl;

    QString cmdline = "cd ";
    cmdline += KProcess::quote(m_part->projectDirectory());
    cmdline += " && automake ";
    cmdline += KProcess::quote(relmakefile);
    cmdline += " && CONFIG_HEADERS=config.h CONFIG_FILES=";
    cmdline += KProcess::quote(relmakefile);
    cmdline += " ./config.status";

    m_part->makeFrontend()->queueCommand( m_part->projectDirectory(), cmdline );
    m_part->makeFrontend()->queueCommand( m_part->projectDirectory(), m_part->configureCommand() );
#endif

    m_part->needMakefileCvs();

    QDialog::accept();
}

#include "addsubprojectdlg.moc"
