/***************************************************************************
                    ckappwizard.h - the kde-application-wizard
                             -------------------                                         

    version              :                                   
    begin                : 9 Sept 1998                                        
    copyright            : (C) 1998 by Stefan Heidrich                         
    email                : sheidric@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef CKAPPWIZARD_H
#define CKAPPWIZARD_H

#include <qdir.h>
#include <kapp.h>
#include <ktmainwindow.h>
#include <kwizard.h>
#include <kdatepik.h>
#include <kbutton.h>
#include <qfile.h>
#include <qmsgbox.h>
#include <qkeycode.h>
#include <kiconloader.h>
#include <qradiobt.h>
#include <qbttngrp.h>
#include <qlabel.h>
#include <qlined.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qmlined.h>
#include <kseparator.h>
#include <kfiledialog.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpicture.h>
#include <iostream.h>
#include <kiconloaderdialog.h>
#include <qstring.h>
#include <keditcl.h>
#include <qbutton.h>
#include <qstrlist.h>
#include <fstream.h>
#include <kprocess.h>
#include <qtooltip.h>
#include <qrect.h>
#include <qregexp.h>
#include "cproject.h"
#include <kquickhelp.h>

/** the KDE-Application-Wizard
  *@author Stefan Heidrich
  */

class CKAppWizard : public KWizard {
  Q_OBJECT
public:
  CKAppWizard(QWidget* parent=0,const char* name=0);
  ~CKAppWizard(){};
  void init();
  void initPages(); 
  QString getProjectFile();
  bool generatedProject();
  
public slots:

void slotDirDialogClicked();
  void slotAppEnd();
  void slotPerlOut(KProcess*,char*,int);
  void slotPerlErr(KProcess*,char*,int);
  void slotHeaderDialogClicked();
  void slotCppDialogClicked();
  void slotNewHeaderButtonClicked();
  void slotNewCppButtonClicked();
  void slotOkClicked();
  void slotDefaultClicked();
  void slotAppClicked();
  void slotMiniClicked();
  void slotCPPClicked();
  void slotProjectnameEntry();
  void slotIconButtonClicked();
  void slotMiniIconButtonClicked();
  void slotProgIconClicked();
  void slotMiniIconClicked();  
  void slotHeaderHeaderClicked();
  void slotCppHeaderClicked(); 
  void slotProcessExited();
  void slotMakeEnd();


private:

  bool  gen_prj;
  QObject* o;
  //  KWizard* view;
  QWidget *widget0,*widget1,*widget2,*widget3,*widget4,*widget5,*widget1a,*widget1b;
  KWizardPage *page0,*page1,*page2,*page3,*page4,*page5;  
  QButtonGroup* bgroup;
  QRadioButton *kna,*kma,*ta;
  QLabel *name,*directory,*authorname,*email,*versionnumber;
  QLineEdit *nameline,*directoryline,*authorline,*emailline,*versionline;
  QCheckBox *apidoc,*userdoc,*lsmfile,*gnufiles,*progicon,*miniicon,*datalink,*hheader,*cppheader;
  QPushButton *hload,*hnew,*directoryload,*cppload,*cppnew,*iconload,*miniload;
  KSeparator *separator0,*separator1,*separator2;
  KDirDialog* dirdialog;
  KFileDialog *headerdialog,*cppdialog;
  KIconLoaderDialog *iload,*mload;
  QPixmap pm,iconpm,minipm,*icontemp,*minitemp;
  QPainter painter;
  KApplication* help;
  QString dir,nametext,directorytext,name1,name2,namelow;
  KEdit *cppedit,*hedit;
  QButton *okButton,*cancelButton,*defaultButton, *prevButton;
  KIconLoader* loader;
  QMultiLineEdit *errOutput,*output;
  KProcess *p;
  CProject *project;
  QStrList sub_dir_list;
};

#endif
