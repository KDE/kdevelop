/***************************************************************************
                          cmakemanualdlg.h  -  description                              
                             -------------------                                         
    begin                : Wed Jul 14 1999                                           
    copyright            : (C) 1999 by The KDevelop Team                         
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


#ifndef CMAKEMANUALDLG_H
#define CMAKEMANUALDLG_H

//Generated area. DO NOT EDIT!!!(begin)
#include <qwidget.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qradiobutton.h>
//Generated area. DO NOT EDIT!!!(end)

#include <qdialog.h>

/**
  *@author The KDevelop Team
  */

class CMakeManualDlg : public QDialog  {
   Q_OBJECT
public: 
	CMakeManualDlg(QWidget *parent=0, const char *name=0);
	~CMakeManualDlg();

protected: 
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QGroupBox *QGroupBox_1;
	QPushButton *ok_button;
	QPushButton *cancel_button;
	QLineEdit *file_edit;
	QLabel *QLabel_1;
	QRadioButton *sgtml2html_radiobutton;
	QRadioButton *ksgml2html_radiobutton;
	QPushButton *file_button;
	//Generated area. DO NOT EDIT!!!(end)

private: 
};

#endif
