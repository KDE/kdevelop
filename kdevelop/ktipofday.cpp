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


#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapp.h>
#include "ktipofday.h"


KTipofDay::KTipofDay( QWidget *parent, const char *name ) : QDialog(parent,name, true) {
	setCaption(i18n("Tip of the Day"));

	main_frame = new QFrame( this, "main_frame" );
	main_frame->setGeometry( 10, 10, 410, 200 );
	main_frame->setMinimumSize( 0, 0 );
	main_frame->setMaximumSize( 32767, 32767 );
	main_frame->setFocusPolicy( QWidget::NoFocus );
	main_frame->setBackgroundMode( QWidget::PaletteMidlight );
	main_frame->setFontPropagation( QWidget::NoChildren );
	main_frame->setPalettePropagation( QWidget::NoChildren );
	main_frame->setFrameStyle( 33 );

	bmp_frame = new QFrame( this, "bmp_frame" );
	bmp_frame->setGeometry( 20, 20, 50, 50 );
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
	bmp_frame->setFrameStyle(49);
	
	QLabel* did_know_label;
	did_know_label = new QLabel( this, "Label_1" );
	did_know_label->setGeometry( 80, 20, 330, 50 );
	did_know_label->setMinimumSize( 0, 0 );
	did_know_label->setMaximumSize( 32767, 32767 );
	{
	    QFont font( "Helvetica", 14, 75, 0 );
		font.setStyleHint( (QFont::StyleHint)0 );
		font.setCharSet( (QFont::CharSet)0 );
		did_know_label->setFont( font );
	}
	{
		QColorGroup normal( QColor((unsigned int) QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ), QColor((unsigned int) QRgb(0) ), QColor((unsigned int) QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ) );
		QColorGroup disabled( QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(0) ), QColor((unsigned int) QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(0) ) );
		QColorGroup active( QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)(unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ) );
		QPalette palette( normal, disabled, active );
		did_know_label->setPalette( palette );
	}
	did_know_label->setFocusPolicy( QWidget::NoFocus );
	did_know_label->setBackgroundMode( QWidget::PaletteLight );
	did_know_label->setFontPropagation( QWidget::NoChildren );
	did_know_label->setPalettePropagation( QWidget::NoChildren );
	did_know_label->setText( i18n("Did you know...?") );
	did_know_label->setFrameStyle(49);
	did_know_label->setAlignment( 1316 );
	
	tip_label = new QLabel( this, "tip_label" );
	tip_label->setGeometry( 20, 80, 390, 120 );
	tip_label->setMinimumSize( 0, 0 );
	tip_label->setMaximumSize( 32767, 32767 );
	tip_label->setFocusPolicy( QWidget::NoFocus );
	tip_label->setBackgroundMode( QWidget::PaletteLight );
	tip_label->setFontPropagation( QWidget::NoChildren );
	tip_label->setPalettePropagation( QWidget::NoChildren );
	tip_label->setFrameStyle( 49 );
	tip_label->setText( "" );
	tip_label->setAlignment( 1313 );
	tip_label->setMargin( 5 );

	show_check = new QCheckBox( this, "show_check" );
	show_check->setGeometry( 20, 220, 260, 30 );
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

	next_button = new QPushButton( this, "next_button" );
	next_button->setGeometry( 50, 260, 130, 30 );
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
	ok_button->setGeometry( 250, 260, 130, 30 );
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


	setGeometry(QApplication::desktop()->width()/2-215, QApplication::desktop()->height()/2-150, 430,300);
	
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

  QString strpath = KApplication::kde_htmldir().copy() + "/";
  QString file;

  file = strpath + klocale->language() + '/' + "kdevelop/tip.database";
  if( !QFileInfo( file ).exists() ){
    // not found: use the default
    file = strpath + "default/" + "kdevelop/tip.database";
  }
  if( !QFileInfo( file ).exists() ){
    tip_label->setText(i18n("Tipdatabase not found ! Please check your installation."));
    return;
  }

	QFile f(file);
	QString	tip_text;
	int text_line=2;
	int next;
	bool found = false;

	KConfig *config = kapp->getConfig();
	config->setGroup("TipOfTheDay");
	next = config->readNumEntry("NextTip", 2);
	
	if (f.open(IO_ReadOnly))
	{
		QTextStream t( &f );
		while ( !t.eof())
		{
  			tip_text = t.readLine();
				tip_label->setText(tip_text);
				if (next < 31)
  				config->writeEntry("NextTip",next+1);
  		  else
  		    config->writeEntry("NextTip",2);
				found = true;
		    if(text_line==next)
  				break;
  			text_line++;				
		}
		f.close();
	}
}


































