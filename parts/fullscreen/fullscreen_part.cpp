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
#include <kgenericfactory.h>
#include <kaction.h>
#include <kmainwindow.h>
#include <kmenubar.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "fullscreen_part.h"

typedef KGenericFactory<FullScreenPart> FullScreenFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfullscreen, FullScreenFactory( "kdevfullscreen" ) )

FullScreenPart::FullScreenPart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin("KDevPart", "kdevpart", parent, name ? name : "FullScreenPart" )
{
  setInstance(FullScreenFactory::instance());
  setXMLFile("kdevpart_fullscreen.rc");

  m_bFullScreen = false;

  m_pFullScreen = new KAction( i18n( "&Full-Screen Mode" ), "window_fullscreen", CTRL+SHIFT+Key_F, this,
        SLOT( slotToggleFullScreen() ), actionCollection(), "fullscreen" );
}

FullScreenPart::~FullScreenPart()
{
}

void FullScreenPart::slotToggleFullScreen( )
{
   KMainWindow* mw = mainWindow()->main();
   m_bFullScreen = !m_bFullScreen;
   if( m_bFullScreen ){
      mw->menuBar()->hide();
      mw->showFullScreen();

      m_pFullScreen->setText( i18n( "Exit Full-Screen Mode" ) );
      m_pFullScreen->setIcon( "window_nofullscreen" );
   } else {
      mw->menuBar()->show();

      mw->showNormal();

      m_pFullScreen->setText( i18n( "Full-Screen Mode" ) );
      m_pFullScreen->setIcon( "window_fullscreen" );
   }
}


#include "fullscreen_part.moc"
