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

#include "filegroupspart.h"
#include "filegroupspart.moc"

#include <q3whatsthis.h>
#include <q3vbox.h>
#include <qtimer.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdialogbase.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "kdevplugininfo.h"

#include "filegroupswidget.h"
#include "filegroupsconfigwidget.h"

#define FILEGROUPS_OPTIONS 1

typedef KDevGenericFactory<FileGroupsPart> FileGroupsFactory;
static const KDevPluginInfo data("kdevfilegroups");
K_EXPORT_COMPONENT_FACTORY( libkdevfilegroups, FileGroupsFactory( data ) )

FileGroupsPart::FileGroupsPart(QObject *parent, const char *name, const QStringList &)
    : KDevPlugin(&data, parent, name ? name : "FileGroupsPart")
{
    deleteRequested = false;
    setInstance(FileGroupsFactory::instance());

    m_filegroups = new FileGroupsWidget(this);
    m_filegroups->setCaption(i18n("File Group View"));
	m_filegroups->setIcon(SmallIcon( info()->icon() ) );
    Q3WhatsThis::add(m_filegroups, i18n("<b>File group view</b><p>"
                                       "The file group viewer shows all files of the project, "
                                       "in groups which can be configured in project settings dialog, <b>File Groups</b> tab."));
    mainWindow()->embedSelectView(m_filegroups, i18n("File Groups"), i18n("File groups in the project directory"));

	_configProxy = new ConfigWidgetProxy( core() );
	_configProxy->createProjectConfigPage( i18n("File Groups"), FILEGROUPS_OPTIONS, info()->icon() );
	connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )), 
		this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );


    // File groups
    connect( project(), SIGNAL(addedFilesToProject(const QStringList&)),
             m_filegroups, SLOT(addFiles(const QStringList&)) );
    connect( project(), SIGNAL(removedFilesFromProject(const QStringList&)),
             m_filegroups, SLOT(removeFiles(const QStringList&)) );
/*    connect( project(), SIGNAL(addedFileToProject(const QString&)),
             m_filegroups, SLOT(addFile(const QString&)) );
    connect( project(), SIGNAL(removedFileFromProject(const QString&)),
             m_filegroups, SLOT(removeFile(const QString&)) );*/
    m_filegroups->refresh();
}

FileGroupsPart::~FileGroupsPart()
{
    deleteRequested = true;
    if (m_filegroups)
        mainWindow()->removeView(m_filegroups);
    delete m_filegroups;
	delete _configProxy;
}

void FileGroupsPart::refresh()
{
    if (deleteRequested)
        return;
    // This method may be called from m_filetree's slot,
    // so we make sure not to modify the list view during
    // the execution of the slot
    QTimer::singleShot(0, m_filegroups, SLOT(refresh()));
}

void FileGroupsPart::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
	if ( pagenumber == FILEGROUPS_OPTIONS )
	{
		FileGroupsConfigWidget *w = new FileGroupsConfigWidget(this, page, "file groups config widget");
		connect( dlg, SIGNAL(okClicked()), w, SLOT(accept()) );
	}
}
