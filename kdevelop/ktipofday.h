/***************************************************************************
                          ktipofday.h  -  description                              
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

#include <qdialog.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qframe.h>

/**"Tip of the Day" dialog" for KDE/Qt applications
  *@author Ralf Nolden
  */

class KTipofDay : public QDialog  {
   Q_OBJECT
public: 
	KTipofDay(QWidget *parent=0, const char *name=0);
	~KTipofDay();

public slots:


protected slots:
  void slotOK();
  void slotNext();

protected:
  QFrame* main_frame;
  QCheckBox* show_check;
  QFrame* bmp_frame;
  QLabel* tip_label;
  QPushButton* ok_button;
  QPushButton* next_button;
};

#endif








