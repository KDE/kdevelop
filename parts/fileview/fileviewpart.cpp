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
    setInstance( FileViewFactory::instance() );
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
    KConfig *cfg = instance()->config();
    QColor fallbackColor = white;

    if (cfg->hasGroup( "VCS Colors" ))
    {
        KConfigGroupSaver gs( cfg, "VCS Colors" );
        vcsColors.added = cfg->readColorEntry( "FileAddedColor", &fallbackColor );
        vcsColors.updated = cfg->readColorEntry( "FileUpdatedColor", &fallbackColor );
        vcsColors.sticky = cfg->readColorEntry( "FileStickyColor", &fallbackColor );
        vcsColors.modified = cfg->readColorEntry( "FileModifiedColor", &fallbackColor );
        vcsColors.conflict = cfg->readColorEntry( "FileConflictColor", &fallbackColor );
        vcsColors.unknown = cfg->readColorEntry( "FileUnknownColor", &fallbackColor );
        vcsColors.defaultColor = cfg->readColorEntry( "DefaultColor", &fallbackColor );
    }
    else
    {
        vcsColors.added = vcsColors.updated = vcsColors.sticky = vcsColors.sticky =
            vcsColors.modified = vcsColors.conflict = vcsColors.unknown =
            vcsColors.defaultColor = fallbackColor;
    }
}

///////////////////////////////////////////////////////////////////////////////

void FileViewPart::storeSettings()
{
    KConfig *cfg = instance()->config();
    // VCS colors
    KConfigGroupSaver gs( cfg, "VCS Colors" );
    cfg->writeEntry( "FileAddedColor", vcsColors.added );
    cfg->writeEntry( "FileUpdatedColor", vcsColors.updated );
    cfg->writeEntry( "FileStickyColor", vcsColors.sticky );
    cfg->writeEntry( "FileModifiedColor", vcsColors.modified );
    cfg->writeEntry( "FileConflictColor", vcsColors.conflict );
    cfg->writeEntry( "FileUnknownColor", vcsColors.unknown );
    cfg->writeEntry( "DefaultColor", vcsColors.defaultColor );
}

#include "fileviewpart.moc"
