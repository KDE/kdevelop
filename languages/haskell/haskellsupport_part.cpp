/***************************************************************************
                          haskellsupport_part.cpp  -  description
                             -------------------
    begin                : Mon Aug 11 2003
    copyright            : (C) 2003 by Peter Robinson
    email                : listener@thaldyron.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qwhatsthis.h>
#include <qdir.h>
#include <qtimer.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kstatusbar.h>
#include <kdevplugininfo.h>

//#include <antlr/ASTFactory.hpp>

#include "kdevpartcontroller.h"
#include "kdevproject.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"

#include "haskellsupport_part.h"

typedef KDevGenericFactory<HaskellSupportPart> HaskellSupportFactory;
static const KDevPluginInfo data("kdevhaskellsupport");
K_EXPORT_COMPONENT_FACTORY( libkdevhaskellsupport, HaskellSupportFactory( data ) )

HaskellSupportPart::HaskellSupportPart(QObject *parent, const char *name, const QStringList& )
  : KDevLanguageSupport(&data, parent, name ? name : "HaskellSupportPart" )
{
	setInstance(HaskellSupportFactory::instance());
	setXMLFile("kdevpart_haskellsupport.rc");

 	connect( core(), SIGNAL( projectOpened() ),
  			 this, SLOT( projectOpened() ) );
  	connect( core(), SIGNAL( projectClosed() ),
  			 this, SLOT( projectClosed() ) );
  	connect( partController(), SIGNAL( savedFile( const KURL& ) ),
 			 this, SLOT( savedFile( const KURL& ) ) );
  	connect( core(), SIGNAL( contextMenu( QPopupenu*, const Context* ) ),
  			 this, SLOT( contextMenu( QPopupenu *, const Context* ) ) );
	connect( core(), SIGNAL( configWidget( KDialogBase* ) ),
  			 this, SLOT( configWidget( KDialogBase* ) ) );
	connect( core(), SIGNAL( projectConfigWidget( KDialogBase* ) ),
  			 this, SLOT( projectConfigWidget( KDialogBase* ) ) );
  	kdDebug( 9000 ) << "Haskell support found";

// @todo embed problem reporter instead:
//  mainWindow()->embedOutputView( m_widget, "name that should appear", "enter a tooltip" );
}


HaskellSupportPart::~HaskellSupportPart()
{
  // remove & delete problemReporter view
}


HaskellSupportPart::Features HaskellSupportPart::features()
{
  	return Features( Functions );
}

void HaskellSupportPart::projectOpened()
{
	connect( project(), SIGNAL( addedFilesToProject( const QStringList& ) ),
					 this, SLOT( addedFilesToProject( const QStringList& ) ) );
	connect( project(), SIGNAL( removedFilesFromProject( const QStringList& ) ),
					 this, SLOT( removedFilesFromProject( const QStringList& ) ) );
	QDir::setCurrent( project()->projectDirectory() );
	m_projectFileList = project()->allFiles();
	m_projectClosed = false;
	QTimer::singleShot( 0, this, SLOT( slotInitialParse() ) );
}

void HaskellSupportPart::projectClosed()
{
	m_projectClosed = true;
}

void HaskellSupportPart::configWidget( KDialogBase *dlg )
{
	Q_UNUSED( dlg );
	return;
}

void HaskellSupportPart::projectConfigWidget( KDialogBase *dlg )
{
	Q_UNUSED( dlg );
	return;
}

void HaskellSupportPart::contextMenu( QPopupMenu *popup, const Context *context )
{
	Q_UNUSED( popup );
	return;
}

void HaskellSupportPart::savedFile( const KURL &fileName )
{
	Q_UNUSED( fileName.path() );
	return;
}

void HaskellSupportPart::addedFilesToProject( const QStringList &fileList )
{
	Q_UNUSED( fileList );
	return;
}

void HaskellSupportPart::removedFilesFromProject( const QStringList &fileList )
{
	Q_UNUSED( fileList );
	return;
}

void HaskellSupportPart::slotProjectCompiled()
{
	return;
}

void HaskellSupportPart::slotInitialParse()
{
	return;
}

KMimeType::List HaskellSupportPart::mimeTypes()
{
 	KMimeType::List list;
 	KMimeType::Ptr mime = KMimeType::mimeType( "text/x-haskell" );
	if( mime ) {
    	list << mime;
	}
	return list;
}

#include "haskellsupport_part.moc"
