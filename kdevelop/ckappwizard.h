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
#include <qframe.h>
#include <qheader.h>
#include <qmessagebox.h>
#include <qkeycode.h>
#include <kiconloader.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qmultilinedit.h>
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
  CKAppWizard(QWidget* parent=0,const char* name=0,QString author_name="",QString author_email="");
  ~CKAppWizard();
  void init();
  void initPages(); 
  QString getProjectFile();
  bool generatedProject();
  void okPermited(); 
  QString getAuthorEmail(){return m_author_email;}
  QString getAuthorName(){return m_author_name;}
  
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
  void slotDefaultClicked(int);
  //  void slotAppClicked();
  //  void slotMiniClicked();
  //  void slotQtClicked();
  //  void slotCPPClicked();
  void slotProjectnameEntry();
  void slotDirectoryEntry();
  void slotIconButtonClicked();
  void slotMiniIconButtonClicked();
  void slotProgIconClicked();
  void slotMiniIconClicked();  
  void slotHeaderHeaderClicked();
  void slotCppHeaderClicked(); 
  void slotProcessExited();
  void slotMakeEnd();
  void slotApplicationClicked();
  void slotLocationButtonClicked();
  void slotVSBoxChanged(int);
  void slotVendorEntry();

private:

  QString m_author_email;
  QString  m_author_name;
  bool  gen_prj, modifyDirectory,modifyVendor;
  QObject* o;
  QWidget *widget0,*widget1,*widget1c,*widget2,*widget3,*widget4,*widget1a,*widget1b;
  QListView* applications;
  QLabel* apphelp;
  QPushButton* directoryload;
  QLineEdit* emailline;
  QLineEdit* authorline;
  QLineEdit* versionline;
  QLineEdit* nameline;
  QLineEdit* directoryline;
  QLabel* name;
  QLabel* email;
  QLabel* authorname;
  QLabel* versionnumber;
  QLabel* directory;
  QCheckBox* datalink;
  QCheckBox* miniicon;
  QCheckBox* progicon;
  QCheckBox* gnufiles;
  QCheckBox* lsmfile;
  QCheckBox* userdoc;
  QCheckBox* apidoc;
  QPushButton* miniload;
  QPushButton* iconload;
  QCheckBox* generatesource;
  QPushButton* hload;
  QPushButton* hnew;
  QCheckBox* hheader;
  QPushButton* cppnew;
  QPushButton* cppload;
  QCheckBox* cppheader;
  QMultiLineEdit* output;
  QMultiLineEdit* errOutput;
  KWizardPage *page0,*page1,*page1a,*page2,*page3,*page4;
  KSeparator *separator0,*separator1,*separator2;
  KDirDialog* dirdialog;
  KFileDialog *headerdialog,*cppdialog;
  KIconLoaderDialog *iload,*mload;
  QPixmap pm,iconpm,minipm,*icontemp,*minitemp;
  QPainter painter;
  KApplication* help;
  QString dir,nametext,directorytext,name1,name2,namelow,nameold;
  KEdit *cppedit,*hedit;
  QButton *okButton,*cancelButton,*defaultButton, *prevButton;
  KIconLoader* loader;
  KShellProcess *q;
  //  KSimpleConfig* settings;
  CProject *project;
  QStrList sub_dir_list;
  QListViewItem *kdeentry;
  //QListViewItem *corbaitem;
  //QListViewItem *komitem;
  QListViewItem *kdenormalitem;
  QListViewItem *kdeminiitem;
  QListViewItem *qtentry;
  QListViewItem *ccppentry;
  //QListViewItem *gtkentry;
  QListViewItem *othersentry;
  QListViewItem *citem;
  QListViewItem *cppitem;
  QListViewItem *customprojitem;
  //QListViewItem *gtknormalitem;
  // QListViewItem *gtkminiitem;
  QListViewItem *qtnormalitem;
  //  QListViewItem *qtminiitem;
  QLineEdit* messageline;
  QLabel* logMessage;
  QLineEdit* vendorline;
  QLabel* vendorTag;
  QLineEdit* releaseline;
  QLabel* releaseTag;
  QLabel* vsInstall;
  QLabel* projectVSLocation;
  QLineEdit* projectlocationline;
  QLineEdit* vsLocation;
  QComboBox* vsBox;
  QLabel* vsSupport;
  QPushButton* locationbutton;
  QButtonGroup* qtarch_ButtonGroup_1;
};

#endif



