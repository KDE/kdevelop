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
#include <qtoolbutton.h>
#include <kcombobox.h>
#include <qtimer.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"

#include "partwidget.h"
#include "filetreewidget.h"

///////////////////////////////////////////////////////////////////////////////
// class factory
///////////////////////////////////////////////////////////////////////////////

typedef KGenericFactory<FileViewPart> FileViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfileview, FileViewFactory( "kdevfileview" ) )

///////////////////////////////////////////////////////////////////////////////
// class FileTreeWidget
///////////////////////////////////////////////////////////////////////////////

FileViewPart::FileViewPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin("FileView", "fileview", parent, name ? name : "FileViewPart"),
    m_widget( 0 )
{
    setInstance(FileViewFactory::instance());
    //    setXMLFile("kdevfileview.rc");

    m_widget = new PartWidget( this );
    m_widget->setIcon( SmallIcon("folder") );
    mainWindow()->embedSelectView( m_widget, i18n("File Tree"), i18n("view on the project directory") );

    // File tree
    connect( project(), SIGNAL( addedFilesToProject( const QStringList & ) ),
             m_widget->m_filetree, SLOT( addProjectFiles( const QStringList & ) ) );
    connect( project(), SIGNAL( removedFilesFromProject( const QStringList & ) ),
             m_widget->m_filetree, SLOT( removeProjectFiles( const QStringList & ) ) );

    m_widget->m_filetree->openDirectory(project()->projectDirectory());
}

FileViewPart::~FileViewPart()
{
    if (m_widget)
        mainWindow()->removeView( m_widget );
    delete m_widget;
}

#include "fileviewpart.moc"
