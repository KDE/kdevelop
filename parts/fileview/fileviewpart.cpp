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
#include "fileviewpart.moc"

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
        
    // File tree
    connect( project(), SIGNAL(addedFileToProject(const QString&)),
             m_filetree, SLOT(hideOrShow()) );
    connect( project(), SIGNAL(removedFileFromProject(const QString&)),
             m_filetree, SLOT(hideOrShow()) );
    m_filetree->openDirectory(project()->projectDirectory());
}

FileViewPart::~FileViewPart()
{
    if (m_filetree)
        topLevel()->removeView(m_filetree);
    delete m_filetree;
}
