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
#include <qtextstream.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>


KTipofDay::KTipofDay( QWidget *parent, const char *name ) : KTipofDayDlg(parent,name, true) {
		
 	QPixmap pixmap;
 	pixmap.load(locate("appdata", "pics/tipday.png"));
 	bmp_frame->setBackgroundPixmap( pixmap );
	bmp_frame->setFrameStyle(49);
	
  QFont font( "Helvetica", 14, 75, 0 );
 	font.setStyleHint( (QFont::StyleHint)0 );
 	font.setCharSet( (QFont::CharSet)0 );
 	did_know_label->setFont( font );
	did_know_label->setFrameStyle(49);
	
	tip_label->setFrameStyle( 49 );
	tip_label->setMargin( 5 );
	
	show_check->setChecked( TRUE );
  slotNext();
}

KTipofDay::~KTipofDay(){
}

void KTipofDay::slotOK()
{
	KConfig *config = KGlobal::config();
	config->setGroup("TipOfTheDay");
  config->writeEntry("show_tod",show_check->isChecked());
  hide();
}

void KTipofDay::slotNext()
{
  QString file=locate("html", "default/kdevelop/tip.database");

  if( file.isEmpty() ){
    tip_label->setText(i18n("Tipdatabase not found ! Please check your installation."));
    return;
  }

	QFile f(file);
	QString	tip_text;
	int text_line=2;
	int next;
	bool found = false;

	KConfig *config = KGlobal::config();
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
#include "ktipofday.moc"
