/***************************************************************************
                          ktipofday.cpp  -  description                              
                             -------------------                                         
    begin                : Tue Mar 2 1999                                           
    copyright            : (C) 1999 by Ralf Nolden, Steen Rabol
    email                : Ralf.Nolden@post.rwth-aachen.de,rabol@get2net.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "ktipofday.h"
#include <kapp.h>
#include <qfile.h>
#include <qfileinfo.h>



KTipofDay::KTipofDay(QWidget *parent, const char *name ) : QDialog(parent,name, true) {
	setCaption(i18n("Tip of the Day"));
	
	main_frame = new QFrame( this, "main_frame" );
	main_frame->setGeometry( 10, 10, 410, 160 );
	main_frame->setMinimumSize( 0, 0 );
	main_frame->setMaximumSize( 32767, 32767 );
	main_frame->setFocusPolicy( QWidget::NoFocus );
	main_frame->setBackgroundMode( QWidget::PaletteMidlight );
	main_frame->setFontPropagation( QWidget::NoChildren );
	main_frame->setPalettePropagation( QWidget::NoChildren );
	main_frame->setFrameStyle( 33 );

	show_check = new QCheckBox( this, "show_check" );
	show_check->setGeometry( 20, 170, 260, 30 );
	show_check->setMinimumSize( 0, 0 );
	show_check->setMaximumSize( 32767, 32767 );
	show_check->setFocusPolicy( QWidget::TabFocus );
	show_check->setBackgroundMode( QWidget::PaletteBackground );
	show_check->setFontPropagation( QWidget::NoChildren );
	show_check->setPalettePropagation( QWidget::NoChildren );
	show_check->setText(i18n("show Tip of the day on next start"));
	show_check->setAutoRepeat( FALSE );
	show_check->setAutoResize( TRUE );
	show_check->setChecked( TRUE );

	bmp_frame = new QFrame( this, "bmp_frame" );
	bmp_frame->setGeometry( 25, 40, 70, 100 );
	bmp_frame->setMinimumSize( 0, 0 );
	bmp_frame->setMaximumSize( 32767, 32767 );

	{
		QPixmap pixmap;
		pixmap.load(KApplication::kde_datadir() + "/kdevelop/pics/tipday.bmp");
		bmp_frame->setBackgroundPixmap( pixmap );
	}
	bmp_frame->setFocusPolicy( QWidget::NoFocus );
	bmp_frame->setFontPropagation( QWidget::NoChildren );
	bmp_frame->setPalettePropagation( QWidget::NoChildren );
	bmp_frame->setFrameStyle( 49 );

	tip_label = new QLabel( this, "tip_label" );
	tip_label->setGeometry( 110, 20, 300, 140 );
	tip_label->setMinimumSize( 0, 0 );
	tip_label->setMaximumSize( 32767, 32767 );
	tip_label->setFocusPolicy( QWidget::NoFocus );
	tip_label->setBackgroundMode( QWidget::PaletteLight );
	tip_label->setFontPropagation( QWidget::NoChildren );
	tip_label->setPalettePropagation( QWidget::NoChildren );
	tip_label->setFrameStyle( 49 );
	tip_label->setText( "" );
	tip_label->setAlignment( 1313 );
	tip_label->setMargin( -1 );

	next_button = new QPushButton( this, "next_button" );
	next_button->setGeometry( 50, 210, 130, 30 );
	next_button->setMinimumSize( 0, 0 );
	next_button->setMaximumSize( 32767, 32767 );
	connect( next_button, SIGNAL(clicked()), SLOT(slotNext()) );
	next_button->setFocusPolicy( QWidget::TabFocus );
	next_button->setBackgroundMode( QWidget::PaletteBackground );
	next_button->setFontPropagation( QWidget::NoChildren );
	next_button->setPalettePropagation( QWidget::NoChildren );
	next_button->setText(i18n("Next Tip") );
	next_button->setAutoRepeat( FALSE );
	next_button->setAutoResize( FALSE );
	
	ok_button = new QPushButton( this, "ok_button" );
	ok_button->setGeometry( 250, 210, 130, 30 );
	ok_button->setMinimumSize( 0, 0 );
	ok_button->setMaximumSize( 32767, 32767 );
	connect( ok_button, SIGNAL(clicked()), SLOT(slotOK()) );
	ok_button->setFocusPolicy( QWidget::TabFocus );
	ok_button->setBackgroundMode( QWidget::PaletteBackground );
	ok_button->setFontPropagation( QWidget::NoChildren );
	ok_button->setPalettePropagation( QWidget::NoChildren );
	ok_button->setText(i18n("Close"));
	ok_button->setAutoRepeat( FALSE );
	ok_button->setAutoResize( FALSE );
	ok_button->setDefault( TRUE );


	KConfig *config = kapp->getConfig();
	config->setGroup("TipOfTheDay");
 	bool showTip=config->readBoolEntry("show_tod",true);

	if(showTip)
	  setGeometry(QApplication::desktop()->width()/2-215, QApplication::desktop()->height()/2-130, 430,260);
	else
		resize(430,260);
	
	setMinimumSize( 0, 0 );
	setMaximumSize( 32767, 32767 );
  slotNext();
}

KTipofDay::~KTipofDay(){
}

void KTipofDay::slotOK()
{
	KConfig *config = kapp->getConfig();
	config->setGroup("TipOfTheDay");
  config->writeEntry("show_tod",show_check->isChecked());
  hide();
}

void KTipofDay::slotNext()
{
  KLocale *kloc = KApplication::getKApplication()->getLocale();

  QString strpath = KApplication::kde_htmldir().copy() + "/";
  QString file;

  file = strpath + kloc->language() + '/' + "kdevelop/tipdatabase";
  if( !QFileInfo( file ).exists() ){
    // not found: use the default
    file = strpath + "default/" + "kdevelop/tipdatabase";
  }
  if( !QFileInfo( file ).exists() ){
    tip_label->setText(i18n("Tipdatabase not found ! Please check your installation."));
    return;
  }

	QFile f(file);
	QString	tip_text;
	int text_line=1;
	int next;
	bool found = false;

	KConfig *config = kapp->getConfig();
	config->setGroup("TipOfTheDay");
	next = config->readNumEntry("NextTip", 1);
	
	if (f.open(IO_ReadOnly))
	{
		QTextStream t( &f );
		while ( !t.eof())
		{
  			tip_text = t.readLine();
				tip_label->setText(tip_text);
				if (next < 30)
  				config->writeEntry("NextTip",next+1);
  		  else
  		    config->writeEntry("NextTip",1);
				found = true;
		    if(text_line==next)
  				break;
  			text_line++;				
		}
		f.close();
	}
}





















