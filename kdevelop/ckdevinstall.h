/***************************************************************************
                          ckdevinstall.h  -  description                              
                             -------------------                                         
    begin                : Thu Mar 4 1999                                           
    copyright            : (C) 1999 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef CKDEVINSTALL_H
#define CKDEVINSTALL_H

#include <qprogressdialog.h>
#include <qdialog.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlined.h>
#include <qpushbutton.h>
#include <kapp.h>
#include <kprocess.h>
/**Provides an installation module checking
  *installed programs needed by KDevelop. Uses CToolClass
  *for checking programs.
  *@author Ralf Nolden
  */

class CKDevInstall : public QDialog  {
   Q_OBJECT
public: 
	CKDevInstall(QWidget *parent=0, const char *name=0);
	~CKDevInstall();
public slots:


protected slots:

    void slotQTpressed();
    void slotKDEpressed();
    void slotHelp();
    void slotAuto();
    void slotCancel();
    void slotReceivedStdout(KProcess*,char*,int);
    void slotReceivedStderr(KProcess*,char*,int);
    void slotProcessExited(KProcess*);
 protected:
	  KConfig *config;
    QFrame* main_frame;
    QLabel* welcome_label;

    QLabel* qt_label;
    QLineEdit* qt_edit;
    QPushButton* qt_button;

    QLabel* kde_label;
    QLineEdit* kde_edit;
    QPushButton* kde_button;

    QLabel* hint_label;

    QPushButton* help_button;
    QPushButton* auto_button;
    QPushButton* cancel_button;

    KShellProcess* shell_process;
private:
    bool qt_test;
    bool kde_test;
    bool till_doc;
    bool successful;
    
    bool glimpse;
    bool glimpseindex;
    bool finished_glimpse;

};

#endif





