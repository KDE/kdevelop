/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <qdir.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevpartcontroller.h"
#include "kdevproject.h"

#include "pascalsupport_part.h"

typedef KGenericFactory<PascalSupportPart> PascalSupportFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpascalsupport, PascalSupportFactory( "kdevpascalsupport" ) );

PascalSupportPart::PascalSupportPart(QObject *parent, const char *name, const QStringList &)
  : KDevLanguageSupport("PascalSupport", "pascal", parent, name ? name : "KDevPascalSupport" )
{
    setInstance(PascalSupportFactory::instance());
    setXMLFile("kdevpascalsupport.rc");

    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( partController(), SIGNAL(savedFile(const QString&)),
             this, SLOT(savedFile(const QString&)) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );
    connect( core(), SIGNAL(configWidget(KDialogBase*)),
             this, SLOT(configWidget(KDialogBase*)) );
    connect( core( ), SIGNAL( projectConfigWidget( KDialogBase* ) ), this,
             SLOT( projectConfigWidget( KDialogBase* ) ) );
}

PascalSupportPart::~PascalSupportPart()
{
}

PascalSupportPart::Features PascalSupportPart::features()
{
    return Features(Classes | Structs | Functions | Variables | Declarations);
}

QStringList PascalSupportPart::fileFilters()
{
    return QStringList::split(",", "*.pp,*.pas,*.dpr");
}

void PascalSupportPart::projectOpened()
{
    connect(project(), SIGNAL(addedFilesToProject(const QStringList &)),
        this, SLOT(addedFilesToProject(const QStringList &)));
    connect(project(), SIGNAL(removedFilesFromProject(const QStringList &)),
        this, SLOT(removedFilesFromProject(const QStringList &)));
    connect(project(), SIGNAL(projectCompiled()),
        this, SLOT(slotProjectCompiled()) );

    QDir::setCurrent(project()->projectDirectory());
    m_projectFileList = project()->allFiles();
    m_projectClosed = false;
}

void PascalSupportPart::projectClosed()
{
    m_projectClosed = true;
}

void PascalSupportPart::configWidget(KDialogBase *dlg)
{
    return;
}

void PascalSupportPart::projectConfigWidget(KDialogBase *dlg)
{
    return;
}

void PascalSupportPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    return;
}

void PascalSupportPart::savedFile(const QString &fileName)
{
    return;
}

void PascalSupportPart::addedFilesToProject(const QStringList &fileList)
{
    return;
}

void PascalSupportPart::removedFilesFromProject(const QStringList &fileList)
{
    return;
}

void PascalSupportPart::slotProjectCompiled()
{
    return;
}

#include "pascalsupport_part.moc"
