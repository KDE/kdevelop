/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
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
// static members
///////////////////////////////////////////////////////////////////////////////

VCSColors FileViewPart::vcsColors;

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
    QVBox *vbox = dlg->addVBoxPage( i18n("File Tree") );
    VCSColorsConfigWidget *w = new VCSColorsConfigWidget( this, vcsColors, vbox, "vcscolorsconfigwidget" );
    connect( dlg, SIGNAL(okClicked()), w, SLOT(slotAccept()) );
}

///////////////////////////////////////////////////////////////////////////////

void FileViewPart::loadSettings()
{
    const QColor added = QColor( "#CCFF99" ),
        updated = QColor( "#FFFFCC" ),
        modified = QColor( "#CCCCFF" ),
        conflict = QColor( "#FF6666" ),
        sticky = QColor( "#FFCCCC" ),
        needsPatch = QColor( "#FFCCFF" ),
        needsCheckout = QColor( "#FFCCFF" ),
        unknown = QColor( white ),
        defaultColor = QColor( white );

    KConfig *cfg = instance()->config();

    KConfigGroupSaver gs( cfg, "VCS Colors" );
    vcsColors.added = cfg->readColorEntry( "FileAddedColor", &added );
    vcsColors.updated = cfg->readColorEntry( "FileUpdatedColor", &updated );
    vcsColors.sticky = cfg->readColorEntry( "FileStickyColor", &sticky );
    vcsColors.modified = cfg->readColorEntry( "FileModifiedColor", &modified );
    vcsColors.conflict = cfg->readColorEntry( "FileConflictColor", &conflict );
    vcsColors.needsPatch = cfg->readColorEntry( "FileNeedsPatchColor", &needsPatch );
    vcsColors.needsCheckout = cfg->readColorEntry( "FileNeedsCheckoutColor", &needsCheckout );
    vcsColors.unknown = cfg->readColorEntry( "FileUnknownColor", &unknown );
    vcsColors.defaultColor = cfg->readColorEntry( "DefaultColor", &defaultColor );
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
    cfg->writeEntry( "FileNeedsPatchColor", vcsColors.needsPatch );
    cfg->writeEntry( "FileNeedsCheckoutColor", vcsColors.needsCheckout );
    cfg->writeEntry( "FileUnknownColor", vcsColors.unknown );
    cfg->writeEntry( "DefaultColor", vcsColors.defaultColor );
}

#include "fileviewpart.moc"
