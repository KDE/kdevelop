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
#include <qdom.h>
#include <kcombobox.h>
#include <qtimer.h>
#include <kaction.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdialogbase.h>

#include "kdevcore.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"

#include "partwidget.h"
#include "domutil.h"
#include "filetreewidget.h"
#include "vcscolorsconfigwidget.h"
#include "kdevversioncontrol.h"

///////////////////////////////////////////////////////////////////////////////
// class factory
///////////////////////////////////////////////////////////////////////////////

typedef KGenericFactory<FileViewPart> FileViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfileview, FileViewFactory( "kdevfileview" ) )

///////////////////////////////////////////////////////////////////////////////
// static members
///////////////////////////////////////////////////////////////////////////////

VCSColors FileViewPart::vcsColors;

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
    mainWindow()->embedSelectView( m_widget, i18n("File Tree"), i18n("File tree view in the project directory") );

    connect( core(), SIGNAL(projectConfigWidget(KDialogBase*)), this, SLOT(projectConfigWidget(KDialogBase*)) );

    loadSettings();

    m_widget->showProjectFiles();
}

///////////////////////////////////////////////////////////////////////////////

FileViewPart::~FileViewPart()
{
    if (m_widget)
        mainWindow()->removeView( m_widget );
    delete m_widget;

    storeSettings();
}

///////////////////////////////////////////////////////////////////////////////

void FileViewPart::projectConfigWidget( KDialogBase *dlg )
{
    QVBox *vbox = dlg->addVBoxPage( i18n("File tree") );
    VCSColorsConfigWidget *w = new VCSColorsConfigWidget( this, vcsColors, vbox, "vcscolorsconfigwidget" );
    connect( dlg, SIGNAL(okClicked()), w, SLOT(slotAccept()) );
}

///////////////////////////////////////////////////////////////////////////////

void FileViewPart::loadSettings()
{
    QDomDocument &dom =  *projectDom();
    QString cn;
    QString defaultColorName = white.name();

    cn = DomUtil::readEntry( dom, "/kdevfileviewpart/fileaddedcolor", defaultColorName );
    vcsColors.added.setNamedColor( cn );
    cn = DomUtil::readEntry( dom, "/kdevfileviewpart/fileupdatecolor", defaultColorName );
    vcsColors.updated.setNamedColor( cn );
    cn = DomUtil::readEntry( dom, "/kdevfileviewpart/filemodifiedcolor", defaultColorName );
    vcsColors.modified.setNamedColor( cn );
    cn = DomUtil::readEntry( dom, "/kdevfileviewpart/filestickycolor", defaultColorName );
    vcsColors.sticky.setNamedColor( cn );
    cn = DomUtil::readEntry( dom, "/kdevfileviewpart/fileconflictcolor", defaultColorName );
    vcsColors.conflict.setNamedColor( cn );
    cn = DomUtil::readEntry( dom, "/kdevfileviewpart/fileunknowncolor", defaultColorName );
    vcsColors.unknown.setNamedColor( cn );
}

///////////////////////////////////////////////////////////////////////////////

void FileViewPart::storeSettings()
{
    QDomDocument &dom =  *projectDom();
    DomUtil::writeEntry( dom, "/kdevfileviewpart/fileaddedcolor", vcsColors.added.name() );
    DomUtil::writeEntry( dom, "/kdevfileviewpart/fileupdatecolor", vcsColors.updated.name() );
    DomUtil::writeEntry( dom, "/kdevfileviewpart/filemodifiedcolor", vcsColors.modified.name() );
    DomUtil::writeEntry( dom, "/kdevfileviewpart/filestickycolor", vcsColors.sticky.name() );
    DomUtil::writeEntry( dom, "/kdevfileviewpart/fileconflictcolor", vcsColors.conflict.name() );
    DomUtil::writeEntry( dom, "/kdevfileviewpart/fileunknowncolor", vcsColors.unknown.name() );
}

#include "fileviewpart.moc"
