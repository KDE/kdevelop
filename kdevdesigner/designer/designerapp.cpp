 /**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "designerappiface.h"
#include "designerapp.h"
#include "mainwindow.h"
#include "formwindow.h"

#include <kiconloader.h>

#include <qfile.h>
#include <qdir.h>
#include <qsettings.h>
#include <qsplashscreen.h>

#ifdef Q_WS_WIN
#include <qt_windows.h>
#include <process.h>
#endif

static QSplashScreen *splash = 0;

void set_splash_status( const QString &txt )
{
    if ( !splash )
	return;
    QString splashText = "Licensed to "
			 + QString::fromLatin1( QT_PRODUCT_LICENSEE ) + "\n"
			 + txt;
    splash->message( splashText, Qt::AlignRight|Qt::AlignTop );
}

DesignerApplication::DesignerApplication( int &argc, char **argv )
    : QApplication( argc, argv )
{
#if defined(Q_WS_WIN)
    if ( winVersion() & Qt::WV_NT_based )
	DESIGNER_OPENFILE = RegisterWindowMessage((TCHAR*)"QT_DESIGNER_OPEN_FILE");
    else
	DESIGNER_OPENFILE = RegisterWindowMessageA("QT_DESIGNER_OPEN_FILE");
#endif
}

QSplashScreen *DesignerApplication::showSplash()
{
    QRect screen = QApplication::desktop()->screenGeometry();
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QRect mainRect;
    QString keybase = settingsKey();
    bool show = config.readBoolEntry( keybase + "SplashScreen", TRUE );
    mainRect.setX( config.readNumEntry( keybase + "Geometries/MainwindowX", 0 ) );
    mainRect.setY( config.readNumEntry( keybase + "Geometries/MainwindowY", 0 ) );
    mainRect.setWidth( config.readNumEntry( keybase + "Geometries/MainwindowWidth", 500 ) );
    mainRect.setHeight( config.readNumEntry( keybase + "Geometries/MainwindowHeight", 500 ) );
    screen = QApplication::desktop()->screenGeometry( QApplication::desktop()->screenNumber( mainRect.center() ) );

    if ( show ) {
	splash = new QSplashScreen( UserIcon("designer_splash.png") );
	splash->show();
	set_splash_status( "Initializing..." );
    }

    return splash;
}

void DesignerApplication::closeSplash()
{
    splash->hide();
}

static QString *settings_key = 0;
static QString *old_settings_key = 0;

QString DesignerApplication::settingsKey()
{
    if ( !settings_key )
	settings_key = new QString( "/Qt Designer/" +
				    QString::number( (QT_VERSION >> 16) & 0xff ) +
	                            "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    return *settings_key;
}

QString DesignerApplication::oldSettingsKey()
{
    if ( !old_settings_key ) {
	int majorVer = (QT_VERSION >> 16) & 0xff;
	int minorVer = (QT_VERSION >> 8) & 0xff;
	
	// If minorVer is 0 (e.g. 4.0) then we don't want to read the
	// old settings, too much might have changed.
	if ( !minorVer == 0 )
	    minorVer--;

	old_settings_key = new QString( "/Qt Designer/" +
				    QString::number( majorVer ) +
	                            "." + QString::number( minorVer ) + "/" );
    }
    return *old_settings_key;
}

void DesignerApplication::setSettingsKey( const QString &key )
{
    if ( !settings_key )
	settings_key = new QString( key );
    else
	*settings_key = key;
}

#if defined(Q_WS_WIN)
bool DesignerApplication::winEventFilter( MSG *msg )
{
    if ( msg->message == DESIGNER_OPENFILE ) {
	QFile f( QDir::homeDirPath() + "/.designerargs" );
	QFileInfo fi(f);
	if ( fi.lastModified() == lastMod )
	    return QApplication::winEventFilter( msg );
	lastMod = fi.lastModified();
	f.open( IO_ReadOnly );
	QString args;
	f.readLine( args, f.size() );
	QStringList lst = QStringList::split( " ", args );

	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	    QString arg = (*it).stripWhiteSpace();
	    if ( arg[0] != '-' ) {
		QObjectList* l = MainWindow::self->queryList( "FormWindow" );
		FormWindow* fw = (FormWindow*) l->first();
		FormWindow* totop = 0;
		bool haveit = FALSE;
		while ( fw ) {
		    haveit = haveit || fw->fileName() == arg;
		    if ( haveit )
			totop = fw;

		    fw = (FormWindow*) l->next();
		}

		if ( !haveit ) {
		    FlashWindow( MainWindow::self->winId(), TRUE );
		    MainWindow::self->openFormWindow( arg );
		} else if ( totop ) {
		    totop->setFocus();
		}
		delete l;
	    }
	}
	return TRUE;
    }
    return QApplication::winEventFilter( msg );
}
#endif
