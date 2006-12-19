/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kstandardaction.h>
#include <kmainwindow.h>
#include <kmenubar.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevplugininfo.h>

#include "fullscreen_part.h"

static const KDevPluginInfo data("kdevfullscreen");

typedef KDevGenericFactory<FullScreenPart> FullScreenFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfullscreen, FullScreenFactory( data ) )

FullScreenPart::FullScreenPart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin(&data, parent/*, name ? name : "FullScreenPart"*/ )
{
  setInstance(FullScreenFactory::instance());
//  const KAboutData &abdata1 = *(info());
//  kDebug() << abdata1.appName() << endl;
  const KDevPluginInfo &_info = *info();
  const KAboutData *abdata = _info;
  kDebug() << abdata->appName() << endl;
  setXMLFile("kdevpart_fullscreen.rc");

  m_bFullScreen = false;

  m_pFullScreen = KStandardAction::fullScreen(this, SLOT(slotToggleFullScreen()), actionCollection(), mainWindow()->main());
}

FullScreenPart::~FullScreenPart()
{
}

void FullScreenPart::slotToggleFullScreen( )
{
   m_bFullScreen = !m_bFullScreen;
   if( m_bFullScreen ){
      //mw->menuBar()->hide();
      mainWindow()->main()->showFullScreen();

      /*m_pFullScreen->setText( i18n( "Exit Full-Screen Mode" ) );
      m_pFullScreen->setToolTip( i18n( "Exit full-screen mode" ) );
      m_pFullScreen->setIcon( "window_nofullscreen" );*/
   } else {
      //mw->menuBar()->show();

      mainWindow()->main()->showNormal();

      /*m_pFullScreen->setText( i18n( "&Full-Screen Mode" ) );
      m_pFullScreen->setToolTip(i18n("Full-screen mode"));
      m_pFullScreen->setIcon( "window_fullscreen" );*/
   }
}


#include "fullscreen_part.moc"
