/***************************************************************************
                            ktipofday.cpp  -  "tip of day" dialog
 
                                -------------------
 
	begin                : Tue Mar 2 1999

	copyright            : (C) 1999,2000 by Ralf Nolden,
       						Steen Rabol,
						Martin Piskernig

	email                : Ralf.Nolden@post.rwth-aachen.de,
			       rabol@get2net.dk,
			       mpiskernig@kdevelop.de
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
#include <kconfig.h>
#include <kstddirs.h>
#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include "ktipofday.h"
#include "ctoolclass.h"

KTipofDay::KTipofDay( QWidget *parent, const char *name ) : KDialogBase(parent,name, true, i18n("Tip of the Day"),Ok|User1,Ok,false,i18n("Next Tip"))
{
 QWidget* page = new QWidget(this);
 setMainWidget(page);

 QVBoxLayout* layout = new QVBoxLayout(page,0,spacingHint());
 
 QHBoxLayout* hbox = new QHBoxLayout(layout);
 
 bmp_frame = new QFrame(page);
 QPixmap pix(locate("kdev_pic","tipday.bmp"));
 bmp_frame->setBackgroundPixmap(pix);
 bmp_frame->setFrameStyle(49);
 bmp_frame->setFixedSize(pix.width(),pix.height());
 hbox->addWidget(bmp_frame);	
 
 QLabel* did_know_label = new QLabel(i18n("Did you know...?"),page);
 did_know_label->setFont(QFont("Helvetica",14,75,0));
 did_know_label->setAlignment(AlignCenter);
 hbox->addWidget(did_know_label);
 
 QColorGroup normal( QColor((unsigned int) QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ), QColor((unsigned int) QRgb(0) ), QColor((unsigned int) QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ) );
 QColorGroup disabled( QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(0) ), QColor((unsigned int) QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(8421504) ), QColor( (unsigned int)QRgb(0) ) );
 QColorGroup active( QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(0) ), QColor( (unsigned int)(unsigned int)QRgb(0) ), QColor( (unsigned int)QRgb(16777215) ), QColor( (unsigned int)QRgb(0) ) );
 QPalette palette( normal, disabled, active );
 did_know_label->setPalette( palette );
 did_know_label->setFrameStyle(49);

 tip_label = new QLabel("",page);
 tip_label->setFrameStyle( 49 );
 tip_label->setMargin( 5 );
 tip_label->setMinimumWidth(tip_label->width()*3);
 tip_label->setMinimumHeight(tip_label->height()*3);
 layout->addWidget(tip_label);

 show_check = new QCheckBox(i18n("Show tip of the day on next start"),page);
 show_check->setChecked(true);
 layout->addWidget(show_check);

 move(parent->width()/2-width()/2,parent->height()/2-height()/2);
 
 layout->activate();
 
 slotUser1();
}

KTipofDay::~KTipofDay()
{}

void KTipofDay::slotOK()
{
 KConfig *config = kapp->config();
 config->setGroup("TipOfTheDay");
 config->writeEntry("show_tod",show_check->isChecked());
 
 hide();
}

void KTipofDay::slotUser1()
{
 QString file = CToolClass::locatehtml("kdevelop/tip.database");
 if(file.isNull()) 
  {
   tip_label->setText(i18n("Tipdatabase not found!\n"
			   "Please check your installation."));
   return;
  }

 QFile f(file);
 QString tip_text;
 int text_line=2;
 int next;
 bool found = false;

 KConfig *config = kapp->config();
 config->setGroup("TipOfTheDay");
 next = config->readNumEntry("NextTip", 2);

 if(f.open(IO_ReadOnly))
  {
   QTextStream t( &f );
   while ( !t.eof())
    {
     tip_text = t.readLine();
     tip_label->setText(tip_text);
     if (next < 7)
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
