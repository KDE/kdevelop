/***************************************************************************
                          kdlgnewwidget.h  -  description                              
                             -------------------                                         
    begin                : Thu Mar 18 1999                                           
    copyright            : (C) 1999 by                          
    email                :                                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KDLGREADMEDLG_H
#define KDLGREADMEDLG_H

#include <qdialog.h>
#include <qcheckbox.h>

class QMultiLineEdit;
class QPushButton;

/**
  *@author 
  */

class KDlgReadmeDlg : public QDialog  {
   Q_OBJECT
public: 
	KDlgReadmeDlg(QWidget *parent=0);
	~KDlgReadmeDlg();
  bool isShowAgain() { return cb->isChecked(); }
		
private:
  QMultiLineEdit *mle;
  QPushButton *btn;
  QCheckBox *cb;
};

#endif
