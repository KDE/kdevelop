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

#include "fileviewpart.h"

#include <qwhatsthis.h>
#include <qvbox.h>
#include <qtimer.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevtoplevel.h"

#include "filetreewidget.h"
#include "filegroupswidget.h"
#include "filegroupsconfigwidget.h"


typedef KGenericFactory<FileViewPart> FileViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfileview, FileViewFactory( "kdevfileview" ) );

FileViewPart::FileViewPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(parent, name)
{
    setInstance(FileViewFactory::instance());
    //    setXMLFile("kdevfileview.rc");
    
    m_filetree = new FileTreeWidget(this);
    m_filetree->setCaption(i18n("File Tree"));
    QWhatsThis::add(m_filetree, i18n("File Tree\n\n"
                                     "The file viewer shows all files of the project "
                                     "in a tree layout."));
    topLevel()->embedSelectView(m_filetree, i18n("File Tree"));

    m_filegroups = new FileGroupsWidget(this);
    m_filegroups->setCaption(i18n("File Group View"));
    QWhatsThis::add(m_filegroups, i18n("File Group View\n\n"
                                       "The file group viewer shows all files of the project, "
                                       "in groups which can be configured by you."));
    topLevel()->embedSelectView(m_filegroups, i18n("File Groups"));

    
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
    
    // File tree
    m_filetree->openDirectory(project()->projectDirectory());

    // File groups
    connect( project(), SIGNAL(addedFileToProject(const QString&)),
             m_filetree, SLOT(hideOrShow()) );
    connect( project(), SIGNAL(removedFileFromProject(const QString&)),
             m_filetree, SLOT(hideOrShow()) );
    connect( project(), SIGNAL(addedFileToProject(const QString&)),
             m_filegroups, SLOT(addFile(const QString&)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString&)),
             m_filegroups, SLOT(removeFile(const QString&)) );
    m_filegroups->refresh();
}


FileViewPart::~FileViewPart()
{
    if (m_filetree)
        topLevel()->removeView(m_filetree);
    if (m_filegroups)
    topLevel()->removeView(m_filegroups);

    delete m_filetree;
    delete m_filegroups;
}


void FileViewPart::refresh()
{
    // This method may be called from m_filetree's slot,
    // so we make sure not to modify the list view during
    // the execution of the slot
    QTimer::singleShot(0, m_filegroups, SLOT(refresh()));
}


void FileViewPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("File Groups"));
    FileGroupsConfigWidget *w = new FileGroupsConfigWidget(this, vbox, "file groups config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

#include "fileviewpart.moc"
