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
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qradiobutton.h>
//Generated area. DO NOT EDIT!!!(end)

#include <qdialog.h>
#include <qbuttongroup.h>

/**
  *@author The KDevelop Team
  */

class CMakeManualDlg : public QDialog  {
   Q_OBJECT
public: 
	CMakeManualDlg(QWidget *parent=0, const char *name=0,QString manual_file="");
	~CMakeManualDlg();
	
	/**public access attribute, contains "ksgml2html" or "sgml2html"*/
	QString program;
	QString file;

protected: 
	void initDialog();
	//Generated area. DO NOT EDIT!!!(begin)
	QPushButton *ok_button;
	QPushButton *cancel_button;
	QLineEdit *file_edit;
	QLabel *QLabel_1;
	QRadioButton *sgml2html_radiobutton;
	QRadioButton *ksgml2html_radiobutton;
	QPushButton *file_button;
	//Generated area. DO NOT EDIT!!!(end)

	QButtonGroup* program_group;
private:

protected slots:
	void slotOkClicked();
	void slotFileButtonClicked();
};

#endif


