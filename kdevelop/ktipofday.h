/***************************************************************************
                            ktipofday.h  -  "tip of day" dialog
 
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

#ifndef KTIPOFDAY_H
#define KTIPOFDAY_H

#include <kdialogbase.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <kapp.h>
#include <iostream.h>

/** "Tip of the Day" Dialog
  *@author Ralf Nolden
  *@author Martin Piskernig
  */
class KTipofDay : public KDialogBase
{
 Q_OBJECT
public: 
 /** Constructor */
 KTipofDay(QWidget *parent=0, const char *name=0);
 /** Destructor */
 ~KTipofDay();
protected slots:
 void slotOK();
 void slotUser1();
protected:
 QFrame* main_frame;
 QCheckBox* show_check;
 QFrame* bmp_frame;
 QLabel* tip_label;
 QPushButton* ok_button;
 QPushButton* next_button;
};

#endif
