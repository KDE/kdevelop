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

#include "fileviewwidget.h"
#include "fileviewconfigwidget.h"
#include "fileviewpart.h"

typedef KGenericFactory<FileViewPart> FileViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfileview, FileViewFactory( "kdevfileview" ) );

FileViewPart::FileViewPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(parent, name)
{
    setInstance(FileViewFactory::instance());
    //    setXMLFile("kdevfileview.rc");
    
    m_filetree = new FileViewWidget(this);
    m_filetree->setCaption(i18n("File View"));
    QWhatsThis::add(m_filetree, i18n("File View\n\n"
                                     "The file viewer shows all files of the project, "
                                     "ins groups which can be configured by you."));

    topLevel()->embedSelectView(m_filetree, i18n("Files"));

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectChanged()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectChanged()) );
    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)),
             this, SLOT(projectConfigWidget(KDialogBase*)) );
}


FileViewPart::~FileViewPart()
{
    topLevel()->removeView(m_filetree);
    delete m_filetree;
}


void FileViewPart::projectChanged()
{
    if (project()) {
        connect( project(), SIGNAL(addedFileToProject(const QString &)),
                 m_filetree, SLOT(addFile(const QString &)) );
        connect( project(), SIGNAL(removedFileFromProject(const QString &)),
                 m_filetree, SLOT(removeFile(const QString &)) );
    }
    m_filetree->refresh();
}


void FileViewPart::refresh()
{
    // This method may be called from m_filetree's slot,
    // so we make sure not to modify the list view during
    // the execution of the slot
    QTimer::singleShot(0, m_filetree, SLOT(refresh()));
}


void FileViewPart::projectConfigWidget(KDialogBase *dlg)
{
    QVBox *vbox = dlg->addVBoxPage(i18n("File View"));
    FileViewConfigWidget *w = new FileViewConfigWidget(this, vbox, "documentation tree config widget");
    connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
}

#include "fileviewpart.moc"
